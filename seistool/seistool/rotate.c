#ifndef lint
static char id[] = "$Id: rotate.c,v 1.2 2013/02/28 21:24:58 lombard Exp $";
#endif

/*
 * rotate.c--
 *    handles rotations
 *
 * Copyright (c) 1992-3 Andrew K. Yu, Lawrence Berkeley Laboratory
 *      and University of California, Berkeley.
 * All rights reserved.
 */
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <xview/xview.h>
#include <xview/canvas.h>

#include "proto.h"
#include "xv_proto.h"

static float getRotValue(int which, int idx, Trace *trcZ, Trace *trcN,
			 Trace *trcE, float angle);

/*
 * this module is not quite working right now. Well, at least not for
 * rotating traces that doesn't start at the same time.
 */

int Mode_rotate;

#define DEG2RAD(d)  ((d)*3.14159265/180)

static int rt_cur_active= 1; /* controls on rt_panel currently active */
extern Trace **traces;
extern int lowZTrkIndex;

/* set triplet angle */
void rt_setAngle(float ang)
{
    Triplet *trip= traces[lowZTrkIndex]->trip;
    trip->rot_theta = ang;
    trip->rot_unc = DEF_UNCERTAINTY;
    
    return;
}

/* set flag rotated */
void rt_setflag (int val)
{
    Triplet *trip=traces[lowZTrkIndex]->trip;
    trip->rotated=val;
}

static float getRotValue(int which, int idx, Trace *trcZ, Trace *trcN,
			 Trace *trcE, float angle)
{
    float t;
    int idx_n, idx_e;
    STI_TIME tm;

    switch (which) {
    case TRC_Z:
	t = trcZ->wave->data[idx];
	break;
    case TRC_Y:   /* N */
	tm = indexToTime(trcN,idx,1);
	idx_e = timeToIndex(trcE, tm);
	t = trcN->wave->data[idx]*cos(DEG2RAD(angle))
	    + trcE->wave->data[idx_e]*sin(DEG2RAD(angle));
	break;
    case TRC_X: /* E */
	tm = indexToTime(trcE,idx,1);
	idx_n = timeToIndex(trcN, tm);
	t = - trcN->wave->data[idx_n]*sin(DEG2RAD(angle))
	    + trcE->wave->data[idx]*cos(DEG2RAD(angle));
	break;
    default:
	t = F4FLAG;
    }
    return t;
}

void rt_PlotWave(Trace *trc, Axis *axis, int width, int height, Window xwin,
		 GC gc, int toClip, int yoffset, int itrc)
{
    int i, idx;
    XPoint *points, *plot;
    Triplet *trip= trc->trip;
    /* caller (UpdatePlotZTrack) of this function has verified that 
     * trip->x, trip->y, and trip->z are not null. */
    Trace *trcN = trip->trc[TRC_Y], *trcE = trip->trc[TRC_X];
    Trace *trcZ = trip->trc[TRC_Z];
    float hs, vs, t;
    float rot = trc->trip->rot_theta - trc->trip->sta_rotation;
    int k, k2, y0;
    int which, offset, n_val;

    which = (trc==trip->trc[TRC_Z])? TRC_Z :
	((trc==trip->trc[TRC_Y])? TRC_Y : TRC_X);
    k= axis->ix1, k2=axis->ix2;
    hs= axis->hs, vs= axis->vs;
    y0= axis->y0;
    points= plot= (XPoint *)Malloc(sizeof(XPoint) * width);
    if (points==NULL) return;	/* not enuff mem */
    n_val= trc->wave->info.n_values;
    offset= (k<0)? (-k/hs) : 0;
    if(!fixIxBounds(&k,&k2,n_val))
	return;	/* no need to plot */
    if (hs > 1) {
	i=0; idx= k;
	if (toClip) {
	    while(i<width && idx<n_val) {
		int y;
		plot->x= i + offset;
		t= getRotValue(which,idx,trcZ,trcN,trcE,rot);
		y= y0 - t / vs;
		if (y<0) {
		    y=0;
		}else if(y>=height) {
		    y=height-1;
		}
		plot->y= yoffset + y;
		i++; idx= k+i*(hs);
		plot++;
	    }
	}else {
	    yoffset+=y0;    /* yoffset + y0 */
	    while(i<width && idx<n_val) {
		t= getRotValue(which,idx,trcZ,trcN,trcE,rot);
		plot->x= i + offset;
		plot->y= yoffset - t / vs;
		i++;
		/* note we do this instead of idx+= trc->axis.hs
		   so that truncation won't get worse */ 
		idx= k+i*(hs);
		plot++;
	    }
	}
    }else {  /* trc->axis.hs < 1 */
	if (toClip) {
	    for(idx=k, i=0; idx<=k2; idx++, i++) {
		int y;
		t= getRotValue(which,idx,trcZ,trcN,trcE,rot);
		plot->x= (i + offset)/hs;
		y= y0 - t / vs;
		if (y < 0) {
		    y = 0;
		}else if (y >= height) {
		    y = height -1;
		}
		plot->y= yoffset + y;
		plot++;
	    }
	}else {
	    yoffset+=y0;	/* yoffset + y0 */
	    for(idx=k, i=0; idx<=k2; idx++, i++) {
		t= getRotValue(which,idx,trcZ,trcN,trcE,rot);
		plot->x= (i + offset)/hs;
		plot->y= yoffset - t / vs;
		plot++;
	    }
	}
    }
    if(i>0) 
	XDrawLines(theDisp, xwin, gc, points, i, CoordModeOrigin);
    free(points);
}

void Rot_ZoomContentChanged()
{
    extern Dial *az_dial;
    Triplet *trip = traces[lowZTrkIndex]->trip;
    Trace *trc = traces[lowZTrkIndex];

    rt_setAzimuth();
    /* If rotation is allowed for this triplet, activate rotate panel */
    if (trip) {
	rt_setNewTheta(trip->rot_theta);
	if (trip->rotated != -1) {
	    if(!rt_cur_active) rt_activatePanel();
	    rt_cur_active= 1;
	}else {
	    if(rt_cur_active) rt_inactivatePanel();
	    rt_cur_active= 0;
	}
    }
    refresh_dial(az_dial);

}

