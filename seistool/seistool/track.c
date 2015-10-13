#ifndef lint
static char id[] = "$Id: track.c,v 1.3 2013/02/28 21:24:57 lombard Exp $";
#endif

/*
 * track.c--
 *    handling tracks in the main window
 *
 * Copyright (c) 1992-3 Andrew K. Yu, Lawrence Berkeley Laboratory
 *      and University of California, Berkeley.
 * All rights reserved.
 */
#include <strings.h>
#include <xview/canvas.h>
#include <xview/panel.h>
#include <xview/xv_xrect.h>
#include "proto.h"
#include "xv_proto.h"

int Mode_Auto_redraw=1;
int Mode_labDetails= 0;

extern Canvas tracesScvs;
extern double TotalSeconds;
extern int NumTracks;
extern int TotalTracks;
extern int LastTrack;
extern int lowTrkIndex;
extern int highTrkIndex;
extern int subsample_rate;
extern int TotalWaifTraces;
extern Trace **waifTraces;
extern STI_TIME earl_time, late_time;
extern Axis abs_Axis;    /* hold all the absolute scale info */


/**********************************************************************
 *   Tracks                                                           *
 **********************************************************************/

/* internal function prototypes */
static void CreateTrk();
static void CreateTracks(Frame frame);
static void CreateTrk(Frame frame, int i, int height, int width, int y);
static void DestroyTracks();
static void repaintTracks(Canvas canvas, Xv_Window paint_window, Display *dpy,
			  Window xwin, Xv_xrectlist *area);
static void repaintTrackLabs(Canvas canvas, Xv_Window paint_window,
			     Display *dpy, Window xwin, Xv_xrectlist *area);
static void RescaleTrace(Trace *trc, int height, int width);


/* globals */
Track	**tracks;
Trace   **traces;
int Mode_abs_vscale=0;
extern int auto_demean;

Trace *newTrace()
{
    Trace *trc=(Trace *)Malloc(sizeof(Trace));
    if(trc==NULL)return NULL;
    bzero(trc,sizeof(Trace));
    trc->wave=NULL;
    trc->loadOrder=-1;
    return trc;
}

void cleanTrace(Trace *trc)
{
    if (trc && trc->wave) {
	Pick *p, *t;
	Amp *a, *ta;
	Reg_select *reg,*t_reg;
	
	/* free wave data */
	if (trc->wave->data)
	    free(trc->wave->data);
	free(trc->wave);

	/* free pick data */
	p= trc->picks;
	while(p!=NULL) {
	    t=p; p=p->next;
	    free(t);
	}
	a= trc->amps;
	while(a!=NULL) {
	    ta=a; a = a->next;
	    free(ta);
	}
	
	/* free region data */
	reg=trc->sel_reg;
	while (reg!=NULL) {
	    t_reg=reg;
	    reg=t_reg->next;
	    free(t_reg);
	}
	bzero(trc, sizeof(Trace));
	trc->wave=NULL;
	trc->picks=NULL;
	trc->amps=NULL;
	trc->sel_reg=NULL;
	trc->zaxis.hs=0;
	trc->zaxis.vs=0;
	/* how to deal with triplet? what if just remove one of
	   them, leaving the other two? */
	if(trc->trip)destroyTriplet(trc);
	if(trc->hooks && trc->hooks->cleanup_trace)
	    trc->hooks->cleanup_trace(trc);
	if(trc->client_data) {
	    ClientData *c, *t= trc->client_data;
	    while(c!=NULL) {
		t=c; c=c->next;
		free(t);
	    }
	}
	if(trc->hooks)free(trc->hooks);
    }
}

