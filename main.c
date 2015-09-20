/*

************************************************************
**
** Created by: CodeBench 0.42 (14.10.2013)
**
** Project: Comica
**
** File: 
**
** Date: 07-01-2014 00:31:40
**
************************************************************

*/
//#define ALL_REACTION_CLASSES
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include <exec/exec.h>
#include <intuition/intuition.h>
#include <intuition/icclass.h>
#include <intuition/gui.h>
#include <dos/dos.h>
#include <libraries/asl.h>

#include <classes/window.h>
#include <classes/requester.h>

#include <gadgets/fuelgauge.h>
#include <gadgets/button.h> 
#include <gadgets/layout.h>
#include <gadgets/integer.h>
#include <gadgets/space.h>
#include <gadgets/listbrowser.h>
#include <gadgets/virtual.h>

#include <images/bitmap.h>
#include <images/label.h>

/*
 * Autoinit takes care of these
 */
#include <proto/exec.h>
#include <proto/intuition.h>

/*
 * Simply include the proto file for all ReAction classes you intent to use
 * to trigger their autoinitialization.
 */
#include <proto/fuelgauge.h>
#include <proto/window.h>
#include <proto/layout.h>
#include <proto/integer.h>
#include <proto/space.h>
#include <proto/requester.h>
#include <proto/button.h>
#include <proto/bitmap.h>
#include <proto/listbrowser.h>
#include <proto/virtual.h>
#include <proto/label.h>
#include <proto/dos.h>
#include <proto/utility.h>
#include <proto/graphics.h>
#include <proto/Picasso96API.h>
#include <proto/datatypes.h>
#include <proto/asl.h>
#include <proto/timer.h>

#include <graphics/composite.h>

//#include <reaction/reaction.h>
//#include <reaction/reaction_macros.h>

#include <datatypes/datatypes.h>
#include <datatypes/datatypesclass.h>
#include <datatypes/pictureclass.h>

#define VERSION "0"
#define REVISION "48"

#define PROGNAME "Comica"
#define DEVELOPER "George Sokianos"
#define DATE "01-May-2015"

#define DISPLAY_Dummy    (REACTION_Dummy + 0x39000)
#define DISPLAY_FileName (DISPLAY_Dummy + 1)

STATIC CONST_STRPTR version USED = "$VER: "PROGNAME" "VERSION"."REVISION" "DATE" ©,"DEVELOPER"\n\0";
STATIC CONST_STRPTR stack USED = "$STACK:80000";

CONST_STRPTR DepthIndicatorIn = ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>";
CONST_STRPTR DepthIndicatorEx = "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<";
uint16       DepthLevel     = 1;

uint16 list_count = 0;

STRPTR tagdatas[] = {};

Object *win; 
struct Screen *screen;
struct MsgPort *winAppPort;
struct Window *window;
//struct Window *AppWindow_p;
//struct List listbrowser_list;
struct List browserlist;
struct ColumnInfo *columninfo = NULL;
struct AslIFace *IAsl;


struct BitMap* SrcBitMap_p;
struct BitMap* ScaledBitMap_p;

struct TimeVal tv;
struct RandomState rs;

/* object identifiers (indexes) */
enum
{
 WIN_COMICA_MAIN,
 WIN_LAST
};

enum
{
 OID_LAYOUT_ROOT,
 OID_LAYOUT_BTNS,
 OID_LAYOUT_MAIN,
 OID_BTN_OPEN,
 OID_BITMAP_OPEN,
 OID_BTN_FIRSTPAGE,
 OID_BITMAP_FIRSTPAGE,
 OID_BTN_LASTPAGE,
 OID_BITMAP_LASTPAGE,
 OID_BTN_PREVPAGE,
 OID_BITMAP_PREVPAGE,
 OID_BTN_NEXTPAGE,
 OID_BITMAP_NEXTPAGE,
 OID_BTN_LINK,
 OID_BITMAP_LINK,
 OID_SPACE_1,
 OID_BTN_ZOOMIN,
 OID_BITMAP_ZOOMIN,
 OID_BTN_ZOOMOUT,
 OID_BITMAP_ZOOMOUT,
 OID_SPACE_2,
 OID_BTN_ONEPAGE,
 OID_BITMAP_ONEPAGE,
 OID_BTN_TWOPAGES,
 OID_BITMAP_TWOPAGES,
 OID_Listbrowser_Files,
 OID_LAYOUT_Pic,
 OID_Bitmap_Pic,
 //OID_LAYOUT_THUMBS_WRAPPER,
 //OID_VIRT_THUMBS,
 //OID_LAYOUT_THUMBS,
 OID_ADD_BUT,
 OID_LAST
};

/* object pointer field */
Object *wins[WIN_LAST];
Object *objects[OID_LAST];

