#ifndef lint
static char id[] = "$Id: fsel_xv.c,v 1.2 2013/02/28 21:24:59 lombard Exp $";
#endif

/*
 * fsel_xv.c--
 *    file selection box
 *
 * Copyright (c) 1992-3 Andrew K. Yu, Lawrence Berkeley Laboratory
 *      and University of California, Berkeley.
 * All rights reserved.
 */
#include <sys/param.h>
#include <xview/xview.h>
#include <xview/canvas.h>
#include <xview/panel.h>
#include <xview/xv_xrect.h>
#include <xview/scrollbar.h>
#include <xview/font.h>

#include "proto.h"
#include "xv_proto.h"

/* fonts info */
extern Xv_Font hel_font;
extern GC helb_gc, hel_gc, hel_invgc;
static int line_h, numlines;

/**********************************************************************
 *   Objects & call-back procedures                                   *
 **********************************************************************/

/*   Xv_Objects   */
static Frame fs_frame= 0;
static Canvas d_cvs, f_cvs;
static Panel_item p_text, s_text;
static Scrollbar d_vbar, f_vbar;

/*   XV call back procedures   */
static void fsel_rescaleCanvas();
static void fs_done_proc();
static void InitFilesel(Frame frame, int x, int y);
static void fsel_rescaleCanvas();
static void fs_event_proc(Xv_Window window, Event *event);
static void d_cvs_repaint(Canvas canvas, Xv_Window paint_win, Display *dpy,
			  Window xwin, Xv_xrectlist *xrects);
static void f_cvs_repaint(Canvas canvas, Xv_Window paint_win, Display *dpy,
			  Window xwin, Xv_xrectlist *xrects);
static Notify_value destroy_fs(Notify_client client, Destroy_status status);
static void d_erase(Window xwin);
static void f_erase(Window xwin);
static void d_mark(Window xwin, int idx);
static void f_mark(Window xwin, int idx);
static void highlight(int which, int idx);
static void nohighlight(int which);
static void gotoDir(char *dname);
static void putFile(char *fname);
static void df_event_proc(Xv_Window window, Event *event);
static Panel_setting p_text_notify(Panel_item item, Event *event);
static Panel_setting s_text_notify(Panel_item item, Event *event);
static void b_load_notify();
static void b_reset_notify();
static void b_cancel_notify(Panel_item item, Event *event);



#define DIRFILEKEY  123	    /* key to distinguish bet. d_cvs & f_cvs */
#define D_KEY	    0	    /* event in d_cvs */
#define F_KEY	    1	    /* event in f_cvs */

/**********************************************************************
 *   Global Variables                                                 *
 **********************************************************************/

/*   current directory path   */
char curpath[MAXPATHLEN];   /* MAXPATHLEN in sys/param.h */

/*   from fsel_dir module */
extern int f_i, d_i;
extern char **f_names, **d_names;


/**********************************************************************
 *   Create Filesel window                                            *
 **********************************************************************/
static int  inloop=0;

static FileFormat curFormat;
static int should_return= 0;
static int  double_click_sec=1000000;

static void ChangeDoubleClickDur(int dur);
static void fs_done_proc();
static void InitFilesel(Frame frame, int x, int y);

static void (*load_func)();	/* call back */


/* apparently not used */
static void ChangeDoubleClickDur(int dur)
{
    double_click_sec=dur;
}

void SelectFile(char *title, FileFormat format, void(*func)(), int ret)
{
    curFormat= format;
    load_func= func;
    should_return= ret;

    if(!fs_frame) {
	int x, y;
	x= (int)xv_get(tracesFrame,XV_X);
	y= (int)xv_get(tracesFrame,XV_Y);
	InitFilesel(tracesFrame,x+20,y+30);
    }

    if(*curpath=='\0') {
	fsel_chdir(curpath, ".");  /* initialize */
	fsel_rescaleCanvas();
	d_cvs_repaint(d_cvs, canvas_paint_window(d_cvs), theDisp,
		      xv_get(canvas_paint_window(d_cvs),XV_XID), NULL);
	f_cvs_repaint(f_cvs, canvas_paint_window(f_cvs), theDisp,
		      xv_get(canvas_paint_window(f_cvs),XV_XID), NULL);
    }
    xv_set(fs_frame,
	   FRAME_LABEL, title,
	   XV_SHOW, TRUE,
	   NULL);
    if(ret) {
	inloop=1;
	(void)xv_window_loop(fs_frame);
	inloop=0;
    }
	
}