void initTracks(Frame frame)
{    
    int i;

    tracks= (Track **)Malloc(sizeof(Track*)*NumTracks);
    traces= (Trace **)Malloc(sizeof(Trace*)*TotalTracks);
    if (tracks==NULL || traces==NULL) {
	return;	/* not enuff mem */
    }
    if (!Mode_CLI) {
	CreateTracks(frame);
    }else {
	for(i=0; i < NumTracks; i++) {
	    Track *trk;
	    trk= tracks[i]= (Track *)Malloc(sizeof(Track));
	    trk->height= 100;	/* arbitrary */
	    trk->width= 800;	/* arbitrary */
	}
    }
    
    for(i=0; i < TotalTracks; i++) {
	traces[i]= newTrace();
	if (traces[i]==NULL) return;	/* not enuff mem */
    }

    if(TotalWaifTraces > 0) {
	waifTraces= (Trace **)Malloc(sizeof(Trace*)*TotalWaifTraces);
	if (waifTraces==NULL) {
	    return;	/* not enuff mem */
	}
	for (i = 0; i < TotalWaifTraces; i++){
	    waifTraces[i] = newTrace();
	    if (waifTraces[i] == NULL) return;   /* not unuff mem */
	}
    }
}


static void CreateTracks(Frame frame)
{
    int height, i, y;
    int tsHeight= 30;
    int panel_width = top_panel_width;

    if((FrmHeight-panel_width-tsHeight)/NumTracks < 30)
	tsHeight= 15;
    if (Mode_align) {	/* reserve space for time scale */
	height= (float)(FrmHeight-panel_width-tsHeight)/NumTracks;
	tsHeight= FrmHeight-panel_width-height*NumTracks;
    }else {
	height= (float)(FrmHeight-panel_width)/NumTracks;
    }
    y=panel_width;
    for(i=0; i < NumTracks; i++) {
	CreateTrk(frame, i, height, FrmWidth, y);
	y+=height;
    }
    /* create one for time scale if necessary */
    CreateTimeScale(frame, y, tsHeight);

    if(!Mode_align) {
	extern Canvas tscl_canvas;      
	/*      xv_set(frame,XV_SHOW,FALSE,NULL); */
	/*      printf("remove time bar\n"); */
	xv_set(tscl_canvas,XV_SHOW,FALSE,NULL);
	/*CreateTimeScale(frame, y, tsHeight);*/
    }
}