Object * make_window(void)
{
    columninfo = IListBrowser->AllocLBColumnInfo(2,
        LBCIA_Column, 0,
            LBCIA_Title, " Thumbs",
            LBCIA_AutoSort, FALSE,
	        LBCIA_Sortable, FALSE,
            LBCIA_Weight, 100,
        LBCIA_Column, 1,
            LBCIA_Title, " Files",
            //LBCIA_Title, "",
            LBCIA_AutoSort, TRUE,
            //LBCIA_Weight, 80,
            LBCIA_Width,0,
        TAG_DONE);
   
     wins[WIN_COMICA_MAIN] = IIntuition->NewObject(NULL, "window.class",
        WA_ScreenTitle,        	PROGNAME,
        WA_Title,              	PROGNAME,
        WA_DragBar,            	TRUE,
        WA_CloseGadget,        	TRUE,
        WA_SizeGadget,         	TRUE,
        WA_DepthGadget,        	TRUE,
        WA_Activate,           	TRUE,
        WA_SmartRefresh,		TRUE,
        WA_InnerWidth,         	800,
        WA_InnerHeight,        	800,
        WA_Opaqueness,          255,    /* Initial opaqueness on opening (0..255) */
        WA_OverrideOpaqueness,  TRUE,   /* Override global settings? (TRUE|FALSE) */
        WA_FadeTime,            500000, /* Duration of transition in microseconds */
        WINDOW_UniqueID,		WIN_COMICA_MAIN,
        WINDOW_IconifyGadget,  	TRUE,
        WINDOW_IconTitle,      	PROGNAME,
        WINDOW_PopupGadget,		TRUE,
		WINDOW_JumpScreensMenu, TRUE,
        WINDOW_Position,       	WPOS_CENTERSCREEN,
    	WINDOW_AppPort,			winAppPort,
        WINDOW_Layout,			objects[OID_LAYOUT_ROOT] = IIntuition->NewObject(NULL, "layout.gadget",
			LAYOUT_Orientation, LAYOUT_ORIENT_VERT,
			LAYOUT_DeferLayout, TRUE,
            LAYOUT_SpaceOuter,  TRUE,
            
            LAYOUT_AddChild,   objects[OID_LAYOUT_BTNS] = IIntuition->NewObject(NULL, "layout.gadget",
				LAYOUT_Orientation, LAYOUT_ORIENT_HORIZ,
                LAYOUT_SpaceOuter,  TRUE,
                LAYOUT_BevelStyle,  BVS_GROUP,
                LAYOUT_Label,       " Buttons ",
                
                // First Page Button
                LAYOUT_AddChild, objects[OID_BTN_OPEN] = IIntuition->NewObject(NULL, "button.gadget",
                    GA_ID, OID_BTN_OPEN,
        			GA_RelVerify, TRUE,
                    GA_GadgetHelp, FALSE,
					GA_UserData, 0,
					GA_Text, "",
					BUTTON_Transparent, TRUE,
					BUTTON_AutoButton, 0,
					//BUTTON_BevelStyle, BVS_NONE,
					BUTTON_Justification, 1,
					BUTTON_RenderImage,  objects[OID_BITMAP_OPEN] = IIntuition->NewObject(NULL, "bitmap.image",  // = BitMapObject,
						BITMAP_SourceFile, "TBImages:open",
						BITMAP_DisabledSourceFile, "TBImages:open_g",
						BITMAP_Screen, screen,
						BITMAP_Precision, PRECISION_EXACT,
						BITMAP_Masking, TRUE,
					TAG_END), //BitMapEnd,
      			TAG_END),
				CHILD_MaxWidth, 32,
				CHILD_MinWidth, 32,
                
                // First Page Button
                LAYOUT_AddChild, objects[OID_BTN_FIRSTPAGE] = IIntuition->NewObject(NULL, "button.gadget",
                    GA_ID, OID_BTN_FIRSTPAGE,
        			GA_RelVerify, TRUE,
                    GA_GadgetHelp, FALSE,
					GA_UserData, 0,
					GA_Text, "",
					BUTTON_Transparent, TRUE,
					BUTTON_AutoButton, 0,
					BUTTON_BevelStyle, BVS_NONE,
					BUTTON_Justification, 1,
					BUTTON_RenderImage,  objects[OID_BITMAP_FIRSTPAGE] = IIntuition->NewObject(NULL, "bitmap.image",  // = BitMapObject,
						BITMAP_SourceFile, "TBImages:page_first",
						BITMAP_DisabledSourceFile, "TBImages:page_first_g",
						BITMAP_Screen, screen,
						BITMAP_Precision, PRECISION_EXACT,
						BITMAP_Masking, TRUE,
					TAG_END), //BitMapEnd,
      			TAG_END),
				CHILD_MaxWidth, 32,
				CHILD_MinWidth, 32,
				
                // Last Page Button
                LAYOUT_AddChild, objects[OID_BTN_LASTPAGE] = IIntuition->NewObject(NULL, "button.gadget",
                    GA_ID, OID_BTN_LASTPAGE,
        			GA_RelVerify, TRUE,
                    GA_GadgetHelp, FALSE,
					GA_UserData, 0,
					GA_Text, "",
					BUTTON_Transparent, TRUE,
					BUTTON_AutoButton, 0,
					BUTTON_BevelStyle, BVS_NONE,
					BUTTON_Justification, 1,
					BUTTON_RenderImage,  objects[OID_BITMAP_LASTPAGE] = IIntuition->NewObject(NULL, "bitmap.image",
						BITMAP_SourceFile, "TBImages:page_last",
						BITMAP_DisabledSourceFile, "TBImages:page_last_g",
						BITMAP_Screen, screen,
						BITMAP_Precision, PRECISION_EXACT,
						BITMAP_Masking, TRUE,
					TAG_END),
      			TAG_END),
				CHILD_MaxWidth, 32,
				CHILD_MinWidth, 32,

                // Previous Page Button
                LAYOUT_AddChild, objects[OID_BTN_PREVPAGE] = IIntuition->NewObject(NULL, "button.gadget",
                    GA_ID, OID_BTN_PREVPAGE,
        			GA_RelVerify, TRUE,
                    GA_GadgetHelp, FALSE,
					GA_UserData, 0,
					GA_Text, "",
					BUTTON_Transparent, TRUE,
					BUTTON_AutoButton, 0,
					BUTTON_BevelStyle, BVS_NONE,
					BUTTON_Justification, 1,
					BUTTON_RenderImage,  objects[OID_BITMAP_PREVPAGE] = IIntuition->NewObject(NULL, "bitmap.image",
						BITMAP_SourceFile, "TBImages:page_previous",
						BITMAP_DisabledSourceFile, "TBImages:page_previous_g",
						BITMAP_Screen, screen,
						BITMAP_Precision, PRECISION_EXACT,
						BITMAP_Masking, TRUE,
					TAG_END),
      			TAG_END),
				CHILD_MaxWidth, 32,
				CHILD_MinWidth, 32,

                // Next Page Button
                LAYOUT_AddChild, objects[OID_BTN_NEXTPAGE] = IIntuition->NewObject(NULL, "button.gadget",
                    GA_ID, OID_BTN_NEXTPAGE,
        			GA_RelVerify, TRUE,
                    GA_GadgetHelp, FALSE,
					GA_UserData, 0,
					GA_Text, "",
					BUTTON_Transparent, TRUE,
					BUTTON_AutoButton, 0,
					BUTTON_BevelStyle, BVS_NONE,
					BUTTON_Justification, 1,
					BUTTON_RenderImage,  objects[OID_BITMAP_NEXTPAGE] = IIntuition->NewObject(NULL, "bitmap.image",
						BITMAP_SourceFile, "TBImages:page_next",
						BITMAP_DisabledSourceFile, "TBImages:page_next_g",
						BITMAP_Screen, screen,
						BITMAP_Precision, PRECISION_EXACT,
						BITMAP_Masking, TRUE,
					TAG_END),
      			TAG_END),
				CHILD_MaxWidth, 32,
				CHILD_MinWidth, 32,

                // Link Page Button
                LAYOUT_AddChild, objects[OID_BTN_LINK] = IIntuition->NewObject(NULL, "button.gadget",
                    GA_ID, OID_BTN_LINK,
        			GA_RelVerify, TRUE,
                    GA_GadgetHelp, FALSE,
					GA_UserData, 0,
					GA_Text, "",
					BUTTON_Transparent, TRUE,
					BUTTON_AutoButton, 0,
					BUTTON_BevelStyle, BVS_NONE,
					BUTTON_Justification, 1,
					BUTTON_RenderImage,  objects[OID_BITMAP_LINK] = IIntuition->NewObject(NULL, "bitmap.image",
						BITMAP_SourceFile, "TBImages:link",
						BITMAP_DisabledSourceFile, "TBImages:link_g",
						BITMAP_Screen, screen,
						BITMAP_Precision, PRECISION_EXACT, 
						BITMAP_Masking, TRUE,
					TAG_END),
      			TAG_END),
				CHILD_MaxWidth, 32,
				CHILD_MinWidth, 32,
				
				// Space 1
				LAYOUT_AddChild, objects[OID_SPACE_1] = IIntuition->NewObject(NULL, "space.gadget", // SpaceObject,
					GA_ID, OID_SPACE_1,
					GA_RelVerify, TRUE,
					GA_GadgetHelp, FALSE,
					GA_UserData, 18,
					SPACE_MinWidth, 60,
					SPACE_BevelStyle, BVS_NONE,
				TAG_END),

                // ZoomIn Page Button
                LAYOUT_AddChild, objects[OID_BTN_ZOOMIN] = IIntuition->NewObject(NULL, "button.gadget",
                    GA_ID, OID_BTN_ZOOMIN,
        			GA_RelVerify, TRUE,
                    GA_GadgetHelp, FALSE,
					GA_UserData, 0,
					GA_Text, "",
					BUTTON_Transparent, TRUE,
					BUTTON_AutoButton, 0,
					BUTTON_BevelStyle, BVS_NONE,
					BUTTON_Justification, 1,
					BUTTON_RenderImage,  objects[OID_BITMAP_ZOOMIN] = IIntuition->NewObject(NULL, "bitmap.image",
						BITMAP_SourceFile, "TBImages:zoom_in",
						BITMAP_DisabledSourceFile, "TBImages:zoom_in_g",
						BITMAP_Screen, screen,
						BITMAP_Precision, PRECISION_EXACT,
						BITMAP_Masking, TRUE,
					TAG_END),
      			TAG_END),
				CHILD_MaxWidth, 32,
				CHILD_MinWidth, 32,

                // ZoomOut Page Button
                LAYOUT_AddChild, objects[OID_BTN_ZOOMOUT] = IIntuition->NewObject(NULL, "button.gadget",
                    GA_ID, OID_BTN_ZOOMOUT,
        			GA_RelVerify, TRUE,
                    GA_GadgetHelp, FALSE,
					GA_UserData, 0,
					GA_Text, "",
					BUTTON_Transparent, TRUE,
					BUTTON_AutoButton, 0,
					BUTTON_BevelStyle, BVS_NONE,
					BUTTON_Justification, 1,
					BUTTON_RenderImage,  objects[OID_BITMAP_ZOOMOUT] = IIntuition->NewObject(NULL, "bitmap.image",
						BITMAP_SourceFile, "TBImages:zoom_out",
						BITMAP_DisabledSourceFile, "TBImages:zoom_out_g",
						BITMAP_Screen, screen,
						BITMAP_Precision, PRECISION_EXACT,
						BITMAP_Masking, TRUE,
					TAG_END),
      			TAG_END),
				CHILD_MaxWidth, 32,
				CHILD_MinWidth, 32,
				
				// Space 2
				LAYOUT_AddChild, objects[OID_SPACE_2] = IIntuition->NewObject(NULL, "space.gadget", // SpaceObject,
					GA_ID, OID_SPACE_2,
					GA_RelVerify, TRUE,
					GA_GadgetHelp, FALSE,
					GA_UserData, 18,
					SPACE_MinWidth, 60,
					SPACE_BevelStyle, BVS_NONE,
				TAG_END),
				
				// ZoomOut Page Button
                LAYOUT_AddChild, objects[OID_BTN_ONEPAGE] = IIntuition->NewObject(NULL, "button.gadget",
                    GA_ID, OID_BTN_ONEPAGE,
        			GA_RelVerify, TRUE,
                    GA_GadgetHelp, FALSE,
					GA_UserData, 0,
					GA_Text, "",
					BUTTON_Transparent, TRUE,
					BUTTON_AutoButton, 0,
					BUTTON_BevelStyle, BVS_NONE,
					BUTTON_Justification, 1,
					BUTTON_RenderImage,  objects[OID_BITMAP_ONEPAGE] = IIntuition->NewObject(NULL, "bitmap.image",
						BITMAP_SourceFile, "TBImages:preview1page",
						BITMAP_DisabledSourceFile, "TBImages:preview1page_g",
						BITMAP_Screen, screen,
						BITMAP_Precision, PRECISION_EXACT,
						BITMAP_Masking, TRUE,
					TAG_END),
      			TAG_END),
				CHILD_MaxWidth, 32,
				CHILD_MinWidth, 32,
				
				// ZoomOut Page Button
                LAYOUT_AddChild, objects[OID_BTN_TWOPAGES] = IIntuition->NewObject(NULL, "button.gadget",
                    GA_ID, OID_BTN_TWOPAGES,
        			GA_RelVerify, TRUE,
                    GA_GadgetHelp, FALSE,
					GA_UserData, 0,
					GA_Text, "",
					BUTTON_Transparent, TRUE,
					BUTTON_AutoButton, 0,
					BUTTON_BevelStyle, BVS_NONE,
					BUTTON_Justification, 1,
					BUTTON_RenderImage,  objects[OID_BITMAP_TWOPAGES] = IIntuition->NewObject(NULL, "bitmap.image",
						BITMAP_SourceFile, "TBImages:preview2pages",
						BITMAP_DisabledSourceFile, "TBImages:preview2pages_g",
						BITMAP_Screen, screen,
						BITMAP_Precision, PRECISION_EXACT,
						BITMAP_Masking, TRUE,
					TAG_END),
      			TAG_END),
				CHILD_MaxWidth, 32,
				CHILD_MinWidth, 32,
				
            TAG_DONE),   // End of button Layout         
            CHILD_WeightedHeight, 0,
            
            LAYOUT_AddChild,	objects[OID_LAYOUT_MAIN] = IIntuition->NewObject(NULL, "layout.gadget",
				LAYOUT_Orientation, LAYOUT_ORIENT_HORIZ,
                LAYOUT_SpaceOuter,  TRUE,
                LAYOUT_BevelStyle,  BVS_GROUP,
                LAYOUT_Label,       " LISTER ",
                
                // Files lister
                LAYOUT_AddChild, objects[OID_Listbrowser_Files] = IIntuition->NewObject(NULL, "listbrowser.gadget",
					GA_ID, OID_Listbrowser_Files,
					GA_RelVerify, TRUE,
					//GA_GadgetHelp, TRUE,
					GA_UserData, 13,					 
					LISTBROWSER_AutoFit, 		TRUE,
					LISTBROWSER_ShowSelected, 	TRUE,
					LISTBROWSER_VerticalProp, 	TRUE,
					LISTBROWSER_HorizontalProp, TRUE,	 
					LISTBROWSER_ColumnTitles,  	TRUE,
					LISTBROWSER_Striping,      	LBS_ROWS,
                    LISTBROWSER_ColumnInfo,    	columninfo,
				TAG_END),
				CHILD_WeightedWidth, 30,
				
				// Thumbs list
				/*
LAYOUT_AddChild,	objects[OID_LAYOUT_THUMBS_WRAPPER] = IIntuition->NewObject(NULL, "layout.gadget",
	LAYOUT_Orientation, LAYOUT_ORIENT_VERT,
       LAYOUT_SpaceOuter,  TRUE,
       LAYOUT_BevelStyle,  BVS_GROUP,
       LAYOUT_Label,       " THUMBS ",
	
	LAYOUT_AddChild,	objects[OID_VIRT_THUMBS] = IIntuition->NewObject(NULL, "virtual.gadget",
		VIRTUALA_Contents,	objects[OID_LAYOUT_THUMBS] = IIntuition->NewObject(NULL, "layout.gadget",
			LAYOUT_Orientation, LAYOUT_ORIENT_VERT,
               LAYOUT_SpaceOuter,  FALSE,
              TAG_DONE),
              CHILD_WeightedWidth, 0,
          TAG_DONE),
          
TAG_DONE),
CHILD_WeightedWidth, 35,
				*/
		
                LAYOUT_WeightBar,	TRUE,

                	
				// Image viewer	
				LAYOUT_AddChild,	objects[OID_LAYOUT_Pic] = IIntuition->NewObject(NULL, "layout.gadget",
					LAYOUT_Orientation, LAYOUT_ORIENT_HORIZ,
					LAYOUT_VertAlignment, LALIGN_CENTER,
					LAYOUT_HorizAlignment, LALIGN_CENTER,
					LAYOUT_BevelStyle, BVS_THIN,
					LAYOUT_BevelState, IDS_SELECTED,
					LAYOUT_SpaceOuter, TRUE,
					LAYOUT_BackFill, LAYERS_BACKFILL,
					
					LAYOUT_AddImage, objects[OID_Bitmap_Pic] = IIntuition->NewObject(NULL, "bitmap.image",
						IA_Scalable, FALSE,
						BITMAP_SourceFile, "PROGDIR:logo.png",
						BITMAP_Screen, screen,
						//BITMAP_Precision, PRECISION_EXACT,
						BITMAP_Masking, TRUE,
						//BITMAP_Width, 50,
						//BITMAP_Height, 50,
					TAG_END),
					CHILD_WeightedWidth, 70,
					//CHILD_NoDispose,	TRUE,
					
            	TAG_DONE),
      			//CHILD_NoDispose,	TRUE,
				
            TAG_DONE),
        TAG_END),
    TAG_DONE);
    return wins[WIN_COMICA_MAIN];
}