static void fs_done_proc()
{
    xv_set(fs_frame, XV_SHOW, FALSE, NULL);
    if(inloop)xv_window_return(0);
}

static void InitFilesel(Frame frame, int x, int y)
{
    Panel b_panel, s_panel, p_panel;
    Canvas s_cvs, b_cvs, p_cvs;
    char CWD_path[MAXPATHLEN];	    /* MAXPATHLEN in sys/param.h */

    /* create the file selection frame */
    fs_frame= (Frame)xv_create(frame, FRAME,
			       XV_X, x, XV_Y, y,		       
			       XV_WIDTH, 370, XV_HEIGHT, 540,
			       FRAME_DONE_PROC, fs_done_proc,		       
			       NULL);
    xv_set(fs_frame,
	   WIN_EVENT_PROC, fs_event_proc,
	   WIN_CONSUME_EVENTS, WIN_REPAINT,
	   NULL);
    notify_interpose_destroy_func(fs_frame, destroy_fs);
    
    /* create "path" panel */
    p_cvs= (Canvas) xv_create(fs_frame, CANVAS,
			      XV_X, 10, XV_Y, 30,
			      XV_WIDTH, 350, XV_HEIGHT, 40,
			      NULL);
    p_panel= (Panel)xv_create(p_cvs, PANEL,
			      XV_X, 1,		      
			      XV_WIDTH, 348, XV_HEIGHT, 39,
			      PANEL_LAYOUT, PANEL_HORIZONTAL,
			      NULL);
    fsel_cwd( CWD_path );
    p_text= (Panel_item)xv_create(p_panel, PANEL_TEXT,
				  XV_X, 10, XV_Y, 16,	    
				  PANEL_VALUE_DISPLAY_WIDTH, 330,
				  PANEL_NOTIFY_PROC, p_text_notify,			  
				  PANEL_VALUE, CWD_path,
				  NULL);	    
    /* create "directories" window */
    d_cvs= (Canvas) xv_create(fs_frame, CANVAS,
			      XV_X, 10, XV_Y, 95,
			      XV_WIDTH, 170, XV_HEIGHT, 310,
			      CANVAS_WIDTH, 500, CANVAS_HEIGHT, 310,
			      CANVAS_AUTO_EXPAND, FALSE, CANVAS_AUTO_SHRINK, FALSE,
			      CANVAS_X_PAINT_WINDOW, TRUE,		      
			      CANVAS_REPAINT_PROC, d_cvs_repaint,
			      NULL);
    xv_set(canvas_paint_window(d_cvs),
	   XV_KEY_DATA, DIRFILEKEY, D_KEY,		      
	   WIN_EVENT_PROC, df_event_proc,
	   WIN_CONSUME_EVENTS,
	   LOC_MOVE, LOC_WINENTER, LOC_WINEXIT,
	   WIN_MOUSE_BUTTONS, NULL,
	   NULL);
    /* create "files" window */
    f_cvs= (Canvas) xv_create(fs_frame, CANVAS,
			      XV_X, 200, XV_Y, 95,
			      XV_WIDTH, 160, XV_HEIGHT, 310,
			      CANVAS_WIDTH, 500, CANVAS_HEIGHT, 310,
			      CANVAS_AUTO_EXPAND, FALSE, CANVAS_AUTO_SHRINK, FALSE,
			      CANVAS_X_PAINT_WINDOW, TRUE,		      
			      CANVAS_REPAINT_PROC, f_cvs_repaint,
			      NULL);
    xv_set(canvas_paint_window(f_cvs),
	   XV_KEY_DATA, DIRFILEKEY, F_KEY,		      
	   WIN_EVENT_PROC, df_event_proc,
	   WIN_CONSUME_EVENTS,
	   LOC_MOVE, LOC_WINENTER, LOC_WINEXIT,
	   WIN_MOUSE_BUTTONS, NULL,
	   NULL);
    /* create selection ("file") window */
    s_cvs= (Canvas) xv_create(fs_frame, CANVAS,
			      XV_X, 10, XV_Y, 425,
			      XV_WIDTH, 350, XV_HEIGHT, 40,
			      NULL);
    s_panel= (Panel)xv_create(s_cvs, PANEL,
			      XV_X, 1,		      
			      XV_WIDTH, 348, XV_HEIGHT, 39,
			      PANEL_LAYOUT, PANEL_HORIZONTAL,
			      NULL);
    s_text= (Panel_item)xv_create(s_panel, PANEL_TEXT,
				  XV_X, 10, XV_Y, 16,	    
				  PANEL_VALUE_DISPLAY_WIDTH, 330,
				  PANEL_NOTIFY_PROC, s_text_notify,			  
				  NULL);	    
    /* create buttons window */
    b_cvs= (Canvas) xv_create(fs_frame, CANVAS,
			      XV_X, 10, XV_Y, 485,
			      XV_WIDTH, 350, XV_HEIGHT, 40,
			      NULL);
    b_panel= (Panel)xv_create(b_cvs, PANEL,
			      XV_X, 1,		      
			      XV_WIDTH, 348, XV_HEIGHT, 38,
			      PANEL_LAYOUT, PANEL_HORIZONTAL,
			      NULL);
    (void)xv_create(b_panel, PANEL_BUTTON,
		    XV_X, 40, XV_Y, 10,
		    PANEL_LABEL_STRING, "   Load   ",
		    PANEL_NOTIFY_PROC, b_load_notify,
		    NULL);
    (void)xv_create(b_panel, PANEL_BUTTON,
		    XV_X, 128, XV_Y, 10,	    
		    PANEL_LABEL_STRING, "   Reset   ",
		    PANEL_NOTIFY_PROC, b_reset_notify,
		    NULL);
    (void)xv_create(b_panel, PANEL_BUTTON,
		    XV_X, 220, XV_Y, 10,	    
		    PANEL_LABEL_STRING, "   Cancel   ",
		    PANEL_NOTIFY_PROC, b_cancel_notify,
		    NULL);
    /* show scrollbars */
    d_vbar= xv_create(d_cvs, SCROLLBAR,
		      SCROLLBAR_DIRECTION, SCROLLBAR_VERTICAL,
		      NULL);
    (void)xv_create(d_cvs, SCROLLBAR,
		    SCROLLBAR_DIRECTION, SCROLLBAR_HORIZONTAL,
		    NULL);
    f_vbar= xv_create(f_cvs, SCROLLBAR,
		      SCROLLBAR_DIRECTION, SCROLLBAR_VERTICAL,
		      NULL);
    (void)xv_create(f_cvs, SCROLLBAR,
		    SCROLLBAR_DIRECTION, SCROLLBAR_HORIZONTAL,
		    NULL);
    /* initialze other setup */
    *curpath='\0';
    line_h= (int)xv_get(hel_font, FONT_DEFAULT_CHAR_HEIGHT) + 3;
    numlines= 300/line_h;
}