static void CreateTrk(Frame frame, int i, int height, int width, int y)
{
    Canvas canvas, txtcanvas;
    Track *trk;
    extern Display *theDisp;
    int theScrn;

    trk= tracks[i]= (Track *)Malloc(sizeof(Track));
    if (trk==NULL) return;  /* not enuff mem */
    if (Mode_noBorder) {
	txtcanvas= (Canvas)xv_create(frame, CANVAS,
				     XV_X, 0, XV_Y, y,
				     XV_WIDTH, TXTCANV_WIDTH-2,
				     XV_HEIGHT, height,
				     /*	     CANVAS_AUTO_EXPAND, FALSE,	    -- allow for resize
					     CANVAS_AUTO_SHRINK, FALSE, */
				     OPENWIN_SHOW_BORDERS, FALSE,
				     CANVAS_X_PAINT_WINDOW, TRUE,
				     CANVAS_RETAINED, FALSE,
				     CANVAS_REPAINT_PROC, repaintTrackLabs,
				     XV_KEY_DATA, TRACK_INDEX, i,  /* for identification */
				     NULL);
	canvas= (Canvas)xv_create(frame, CANVAS,
				  XV_X, TXTCANV_WIDTH, XV_Y, y,
				  XV_WIDTH, FrmWidth-TXTCANV_WIDTH-19,
				  XV_HEIGHT, height,
				  CANVAS_X_PAINT_WINDOW, TRUE,
				  OPENWIN_SHOW_BORDERS, FALSE,
				  /*	     CANVAS_AUTO_EXPAND, FALSE,
					     CANVAS_AUTO_SHRINK, FALSE,*/
				  CANVAS_RETAINED, FALSE,
				  CANVAS_REPAINT_PROC, repaintTracks,
				  XV_KEY_DATA, TRACK_INDEX, i,  /* for identification */
				  NULL);
    }else {
	/* well, guess what? OPENWIN_SHOW_BORDERS can only be used in
	   xv_create! */
	txtcanvas= (Canvas)xv_create(frame, CANVAS,
				     XV_X, 0, XV_Y, y,
				     XV_WIDTH, TXTCANV_WIDTH,
				     XV_HEIGHT, height,
				     /*	     CANVAS_AUTO_EXPAND, FALSE,	    -- allow for resize
					     CANVAS_AUTO_SHRINK, FALSE, */
				     CANVAS_X_PAINT_WINDOW, TRUE,
				     CANVAS_RETAINED, FALSE,
				     CANVAS_REPAINT_PROC, repaintTrackLabs,
				     XV_KEY_DATA, TRACK_INDEX, i,  /* for identification */
				     NULL);
	canvas= (Canvas)xv_create(frame, CANVAS,
				  XV_X, TXTCANV_WIDTH, XV_Y, y,
				  XV_WIDTH, FrmWidth-TXTCANV_WIDTH-19,
				  XV_HEIGHT, height,
				  CANVAS_X_PAINT_WINDOW, TRUE,
				  /*	     CANVAS_AUTO_EXPAND, FALSE,
					     CANVAS_AUTO_SHRINK, FALSE,*/
				  CANVAS_RETAINED, FALSE,
				  CANVAS_REPAINT_PROC, repaintTracks,
				  XV_KEY_DATA, TRACK_INDEX, i,  /* for identification */
				  NULL);

    }
    trk->height= height;
    trk->width= width-TXTCANV_WIDTH-19;
    trk->canvas= canvas;
    trk->TxtCanvas= txtcanvas;
    trk->xwin=(Window)xv_get(canvas_paint_window(canvas),XV_XID);
    trk->txtwin=(Window)xv_get(canvas_paint_window(txtcanvas),XV_XID);

    /* standard GC */
    createGC( trk->xwin, &trk->gc);
    /* xor GC */
    createGC( trk->xwin, &trk->ovrGC);
    XSetFunction(theDisp, trk->ovrGC, GXxor);
    theScrn = DefaultScreen(theDisp);
    XSetForeground(theDisp, trk->ovrGC,
		   WhitePixel(theDisp, theScrn)^BlackPixel(theDisp, theScrn));
    /* text font for track labels */
    createGC(trk->txtwin, &trk->txtgc);
    /*    XSetFunction(theDisp, trk->txtgc, GXxor); */

    xv_set(canvas_paint_window(canvas),
	   WIN_EVENT_PROC, Cvs_event_proc,
	   WIN_CONSUME_EVENTS, KBD_DONE, KBD_USE, LOC_WINENTER,
	   LOC_WINEXIT, WIN_ASCII_EVENTS, WIN_MOUSE_BUTTONS, LOC_DRAG,
	   NULL,
	   NULL);
    xv_set(canvas_paint_window(txtcanvas),
	   WIN_EVENT_PROC, TxtCvs_event_proc,
	   WIN_CONSUME_EVENTS, WIN_MOUSE_BUTTONS,
	   NULL,
	   NULL);
}

static void DestroyTracks()
{
    Track *trk;
    int i;
    for(i=0; i < NumTracks; i++) {
	trk= tracks[i];
	xv_destroy_safe(trk->canvas);
	xv_destroy_safe(trk->TxtCanvas);
	XFreeGC(theDisp, trk->gc);
	XFreeGC(theDisp, trk->ovrGC);
	XFreeGC(theDisp, trk->txtgc);
	free(trk);
    }
    free(tracks);
    if (Mode_align) DestroyTimeScale();
}

/**********************************************************************
 *   Redraw & Resize All Tracks                                       *
 **********************************************************************/


void RedrawScreen()
{
    int i;

    if(Mode_CLI) return;

    for(i=0; i < NumTracks; i++) {
	XClearWindow(theDisp, tracks[i]->xwin);
	UpdatePlotTrack(i+lowTrkIndex,tracks[i]);
	UpdateLabTrack(traces[i+lowTrkIndex],tracks[i]);
    }
    if(Mode_align) RedrawTScale();

    if(Mode_noBorder) {
	GC gc= DefaultGC(theDisp, DefaultScreen(theDisp));
	Window win= (Window)xv_get(tracesFrame, XV_XID);
	XDrawRectangle(theDisp, win, gc, 0,100, 500,500);
    }
    XFlush(theDisp);
}


