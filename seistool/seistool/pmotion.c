#ifndef lint
static char id[] = "$Id: pmotion.c,v 1.2 2013/02/28 21:24:58 lombard Exp $";
#endif

/*
 * pmotion--
 *    particle motion
 *
 * Copyright (c) 1992-3 Andrew K. Yu, Lawrence Berkeley Laboratory
 *      and University of California, Berkeley.
 * All rights reserved.
 */
#include <math.h>
#include <stdio.h>
#include <strings.h>
#include <xview/xview.h>
#include <xview/frame.h>
#include <xview/canvas.h>
#include <xview/panel.h>

#include "proto.h"
#include "xv_proto.h"

/**********************************************************************
 *   Particle Motion window                                           *
 **********************************************************************/

/* globals */
static Frame	pm_frame= NULL;
static Canvas	pm_cvs= NULL;
static Window	pm_win;
static GC	pm_gc;

static int pmwin_h= 600;
static int pmwin_w= 600;

static int pm_win_mapped= 0;

/* prototypes */
static void changeZX();
static void changeZY();
static void changeXY();
static void win_max_min(float *max, float *min, Trace *c_trace,
			STI_TIME s_start, STI_TIME s_end);
static void next_motion();
static void InitPmotionFrame();
static void open_pmotion_win(int itrc);
static void close_pmotion_win();
static void redrawPmotion();
static void DrawHortString(Window win, GC gc, int x, int y, char *str);
static void PlotXAxis(float xmax, float *xmin, float *hs, Trace *trcH);
static void PlotYAxis(float ymax, float *ymin, float *vs, Trace *trcV);
static void DrawPmotion();


static int cur_itrc= -1;
static int combo= 0;
static Reg_select *cur_reg = NULL;

extern Trace **traces;
extern int LastTrack;


/**********************************************************************
 *   set up pmotion window                                            *
 **********************************************************************/

static void changeZX()
{
    combo= 0;
    redrawPmotion();
}
static void changeZY()
{
    combo= 1;
    redrawPmotion();
}
static void changeXY()
{
    combo= 2;
    redrawPmotion();
}

static void win_max_min(float *max, float *min, Trace *c_trace,
			STI_TIME s_start, STI_TIME s_end)
{

    int i_start,i_end;
    int i;
    float c_max=-99999999,c_min=99999999;
    float c_data;
    /* 1. find the indexes for the start and end of the window */
  
    i_start=timeToIndex(c_trace,s_start);
    i_end=timeToIndex(c_trace,s_end);

    for(i=i_start;i<=i_end;i++) {
	c_data=c_trace->wave->data[i];
	if (c_data>c_max) {
	    c_max=c_data;
	}
	if (c_data<c_min) {
	    c_min=c_data;
	}
    }

    *max=c_max;
    *min=c_min;
    return;
}

static void next_motion()
{ 
    int i, itrc;
    Reg_select *curr;

    if (cur_reg !=NULL) {
	curr=cur_reg->next;
	/* next region part of same trace */

	if (curr!=NULL) {
	    cur_reg=curr;
	    itrc=cur_itrc;
	} else {
    
	    itrc=-1;
	    /* forward */
	    for(i=cur_itrc+1; i <= LastTrack; i++) {
		curr=traces[i]->sel_reg;
		if (curr!=NULL) {
		    itrc=i;
		    cur_reg=curr;
		    break;
		}      
	    }
	    if (itrc==-1) {		/* wrap around */
		for(i=0; i<=cur_itrc; i++ ) {
		    curr=traces[i]->sel_reg;
		    if (curr!=NULL) {
			itrc=i;
			cur_reg=curr;
			break;
		    }
		}
	    }
	}
    } else {
	itrc=cur_itrc+1;
	if (itrc>LastTrack) itrc=0;
    }

    if (itrc==-1) {		/* no more to plot -- resigns */
	close_spctrm_win();
    }else {
	cur_itrc= itrc;
	redrawPmotion();
    }
}

static void InitPmotionFrame()
{
    Panel panel;
    Menu menu;
    
    pm_frame= xv_create(tracesFrame, FRAME,
			XV_HEIGHT, pmwin_h+40, XV_WIDTH, pmwin_w,
			NULL);		  
    pm_cvs= xv_create(pm_frame, CANVAS,
		      XV_X, 0, XV_Y, 40,		  
		      CANVAS_REPAINT_PROC, redrawPmotion,
		      NULL);		  
    pm_win= xv_get(canvas_paint_window(pm_cvs), XV_XID);
    createGC(pm_win, &pm_gc);
    panel= (Panel)xv_create(pm_frame, PANEL,
			    XV_X, 0, XV_Y, 0, XV_HEIGHT, 40,
			    NULL);
    (void)xv_create(panel, PANEL_BUTTON,
		    PANEL_LABEL_STRING, "Quit",
		    PANEL_NOTIFY_PROC, close_pmotion_win,
		    NULL);
    (void)xv_create(panel, PANEL_BUTTON,
		    PANEL_LABEL_STRING, "redo",
		    PANEL_NOTIFY_PROC, handle_particle_motion,
		    NULL);
    (void)xv_create(panel, PANEL_BUTTON,
		    PANEL_LABEL_STRING, "next selection",
		    PANEL_NOTIFY_PROC, next_motion,
		    NULL);
    (void)xv_create(panel, PANEL_BUTTON,
		    PANEL_LABEL_STRING, "Z-X",
		    PANEL_NOTIFY_PROC, changeZX,
		    NULL);
    (void)xv_create(panel, PANEL_BUTTON,
		    PANEL_LABEL_STRING, "Z-Y",
		    PANEL_NOTIFY_PROC, changeZY,
		    NULL);
    (void)xv_create(panel, PANEL_BUTTON,
		    PANEL_LABEL_STRING, "X-Y",
		    PANEL_NOTIFY_PROC, changeXY,
		    NULL);
}

