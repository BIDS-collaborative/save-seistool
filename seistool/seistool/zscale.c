#ifndef lint
static char id[] = "$Id: zscale.c,v 1.3 2013/02/28 21:24:55 lombard Exp $";
#endif

/*
 * zscale.c--
 *    implements the time scale in Zoom Window
 *
 * Copyright (c) 1992-3 Andrew K. Yu, Lawrence Berkeley Laboratory
 *      and University of California, Berkeley.
 * All rights reserved.
 */
#include <math.h>
#include <strings.h>
#include <sunmath.h>  /* for exp10 */
#include <xview/canvas.h>
#include <xview/panel.h>
#include <xview/xv_xrect.h>
#include "proto.h"
#include "xv_proto.h"

/**********************************************************************
 *   Globals                                                          *
 **********************************************************************/

extern Trace **traces;

int	Mode_ZDisplayTScale = 1;	/* display time scale in ZWin */
Canvas	ztscl_top_canvas;
Canvas	ztscl_bot_canvas;
Window	ztscl_top_win;
Window	ztscl_bot_win;
GC	ztscl_gc;

#define WHICH_TS	    13
#define TOP_TS		    1
#define BOTTOM_TS	    2

/*
 * Precomputed parameters for fast time scale redraw
 */
static float ZTotalSeconds=0.0;	/* total time of zoom window */
static int ts_xstart;		/* starting position of time scale */
static STI_TIME  ts_tstart;	/* starting time of the scale */
static float time_inc;		/* division of time scale */

/* internal function prototypes */
static void RedrawZTScale(Canvas canvas, Xv_Window paint_window, 
			  Rectlist *repaint_area);


/**********************************************************************
 *   Initialization                                                   *
 **********************************************************************/

int initZTScale(Frame frame)
{
    ztscl_top_canvas= (Canvas)xv_create(frame, CANVAS,
					XV_X, 0, XV_Y, 0,
					XV_WIDTH, ZCvsWidth+ZTrkLabWidth+4, XV_HEIGHT, ZTSHEIGHT+1,
					XV_KEY_DATA, WHICH_TS, TOP_TS,
					CANVAS_REPAINT_PROC, RedrawZTScale,
					NULL);
    ztscl_bot_canvas= (Canvas)xv_create(frame, CANVAS,
					XV_X, 0, XV_Y, ZCvsHeight+ZTSHEIGHT-1,
					XV_WIDTH, ZCvsWidth+ZTrkLabWidth+4, XV_HEIGHT, ZTSHEIGHT+1,
					XV_KEY_DATA, WHICH_TS, BOTTOM_TS,
					CANVAS_REPAINT_PROC, RedrawZTScale,
					NULL);
    ztscl_top_win= (Window)xv_get(canvas_paint_window(ztscl_top_canvas),
				  XV_XID);
    ztscl_bot_win= (Window)xv_get(canvas_paint_window(ztscl_bot_canvas),
				  XV_XID);
    createGC(ztscl_top_win, &ztscl_gc);
    if(!Mode_ZDisplayTScale || !Mode_align) {
	xv_set(ztscl_top_canvas, XV_SHOW, FALSE, NULL);
	xv_set(ztscl_bot_canvas, XV_SHOW, FALSE, NULL);
    }
}

/**********************************************************************
 *   Redraws & Resizes                                                *
 **********************************************************************/

void RedrawBothZTScale()
{
    int *temp;
    if (Mode_align && Mode_ZDisplayTScale) {
	XClearWindow(theDisp, ztscl_top_win);
	DrawTimeScale(ztscl_top_win, ztscl_gc, ts_xstart+ZTrkLabWidth,
		      ZTrkLabWidth, ts_tstart, time_inc, ZCvsWidth,
		      ZTSHEIGHT, ZTotalSeconds, TS_UP);
	XClearWindow(theDisp, ztscl_bot_win);
	DrawTimeScale(ztscl_bot_win, ztscl_gc, ts_xstart+ZTrkLabWidth,
		      ZTrkLabWidth, ts_tstart, time_inc, ZCvsWidth,
		      ZTSHEIGHT, ZTotalSeconds, TS_DN);
	xv_set(ztscl_top_canvas,XV_SHOW,TRUE,NULL);
	xv_set(ztscl_bot_canvas,XV_SHOW,TRUE,NULL);
    } else {
	xv_set(ztscl_top_canvas,XV_SHOW,FALSE,NULL);
	xv_set(ztscl_bot_canvas,XV_SHOW,FALSE,NULL);
    }
}

