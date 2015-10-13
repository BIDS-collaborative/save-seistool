#ifndef lint
static char id[] = "$Id: ztrack.c,v 1.3 2013/02/28 21:24:55 lombard Exp $";
#endif

/*
 * ztrack.c--
 *    handles tracks in the zoom window
 *
 * Copyright (c) 1992-3 Andrew K. Yu, Lawrence Berkeley Laboratory
 *      and University of California, Berkeley.
 * All rights reserved.
 */
#include <stdio.h>
#include <xview/xview.h>
#include <xview/canvas.h>
#include <xview/panel.h>
#include <xview/xv_xrect.h>
#include "proto.h"
#include "xv_proto.h"

/**********************************************************************
 *   Variables & prototypes                                           *
 **********************************************************************/
extern Trace **traces;
extern Track **tracks;
extern int lowTrkIndex;
extern int highTrkIndex;
extern int LastTrack;
extern int TotalTracks;
extern int Mode_Auto_redraw;

extern Panel_item ZoomButton;
extern Frame tt_frame;
extern Axis abs_Axis;


/*  static variables  */
/*
 * note that we don't have ADTs for ZTrack (as in Track). We just
 * keep a big window and draw the zoomed traces on it. This will be
 * more convenient when we wanted to manipulate 3 component data.
 */
static Frame ZoomFrame;		    /* the Frame for zoom window */
static Canvas ZoomCanvas= 0;	    /* the Canvas for zoom window */
static Window ZFrmWin;		    /* window ID for zoom window */
static GC   ZFrmGC;		    /* gc for zoom window */

static Canvas ZoomStatusCanvas;	    /* canvas for status line */
static Frame ZTrkPanelFrame=0;	    /* frame for the buttons-panel */

static Canvas ScrollCanvas;	    /* pseudo-canvas for attaching scrollbar */

/*  export variables  */
Window ZCvsWin, ZSCvsWin;	    /* window IDs */
GC   ZCvsGC, ZCvsXorGC, ZSCvsGC;    /* gc, XOR gc's */

int ZoomWindowMapped=0;		    /* 1 when zoom window mapped, 0 otherwise */

/*  (dynamic) sizes  */
int ZCvsWidth= InitZCvsWidth;
int ZCvsHeight= InitZCvsHeight;
int ZTrkHeight;
int ZFactor= 10;		    /* zoom factor */

/* globals */
char StatusString[500];
static char ZoomWinHeader[17];

int Zoom_Mode_sameVertScale= 0;  /* scale vert scale in zoom window */
int ZFrmX=-1, ZFrmY=-1;
int ZPFrmX= -1, ZPFrmY=-1;


/* internal function prototypes */
static Notify_value destroy_ZoomFrame(Notify_client client, 
				      Destroy_status status);
static void ZoomFrame_done_proc();
static void resize_event_proc(Xv_Window window, Event *event);
static void repaintZTrack(Canvas canvas, Xv_Window paint_window, 
			  Xv_xrectlist *area);
static void resizeZTrack(Canvas canvas, int width, int height);
static void repaintZSTrack(Canvas canvas, Xv_Window paint_window, 
			   Xv_xrectlist *area);
static void PlotZTrack_hook(Trace *trc, int iztrk);


/**********************************************************************
 *   Zoom Tracks                                                      *
 **********************************************************************/