void make_selection(void);
void free_listbrowser_list_with_images(struct List *);
struct BitMap* readimage_using_datatype(STRPTR fname, uint32 *Width, uint32 *Height, uint32 *Depth);
struct BitMap* scaled_image(uint32 SrcWidth, uint32 SrcHeight, uint32 MaxWidth, uint32 MaxHeight, ULONG *NewWidth, ULONG *NewHeight);
void showfullimage();
//struct List* directory_contents_to_list(CONST_STRPTR name);
int32 directory_contents_to_list(CONST_STRPTR name);
void fill_browserlist_with_files(struct List* currentlist);
//void file_request(char *folder, char *buffer); 
char *file_request(char *folder);
//char *gen_random(uint16 len);
void gen_random(uint16 len, STRPTR strcode);

int main()
{    
    
    struct  DrawInfo *Dri = NULL;
	
	BOOL    canfade     = FALSE;
    uint32  specialfx   = 0;
    uint32  fade_time   = 50;
    //uint32 waitmask;
    SrcBitMap_p = NULL;
    ScaledBitMap_p = NULL;
    
    char *start_folder;
    
    ITimer->GetSysTime(&tv);
	rs.rs_High = tv.Seconds;
	rs.rs_Low = tv.Microseconds;
    
    
    //struct FileRequester *filereq = NULL;
    
	winAppPort = (struct MsgPort *)IExec->AllocSysObject(ASOT_PORT, NULL);
	screen=IIntuition->LockPubScreen(NULL);
    if(screen)
    {
            win = make_window();
            window = (struct Window*)IIntuition->IDoMethod(win, WM_OPEN, NULL);
            
            if(window)
            {
                BOOL done = FALSE;

                uint32 sigmask = 0;
                IIntuition->GetAttr(WINDOW_SigMask, win, &sigmask);
                
				/* Check that compositing is enabled in GUI prefs */
				if (( Dri = IIntuition->GetScreenDrawInfo(window->WScreen) ))
            	{
                	IIntuition->GetGUIAttrs(NULL, Dri, GUIA_SpecialEffects, &specialfx, TAG_END);
                	canfade = (BOOL)specialfx;
					IIntuition->FreeScreenDrawInfo(window->WScreen, Dri);
            	}

                while (!done)
                {
                    uint32 siggot = IExec->Wait(sigmask | SIGBREAKF_CTRL_C);
                    if (siggot & SIGBREAKF_CTRL_C)
                    {
                        done = TRUE;
                    }

                    uint32 result = 0;
                    uint16 code   = 0;
                    //Object *fileobj;

                    while ((result = IIntuition->IDoMethod(win, WM_HANDLEINPUT, &code)))
                    {
                        switch(result & WMHI_CLASSMASK)
                        {
                            case WMHI_CLOSEWINDOW:
                                done = TRUE;
                                break;
                            case WMHI_GADGETUP:
                               	
                               	switch(result & WMHI_GADGETMASK)
                            	{
                                	case OID_BTN_OPEN:
                                    	IDOS->Printf("Open files\n");
                						char *filebuffer;
                						//IUtility->Strlcpy(start_folder,"PROGDIR:",sizeof("PROGDIR:"));
                						start_folder = IUtility->ASPrintf("PROGDIR:"); 
                						
                						filebuffer = file_request(start_folder);
                						IDOS->Printf( "2: SELECTED FILENAME %s\n", filebuffer );
                						
                						//char *newfoldername;
                						uint16 codelen = 16;
                						//char targetfolder[codelen+2];
                						STRPTR targetfolder;
                						//targetfolder = gen_random(codelen);
                						gen_random(codelen, targetfolder);
                						IDOS->Printf( "NEW CODE %s\n", targetfolder );
                						
                						
                						//gen_random(newfoldername, codelen);
                						//IDOS->Printf( "FOLDER NAME %s\n", newfoldername );
                						
                						/*
										filereq = IAsl->AllocAslRequest(ASL_FileRequest, NULL);
										if (filereq)
										{
										
											char pattern[20] = "#?.(cbr|cbz)";
											BOOL res;
											TEXT buffer[1024];
											
											res = IAsl->AslRequestTags(filereq,
													ASLFR_Window, window,
													ASLFR_SleepWindow,TRUE,
													ASLFR_TitleText,"Load comic file",
													ASLFR_InitialDrawer,"PROGDIR:",
													ASLFR_InitialPattern,&pattern,
													ASLFR_RejectIcons,TRUE,
													ASLFR_DoPatterns,TRUE,
												TAG_END);
											
											if (res)
											{
												IUtility->Strlcpy(buffer,filereq->fr_Drawer,sizeof(buffer));
												IDOS->AddPart(buffer,filereq->fr_File,sizeof(buffer));
												IDOS->Printf( "SELECTED FILENAME %s\n", buffer );
											} else {
												IDOS->Printf( "NO FILE SELECTED \n" );
											}
											
											IAsl->FreeAslRequest(filereq);
										}
										*/
										
                                    	/*
                                    	if(directory_contents_to_list("PROGDIR:tmpcomic/")) {
                                    		// fillbrowserlist
                                    		fill_browserlist_with_files(&browserlist);
                                    	}
                                    	*/
                                    	break;
                                    case OID_Listbrowser_Files:
                                    	IDOS->Printf("result \n");
                                    	make_selection();
                                    	break;
                            	}
                            	
                                break;
                            case WMHI_ICONIFY:
                                IIntuition->SetAttrs( win, WA_FadeTime, 0, TAG_END );
                                if (IIntuition->IDoMethod(win, WM_ICONIFY))
                                {
                                    window = NULL;
                                }
                                break;
                            case WMHI_UNICONIFY:
                                window = (struct Window*)IIntuition->IDoMethod(win, WM_OPEN);
                                IIntuition->SetAttrs( win, WA_FadeTime, fade_time * 10000, TAG_END );
                                break;
                            case WMHI_JUMPSCREEN:
								/*
								RA_CloseWindow(win);
			                    window = RA_OpenWindow(win);
			                    IIntuition->GetAttr(WINDOW_SigMask,win,&waitmask);
			                    if (window)
			                        IIntuition->ScreenToFront(window->WScreen);
			                    else
			                       done = TRUE;
			                    */
			                    break;
                            case WMHI_RAWKEY:
                               
                                break;
                        }
                    }
                }
            }
            
            //IIntuition->DisposeObject(objects[OID_LAYOUT_THUMBS]);
            //objects[OID_LAYOUT_THUMBS] = NULL;

			IIntuition->IDoMethod(win, WM_CLOSE);
			IIntuition->DisposeObject(win);
			
            IDOS->Printf( "LIST COUNT: %ld \n", list_count );
            
            if(list_count > 0) {
            	free_listbrowser_list_with_images(&browserlist);
            	IListBrowser->FreeListBrowserList(&browserlist);  // To investigate if it lets traces in memory
			}			
            
            if(ScaledBitMap_p != NULL) {
                IDOS->Printf( "ScaledBitmap Is not NULL \n" );
            	//IP96->p96FreeBitMap( ScaledBitMap_p );
            	IGraphics->FreeBitMap(ScaledBitMap_p);
			}
            
			IExec->FreeSysObject(ASOT_PORT, winAppPort);
			IIntuition->UnlockPubScreen(NULL, screen);
			screen = NULL;
	}
	return 0;
}