static void RedrawZTScale(Canvas canvas, Xv_Window paint_window, 
			  Rectlist *repaint_area)
{
    Window win;
    int which= (int)xv_get(canvas, XV_KEY_DATA, WHICH_TS);

    if (which==TOP_TS) {
	win= ztscl_top_win;
    }else {
	win= ztscl_bot_win;
    }
    XClearWindow(theDisp, win);
    DrawTimeScale(win, ztscl_gc, ts_xstart+ZTrkLabWidth, ZTrkLabWidth,
		  ts_tstart, time_inc, ZCvsWidth, ZTSHEIGHT, ZTotalSeconds,
		  (which==TOP_TS)? TS_UP:TS_DN);
}

void ResizeZTScale()
{
    xv_set(ztscl_top_canvas, XV_X, 0, XV_Y, 0,
	   XV_WIDTH, ZCvsWidth+ZTrkLabWidth,
	   NULL);
    xv_set(ztscl_bot_canvas, XV_X, 0, XV_Y, ZCvsHeight+ZTSHEIGHT-1,
	   XV_WIDTH, ZCvsWidth+ZTrkLabWidth,
	   NULL);
    return;
}


void ZTimeScale_disp_undisp()
{
    if (ZoomWindowMapped) {
	if(Mode_ZDisplayTScale) {
	    xv_set(ztscl_top_canvas, XV_SHOW, TRUE, NULL);
	    xv_set(ztscl_bot_canvas, XV_SHOW, TRUE, NULL);
	}else {
	    xv_set(ztscl_top_canvas, XV_SHOW, FALSE, NULL);
	    xv_set(ztscl_bot_canvas, XV_SHOW, FALSE, NULL);
	}
    }
    ResizeZoomWindow();
}

void ToggleZTScale()
{
    Mode_ZDisplayTScale= !Mode_ZDisplayTScale;
    ZTimeScale_disp_undisp ();
}

/**********************************************************************
 *                                                                    *
 **********************************************************************/

/* should check totSecs>0 before calling this */
void CalcTimeScaleParam(STI_TIME earlTime, double totSecs, int div,
			float *tincp, STI_TIME *nearTime)
{
    STE_TIME et;
    float tinc;

#define TWO_SECS	  2.0
#define TWO_MINS	120.0
#define TWO_HOURS	7200.0
#define TWO_DAYS        172800.0
    et = sti_to_ste(earlTime);
    if (totSecs < TWO_SECS) {
	int pw= (int)log10(totSecs/div);
	float trunc= exp10((double)(2-pw));
	tinc= (float)((int)(totSecs/div*trunc))/trunc;
    }else if (totSecs < TWO_MINS) {    /* step in seconds */
	tinc= 1.0;
	if (et.usec > 0.0) {
	    et.usec = 0;    /* find nearest sec */
	    et = sti_to_ste(st_add_dtime(ste_to_sti(et), 
					 (double)tinc * USECS_PER_SEC));
	}
    }else if(totSecs < TWO_HOURS) {   /* step in minutes */
	tinc = 60.0;
	if(et.usec > 0 || et.second > 0) {
	    et.second = 0;
	    et.usec = 0.0;  /* find nearest minutes */
	    et.minute ++;
	}
    }else if (totSecs < TWO_DAYS) {  /* step in hours */
	tinc = 3600.0;
	if(et.minute > 0 || et.second > 0 || et.usec > 0) {
	    et.minute = et.second = 0;
	    et.usec = 0;  /* find nearest hours */
	    et.hour++;
	}
    } else {   /* step in whole days so there are <50 increments */
	tinc = 86400;
	if(et.minute > 0 || et.second > 0 || et.usec > 0 || et.hour > 0) {
	    et.hour = et.minute = et.second = et.usec = 0;
	    et.doy++;
	}

    }
    *tincp= tinc;
    *nearTime = ste_to_sti(et);
    return;
}
			       