void InitZoomWindow(Frame frame)
{
    int x, y, ts_h=0;
    int theScrn;

    if(ZFrmX!=-1 || ZFrmY!=-1) {
	x= ZFrmX;
	y= ZFrmY;
    }else {
	x= (int)xv_get(frame, XV_X)+300;
	y= (int)xv_get(frame, XV_Y)+400;
    }
    if (Mode_align && Mode_ZDisplayTScale) {
	ts_h= 2*ZTSHEIGHT;
	ZCvsHeight-= ts_h;
    }
    ZTrkHeight= ZCvsHeight/NumInitZTracks;

    strcpy(ZoomWinHeader, "  Zoom Window  ");

    /* create the zoom window */
    ZoomFrame= (Frame)xv_create(frame, FRAME, 
				FRAME_LABEL, ZoomWinHeader,
				XV_X, x, XV_Y, y,
				XV_WIDTH, ZCvsWidth+ZTrkLabWidth+XVSBARWIDTH,
				XV_HEIGHT, ZCvsHeight+ZSCvsHeight+ts_h,
				FRAME_DONE_PROC, ZoomFrame_done_proc,
				NULL);
    xv_set(ZoomFrame,
	   WIN_EVENT_PROC, resize_event_proc,
	   WIN_CONSUME_EVENTS, WIN_RESIZE, NULL,
	   NULL);
    notify_interpose_destroy_func(ZoomFrame, destroy_ZoomFrame);

    /*
     * the ScrollCanvas is used to attach the scrollbar only.
     * It is used to get around the problem of ZoomCanvas not starting
     * at 0.
     */
    ScrollCanvas= (Canvas)xv_create(ZoomFrame, CANVAS,
				    XV_X, ZTrkLabWidth+ZCvsWidth, XV_Y, 0,
				    XV_WIDTH, XVSBARWIDTH,
				    XV_HEIGHT, ZCvsHeight+ZSCvsHeight+ts_h,
				    OPENWIN_SHOW_BORDERS, FALSE, /* make it invisible */
				    NULL);
    InitZTrkLab(ZoomFrame);
    ZoomCanvas= (Canvas)xv_create(ZoomFrame, CANVAS,
				  XV_X, ZTrkLabWidth,
				  XV_Y, (Mode_align && Mode_ZDisplayTScale)? ZTSHEIGHT : 0,
				  CANVAS_WIDTH, ZCvsWidth,
				  CANVAS_HEIGHT, ZCvsHeight,
				  CANVAS_REPAINT_PROC, repaintZTrack,
				  NULL);
    ZoomStatusCanvas= (Canvas)xv_create(ZoomFrame, CANVAS,
					XV_X, 0, XV_Y, ZCvsHeight+ts_h,
					CANVAS_WIDTH, ZCvsWidth+ZTrkLabWidth,
					CANVAS_HEIGHT, ZSCvsHeight,
					CANVAS_REPAINT_PROC, repaintZSTrack,
					NULL);

    initZScrollbar(ScrollCanvas);	/* create scroll bar */
    initZTScale(ZoomFrame);		/* create time scale */
    window_fit(ZoomFrame);

    if (Mode_UWPickStyle) {
	xv_set(canvas_paint_window(ZoomCanvas),
	       WIN_EVENT_PROC, ZCvs_event_proc2,
	       WIN_CONSUME_EVENTS,
	       LOC_DRAG, LOC_MOVE, WIN_ASCII_EVENTS, WIN_MOUSE_BUTTONS,
	       LOC_WINENTER, LOC_WINEXIT,
	       NULL,
	       NULL);
    }else {
	xv_set(canvas_paint_window(ZoomCanvas),
	       WIN_EVENT_PROC, ZCvs_event_proc,
	       WIN_CONSUME_EVENTS,
	       LOC_DRAG, LOC_MOVE, WIN_ASCII_EVENTS, WIN_MOUSE_BUTTONS,
	       LOC_WINENTER, LOC_WINEXIT,
	       NULL,
	       NULL);
    }
    ZFrmWin= (Window)xv_get(ZoomFrame, XV_XID);
    ZCvsWin= (Window)xv_get(canvas_paint_window(ZoomCanvas), XV_XID);
    ZSCvsWin= (Window)xv_get(canvas_paint_window(ZoomStatusCanvas), XV_XID);
    /*    createGC( ZFrmWin, &ZFrmGC); */
    createGC( ZCvsWin, &ZCvsGC);
    createGC( ZCvsWin, &ZCvsXorGC);
    createGC( ZSCvsWin, &ZSCvsGC);
    XSetFunction(theDisp, ZCvsXorGC, GXxor);
    theScrn = DefaultScreen(theDisp);
    XSetForeground(theDisp, ZCvsXorGC,
		   WhitePixel(theDisp, theScrn)^BlackPixel(theDisp, theScrn));

    if(ZPFrmX!=-1 || ZPFrmY!=-1) {
	x= ZPFrmX;
	y= ZPFrmY;
    }else {
	x+= 30+ZCvsWidth+ZTrkLabWidth;
    }
    ZTrkPanelFrame= (Frame)xv_create(ZoomFrame, FRAME, 
				     FRAME_LABEL, "",
				     XV_X, x, XV_Y, y,
				     FRAME_DONE_PROC, ZoomFrame_done_proc,
				     NULL );
    notify_interpose_destroy_func(ZTrkPanelFrame, destroy_ZoomFrame);
    InitZPanel(ZTrkPanelFrame);
    window_fit(ZTrkPanelFrame);
}

