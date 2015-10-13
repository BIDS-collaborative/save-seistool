#ifndef lint
static char id[] = "$Id: panel.c,v 1.2 2013/02/28 21:24:58 lombard Exp $";
#endif

/*
 * panel.c--
 *    set up of the main window panel
 *
 * Copyright (c) 1992-3 Andrew K. Yu, Lawrence Berkeley Laboratory
 *      and University of California, Berkeley.
 * All rights reserved.
 */
#include <xview/xview.h>
#include <xview/canvas.h>
#include <xview/panel.h>
#include <xview/window.h>
#include <xview/rect.h>

#include "proto.h"
#include "xv_proto.h"
#include "sort.h"

/* the follow keeps track of the buttons, inactivated when necessary */
Panel_item  PrevEvtButton;
Panel_item  NextEvtButton;
Panel_item  ZoomButton;
Panel_item  RefreshSButton;
int top_panel_width=40;
static Menu sort_menu, rot_menu;
extern Menu rot_pan_menu;

/* prototypes */
static Menu MakeFormatSubmenu(void (*func)());
static void files_submenu_proc(Menu menu, Menu_item menu_item);
static void save_submenu_proc(Menu menu, Menu_item menu_item);
static void edit_menu_proc(Menu menu, Menu_item menu_item);
static void files_menu_proc(Menu menu, Menu_item menu_item);
static void special_menu_proc(Menu menu, Menu_item menu_item);
static void analyse_menu_proc(Menu menu, Menu_item menu_item);
static void sele_menu_proc(Menu menu, Menu_item menu_item);
static void coin_menu_proc(Menu menu, Menu_item menu_item);
static void sort_menu_proc(Menu menu, Menu_item menu_item);

extern int sort_type;

#define FUNC_KEY	33

static Menu MakeFormatSubmenu(void (*func)())
{
    Menu submenu;
    submenu= (Menu)xv_create(NULL,MENU,
			     MENU_TITLE_ITEM, "Formats",
			     MENU_STRINGS, "Default",
			     "TRC format", "BIS format", "MSEED format",
			     "SEGY format","SAC format", NULL,
			     MENU_NOTIFY_PROC, files_submenu_proc,
			     XV_KEY_DATA, FUNC_KEY, func,
			     NULL);
    return submenu;
}