void ResizeScreen(Xv_Window window, Event *event)
{
    int newht, cfght, neww, cfgw;
    char *s;
    int tsht= 30;
    int panel_width = top_panel_width;

    if (event_action(event)==WIN_REPAINT) {
	if(Mode_noBorder) {
	    GC gc= DefaultGC(theDisp, DefaultScreen(theDisp));
	    XFillRectangle(theDisp, xv_get(window,XV_XID), gc,
			   TXTCANV_WIDTH-2,0,5,FrmHeight);
	}
	return;
    }

    
    /* only handles RESIZE events for this event_proc */
    if (event_action(event)!=WIN_RESIZE) return;

    newht= (int)xv_get(tracesFrame, XV_HEIGHT);
    neww=  (int)xv_get(tracesFrame, XV_WIDTH);

    /* ignore synthetic events from window manager */
    if (event_xevent(event)->xconfigure.send_event!=TRUE) {
	int y, i, newtrkht;
	Track *trk;

	FrmHeight=newht; FrmWidth=neww;
	if((newht-panel_width-tsht)/NumTracks<30)
	    tsht= 15;
	if(Mode_align) {
	    newtrkht= (float)(newht-panel_width-tsht)/NumTracks;
	    tsht= newht-panel_width-newtrkht*NumTracks;
	}else {
	    newtrkht= (float)(newht-panel_width)/NumTracks;
	}
	y=panel_width;
	xv_set(tracesScvs, XV_X, neww-XVSBARWIDTH, XV_Y, panel_width, NULL);
	for(i=0; i < NumTracks; i++) {
	    trk= tracks[i];
	    trk->height=newtrkht; trk->width=neww-TXTCANV_WIDTH-XVSBARWIDTH;
	    if (trk->TxtCanvas) {
		if(Mode_noBorder) {
		    xv_set(trk->TxtCanvas,
			   XV_X, 0, XV_Y, y,
			   XV_WIDTH, TXTCANV_WIDTH-2,
			   XV_HEIGHT, newtrkht,
			   NULL);
		}else {
		    xv_set(trk->TxtCanvas,
			   XV_X, 0, XV_Y, y,
			   XV_WIDTH, TXTCANV_WIDTH,
			   XV_HEIGHT, newtrkht,
			   NULL);
		}
	    }
	    if (trk->canvas) {
		xv_set(trk->canvas,
		       XV_X, TXTCANV_WIDTH, XV_Y, y,
		       XV_WIDTH, neww-TXTCANV_WIDTH-19,
		       XV_HEIGHT, newtrkht,
		       NULL);
	    }
	    y+=newtrkht;
	}
	AdjustSbar();
	SetSbarPosition();
	for(i=0; i <= LastTrack; i++) {
	    /* can't do this: clipping won't work 
	     *	traces[i]->axis_needScale= 1;
	     */
	    RescaleTrace(traces[i], newtrkht, tracks[0]->width);
	}
	if (Mode_align) {
	    ResizeTScale(y, neww-19, tsht);
	}
	/* don't trust all the canvas being auto-repainted, known
	   to fail user's expectations: here's the forbidden fix,
	   but let the user control it. */
	if (Mode_Auto_redraw)
	    RedrawScreen();
    }
    return;
}

/**********************************************************************
 *   Repaint Tracks                                                   *
 **********************************************************************/

static void repaintTracks(Canvas canvas, Xv_Window paint_window, Display *dpy,
			  Window xwin, Xv_xrectlist *area)
{
    int itrk;
    
    if (!Mode_Auto_redraw) return;
    itrk=(int)xv_get(canvas, XV_KEY_DATA, TRACK_INDEX);
    XClearWindow(dpy, xwin);    /* safer */
    UpdatePlotTrack(itrk+lowTrkIndex, tracks[itrk]);
}

static void repaintTrackLabs(Canvas canvas, Xv_Window paint_window,
			     Display *dpy, Window xwin, Xv_xrectlist *area)
{
    int itrk;

    if (!Mode_Auto_redraw) return;
    itrk =(int)xv_get(canvas,XV_KEY_DATA,TRACK_INDEX);
    UpdateLabTrack(traces[itrk+lowTrkIndex], tracks[itrk]);
}