static void fsel_rescaleCanvas()
{
    if (d_i>numlines) xv_set(d_cvs, CANVAS_HEIGHT, line_h * (d_i+1), NULL);
    if (f_i>numlines) xv_set(f_cvs, CANVAS_HEIGHT, line_h * (f_i+1), NULL);
    xv_set(d_vbar,
	   SCROLLBAR_PIXELS_PER_UNIT, line_h,
	   SCROLLBAR_OBJECT_LENGTH, d_i+1,
	   SCROLLBAR_PAGE_LENGTH, numlines,
	   SCROLLBAR_VIEW_LENGTH, numlines,
	   NULL);
    xv_set(f_vbar,
	   SCROLLBAR_PIXELS_PER_UNIT, line_h,
	   SCROLLBAR_OBJECT_LENGTH, f_i+1,
	   SCROLLBAR_PAGE_LENGTH, numlines,
	   SCROLLBAR_VIEW_LENGTH, numlines,
	   NULL);
    /* forget about horizontal! not essential */
}


/**********************************************************************
 *   Call-back procedures                                             *
 **********************************************************************/

static void fs_event_proc(Xv_Window window, Event *event)
{
    /* draw the labels */
    switch (event_action(event)) {
    case WIN_REPAINT:
	XDrawString(theDisp, xv_get(window,XV_XID), helb_gc,
		    10, 25, "Path:", 5);
	XDrawString(theDisp, xv_get(window,XV_XID), helb_gc,
		    10, 90, "Directories", 11);
	XDrawString(theDisp, xv_get(window,XV_XID), helb_gc,
		    200, 90, "Files", 5);
	XDrawString(theDisp, xv_get(window,XV_XID), helb_gc,
		    10, 420, "File:", 5);
	XFlush(theDisp);	
	break;
    }
}