void make_selection(void)
{
	uint32 	result = 0,
			lastresult = 0;
	struct Node	*node;
    char *cmdline;
	STRPTR NodeString;
	
	uint32 SrcWidth = 0;
	uint32 SrcHeight = 0;
	uint32 SrcDepth = 0;
	SrcBitMap_p = NULL;
	//ScaledBitMap_p = NULL;
	
	ULONG layoutres;
	uint32 MaxWidth = 0;
	uint32 MaxHeight = 0;
	
	ULONG NewWidth = 0;
	ULONG NewHeight = 0;
	
	//UWORD imgwidth=0, imgheight=0;
	//int newwidth = 0, newheight = 0;


	result = IIntuition->GetAttr(LISTBROWSER_RelEvent, objects[OID_Listbrowser_Files], &lastresult);
	switch(result)
	{
		case LBRE_NORMAL:                               
			IIntuition->GetAttr(LISTBROWSER_SelectedNode, objects[OID_Listbrowser_Files],(ULONG *)&node);
			IListBrowser->GetListBrowserNodeAttrs(node,
					LBNA_Column, 1,
					//LBNA_UserData, &NodeString,
					LBNCA_Text, &NodeString,
				TAG_DONE);
			cmdline = IUtility->ASPrintf("PROGDIR:tmpcomic/%s", NodeString); 
			IDOS->Printf("Selected NODESTRING: %s \n", cmdline);
			
			// Load the image using datatype and show it
			if((SrcBitMap_p = readimage_using_datatype(cmdline, &SrcWidth, &SrcHeight, &SrcDepth)) != NULL) {
				
			    // Trying to calculate the width of the layout object
    			layoutres = IIntuition->GetAttrs( objects[OID_LAYOUT_Pic], 
			            GA_Width, 	&MaxWidth,
			            GA_Height, 	&MaxHeight,
					TAG_DONE );
			    if ( layoutres )
			    {
			    	IDOS->Printf( "layout max width! %ld \n", MaxWidth );
			    	IDOS->Printf( "layout max height! %ld \n", MaxHeight );
			    }
			    
           	 	if(ScaledBitMap_p != NULL) {
	                IDOS->Printf( "ScaledBitmap Is not NULL. Now lets clear it \n\n" );
	            	//IP96->p96FreeBitMap( ScaledBitMap_p );
	            	IGraphics->FreeBitMap(ScaledBitMap_p);
				} else {
					IDOS->Printf( "ScaledBitmap Is NULL. \n\n" );
				}
				
			    // Scale the image
			    if((ScaledBitMap_p = scaled_image(SrcWidth, SrcHeight, MaxWidth, MaxHeight, &NewWidth, &NewHeight)) != NULL) {
			        
			        showfullimage(NewWidth, NewHeight);
			        
			        // Free the used bitmaps
			        //IP96->p96FreeBitMap( SrcBitMap_p );
			        //IP96->p96FreeBitMap( ScaledBitMap_p );
			        IGraphics->FreeBitMap(SrcBitMap_p);
			        
			    }
			    
			    
			}
			//IDOS->Printf("Selection: %s \n", NodeString);
			IDOS->Printf("picture width: %ld \n", SrcWidth);
			IDOS->Printf("picture height: %ld \n", SrcHeight);
			break;
		case LBRE_DOUBLECLICK:
			break;
	}
}