void initPanel(Frame frame)
{
    Panel   panel;
    Menu    file_menu, edit_menu, an_menu, sp_menu;
    Menu    sele_menu, submenu1, submenu2, submenu3, submenu4;
    Menu    in_menu;
    Rect    rect1 ,rect2;
    int     frame_l;
    extern  Canvas  tracesScvs;

    panel = (Panel)xv_create(frame,PANEL,
			     XV_X, 0, XV_Y, 0,		     
			     XV_WIDTH,  FrmWidth,
			     XV_HEIGHT, 40,
			     NULL);

    submenu1= MakeFormatSubmenu(handle_newload);
    submenu2= MakeFormatSubmenu(handle_load);
    submenu3= MakeFormatSubmenu(handle_load_event);

    submenu4= (Menu)xv_create(NULL,MENU,
			      MENU_STRINGS,
			      "BIS format", "SAC format", "SEGY format",
			      NULL,	      
			      MENU_NOTIFY_PROC, save_submenu_proc,
			      NULL);

    sort_menu= (Menu)xv_create(NULL,MENU,
			       MENU_GEN_PIN_WINDOW, tracesFrame, "",
			       MENU_TITLE_ITEM, "Sort Menu",
			       MENU_STRINGS,
			       "Stat name", "Channel", "Distance", 
			       "Azimuth", "Stat lat",
			       "Stat lon", "First Pick",NULL,
			       MENU_NOTIFY_PROC, sort_menu_proc,
			       NULL);

    sele_menu= (Menu)xv_create(NULL,MENU,
			       MENU_GEN_PIN_WINDOW, tracesFrame, "",
			       MENU_STRINGS,
			       "enter", "script", "clear", " ",
			       "select", "sele rest",
			       "keep", "keep rest", " ",
			       "rearrange",
			       "filter", "no filter", NULL,
			       MENU_NOTIFY_PROC, sele_menu_proc,
			       NULL);

    file_menu= (Menu)xv_create(NULL,MENU,
			       MENU_TITLE_ITEM, "Files",		  
			       MENU_PULLRIGHT_ITEM, "New file", submenu1,
			       MENU_PULLRIGHT_ITEM, "Load file", submenu2,
			       MENU_PULLRIGHT_ITEM, "Load event", submenu3,
			       MENU_STRINGS,	
			       /*  "Preview",   Waveform database doesn't work */
			       "Event manager", " ",
			       "Load picks", "Save picks", " ",	
			       "Event Location"," ",
			       NULL,	       
			       MENU_PULLRIGHT_ITEM, "Save file", submenu4,
			       MENU_STRINGS,	
			       "Write elf",
			       " ",
			       "Close all", "Reload", "Set exec",
			       NULL,
			       MENU_NOTIFY_PROC,   files_menu_proc,
			       NULL);

    edit_menu= (Menu)xv_create(NULL,MENU,
			       MENU_TITLE_ITEM, "Edit",
			       MENU_PULLRIGHT_ITEM, "Select trace", sele_menu,
			       MENU_STRINGS,
			       "Select all", "Deselect all",
			       "Discard selected", "Top selected", " ",
			       "Perm clip", " ",NULL,
			       MENU_PULLRIGHT_ITEM,"Sort",sort_menu,
			       MENU_STRINGS,
			       " ", "Group comp", "Regroup ZNE", "Restore order", 
			       NULL,
			       MENU_NOTIFY_PROC,   edit_menu_proc,
			       NULL);

    in_menu= (Menu)xv_create(NULL,MENU,
			     MENU_STRINGS,
			     "Deconvolve to Ground","Deconvolve to Velocity",
			     "Deconvolve to Acceleration", "Benioff 100kg",
			     "Butterworth 20s BP", "Wood Anderson", NULL,
			     MENU_NOTIFY_PROC, coin_menu_proc,
			     NULL);
    rot_menu=(Menu)xv_create(NULL,MENU,
			     MENU_STRINGS,
			     "rotate rms",
			     "rotate event",
			     NULL,
			     MENU_NOTIFY_PROC,autor_events ,
			     NULL);

    an_menu= (Menu)xv_create(NULL,MENU,
			     MENU_TITLE_ITEM, "Analyse",		  
			     MENU_DEFAULT, 1, /* hack for LEFT_MS not selecting 1st item */
			     MENU_NOTIFY_PROC,   analyse_menu_proc,
			     MENU_STRINGS,
			     "read resp", "read def resp", " ",
			     "spectrum", "Filters ...", NULL,
			     MENU_PULLRIGHT_ITEM, "conv resp", in_menu,
			     MENU_PULLRIGHT_ITEM, "Auto Rotate", rot_menu,
			     MENU_STRINGS,
			     "calc ampli", " ",
			     "freq-time", "x-spectrum", "Part motion", 
			     NULL,
			     NULL);

    sp_menu= (Menu)xv_create(NULL,MENU,
			     MENU_TITLE_ITEM, "Special",
			     MENU_DEFAULT, 1, /* hack for LEFT_MS not selecting 1st item */
			     MENU_STRINGS,
			     "Load windows", "Start triple",	"End triple",
			     "Reinit TRC",
			     "Retain config",
			     NULL,
			     MENU_NOTIFY_PROC, special_menu_proc,
			     NULL);

    (void)xv_create(panel, PANEL_BUTTON,
		    PANEL_LABEL_STRING,	"Quit",
		    PANEL_NOTIFY_PROC,	quit,
		    NULL);
    (void)xv_create(panel, PANEL_BUTTON,
		    PANEL_LABEL_STRING,	"Files",
		    PANEL_ITEM_MENU,    file_menu,
		    NULL);
    (void)xv_create(panel, PANEL_BUTTON,
		    PANEL_LABEL_STRING,	"Edit",
		    PANEL_ITEM_MENU,    edit_menu,
		    NULL);
    (void)xv_create(panel, PANEL_BUTTON,
		    PANEL_LABEL_STRING,	"Print",
		    PANEL_NOTIFY_PROC,    show_print_panel,
		    NULL);
    PrevEvtButton= (Panel_item)xv_create(panel, PANEL_BUTTON,
					 PANEL_LABEL_STRING,	"Prv Ev",
					 PANEL_NOTIFY_PROC,	goto_prev_event,
					 NULL);
    NextEvtButton= (Panel_item)xv_create(panel, PANEL_BUTTON,
					 PANEL_LABEL_STRING,	"Nxt Ev",
					 PANEL_NOTIFY_PROC,	goto_next_event,
					 NULL);
    (void)xv_create(panel, PANEL_BUTTON,
		    PANEL_LABEL_STRING,	"Write",
		    PANEL_NOTIFY_PROC,	WriteOutPickFile,
		    NULL);
    (void)xv_create(panel, PANEL_BUTTON,
		    PANEL_LABEL_STRING,	"Analyse",
		    PANEL_ITEM_MENU,    an_menu,
		    NULL);
    (void)xv_create(panel, PANEL_BUTTON,
		    PANEL_LABEL_STRING,	"Special",
		    PANEL_ITEM_MENU,    sp_menu,
		    NULL);
    ZoomButton= (Panel_item)xv_create(panel, PANEL_BUTTON,
				      PANEL_LABEL_STRING,	"Zoom",
				      PANEL_NOTIFY_PROC,	open_zoom_window,
				      NULL);
    RefreshSButton=(Panel_item)xv_create(panel, PANEL_BUTTON,
					 PANEL_LABEL_STRING,"Refresh",
					 PANEL_NOTIFY_PROC, RedrawScreen,
					 NULL);
    (void)xv_create(panel, PANEL_BUTTON,
		    PANEL_LABEL_STRING,	"Control",
		    PANEL_NOTIFY_PROC,    show_control_panel,
		    NULL);
    /* see if the panel over laps the display area */
    window_fit(panel);
    rect1=*(Rect *)xv_get(panel,XV_RECT);
    rect2=*(Rect *)xv_get(frame,XV_RECT);
    top_panel_width=rect_bottom(&rect1);

    xv_set(panel,XV_HEIGHT,top_panel_width,XV_WIDTH,FrmWidth,NULL);
}