void open_zoom_window()
{
    int i;

    if (!ZoomWindowMapped) {
	/* map the zoom window */
	xv_set(ZoomFrame, XV_SHOW, TRUE, NULL);
	xv_set(ZTrkPanelFrame, XV_SHOW, TRUE, NULL);

	/* these resizes has to be here for technical reasons: */
	resizeZTrack(ZoomCanvas,xv_get(ZoomCanvas,CANVAS_WIDTH),
		     xv_get(ZoomCanvas,CANVAS_HEIGHT));
	XFlush(theDisp);
	ZoomWindowMapped= 1;

	lowZTrkIndex= lowTrkIndex;	/* the first trace in main window */
	highZTrkIndex= lowTrkIndex+NumZTracks-1;
	
	if (Mode_triplet)
	    Trip_ZoomContentChanged(lowZTrkIndex);
	i=lowZTrkIndex;	/* do the first one manually */
	ScaleZTrack(traces[i], NULL);
	if (Zoom_Mode_sameVertScale) traces[i]->zaxis_needScale= 1;
	UpdateMarks(i);
	for(i=lowZTrkIndex+1; i <= highZTrkIndex; i++) {
	    ScaleZTrack(traces[i], traces[i-1]);
	    if (Zoom_Mode_sameVertScale) traces[i]->zaxis_needScale= 1;
	    UpdateMarks(i);
	}
	if (Zoom_Mode_sameVertScale) newVertScale();
	xv_set(ZoomButton, PANEL_INACTIVE, TRUE, NULL);
	RedrawZoomWindow("open_zoom_window");
    }
}

void close_zoom_window()
{
    int i;
    if (ZoomWindowMapped) {
	if(Mode_trvlTime) ExitTTMode();
        if(Mode_rotate)close_rotate_panel();
	xv_set(ZoomFrame, XV_SHOW, FALSE, NULL);
	xv_set(ZTrkPanelFrame, XV_SHOW, FALSE, NULL);
	XFlush(theDisp);
	for(i=lowZTrkIndex; i <= highZTrkIndex; i++) {
	    UpdateMarks(i);
	}
	ZoomWindowMapped = 0;
	xv_set(ZoomButton, PANEL_INACTIVE, FALSE, NULL);
    }
}

static Notify_value destroy_ZoomFrame(Notify_client client, 
				      Destroy_status status)
{
    if (status==DESTROY_CLEANUP) {
	close_zoom_window();
	/* allow frame to be destroyed */
	return notify_next_destroy_func(client, status);
    }else {
	/* status== DESTROY_CHECKING || DESTROY_SAVE_YOURSELF || etc */
	if (status==DESTROY_CHECKING) {
#ifdef DEBUG
	    printf("destroy_ZoomFrame: DESTROY_CHECKING\n");
#endif	    
	}else if(status==DESTROY_SAVE_YOURSELF) {
#ifdef DEBUG
	    printf("destroy_ZoomFrame: DESTROY_SAVE_YOURSELF\n");
#endif	    
	}
	return NOTIFY_DONE;
    }
}

static void ZoomFrame_done_proc()
{
    close_zoom_window();
}

/**********************************************************************
 *   Scaling a Zoom Track                                             *
 **********************************************************************/