/*
 * Code by Thomas (http://eab.abime.net/showpost.php?p=957859&postcount=3)
 */
void free_listbrowser_list_with_images (struct List *list)
{
	IDOS->Printf("CLEAR NOW THE BROWSERLIST \n");
	struct Node *node;
	int cnt = 0;
	while ((node = IExec->RemTail (list))) {
   	 	Object *img = NULL;
	    IListBrowser->GetListBrowserNodeAttrs (node,LBNCA_Image,&img,TAG_END);
	    if (img)
	        IIntuition->DisposeObject (img);
	    IListBrowser->FreeListBrowserNode (node);
		IDOS->Printf("DELETE ROW: %ld \n", cnt);
		cnt++;
    }
    //return cnt;
    //return NULL;
}


/*
 * V2 functions
 */
struct BitMap* readimage_using_datatype(STRPTR fname, uint32 *Width, uint32 *Height, uint32 *Depth)
{
	//int32 Lock;
	//UBYTE imgdata;
	APTR imglock;
	Object *dt_Obj = NULL;
	ULONG res;
	APTR *base;
	ULONG bytesperrow = 0;
	struct BitMapHeader *bmhd;
	struct BitMap* bitmapsrc = NULL;
	//struct RenderInfo RenderInfoBuffer;
	struct pdtBlitPixelArray pdtBlitPixelArrayBuffer;
	