static void open_pmotion_win(int itrc)
{
    char buf[100], *t;
    BIS3_HEADER *bh= &traces[itrc]->wave->info;

    if(!pm_frame) InitPmotionFrame();
    t=(char *)rindex(traces[itrc]->filename,'/');
    t= (t!=NULL)? t+1:traces[itrc]->filename;
    sprintf(buf,"[%d] %s %s %s %s Particle Motion (%s)", itrc, 
	    bh->station, bh->network, bh->channel, 
	    (strlen(bh->location) > 0) ? bh->location : "--", t);
    xv_set(pm_frame, XV_SHOW, TRUE,
	   FRAME_LABEL, buf,
	   NULL);
    if (Mode_triplet== 0 ) {
	StartTripletMode();
	EndTripletMode();    /* just want to group them */
    }
}

static void close_pmotion_win()
{
    xv_set(pm_frame, XV_SHOW, FALSE, NULL);
}
    
/**********************************************************************
 *   Plotting the Particle Motion                                     *
 **********************************************************************/

static void redrawPmotion()
{
    if(pm_win_mapped) {
	XClearWindow(theDisp, pm_win);
	DrawPmotion();
    }
    pm_win_mapped= 1;	/* kludge-- get around the first redraw */
}

#define PMWIN_X_OFF 50
#define PMWIN_Y_OFF 50
#define MARGIN	10

static void DrawHortString(Window win, GC gc, int x, int y, char *str)
{
    int i;
    for(i=0; i<strlen(str); i++) {
	XDrawString(theDisp, win, gc, x, y, str+i, 1);
	x+= 20;
    }
}

static void PlotXAxis(float xmax, float *xmin, float *hs, Trace *trcH)
{
    char buf[30], sncl[20];
    int i;
    double n_min,n_max,step;

    /* axis */
    scale_line_axis(xmax, *xmin, &step, &n_min, &n_max); 

    *hs=(float)(pmwin_h-PMWIN_Y_OFF-MARGIN)/(n_max-n_min);
    *xmin=n_min;

    XDrawLine(theDisp, pm_win, pm_gc, PMWIN_X_OFF, pmwin_h-PMWIN_Y_OFF+10, 
	      pmwin_w-MARGIN, pmwin_h-PMWIN_Y_OFF+10);
    XDrawLine(theDisp, pm_win, pm_gc, PMWIN_X_OFF, pmwin_h-PMWIN_Y_OFF+10, 
	      PMWIN_X_OFF, pmwin_h-PMWIN_Y_OFF-5+10); 
    XDrawLine(theDisp, pm_win, pm_gc,
	      pmwin_w-MARGIN, pmwin_h-PMWIN_Y_OFF+10,
	      pmwin_w-MARGIN, pmwin_h-PMWIN_Y_OFF-5+10);

    /* labels */
    sprintf(sncl, "%s %s %s %s", trcH->wave->info.station, 
	    trcH->wave->info.network, trcH->wave->info.channel,
	    (strlen(trcH->wave->info.location) > 0) ? 
	    trcH->wave->info.location : "--");
    /* May need to adjust string location */
    DrawHortString(pm_win,pm_gc,250,
		   pmwin_h-PMWIN_Y_OFF+30, sncl);
    sprintf(buf, "%g", n_min);
    XDrawString(theDisp, pm_win, pm_gc, PMWIN_X_OFF,
		pmwin_h-PMWIN_Y_OFF+30, buf, strlen(buf));
    sprintf(buf, "%g", n_max);
    XDrawString(theDisp, pm_win, pm_gc, pmwin_w-MARGIN-40,
		pmwin_h-PMWIN_Y_OFF+30, buf, strlen(buf));
}