static void files_submenu_proc(Menu menu, Menu_item menu_item)
{
    char *s= (char *)xv_get(menu_item, MENU_STRING);
    void (*func)()= (void(*)())xv_get(menu,
				      XV_KEY_DATA, FUNC_KEY);
    extern FileFormat defaultFormat;
    
    if (!strcmp(s, "Default")) {
	func(defaultFormat);
    }else if(!strcmp(s, "BIS format")) {
	func(BIS_Format);
    }else if(!strcmp(s, "TRC format")) {
	func(TRC_Format);
    }else if(!strcmp(s, "MSEED format")) {
	func(SDR_Format);
    }else if(!strcmp(s, "SEGY format")) {
	func(SEGY_Format);
    }else if(!strcmp(s, "SAC format")) {
	func(SAC_Format);
    }
}

static void save_submenu_proc(Menu menu, Menu_item menu_item)
{
    char *s= (char *)xv_get(menu_item, MENU_STRING);
    
    if(!strcmp(s, "BIS format")) {
	DumpBIS();
    }else if(!strcmp(s, "SAC format")) {
	DumpSAC();
    }else if(!strcmp(s, "SEGY format")) {
	DumpSEGY();
    }
}

static void edit_menu_proc(Menu menu, Menu_item menu_item)
{
    char *s= (char *)xv_get(menu_item, MENU_STRING);

    if(!strcmp(s, "Select all")) {
	Select_all();
    }else if(!strcmp(s, "Deselect all")) {
	Deselect_all();
    }else if(!strcmp(s, "Discard selected")) {
	Discard_Selected();
    }else if (!strcmp(s, "Top selected")) {
	Rearrange_Selected_top();
    }else if(!strcmp(s, "Perm clip")) {
	PermanentClip();
    }else if (!strcmp(s, "Group comp")) {
	if (Mode_rotate)
	    close_rotate_panel();
	if (Mode_trvlTime)
	    ExitTTMode();
	if (Mode_triplet)
	    EndTripletMode();
	Group_ZNE();
    }else if (!strcmp(s, "Regroup ZNE")) {
	regroup_3comp();
    }else if (!strcmp(s, "Restore order")) {
	restore_orig_order();
    }
}
    