/**********************************************************************
 *   Update Track Plots & Labels                                      *
 **********************************************************************/

static void RescaleTrace(Trace *trc, int height, int width)
{    
    /* rescale axis */
    float srate;
    ScaleYAxis(&trc->axis, height);
    srate= trc->wave->info.sample_rate;
    if (Mode_align) {
	trc->axis.hs = (float)TotalSeconds*srate/width;
    }else {
	ScaleTAxis(&trc->axis, srate, width);
    }
}

/*
 * UpdatePlotTrack updates the plot in a specific track window.
 */
void UpdatePlotTrack(int itrc, Track *trk)
{
    int k,i;
    XPoint *plot, *points;
    Trace *trc= traces[itrc];

    if (trc && trc->wave) {
	float *data= trc->wave->data;

	if (trc->axis_needScale) {
	    RescaleTrace(trc, trk->height, trk->width);
	    trc->axis_needScale=0;  /* no need now */
	}
	/* no clipping, no yoffset */
	if (Mode_decimPlot) {
	    PlotWave_decim(trc, &trc->axis, trk->width, trk->height, trk->xwin,
			   trk->gc, 0, 0, itrc);
	}else {
	    PlotWave(trc, &trc->axis, trk->width, trk->height, trk->xwin,
		     trk->gc, 0, 0, itrc);
	}
	/* update zoom track marks, if any */
	if (ZoomWindowMapped && (itrc>=lowZTrkIndex) && (itrc<=highZTrkIndex)){
	    /* now zoomed */
	    UpdateZTrkMark(itrc, trc, trk);
	}
	if (InTrkRange(itrc)) {
	    /* update picks */
	    DrawPicks(trc->picks, itrc);

	    /* update marked interval(s) */
	    UpdateInterval(itrc);
	}
    }
}


/*
 * UpdateLabTrack updates the label of a specific Track window.
 */
void UpdateLabTrack(Trace *trc, Track *trk)
{
    char sncl[20];
    char buf[100];

    XClearWindow(theDisp, trk->txtwin);
    if (trc && trc->wave) {
	if(Mode_labDetails) {
	    if(trc->wave) {
		sprintf(buf, "%d\n", trc->wave->info.n_values);
		XDrawString(theDisp, trk->txtwin, trk->txtgc, 10,
			    (trk->height-10)/2, buf, strlen(buf));
		sprintf(buf, "%g\n", trc->wave->info.sample_rate);
		XDrawString(theDisp, trk->txtwin, trk->txtgc, 10,
			    (trk->height-10)/2+15, buf, strlen(buf));
		sprintf(buf,"[%d]",trc->itrc);
		XDrawString(theDisp, trk->txtwin, trk->txtgc, 70,
			    (trk->height-10)/2+15, buf, strlen(buf));
	    }
	    return;
	}
	sprintf(sncl, "%s %s %s %s", trc->wave->info.station, 
		trc->wave->info.network, trc->wave->info.channel,
		strlen(trc->wave->info.location) > 0 ? 
		trc->wave->info.location : "--");
	if(trk->height<30) {
	    XDrawString(theDisp, trk->txtwin, trk->txtgc, 10, trk->height-2,
			sncl, strlen(sncl));
	}else {
	    XDrawString(theDisp, trk->txtwin, trk->txtgc, 10,
			(trk->height-10)/2, sncl, strlen(sncl));
	    /*	    sprintf(buf,"%3.5f\n",trc->axis.vs);
	     *	    XDrawString(theDisp, trk->txtwin, trk->txtgc, 10,
	     *		(trk->height-10)/2+15, buf, strlen(buf));      */
	}
	if (trc->selected) {
	    sprintf(buf,"%d",trc->itrc);
	    XDrawString(theDisp, trk->txtwin, trk->txtgc, 80,
			(trk->height-10)/2+15, buf, strlen(buf));
	    XFillRectangle(theDisp, trk->txtwin, trk->ovrGC, 0, 0,
			   TXTCANV_WIDTH, trk->height);
	}
    }
}

/*
 * UpdateZTrkMark updates appropriate marks showing the whereabouts of
 * the zoomed traces.
 */