void ScaleZTrack(Trace *trc, Trace *reftrc)
{
    int n_vals, len;
    float vsmag= 1.0;

    /* don't scale if it's scaled already--
       apparently, users like it the other way around:
       if (trc->wave && trc->zaxis.hs==0) { */
    if (trc->wave) {
	/* determine position of the zoom window */
	if (Mode_align) {
	    n_vals= trc->wave->info.n_values - 1;
	    len= trc->axis.ix2 - trc->axis.ix1 + 1;
	    if (reftrc==NULL || reftrc->zaxis.hs==0) {
		/* reference trace is not scaled yet */
		trc->zaxis.ix1= len/2;
		trc->zaxis.ix2= trc->zaxis.ix1 + len/ZFactor; /* scales down */
	    }else {
		if (Mode_align==1) {
		    STI_TIME t1, t2;
		    int newix1, newix2;
		
		    t1= indexToTime(reftrc, reftrc->zaxis.ix1,1);
		    t2= indexToTime(reftrc, reftrc->zaxis.ix2,1);
		    newix1= timeToIndex(trc, t1);
		    newix2= timeToIndex(trc, t2);
		    trc->zaxis.ix1= newix1;
		    trc->zaxis.ix2= newix2;
		    vsmag= reftrc->zaxis.vsmag;
		} else if (Mode_align==2) {
		    float beg_sec,end_sec;
		    int newix1, newix2;
		    float srate=trc->wave->info.sample_rate;
		    float ref_srate=reftrc->wave->info.sample_rate;

		    beg_sec=reftrc->zaxis.ix1/ref_srate;
		    end_sec=reftrc->zaxis.ix2/ref_srate;
		    newix1=beg_sec*srate;
		    newix2=end_sec*srate;

		    trc->zaxis.ix1= newix1;
		    trc->zaxis.ix2= newix2;
		    vsmag= reftrc->zaxis.vsmag;
		}
	    }
	    ScaleTAxis(&trc->zaxis, trc->wave->info.sample_rate, ZCvsWidth);
	}else {	    /* not Mode_align */
	    n_vals= trc->wave->info.n_values - 1;
	    if (reftrc==NULL || reftrc->zaxis.hs==0) {
		/* reference trace is not scaled yet */
		trc->zaxis.ix1= n_vals/2;
		trc->zaxis.ix2= trc->zaxis.ix1 + n_vals/ZFactor; /* scales down */
	    }else {
		int x1, x2;

		/* use coord for ref */
		x1= indexToCoord(&reftrc->axis, reftrc->zaxis.ix1,1);
		x2= indexToCoord(&reftrc->axis, reftrc->zaxis.ix2,1);
		trc->zaxis.ix1= coordToIndex(&trc->axis, x1, 1);
		trc->zaxis.ix2= coordToIndex(&trc->axis, x2, 1);
		vsmag= reftrc->zaxis.vsmag;
	    }
	    ScaleTAxis(&trc->zaxis, trc->wave->info.sample_rate, ZCvsWidth);
	}
	ScaleYAxis(&trc->zaxis, ZTrkHeight);
	trc->zaxis.vs/= vsmag;	    /* preserve magnification */
	trc->zaxis.vsmag= vsmag;
    }
}

/**********************************************************************
 *   Redraw & Resize Zoom window                                      *
 **********************************************************************/

/*
 * Redraw the zoom canvas (originally RedrawZoomWindow). This is here so
 * that the repaint procedure can repaint the zoom canvas and time scales
 * separately. 
 */
void redrawZoomCanvas(char *funcname)
{
    int i;
    XClearWindow(theDisp, ZFrmWin);
    XClearWindow(theDisp, ZCvsWin);
    CleanZLab();
#ifdef DEBUG
    if (funcname)
	fprintf(stderr,"redrawZoomCanvas: from %s\n", funcname);
#endif

    for(i=0; i < NumZTracks; i++) {
	UpdatePlotZTrack(traces[i+lowZTrkIndex], i);
	Zoom_UpdateInterval(i+lowZTrkIndex);
    }

    SetZSbarPosition();	/* position the scale bar correctly */

    if (Mode_trvlTime) replotTrvlTime();

    XFlush(theDisp);
}

/*
 * Redraw the contents of the Zoom Window (including the canvas and the
 * time scales, if necessary).
 */