static void d_cvs_repaint(Canvas canvas, Xv_Window paint_win, Display *dpy,
			  Window xwin, Xv_xrectlist *xrects)
{
    int i, y;
    char *s;

    XClearWindow(dpy, xwin);
    y= line_h;
    for(i=0; i < d_i; i++) {
	s= d_names[i];
	XDrawString(dpy,xwin,hel_gc,5,y,s,strlen(s));
	y+=line_h;
    }
}

static void f_cvs_repaint(Canvas canvas, Xv_Window paint_win, Display *dpy,
			  Window xwin, Xv_xrectlist *xrects)
{
    int i, y;
    char *s;

    XClearWindow(dpy, xwin);
    y= line_h;
    for(i=0; i < f_i; i++) {
	s= f_names[i];
	XDrawString(dpy,xwin,hel_gc,5,y,s,strlen(s));
	y+=line_h;
    }
}

static Notify_value destroy_fs(Notify_client client, Destroy_status status)
{
    if(inloop) xv_window_return(0);    /* just in case */
    return NOTIFY_DONE;
}
   
/**********************************************************************
 *   Highlighting selections                                          *
 **********************************************************************/

/* indices tracking the last highlighted item */
static int d_oldi=-1, f_oldi=-1;

static void d_erase(Window xwin)
{
    char *s= d_names[d_oldi];
    XDrawImageString(theDisp, xwin, hel_gc, 5, (d_oldi+1)*line_h,
		     s, strlen(s));
    XFlush(theDisp);
    d_oldi=-1;	/* reset old index */
}

static void f_erase(Window xwin)
{
    char *s= f_names[f_oldi];
    XDrawImageString(theDisp, xwin, hel_gc, 5, (f_oldi+1)*line_h,
		     s, strlen(s));
    XFlush(theDisp);
    f_oldi=-1;	/* reset old index */
}

static void d_mark(Window xwin, int idx)
{
    char *s= d_names[idx];
    XDrawImageString(theDisp, xwin, hel_invgc, 5, (idx+1)*line_h,
		     s, strlen(s));
    XFlush(theDisp);
    d_oldi=idx;
}

static void f_mark(Window xwin, int idx)
{
    char *s= f_names[idx];
    XDrawImageString(theDisp, xwin, hel_invgc, 5, (idx+1)*line_h,
		     s, strlen(s));
    XFlush(theDisp);
    f_oldi=idx;
}

static void highlight(int which, int idx)
{
    Window xwin;

    if (idx<0) return;
    if (which==D_KEY) {
	if (idx==d_oldi) return; /* already done */
	xwin=(Window)xv_get(canvas_paint_window(d_cvs),XV_XID);
	if (d_oldi!=-1) d_erase(xwin);  /* erase old mark */
	if (idx<d_i) d_mark(xwin, idx);	/* new mark */
    }else {
	if (idx==f_oldi) return; /* already done */
	xwin=(Window)xv_get(canvas_paint_window(f_cvs),XV_XID);
	if (f_oldi!=-1) f_erase(xwin);  /* erase old mark */
	if (idx<f_i) f_mark(xwin, idx);	/* new mark */
    }
}