    dt_Obj = IDataTypes->NewDTObject((APTR) fname,
         	DTA_SourceType,        DTST_FILE,
        	DTA_GroupID,           GID_PICTURE,  
        	PDTA_DestMode,         PMODE_V43,    
        	PDTA_Remap,            TRUE,         
        	PDTA_Screen,           screen,       
        	PDTA_FreeSourceBitMap, TRUE,         
			//PDTA_ScaleQuality,     scalequality, 
		TAG_DONE);
	if(dt_Obj) {
	       
	    res = IDataTypes->GetDTAttrs(dt_Obj,
	    		PDTA_BitMapHeader, &bmhd,
	    	TAG_DONE);
	    
		//bitmapsrc = IP96->p96AllocBitMap( bmhd->bmh_Width, bmhd->bmh_Height, 32, 0, NULL, RGBFB_A8R8G8B8 );
		//IGraphics->AllocBitMap( bmhd->bmh_Width, bmhd->bmh_Height, 32, BMF_CLEAR, bitmapsrc );
		//Lock = IP96->p96LockBitMap( bitmapsrc, (uint8*) &RenderInfoBuffer, sizeof RenderInfoBuffer );
		//imgdata = IExec->DEPRECATED APTR IExec->AllocVec( ULONG byteSize, ULONG attributes )
		bitmapsrc = IGraphics->AllocBitMapTags(bmhd->bmh_Width, bmhd->bmh_Height, 32, BMATags_PixelFormat,RGBFB_A8R8G8B8, TAG_END);
		
		imglock = IGraphics->LockBitMapTags(bitmapsrc,
				LBM_BaseAddress, &base,
				LBM_BytesPerRow, &bytesperrow,
			TAG_DONE);
	
	  	pdtBlitPixelArrayBuffer.MethodID           = PDTM_READPIXELARRAY;
		pdtBlitPixelArrayBuffer.pbpa_PixelData     = &base;
		pdtBlitPixelArrayBuffer.pbpa_PixelFormat   = PBPAFMT_ARGB;
		pdtBlitPixelArrayBuffer.pbpa_PixelArrayMod = bytesperrow, //bmhd->bmh_Width*4;
		pdtBlitPixelArrayBuffer.pbpa_Left          = 0;
		pdtBlitPixelArrayBuffer.pbpa_Top           = 0;
		pdtBlitPixelArrayBuffer.pbpa_Width         = bmhd->bmh_Width;
		pdtBlitPixelArrayBuffer.pbpa_Height        = bmhd->bmh_Height;
			
		*Width  = bmhd->bmh_Width;
		*Height = bmhd->bmh_Height;
		*Depth  = bmhd->bmh_Depth;
		printf("Bytes Per Row: %ld \n", bytesperrow);
		IIntuition->IDoMethod(dt_Obj, 
			PDTM_READPIXELARRAY, base, 
			PBPAFMT_ARGB, bytesperrow,
			0, 0, bmhd->bmh_Width, bmhd->bmh_Height);
		
		//IIntuition->IDoMethodA( dt_Obj, ( Msg ) &pdtBlitPixelArrayBuffer );
		
		//IP96->p96UnlockBitMap( bitmapsrc, Lock );
		IGraphics->UnlockBitMap(imglock);
		
		IDataTypes->DisposeDTObject( dt_Obj );
	    
	    return bitmapsrc;
	}
	return NULL;
}