void RedrawZoomWindow(char *funcname)
{
    if(!ZoomWindowMapped)
	return;
    redrawZoomCanvas(funcname);
    /*    printf("display time %d %d\n",Mode_align,Mode_ZDisplayTScale); */
    if (Mode_align && Mode_ZDisplayTScale) {
	/*
	 * we recalculate the time scale parameters here for the sake of
	 * convenience. Should have done it where content has clearly
	 * changed. (It might just be a repaint here.)
	 */
	/*      printf("drawing zoom t scale\n"); */
	TS_ZoomContentChanged(lowZTrkIndex);
	RedrawBothZTScale();
    }
}

void ResizeZoomWindow()
{
    int newht, neww, ts_h=0;
    int needRedraw;

    if (Mode_align && Mode_ZDisplayTScale)
	ts_h= 2*ZTSHEIGHT;
    newht= (int)xv_get(ZoomFrame, XV_HEIGHT)-ZSCvsHeight-ts_h;
    neww= (int)xv_get(ZoomFrame, XV_WIDTH)-ZTrkLabWidth-XVSBARWIDTH+4;

    /* ignore same sizes */
    if (newht==ZCvsHeight && neww==ZCvsWidth)
	return;

    needRedraw= newht<=ZCvsHeight || neww<=ZCvsWidth;
#ifdef DEBUG
    fprintf(stderr, "newht(ZCvsht): %d(%d) neww(ZCvsw): %d(%d) needredraw %d\n",
	    newht, ZCvsHeight, neww, ZCvsWidth, needRedraw);
#endif	   
    resizeZTrack(ZoomCanvas, neww, newht);
    /*
     * this is a kludge to get around XView's reluctance to redraw 
     * when a canvas is resized to a smaller size.
     */
    if(needRedraw) {
	RedrawZoomWindow("ResizeZoomWindow");
    }
}

static void resize_event_proc(Xv_Window window, Event *event)
{
    /* ignore synthetic events from window manager */
    if (event_xevent(event)->xconfigure.send_event!=TRUE) {
	ResizeZoomWindow();
    }
}

/**********************************************************************
 *   Plotting, Redrawing & Resizing a Zoom Track                      *
 **********************************************************************/

void UpdatePlotZTrack(Trace *trc, int iztrk)
{
    int k,i;
    XPoint *plot, *points;

    if (trc && trc->wave && ZoomWindowMapped
        && trc->zaxis.hs!=0) {	/* make sure get scaled */
	float *data= trc->wave->data;
	int yoffset= iztrk * ZTrkHeight, y;

	if (trc->zaxis_needScale) {
	    float vsmag= 1.0;
	    /* rescale Zaxis */
	    if(!Zoom_Mode_sameVertScale) {
		if(trc->zaxis.vsmag>0.0) vsmag= trc->zaxis.vsmag;
		ScaleYAxis(&trc->zaxis, ZTrkHeight);
		trc->zaxis.vs/= vsmag;
		trc->zaxis.vsmag= vsmag;
	    }else {
		UniformScaleYAxis(&trc->zaxis, ZTrkHeight);
	    }
	    ScaleTAxis(&trc->zaxis, trc->wave->info.sample_rate, ZCvsWidth);
	    trc->zaxis_needScale=0;	/* no need now */

	}

	if (Mode_rotate && trc->trip->rotated == 1) {
	    rt_PlotWave(trc, &trc->zaxis, ZCvsWidth, ZTrkHeight, ZCvsWin,
			ZCvsGC, Mode_clipPlot, yoffset, iztrk+lowZTrkIndex);
	}else if(Mode_decimPlot){
	    PlotWave_decim(trc, &trc->zaxis, ZCvsWidth, ZTrkHeight, ZCvsWin,
			   ZCvsGC, Mode_clipPlot, yoffset, iztrk+lowZTrkIndex);
	}else {
	    PlotWave(trc, &trc->zaxis, ZCvsWidth, ZTrkHeight, ZCvsWin,
		     ZCvsGC, Mode_clipPlot, yoffset, iztrk+lowZTrkIndex);
	}

	UpdateZlab(iztrk);  /* Label */
	DrawZPicks(trc->picks, iztrk);	/* update picks */
	PlotZTrack_hook(trc,iztrk);

	XFlush(theDisp);
    }
}