static void PlotYAxis(float ymax, float *ymin, float *vs, Trace *trcV)
{
    char buf[30], sncl[20];
    double step,n_min,n_max;

    scale_line_axis(ymax, *ymin,&step,&n_min,&n_max); 
    *vs=(float)(pmwin_h-PMWIN_Y_OFF-MARGIN)/(n_max-n_min);
    *ymin=n_min;

    XDrawLine(theDisp, pm_win, pm_gc, PMWIN_X_OFF-10, MARGIN,
	      PMWIN_X_OFF-10, pmwin_h-PMWIN_Y_OFF);
    XDrawLine(theDisp, pm_win, pm_gc, PMWIN_X_OFF-10, MARGIN,
	      PMWIN_X_OFF+5-10, MARGIN);
    XDrawLine(theDisp, pm_win, pm_gc, PMWIN_X_OFF-10, pmwin_h-PMWIN_Y_OFF,
	      PMWIN_X_OFF+5-10, pmwin_h-PMWIN_Y_OFF);

    /* label */
    sprintf(sncl, "%s %s %s %s", trcV->wave->info.station, 
	    trcV->wave->info.network, trcV->wave->info.channel,
	    (strlen(trcV->wave->info.location) > 0) ? 
	    trcV->wave->info.location : "--");
    /* May need to adjust string location */
    DrawVertString(pm_win, pm_gc, 10, 200, sncl);

    sprintf(buf, "%g", n_max);
    XDrawString(theDisp, pm_win, pm_gc, PMWIN_X_OFF/2-10, MARGIN+5,
		buf, strlen(buf));
    sprintf(buf, "%g", n_min);
    XDrawString(theDisp, pm_win, pm_gc, PMWIN_X_OFF/2-10,
		pmwin_h-PMWIN_Y_OFF-5, buf, strlen(buf));
}

static void DrawPmotion()
{
    XPoint *points, *plot;
    Trace *trc=traces[cur_itrc];
    Trace *trcV, *trcH;
    Triplet *trip= trc->trip;
    float ymax, ymin, xmax, xmin;
    float vs, hs;
    float *dataV, *dataH;
    int s_idxV, s_idxH, len;
    int i;

    switch(combo) {
    case 0: /* Z-X */
	if(! (trcV = trip->trc[TRC_Z]) || ! (trcH = trip->trc[TRC_X]))
	    return;
	break;
    case 1: /* Z-Y */
	if(! (trcV = trip->trc[TRC_Z]) || ! (trcH = trip->trc[TRC_Y]))
	    return;
	break;
    case 2: /* X-Y */
	if(! (trcV = trip->trc[TRC_Y]) || ! (trcH = trip->trc[TRC_X]))
	    return;
	break;
    }

    if (cur_reg!=NULL) {
	STI_TIME s_time,e_time;
	Trace *sel_trace=SReg_in_trace(cur_reg,trcV,trcH);
	sel_trace=trc;

	if (sel_trace==NULL) {
	    fprintf(stderr,"problem finding region\n");
	}

	s_time=indexToTime(sel_trace,cur_reg->left_index,1);
	e_time=indexToTime(sel_trace,cur_reg->right_index,1);

	win_max_min(&ymax,&ymin,trcV,s_time,e_time);
	win_max_min(&xmax,&xmin,trcH,s_time,e_time);

	s_idxV=timeToIndex(trcV, s_time);
	s_idxH=timeToIndex(trcH, s_time);
	len=cur_reg->right_index-cur_reg->left_index; 

    } else {
	ymax= trcV->axis.ymax;
	ymin= trcV->axis.ymin;
	xmax= trcH->axis.ymax;
	xmin= trcH->axis.ymin;

	s_idxV= timeToIndex(trcV, trip->sovrlap);
	s_idxH= timeToIndex(trcH, trip->sovrlap);
	len= timeToIndex(trcV, trip->eovrlap)-s_idxV+1;
    }

    /* compute scale */
    vs= (float)(pmwin_h-PMWIN_Y_OFF-MARGIN)/(ymax-ymin);
    hs= (float)(pmwin_w-PMWIN_X_OFF-MARGIN)/(xmax-xmin);

    dataV= trcV->wave->data;
    dataH= trcH->wave->data;


    /* plot & label Y-axis */
    PlotYAxis(ymax, &ymin, &vs, trcV);

    /* plot X-axis */
    PlotXAxis(xmax, &xmin, &hs, trcH);


    plot= points= (XPoint *)Malloc(sizeof(XPoint)*2000);
    for(i=0; i < len; i+= 2000) {
	int npts= ((len-i)>2000)? 2000:len-i;
	int j;
	points= plot;
	for(j=0; j < npts; j++) {
	    points->x= (dataH[s_idxH+i+j]-xmin)*hs + PMWIN_X_OFF;
	    points->y= pmwin_h-PMWIN_Y_OFF-(dataV[s_idxV+i+j]-ymin)*vs;
	    points++;
	}
	XDrawLines(theDisp, pm_win, pm_gc, plot, npts, CoordModeOrigin);
    }

    XFlush(theDisp);
    free(plot);
}

/**********************************************************************
 *   handle events for Pmotion operations                             *
 **********************************************************************/

void handle_particle_motion()
{
    int draw,i,found=0;

    /* find first selection region */    
    for(i=0; i <= LastTrack; i++) {
	Trace *trc= traces[i];
	if (trc->sel_reg!=NULL) {
	    cur_itrc=i;
	    cur_reg=trc->sel_reg;
	    found=1;
	    break;
	}
    }

    /* find selected trace */
    if (found==0) {
	cur_itrc = firstSelectedTrc();
	if(cur_itrc<0)cur_itrc=0;
    }

    open_pmotion_win(cur_itrc);
    redrawPmotion();
}