void TS_ZoomContentChanged(int itrc)
{
    STI_TIME earl_time, nr_time;

    if (traces[itrc] && traces[itrc]->wave) {
	Trace *trc= traces[itrc];
	
	/* use the first trace for the time scale */
	ZTotalSeconds= (float)(trc->zaxis.ix2 - trc->zaxis.ix1)/
	    trc->wave->info.sample_rate;

	if (Mode_align==2) {
	    double sec_off = (double)trc->zaxis.ix1/trc->wave->info.sample_rate;
	    bzero(&earl_time, sizeof(STI_TIME));
	    earl_time = st_add_dtime(earl_time, sec_off * USECS_PER_SEC);
	  
	} else {
	    earl_time= indexToTime(trc, trc->zaxis.ix1,1);
	}

	if (ZTotalSeconds>0) {
	    CalcTimeScaleParam(earl_time, ZTotalSeconds, 5,
			       &time_inc, &nr_time);
	    ts_xstart= st_tdiff(nr_time, earl_time) * ZCvsWidth
		/ ZTotalSeconds;
	    ts_tstart= nr_time;
	}
	/* if ZTotalSeconds is 0, just leave ts_tstart where it was */
    }
}

void DrawTimeScale(Window win, GC gc, int xstart, int xoff, STI_TIME tstart,
		   float tinc, int width, int height, float totSecs, int dir)
{
    float xinc;
    int x, skip, ninterval, nskip;
    char label[10];
    STI_TIME tm;
    STE_TIME et;
    int y_base, y_small, y_large, y_label;

    if (totSecs<=0)
	return;
    if ((x=xstart) < 0)
	return; /* not possible unless something screws up */
	
    tm= tstart;
    if (dir==TS_UP) {
	y_base=height, y_small=height-6,
	    y_large=height-12, y_label= height-10;
    }else {
	y_base=0, y_small=6, y_large=12, y_label= 20;
    }
    if (tinc<1.0) {
	strcpy(label, "sec.");
    }else if(tinc==1.0) {
	strcpy(label, "mm:ss");
    }else if(tinc==60.0) {
	strcpy(label, "hh:mm");
    }else if (tinc==3600) {
	strcpy(label, "hours");
    }else {
	strcpy(label, "days");
    }
    XDrawString(theDisp, win, gc, xoff-50, y_label, label, strlen(label));
    xinc= ((float)tinc*width)/totSecs;
    /*printf("xinc: %f  tinc %f width %d totsecs %f\n",
      xinc, tinc, width, totSecs);*/
    ninterval= (tinc>=1.0)? 6 : 10;
    skip= (xinc<50.)? ninterval : 1;
    if ((x-xoff) > xinc*skip/ninterval) {
	int i, xx;
	/* patch small marks backwards */
	for(i=1; i < ninterval; i++) {
	    xx= x - i*xinc*skip/ninterval;
	    if(xx > xoff) {
		XDrawLine(theDisp, win, gc,
			  xx, y_base, xx, y_small);
	    }else {
		break;
	    }
	}
    }

    nskip= 0;
    while((x= xstart+xinc*nskip) < width+xoff) {
	int i, xx;
	    
	/* draw the mark */
	XDrawLine(theDisp, win, gc,
		  x, y_base, x, y_large);
	/* smaller mark if possible */
	for(i=1; i < ninterval; i++) {
	    xx= x + xinc*i*skip/ninterval;
	    XDrawLine(theDisp, win, gc,
		      xx, y_base, xx, y_small);
	}
	et = sti_to_ste(tm);
	if(tinc==1.0) {
	    sprintf(label,"%d:%02d", et.minute, et.second);
	}else if(tinc==60.0) {
	    sprintf(label,"%d:%02d", et.hour, et.minute);
	}else if(tinc==3600.0) {
	    sprintf(label, "%02d", et.hour);
	}else if(tinc<1.0) {
	    sprintf(label,"%g", (float)et.usec/USECS_PER_SEC);
	}else {
	    sprintf(label,"%03d",et.doy);
	}
	XDrawString(theDisp, win, gc,
		    x, y_label, label, strlen(label));
	tm = st_add_dtime(tm, (double)tinc*skip * USECS_PER_SEC);
	/*	x+=xinc*skip;*/
	nskip+= skip;
    }
    XFlush(theDisp);
    return;
}