static void repaintZTrack(Canvas canvas, Xv_Window paint_window, 
			  Xv_xrectlist *area)
{
    if (!Mode_Auto_redraw) return;
    if (ZoomWindowMapped) {
	redrawZoomCanvas("repaintZTrack");
    }
}

/* %%% */
static void resizeZTrack(Canvas canvas, int width, int height)
{
    int i, y, ts_h= 0;
    int shrink= height<ZCvsHeight;

    if (Mode_align && Mode_ZDisplayTScale)
	ts_h= 2*ZTSHEIGHT;
    ZCvsWidth= width;
    ZCvsHeight= height; 
    ZTrkHeight= ZCvsHeight/NumZTracks;
    if (ZoomCanvas) {
	xv_set(ScrollCanvas,
	       XV_X, ZTrkLabWidth+width-4, XV_Y, 0,
	       XV_WIDTH, XVSBARWIDTH,
	       XV_HEIGHT, ZCvsHeight+ZSCvsHeight+ts_h,
	       NULL);    
	y= (Mode_align && Mode_ZDisplayTScale) ? ZTSHEIGHT : 0;
	xv_set(ZoomCanvas,
	       XV_X, ZTrkLabWidth, XV_Y, y,    
	       XV_WIDTH, ZCvsWidth, 
	       XV_HEIGHT, ZCvsHeight,
	       NULL);
	AdjustZSbar(height);
	SetZSbarPosition();
	/* mark all need resize */
	for(i=0; i <= LastTrack; i++) {
	    traces[i]->zaxis_needScale= 1;
	}
	if(Zoom_Mode_sameVertScale) newVertScale();
	if(Mode_trvlTime) Trvl_Rescale();
	ResizeZlab(shrink);   /* resize the labels also */
	/* do the status window also */
	xv_set(ZoomStatusCanvas,
	       XV_Y, ZCvsHeight+ts_h, 
	       XV_WIDTH, ZCvsWidth+ZTrkLabWidth,
	       XV_HEIGHT, ZSCvsHeight,
	       NULL);
	ResizeZTScale();
    }
}


/**********************************************************************
 *   Updating the Status Window                                       *
 **********************************************************************/

void CleanZStatus()
{
    XClearWindow(theDisp, ZSCvsWin);
    XFlush(theDisp);
}

void UpdateZStatus()
{
    XClearWindow(theDisp, ZSCvsWin);
    XDrawString(theDisp, ZSCvsWin, ZSCvsGC, 20, 25,
		StatusString, strlen(StatusString));
    if(Zoom_Mode_sameVertScale) {
	XDrawString(theDisp, ZSCvsWin, ZSCvsGC, 550, 25,
		    "UniVS", 5);
    }
    XFlush(theDisp);
}

static void repaintZSTrack(Canvas canvas, Xv_Window paint_window, 
			   Xv_xrectlist *area)
{
    if (ZoomWindowMapped)
	UpdateZStatus();
}

/**********************************************************************
 *   Changing number of zoomed traces                                 *
 **********************************************************************/

void internal_ChangeNumZTracks(int total)
{
    int i;

    if (total>0 && total<=TotalTracks) {
	/* ZTrkLab */
	DestroyZTrkLab();

	NumZTracks= total;

	ZTrkHeight= ZCvsHeight/NumZTracks;
	InitZTrkLab(ZoomFrame);
    }
}

void ChangeNumZTracks(int total)
{
    int i;
    if(Mode_triplet) return; /* nope, can't do it */
    if (total>0 && total<=TotalTracks) {
	if (ZoomWindowMapped) CleanMarks(); /* cleanup first */
	internal_ChangeNumZTracks(total);
	lowZTrkIndex= 0;
	highZTrkIndex= total - 1;

	for(i=lowZTrkIndex; i <= highZTrkIndex; i++) {
	    ScaleZTrack(traces[i], traces[lowZTrkIndex]);
	    /*	    if (Zoom_Mode_sameVertScale) traces[i]->zaxis_needScale= 1;*/
	    if(ZoomWindowMapped)UpdateMarks(i);
	}
	for(i=0; i<= LastTrack; i++) {
	    traces[i]->zaxis_needScale= 1;
	}
	ResizeZlab(0);   /* resize the labels also */
	if (Zoom_Mode_sameVertScale)  newVertScale();
	if (ZoomWindowMapped) RedrawZoomWindow("ChangeNumZTrack");
    }
}