static void nohighlight(int which)
{
    if (which==D_KEY) {
	if (d_oldi!=-1)    /* erase old mark */
	    d_erase((Window)xv_get(canvas_paint_window(d_cvs),XV_XID));
    }else {
	if (f_oldi!=-1)    /* erase old mark */
	    f_erase((Window)xv_get(canvas_paint_window(f_cvs),XV_XID));
    }
}

/**********************************************************************
 *   Actions                                                          *
 **********************************************************************/

static void gotoDir(char *dname)
{
    char CWD_path[MAXPATHLEN];
    
    /* open the new directory */
    xv_set(fs_frame, FRAME_BUSY, TRUE, NULL);
    fsel_chdir(curpath, dname);
    if (*curpath!='/') {
	fsel_cwd(CWD_path);
	strcat(CWD_path,curpath);
    }else {
	strcpy(CWD_path,curpath);
    }
    xv_set(p_text, PANEL_VALUE, CWD_path, NULL);
    fsel_rescaleCanvas();
    f_oldi=d_oldi= -1;
    d_cvs_repaint(d_cvs, canvas_paint_window(d_cvs), theDisp,
		  xv_get(canvas_paint_window(d_cvs),XV_XID), NULL);
    f_cvs_repaint(f_cvs, canvas_paint_window(f_cvs), theDisp,
		  xv_get(canvas_paint_window(f_cvs),XV_XID), NULL);
    xv_set(fs_frame, FRAME_BUSY, FALSE, NULL);
}

static void putFile(char *fname)
{
    xv_set(s_text, PANEL_VALUE, fname, NULL);
}

static void df_event_proc(Xv_Window window, Event *event)
{
    int y, which, idx;
    static long oldsec=-1, oldusec=-1, oldidx=-1;
    
    y=event_y(event);
    which= (int)xv_get(window, XV_KEY_DATA, DIRFILEKEY);
    idx= y/line_h;
    switch (event_action(event)) {
    case LOC_MOVE:
	highlight(which,idx);
	break;
    case LOC_WINENTER:
	highlight(which,idx);
	break;
    case LOC_WINEXIT:
	nohighlight(which);
	break;
    case ACTION_SELECT:
    case MS_LEFT: {
	if (event_is_up(event)) {
	    long nowsec, nowusec;

	    if (which==D_KEY) {
		if (idx<d_i) gotoDir(d_names[idx]);
	    }else {
		if (idx<f_i) putFile(f_names[idx]);
		nowsec=event->ie_time.tv_sec;
		nowusec=event->ie_time.tv_usec;
		if (oldsec!=-1) {
		    if (oldidx==idx && nowusec-oldusec+
			(nowsec-oldsec)*1000000 < double_click_sec) {

			/* double click */
			b_load_notify();
		    }
		}
		oldsec=nowsec, oldusec=nowusec; oldidx=idx;
	    }
	}
	break;
    }
    default:
	return;
    }
}

static Panel_setting p_text_notify(Panel_item item, Event *event)
{
    char *pname=(char*)xv_get(item, PANEL_VALUE);
    /* take the trailing "/" out */
    if (pname[strlen(pname)-1]=='/') pname[strlen(pname)-1]='\0';
    substTilde(pname);	/* try substitute tilde */
    strcpy(curpath,pname);
    gotoDir(".");
}

static Panel_setting s_text_notify(Panel_item item, Event *event)
{
    b_load_notify();
}

static void b_load_notify()
{
    char *f=(char*)xv_get(s_text, PANEL_VALUE);
    char buf[MAXPATHLEN];

    *buf='\0';
    if(*curpath!='\0') {
	sprintf(buf,"%s/%s", curpath, f);
    }else {
	strcpy(buf,f);
    }
    load_func(buf, curFormat);

    if(should_return)fs_done_proc();
}

static void b_reset_notify()
{
    *curpath='\0';
    gotoDir(".");
}


static void b_cancel_notify(Panel_item item, Event *event)
{
    fs_done_proc();
}