void UpdateZTrkMark(int itrc, Trace *trc, Track *trk)
{
    int x1,x2;

    if (trc->wave && ZoomWindowMapped
	&& trc->zaxis.hs!=0) { /* make sure get scaled */
	x1= indexToCoord(&trc->axis, trc->zaxis.ix1, 1);
	x2= indexToCoord(&trc->axis, trc->zaxis.ix2, 1);
	if ((x2-x1) < 16) {
	    /* draw a box */
	    XDrawRectangle(theDisp, trk->xwin, trk->ovrGC,
			   x1, 4, (x2-x1),	trk->height-8);
	}else {
	    if (x1>=0)	/* draw a '[' */
		drawBracket(trk->xwin, trk->ovrGC, x1, 4,
			    trk->height-4, LEFT_BRACKET);
	    if (x2<=trk->width)	/* draw a ']' */
		drawBracket(trk->xwin, trk->ovrGC, x2, 4,
			    trk->height-4, RIGHT_BRACKET);
	}
	XFlush(theDisp);
    }
}

/**********************************************************************
 *   Change Number of displayed Tracks                                *
 **********************************************************************/

void ChangeNumTracks(int total)
{
    int i;
    
    if (total>0 && total <= TotalTracks) {
	/* scrap everything */
	DestroyTracks();

	NumTracks= total;
	lowTrkIndex= 0;
	highTrkIndex= total - 1;

	tracks= (Track **)Malloc(sizeof(Track*)*NumTracks);
	if (tracks==NULL) return;   /* not enuff mem */

	CreateTracks(tracesFrame);

	for(i=0; i <= LastTrack; i++) {
	    Trace *trc= traces[i];
	    /* resize? */
	    if (trc && trc->wave) {
		/* cp from ``trackmgr.c'' */
		ScaleYAxis(&trc->axis, tracks[0]->height);
		/* lazy-- taxis is the same */
	    }
	}
	AdjustSbar();
	RedrawScreen();
    }
}

/**********************************************************************
 *   Clipping                                                         *
 **********************************************************************/
void ClipTracksLeft(int x)
{
    int i,idx, idx2;
    Trace *trc;
    double sdiff;

    if (Mode_align) {
	/* get secs offset */
	trc=traces[lowTrkIndex];    /* don't use trace 0, might not scaled */
	sdiff= (double)(trc->axis.hs*x)/trc->wave->info.sample_rate;
	earl_time = st_add_dtime(earl_time, sdiff * USECS_PER_SEC);
	/* adjust Total Seconds */
	TotalSeconds = st_tdiff(late_time,earl_time);
	for(i=0; i <= LastTrack; i++) {
	    trc= traces[i];
	    idx = coordToIndex(&trc->axis, x, 1);
	    trc->axis.ix1=idx;
	    idx2= trc->axis.ix2;
	    if(idx2<trc->wave->info.n_values && auto_demean) {
		demean_trace(trc->wave,&trc->axis,idx,idx2);
	    }
	    RescaleTrace(trc, tracks[0]->height, tracks[0]->width);
	    trc->zaxis_needScale= 1;
	    trc->zaxis.ymax= trc->zaxis.y1= trc->axis.ymax;
	    trc->zaxis.ymin= trc->zaxis.y2= trc->axis.ymin;
#if 0
	    trc->zaxis.vs= trc->zaxis.hs= 0;
#endif
	}
    }else {
	for(i=0; i <= LastTrack; i++) {
	    trc= traces[i];
	    idx= coordToIndex(&trc->axis, x, 1);
	    trc->axis.ix1=idx;
	    RescaleTrace(trc, tracks[0]->height, tracks[0]->width);
	}
    }
    if(ZoomWindowMapped) {
	for(i=lowZTrkIndex; i<= highZTrkIndex; i++) {
	    Trace *trc=traces[i];
	    fixZtrkBounds(trc, trc->zaxis.ix2-trc->zaxis.ix1);
	}
	RedrawZoomWindow("ClipTracksLeft");
    }
    RedrawScreen();
}