static void files_menu_proc(Menu menu, Menu_item menu_item)
{
    char *s= (char *)xv_get(menu_item, MENU_STRING);

    if(!strcmp(s, "Preview")) {
	handle_preview_event();
    }else if(!strcmp(s, "Event manager")) {
	handle_eqevtmgr_event();
    }else if(!strcmp(s, "Write elf")) {
	Write_event_list_file();
    }else if(!strcmp(s, "Close all")) {
	CloseAllTracks();
	CleanCurEvent();
    }else if(!strcmp(s, "Reload")) {
	goto_cur_event();
    }else if(!strcmp(s, "Set exec")) {
	set_execscript();
    }else if (!strcmp(s,"Event Location")) {
	get_location_info();
    }else if(!strcmp(s, "Load picks")) {
	load_pickfile();
    }else if (!strcmp(s, "Save picks")) {
	dump_all_picks();
    }else {
	/* ignored */
    }
}

static void special_menu_proc(Menu menu, Menu_item menu_item)
{
    extern int LastTrack;

    char *s= (char *)xv_get(menu_item, MENU_STRING);
    if (!strcmp(s, "Load windows")) {
	char buf[1000];
	GetString("Windows File: ", buf, 1000);
	if (*buf!='\0') LoadWindows(buf);
    }else if (!strcmp(s, "Start triple")) {
	if (Mode_triplet==0) {
	    StartTripletMode();
	}
    }else if (!strcmp(s, "End triple")) {
	EndTripletMode();
    }else if(!strcmp(s, "Reinit TRC")) {
	Demand_TRC_setup();
    }else if (!strcmp(s, "Retain config")) {
	save_config();
    }
}

static void analyse_menu_proc(Menu menu, Menu_item menu_item)
{
    extern int LastTrack;

    char *s= (char *)xv_get(menu_item, MENU_STRING);
    if (!strcmp(s, "read def resp")) {
	extern char *default_instr_file;
	extern char *alt_instr_file;
	GetResponses(NULL);
    }else if (!strcmp(s, "read resp")) {
	char buf[200];
	GetString("Enter instr file: ",buf,200);
	if(*buf!='\0') {
	    printf("Use instrument file %s\n",buf);
	    GetResponses(buf);
	}
    }else if (!strcmp(s, "spectrum")) {
	handle_spectrum();
    }else if (!strncmp(s, "Filters", 7)) {
	open_freq_panel();
    }else if (!strncmp(s, "conv resp", 9)) {
	/* ignore */
    }else if (!strcmp(s, "calc ampli")) {
	CalcAmpli();
    }else if (!strcmp(s, "freq-time")) {
	handle_freqplot();
    }else if (!strcmp(s, "x-spectrum")) {
	/*handle_xspectrum();*/
    }else if (!strcmp(s, "Part motion")) {
	handle_particle_motion();
    }else if (!strcmp(s, "test")) {
	handle_xcorr ();
    }else {
	/* ignored */
    }
}

