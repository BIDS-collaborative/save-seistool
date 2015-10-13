#ifndef lint
static char id[] = "$Id: scale.c,v 1.3 2013/02/28 21:24:58 lombard Exp $";
#endif

/*
 * scale.c--
 *    implements the time scale
 *
 * Copyright (c) 1992-3 Andrew K. Yu, Lawrence Berkeley Laboratory
 *      and University of California, Berkeley.
 * All rights reserved.
 */
#include <math.h>
#include <stdio.h>
#include <xview/canvas.h>
#include <xview/panel.h>
#include <xview/xv_xrect.h>
#include "xv_proto.h"

int TScaleHeight=0;
Canvas tscl_canvas;
Window tscl_win;
GC     tscl_gc;


void CreateTimeScale(Frame frame, int y, int height)
{
    TScaleHeight= height;
/*    printf("creating time scale\n"); */
    tscl_canvas= (Canvas)xv_create(frame, CANVAS,
	   XV_X, 0, XV_Y, y,
	   XV_WIDTH, FrmWidth-19, XV_HEIGHT, height,
	   CANVAS_REPAINT_PROC, RedrawTScale,
	   NULL);
    tscl_win= (Window)xv_get(canvas_paint_window(tscl_canvas), XV_XID);
    createGC(tscl_win, &tscl_gc);
}

void DestroyTimeScale()
{
    xv_destroy_safe(tscl_canvas);
}

extern STI_TIME earl_time, late_time;
extern double TotalSeconds;

void RedrawTScale()
{
    STI_TIME tm;
    int xinc, x, skip, ninterval;
    float tinc;
    
    XClearWindow(theDisp, tscl_win);
    if (fabs(TotalSeconds) > 0.1) {
      /* calculate tinc mostly and tm filled with truncated earl_time */
	CalcTimeScaleParam(earl_time, TotalSeconds, 10, &tinc, &tm);


	x = st_tdiff(tm, earl_time) * (FrmWidth-TXTCANV_WIDTH-19)
	    / TotalSeconds + TXTCANV_WIDTH;

	DrawTimeScale(tscl_win, tscl_gc, x, TXTCANV_WIDTH, tm, tinc,
		      FrmWidth-TXTCANV_WIDTH-19, 0 /* dummy */, 
		      TotalSeconds, TS_DN);
    }
}

void ResizeTScale(int y, int width, int height)
{
    xv_set(tscl_canvas,
	   XV_X, 0, XV_Y, y,
	   XV_WIDTH, width, XV_HEIGHT, height,
	   NULL);
}