/*
 * note: has to rescale everything immediately, otherwise coordToIndex
 * will give wrong answer the next time around.
 */
 
void ClipTracksRight(int x)
{
    int i,idx, idx1, n_val, len;
    Trace *trc;
    double sdiff;

    if (Mode_align) {
	/* get secs offset */
	trc=traces[lowTrkIndex];    /* don't use 0! it might not get
				       scaled if not exposed */
	len= tracks[0]->width-x;
	sdiff= (trc->axis.hs*len)/trc->wave->info.sample_rate;
	/* adjust Total Seconds */
	TotalSeconds-=sdiff;
	late_time = st_add_dtime(earl_time, TotalSeconds * USECS_PER_SEC);
	
	for(i=0; i <= LastTrack; i++) {
	    trc= traces[i];
	    n_val=trc->wave->info.n_values-1;
	    idx= coordToIndex(&trc->axis, x, 1);
	    trc->axis.ix2=idx;
	    idx1= trc->axis.ix1;
	    if (auto_demean) {
		demean_trace(trc->wave,&trc->axis,idx1,idx);
	    }
	    RescaleTrace(trc, tracks[0]->height, tracks[0]->width);
	    trc->zaxis_needScale= 1;
	    trc->zaxis.ymax= trc->zaxis.y1= trc->axis.ymax;
	    trc->zaxis.ymin= trc->zaxis.y2= trc->axis.ymin;
#if 0
	    trc->zaxis.vs= trc->zaxis.hs= 0;
#endif
	}
    }else {
	for(i=0; i <= LastTrack; i++) {
	    trc= traces[i];
	    n_val=trc->wave->info.n_values-1;
	    idx= coordToIndex(&trc->axis, x, 1);
	    trc->axis.ix2= idx;
	    RescaleTrace(trc, tracks[0]->height, tracks[0]->width);
	    trc->zaxis_needScale= 1;
	}
    }
    if(ZoomWindowMapped) {
	for(i=lowZTrkIndex; i<= highZTrkIndex; i++) {
	    Trace *trc=traces[i];
	    fixZtrkBounds(trc, trc->zaxis.ix2-trc->zaxis.ix1);
	}
	RedrawZoomWindow("ClipTracksRight");
    }
    RedrawScreen();
}

/* determine to clip to left or to right */
void ClipTracks(int x)
{
    if  (x>=tracks[0]->width/2) {
	ClipTracksRight(x);
    }else {
	ClipTracksLeft(x);
    }
}

void FullScale()
{
    int i;
    Trace *trc;

    if (Mode_align) {
	if (Mode_align==1) {
	    UnifyTimeScale();
	} else if (Mode_align==2) {
	    timescale_noalign();
	}
	for(i=0; i <=LastTrack; i++) {
	    trc= traces[i];
	    if (auto_demean) {
		demean_trace(trc->wave,&trc->axis,0,trc->wave->info.n_values-1);
	    }
	    RescaleTrace(trc, tracks[0]->height, tracks[0]->width);
	    trc->zaxis_needScale= 1;
	    trc->zaxis.ymax= trc->zaxis.y1= trc->axis.ymax;
	    trc->zaxis.ymin= trc->zaxis.y2= trc->axis.ymin;
	}
    }else {
	float sps;
	for(i=0; i <=LastTrack; i++) {
	    trc= traces[i];
	    trc->axis.ix1=0;
	    trc->axis.ix2=trc->wave->info.n_values-1;
	    sps= trc->wave->info.sample_rate;
	    ScaleTAxis(&trc->axis, sps, tracks[0]->width);
	}
    }
    /* Should this redraw be controlled? */
    RedrawScreen();
    if(ZoomWindowMapped)
	RedrawZoomWindow("FullScale");
}