struct BitMap* scaled_image(uint32 SrcWidth, uint32 SrcHeight, uint32 MaxWidth, uint32 MaxHeight, ULONG *NewWidth, ULONG *NewHeight) 
{
    float scale_factor = 0;
    struct BitMap* scaledbitmapsrc = NULL;
			
	if(SrcWidth > SrcHeight) {   // landscape 
		scale_factor = (float)MaxWidth / (float)SrcWidth;
        *NewWidth = MaxWidth-6;
        *NewHeight = round(SrcHeight * scale_factor);
    } else {        			// portrait
        scale_factor = (float)MaxHeight / (float)SrcHeight;
		*NewWidth = round(SrcWidth * scale_factor);
        *NewHeight = MaxHeight-6;
    }	
	
	IDOS->Printf("scale factor: %ld \n", scale_factor);

	IDOS->Printf("picture newwidth: %ld \n", (long)NewWidth);
	IDOS->Printf("picture newheight: %ld \n", (long)NewHeight);
	
	IDOS->Printf("MAXwidth: %ld \n", MaxWidth);
	IDOS->Printf("MAXheight: %ld \n", MaxHeight);
    
    // Scale
    //scaledbitmapsrc = IP96->p96AllocBitMap( *NewWidth, *NewHeight, 32, 0, NULL, RGBFB_A8R8G8B8 );
	//IGraphics->AllocBitMap( *NewWidth, *NewHeight, 32, 0, scaledbitmapsrc );
	scaledbitmapsrc = IGraphics->AllocBitMapTags(*NewWidth, *NewHeight, 32, BMATags_PixelFormat,RGBFB_A8R8G8B8, TAG_END);
	if ( IGraphics->CompositeTags( COMPOSITE_Src, SrcBitMap_p, scaledbitmapsrc,
	                                 COMPTAG_ScaleX, COMP_FLOAT_TO_FIX( scale_factor ),
	                                 COMPTAG_ScaleY, COMP_FLOAT_TO_FIX( scale_factor ),
	                                 COMPTAG_Flags, COMPFLAG_SrcFilter,
	                                 TAG_DONE ) != COMPERR_Success )
	{
	    /* error */
	    IDOS->Printf( ( CONST_STRPTR ) "%s\n", "CompositeTags() failed" );
	    IDOS->Printf( ( CONST_STRPTR ) "%s\n", "ScaleImage() failed" );
	    //IP96->p96FreeBitMap( scaledbitmapsrc );
	    IGraphics->FreeBitMap(scaledbitmapsrc);
		return NULL;	
	} /* if */	

    return scaledbitmapsrc;
    
}

void showfullimage(uint32 NewWidth, uint32 NewHeight) 
{    
	struct lmRemoveChild RemoveMsgBuffer;
  	struct lmAddImage AddMsgBuffer;
	Object *Temp_p;
	
	RemoveMsgBuffer.MethodID  = LM_REMOVECHILD;
	RemoveMsgBuffer.lm_Window = window;
	RemoveMsgBuffer.lm_Object = objects[OID_Bitmap_Pic];
       
    /* Remove object from active layout */
	if ( IIntuition->IDoMethodA( objects[OID_LAYOUT_Pic], (void*)&RemoveMsgBuffer ) ) {
		objects[OID_Bitmap_Pic] = NULL;
	}
	
	
	Temp_p = IIntuition->NewObject( NULL, "bitmap.image",
		IA_Width, NewWidth,
		IA_Height, NewHeight,
		IA_Scalable, FALSE,
              BITMAP_Screen, screen,
              BITMAP_Masking, TRUE,
              BITMAP_Transparent, TRUE,
               BITMAP_BitMap, ScaledBitMap_p,
              BITMAP_Width, NewWidth,
              BITMAP_Height, NewHeight,
              //BITMAP_SourceFile, filename,
              BITMAP_Precision, PRECISION_IMAGE,
           TAG_DONE );
	
	
	AddMsgBuffer.MethodID       = LM_ADDIMAGE;
 		AddMsgBuffer.lm_Window      = window;
 		AddMsgBuffer.lm_Object      = Temp_p;
 		AddMsgBuffer.lm_ObjectAttrs = NULL;
	
	if ( IIntuition->IDoMethodA( objects[OID_LAYOUT_Pic], (void*)&AddMsgBuffer ) )
 		{
		objects[OID_Bitmap_Pic] = Temp_p;
		IIntuition->IDoMethod( win, WM_RETHINK );
		IIntuition->RefreshWindowFrame( window );		
	}
}

