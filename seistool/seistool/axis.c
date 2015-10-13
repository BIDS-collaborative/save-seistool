#ifndef lint
static char id[] = "$Id: axis.c,v 1.2 2013/02/28 21:25:01 lombard Exp $";
#endif

/*
 * axis.c--
 *    handles Axis abstraction
 *
 * Copyright (c) 1992-3 Andrew K. Yu, Lawrence Berkeley Laboratory
 *      and University of California, Berkeley.
 * All rights reserved.
 */
#include <stdio.h>
#include <math.h>
#include "proto.h"

#define ROUND(x)    ((int)((x)+0.5))

/* Global variables */
extern int Mode_abs_vscale;  /* are we in absolute scale mode */
/* defined in track.c            */
Axis abs_Axis;               /* a holder for absolute axis    */
/* note ix1 for the fised height */

static void recalc_y0 (Axis *ax, int height);


void InitAxis(BIS3_HEADER *bh, Axis *ax)
{
    switch(bh->format) {
    case I16_FORMAT:
	ax->ymax= (float) bh->max_value.sflag;
	ax->ymin= (float) bh->min_value.sflag;
	break;
    case I32_FORMAT:
	ax->ymax= (float) bh->max_value.iflag;
	ax->ymin= (float) bh->min_value.iflag;
	break;
    case R32_FORMAT:
	ax->ymax= bh->max_value.fflag;
	ax->ymin= bh->min_value.fflag;
	break;
    case R64_FORMAT:
	ax->ymax= (float) bh->max_value.dflag;
	ax->ymin= (float) bh->min_value.dflag;
	break;
    }
    ax->ix1=0;
    ax->ix2= bh->n_values - 1;
    ax->y1=ax->ymax;
    ax->y2=ax->ymin;
    
    return;
}


/********************************************************
 *      This recalcs y0 and vs when height changes      *
 ********************************************************/
static void recalc_y0 (Axis *ax, int height)
{
    float vs;
    int y0;

    vs=(float)(ax->y1-ax->y2)/height;
  
    if (ax->y1==0 && ax->y2==0) {
	/* should mark dead trace but leave it for now */
	y0=height/2;
	vs=1;			/* doesn't matter */
    }else if (ax->y1>=0 && ax->y2 <= 0) {
	y0= ax->y1/vs;
    }else if (ax->y1<=0 && ax->y2<=0 && (ax->y2<ax->y1)){
	y0= ax->y1 / vs;
    }else if (ax->y1>=0 && ax->y2>=0 && (ax->y1>ax->y2)) {
	y0= height + ax->y2 / vs;
    }else {
	fprintf(stderr,"Warning: ScaleAxis-- scale range problem.\n");
	fprintf(stderr,"         y1= %f   y2= %f\n", ax->y1, ax->y2);
	vs= height/10;
	y0= height/2;
    }
    ax->y0= y0;
    ax->vs= vs;
    ax->vsmag= 1.0;
    /* yeah I know I should not us ix1 for the height */
    ax->ix1=height;
}


/********************************************************
 *    fill in y0 and vs from abs_Axis into this axis    *
 ********************************************************/
void UniformScaleYAxis(Axis *ax, int height)
{
    int y0;
    float vs;
    static int prev_height=0;

    recalc_y0(&abs_Axis,height);

    ax->y0= abs_Axis.y0;
    ax->vs= abs_Axis.vs;
    ax->vsmag= 1.0;
}


/********************************************************
 *  The old y axis scaling routine used in normal cond. *
 ********************************************************/
void ScaleYAxis(Axis *ax, int height)
{
    int y0;
    float vs;

    if (Mode_abs_vscale==1) {
	UniformScaleYAxis(ax, height);
    } else {

	/* calculate vs (1 pixel to vs units) */
	if (ax->y1==ax->y2) {	/* including both 0 */
	    /* should mark dead trace but leave it for now */
	    y0=height/2;
	    vs=1;			/* doesn't matter */
	}else if (ax->y1>=0 && ax->y2 <= 0) {
	    vs= (float)(ax->y1 - ax->y2)/height;
	    y0= ax->y1/vs;
	}else if (ax->y1<=0 && ax->y2<=0 && (ax->y2<ax->y1)){
	    vs= (float)(ax->y1 - ax->y2)/height;
	    y0= ax->y1 / vs;
	}else if (ax->y1>=0 && ax->y2>=0 && (ax->y1>ax->y2)) {
	    vs= (float)(ax->y1 - ax->y2)/height;
	    y0= height + ax->y2 / vs;
	}else {
	    fprintf(stderr,"Warning: ScaleAxis-- scale range problem.\n");
	    fprintf(stderr,"         y1= %f   y2= %f\n", ax->y1, ax->y2);
	    vs= height/10;
	    y0= height/2;
	}
	ax->y0= y0;
	ax->vs= vs;
	ax->vsmag= 1.0;
    }
}

void ScaleTAxis(Axis *ax, float samplePerSec, int width)
{
    /* calculate hs (1 pixel to hs ticks) */
    ax->hs=((float)(ax->ix2-ax->ix1))/width;
}


int fixIxBounds(int *ix1p, int *ix2p, int nval)
{
    int ix1=*ix1p, ix2=*ix2p;

    if(ix2<0 || ix1>=nval) /* assume ix1 < ix2 */
	return 0;
    if(ix1<0) *ix1p= 0;
    if(ix2>=nval) *ix2p=nval-1;
    return 1;
}