void PermanentClip()
{
    int i;
    Trace *trc;
    float *data, *newd;
    int nsamp;
    int j, k, ix1, ix2, pidx;
    STI_TIME ntm;
    double trimmed;
    BIS3_HEADER *bh;
    Pick *p, *t, *newp;
    Amp *pa, *ta, *newa;
    
    for(i=0; i<= LastTrack; i++) {
	trc=traces[i];
	if(trc->wave) {
	    bh = &trc->wave->info;
	    
	    ix1= trc->axis.ix1;
	    ix2= trc->axis.ix2;
	    if(ix1>0) {
		trc->axis.ix1-= ix1;
		trc->axis.ix2-= ix1;
	    }

	    if (!fixIxBounds(&ix1,&ix2,trc->wave->info.n_values)) {
		/* can discard this one */
		cleanTrace(trc);
		continue;
	    }

	    /* ix1, ix2 fixed */
	    nsamp= ix2-ix1+1;  
	    newd= (float *)Malloc(sizeof(float)*nsamp);
	    data= trc->wave->data;
	    trimmed = ((double)ix1)/bh->sample_rate;
	    ntm = st_add_dtime(bh->start_it, trimmed * USECS_PER_SEC);
	    for(j=0,k= ix1; j< nsamp; j++, k++) {
		newd[j]= data[k];
	    }
	    trc->wave->data= newd;
	    free(data);
	    bh->n_values= nsamp;

	    trc->zaxis_needScale= 1;

	    /* adjust picks */
	    newp=p= NULL;
	    t= trc->picks;
	    while(t) {
		/* discard picks outside the retained region */
		/* note that bh->start_it must NOT be changed yet */
		pidx = pick_index(t, trc, 1);
		if(pidx >= ix1 && pidx <= ix2) {
		    t->secOffset -= trimmed ;
		    /* add to the list to retain */
		    if (p==NULL) {
			newp=p= t;
		    }else {
			p->next= t;
			p= p->next;
		    }
		}
		t= t->next;
	    }
	    if(p!=NULL) p->next= NULL;
	    trc->picks= newp;
	    bh->start_it = ntm;

	    /* do similar for regions */
	    {
		Reg_select *cur_reg=trc->sel_reg;
		Reg_select *pre_reg=NULL;
		int remove=0;

		while (cur_reg != NULL) {
		    if (cur_reg->right_index<ix1 || cur_reg->left_index>ix2) remove=1;
		    /* adjust region to be in the new region */
		    cur_reg->right_index -= ix1;
		    cur_reg->left_index -= ix1;

		    /* ie adjust clipped off edges */
		    if (cur_reg->left_index<0) cur_reg->left_index=0;
		    if (cur_reg->right_index>=nsamp) cur_reg->right_index=nsamp;

		    pre_reg=cur_reg;
		    cur_reg=cur_reg->next;

		    if (remove) RemoveSRegion(pre_reg,trc);
		    remove=0;
		}
	    }
	    if (ix1<0) ix1= -ix1;   /* note we flip ix1 if < 0 */
	    trc->zaxis.ix1-= ix1;
#if 0
	    if(trc->zaxis.ix1<0) trc->zaxis.ix1=0;
#endif
	    trc->zaxis.ix2-= ix1;
#if 0
	    if(trc->zaxis.ix2>=nsamp) trc->zaxis.ix1=nsamp-1;
#endif

	    /* what about triplets?? -- sovrlap, etc. */
	    /* what about amplitdes??? */
	}
    }
    RedrawScreen();
    if(ZoomWindowMapped)
	RedrawZoomWindow("permanentClip");
}

/*******************************************************
 *     absolute vertical scaling                       *
 *******************************************************/

void newAbsVertScale()
{
    int i;
    Axis *ax;
    float y1, y2;

    ax= &traces[0]->zaxis;
    y1= ax->y1, y2= ax->y2;
    for(i=1; i<=LastTrack; i++) {
	ax= &traces[i]->zaxis;
	if (ax->y1>y1) {
	    y1= ax->y1;
	}
	if (ax->y2<y2) {
	    y2= ax->y2;
	}
    }

    /* store the info in abs_Axis                     */
    /* yeah I know I should not us ix1 for the height */
    abs_Axis.vs=(float)(y1-y2)/tracks[0]->height;
    abs_Axis.ymax=abs_Axis.y1=y1;
    abs_Axis.ymin=abs_Axis.y2=y2;
    abs_Axis.ix1=tracks[0]->height;
    abs_Axis.y0=y1/tracks[0]->height;
}