//struct List* directory_contents_to_list(CONST_STRPTR name) 
int32 directory_contents_to_list(CONST_STRPTR name) 
{
    APTR context = IDOS->ObtainDirContextTags( EX_StringNameInput,name,
                EX_DoCurrentDir,TRUE, /* for relative cd lock */
                EX_DataFields,(EXF_NAME|EXF_LINK|EXF_TYPE),
                TAG_END);
    struct Node *file_node;
    int cnt = 0;                   
    char *thumbfile;
    int ret = FALSE;
    
    if( context )
    {
        struct ExamineData *dat;
    	IExec->NewList(&browserlist);

		
        
        while((dat = IDOS->ExamineDir(context)))
        {
            if( EXD_IS_LINK(dat) ) /* all link types - check first ! */
            {
                if( EXD_IS_SOFTLINK(dat) ) 
                {
                }
                else   /* a hardlink or alt link */
                {
                }
            }
            else if( EXD_IS_FILE(dat) )
            {
                
                thumbfile = IUtility->ASPrintf("PROGDIR:tmpcomic/%s", dat->Name);
                //IDOS->Printf("Thumb FIle: %s \n", thumbfile);
 				printf("Thumb FIle: %s \n", thumbfile);
				
				
				file_node = IListBrowser->AllocListBrowserNode(2,
						LBNA_Column, 0,
						LBNCA_Image, IIntuition->NewObject(NULL, "bitmap.image",
							IA_Width, 60,
							IA_Height, 100 ,
							IA_Scalable, TRUE,
							BITMAP_SourceFile, thumbfile,
							BITMAP_Screen, screen,
							BITMAP_Precision, PRECISION_EXACT,
							BITMAP_Masking, TRUE,
						TAG_END), 
						LBNA_Column, 1,
						LBNCA_CopyText, TRUE,
						LBNCA_Text, dat->Name,                			
					TAG_DONE);

         		if(file_node)
	           	{
           			IExec->AddTail(&browserlist, file_node);
	           	} 
	           	
	           	//else {
	            //   return NULL;
	            //}	
           		
           		
           		
           		
                cnt++;
            }
            else if( EXD_IS_DIRECTORY(dat) )
            {
            }
        }


        if( ERROR_NO_MORE_ENTRIES == IDOS->IoErr() )
        {
            //success = TRUE;           /* normal exit */
            //IDOS->ReleaseDirContext(context);
            //return browserlist_p;
        	ret = TRUE;
        }
        else
        {
            IDOS->PrintFault(IDOS->IoErr(),NULL); /* failure - why ? */
        }
        
    }
    else
    {
        IDOS->PrintFault(IDOS->IoErr(),NULL);  /* no context - why ? */
    }
     
    IDOS->ReleaseDirContext(context);
    //if(ret) {
    //	return browserlist_p;
   	//} else {
   		IDOS->Printf( "LIST ITEMS COUNT: %ld \n", cnt );
   		list_count = cnt;
   		return ret;
   	//}
}

void fill_browserlist_with_files(struct List* currentlist) 
{
	IIntuition->SetGadgetAttrs((struct Gadget*)objects[OID_Listbrowser_Files], window, NULL,
    			LISTBROWSER_Labels, 		currentlist,
    			LISTBROWSER_SortColumn,		1,
    			LISTBROWSER_Selected,		-1,			//avoid having wrong selections from previous list
                LISTBROWSER_ColumnInfo,    	columninfo,
    		TAG_DONE);
	
}

char *file_request(char *folder)
{	
	char *buffer;

    struct FileRequester *filereq = NULL;
    
	filereq = IAsl->AllocAslRequest(ASL_FileRequest, NULL);
	if (filereq)
	{
	
		char pattern[20] = "#?.(cbr|cbz)";
		BOOL res;
		//TEXT buffer[1024];
		
		res = IAsl->AslRequestTags(filereq,
				ASLFR_Window, window,
				ASLFR_SleepWindow,TRUE,
				ASLFR_TitleText,"Load comic file",
				ASLFR_InitialDrawer, folder, 
				ASLFR_InitialPattern, &pattern,
				ASLFR_RejectIcons,TRUE,
				ASLFR_DoPatterns,TRUE,
			TAG_END);
		
		if (res)
		{
			//IUtility->Strlcpy(buffer,filereq->fr_Drawer,sizeof(*buffer));
			//IDOS->AddPart(buffer,filereq->fr_File,sizeof(*buffer));
			
			buffer = IUtility->ASPrintf("%s/%s", filereq->fr_Drawer, filereq->fr_File); 
			IDOS->Printf( "SELECTED FILENAME %s\n", buffer );
			
			return buffer;
		} else {
			IDOS->Printf( "NO FILE SELECTED \n" );
		}
		
		IAsl->FreeAslRequest(filereq);
	}

	return NULL;
}

void gen_random(uint16 len, STRPTR strcode) 
{
	//IDOS->Printf( "CODE LENGTH %ld\n", len );
	uint16 i, rnum;
	//char strcode[len+2];
	char alphanum[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_-*$@#!";	
	
	for(i = 0; i < len; ++i) {		
		rnum = (IUtility->Random(&rs) % (sizeof(alphanum) - 1));
		strcode = IUtility->ASPrintf("%s", alphanum[rnum]);
//		strcode[i] = alphanum[rnum];
        //IDOS->Printf( "%ld - %ld - %c \n", i, rnum, alphanum[rnum]);
    }
    //strcode[len] = '\0';
//    IDOS->Printf( "NEW CODE 1: %s\n", strcode );
    //return strcode;
    //return NULL;
}