/**********************************************************************
 *   Zoom Window Header                                               *
 **********************************************************************/

void ZWHdr_ShowChange()
{
    ZoomWinHeader[14]='*';
    xv_set(ZoomFrame, FRAME_LABEL, ZoomWinHeader, NULL);
}

void ZWHdr_DismissChange()
{
    ZoomWinHeader[14]=' ';
    xv_set(ZoomFrame, FRAME_LABEL, ZoomWinHeader, NULL);
}

/**********************************************************************
 *   Set mode                                                         *
 **********************************************************************/

/* setmode_UWPickStyle in ztrack.c */
void setmode_UWPickStyle_on()
{
    xv_set(canvas_paint_window(ZoomCanvas),
	   WIN_EVENT_PROC, ZCvs_event_proc2, NULL);
}

void setmode_UWPickStyle_off()
{
    xv_set(canvas_paint_window(ZoomCanvas),
	   WIN_EVENT_PROC, ZCvs_event_proc, NULL);
}


void open_tt_panel()
{
    if (!tt_frame) {
	InitTTPanel(ZoomFrame);
    }
    xv_set(tt_frame, XV_SHOW, TRUE, NULL);
}

void close_tt_panel()
{
    xv_set(tt_frame, XV_SHOW, FALSE, NULL);
}

void SwitchTTZevtProc()
{
    xv_set(canvas_paint_window(ZoomCanvas),
	   WIN_EVENT_PROC, TTMode_Zevent_proc, NULL);
}

void RestoreZevtProc()
{
    if(Mode_UWPickStyle) {
	xv_set(canvas_paint_window(ZoomCanvas),
	       WIN_EVENT_PROC, ZCvs_event_proc2, NULL);
    }else {
	xv_set(canvas_paint_window(ZoomCanvas),
	       WIN_EVENT_PROC, ZCvs_event_proc, NULL);
    }
}

/**********************************************************************
 *   Same Vert Scaling                                                *
 **********************************************************************/

void newVertScale()
{
    int i;
    Axis *ax;
    float y1, y2;
    ax= &traces[lowZTrkIndex]->zaxis;
    y1= ax->y1, y2= ax->y2;
    for(i=lowZTrkIndex+1; i<=highZTrkIndex; i++) {
	ax= &traces[i]->zaxis;
	if (ax->y1>y1) y1= ax->y1;
	if (ax->y2<y2) y2= ax->y2;
    }

    /* store the info in abs_Axis */
    /* yeah I know I should not us ix1 for the height */
    abs_Axis.vs=(float)(y1-y2)/ZTrkHeight;
    abs_Axis.ymax=abs_Axis.y1=y1;
    abs_Axis.ymin=abs_Axis.y2=y2;
    abs_Axis.ix1=ZTrkHeight;
    abs_Axis.y0=y1/ZTrkHeight;
}


/**********************************************************************
 *   hooks                                                            *
 **********************************************************************/

static void PlotZTrack_hook(Trace *trc, int iztrk)
{
    void (*func_hook)();
    
    if(trc->hooks && trc->hooks->plot_ztrack) {
	func_hook= trc->hooks->plot_ztrack;
	func_hook(trc, iztrk);
    }
}

void printZoomFrameDim(FILE *fp)
{
    int x, y;
    x= xv_get(tracesFrame,XV_X)+xv_get(ZoomFrame,XV_X);
    y= xv_get(tracesFrame,XV_Y)+xv_get(ZoomFrame,XV_Y);
    fprintf(fp,"position zwin %d %d\n",x,y);
    fprintf(fp,"position zpan %d %d\n",xv_get(ZTrkPanelFrame,XV_X)+x,
	    xv_get(ZTrkPanelFrame,XV_Y)+y);
}