static void sele_menu_proc(Menu menu, Menu_item menu_item)
{
    char *s= (char *)xv_get(menu_item, MENU_STRING);
    if (!strcmp(s, "script")) {
	handle_select_trace_script();
    }else if (!strcmp(s, "enter")) {
	handle_select_trace_enter();
    }else if (!strcmp(s, "select")) {
	handle_select_trace_select();
    }else if (!strcmp(s, "sele rest")) {
	handle_select_trace_selerest();
    }else if (!strcmp(s, "keep")) {
	handle_select_trace_keep();
    }else if (!strcmp(s, "keep rest")) {
	handle_select_trace_keeprest();
    }else if (!strcmp(s, "rearrange")) {
	handle_select_trace_rearrange();
    }else if (!strcmp(s, "filter")) {
	handle_select_trace_filter();
    }else if (!strcmp(s, "no filter")) {
	Mode_autoFilterNames= 0;
    }else if (!strcmp(s, "clear")) {
	CleanSeleStrings();
    }
}

static void coin_menu_proc(Menu menu, Menu_item menu_item)
{
    char *s= (char *)xv_get(menu_item, MENU_STRING);
    if (strstr(s, "Ground")!=NULL) {
	ConvertResponses(20);
    }else if (strstr(s, "Velocity")!=NULL) {
	ConvertResponses(21);	
    }else if (strstr(s, "Acceleration")!=NULL) {
	ConvertResponses(22);	
    }else if (!strncmp(s, "Ben",3)) {
	ConvertResponses(1);
    }else if (!strncmp(s, "But",3)) {
	ConvertResponses(2);
    }else if (!strncmp(s, "Woo",3)) {
	ConvertResponses(3);
    }else {
	printf("unknown: %s\n",s);
    }
}

static void sort_menu_proc(Menu menu, Menu_item menu_item)
{
    char *s= (char *)xv_get(menu_item, MENU_STRING);

    if (!strcmp(s,"Stat name")) {
	sort_type=SORT_STATION;
    } else if (!strcmp(s,"Channel")) {
	sort_type=SORT_CHANNEL;
    } else if (!strcmp(s,"Distance")) {
	sort_type=SORT_DELTA;
	GetResponses( NULL );
    } else if (!strcmp(s,"Azimuth")) {
	sort_type=SORT_AZIMUTH;
	GetResponses( NULL );
    } else if (!strcmp(s,"Stat lat")) {
	sort_type=SORT_LAT;
	GetResponses( NULL );
    } else if (!strcmp(s,"Stat lon")) {
	sort_type=SORT_LON;
	GetResponses( NULL );
    } else if (!strcmp(s,"First Pick")) {
	sort_type=SORT_PICK;
    }

    sort_traces ();
}

void activate_sort_event(int tf)
{
    Menu_item dist_mi, azim_mi, rot_mi, rot_p_mi;
    
    dist_mi = xv_get(sort_menu, MENU_NTH_ITEM, 4);
    azim_mi = xv_get(sort_menu, MENU_NTH_ITEM, 5);
    rot_mi = xv_get(rot_menu, MENU_NTH_ITEM, 2);
    rot_p_mi = xv_get(rot_pan_menu, MENU_NTH_ITEM, 2);

    if (tf) {
	xv_set(dist_mi, MENU_INACTIVE, FALSE, NULL);
	xv_set(azim_mi, MENU_INACTIVE, FALSE, NULL);
	xv_set(rot_mi, MENU_INACTIVE, FALSE, NULL);
	xv_set(rot_p_mi, MENU_INACTIVE, FALSE, NULL);
    } else {
	xv_set(dist_mi, MENU_INACTIVE, TRUE, NULL);
	xv_set(azim_mi, MENU_INACTIVE, TRUE, NULL);
	xv_set(rot_mi, MENU_INACTIVE, TRUE, NULL);
	xv_set(rot_p_mi, MENU_INACTIVE, TRUE, NULL);
    }
    return;
}

	
