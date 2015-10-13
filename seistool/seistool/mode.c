#ifndef lint
static char id[] = "$Id: mode.c,v 1.2 2013/02/28 21:24:59 lombard Exp $";
#endif

/*
 * mode.c--
 *    definition of different modes
 *
 * Copyright (c) 1992-3 Andrew K. Yu, Lawrence Berkeley Laboratory
 *      and University of California, Berkeley.
 * All rights reserved.
 */

#include "xv_proto.h"

/* mode toggle switches */
int Mode_align= 1;		/* defaults to time align */
int Mode_timescale=1;           /* time scale same on all traces */
int Mode_autoLoadPicks=1;	/* load in picks along with data (eg. UW) */
int Mode_autoScroll=1;		/* main window scroll togther with zoom win */
int Mode_noBorder= 0;		/* show border in main window */
int Mode_UWPickStyle= 0;	/* don't follow UW package pick style */
int Mode_clipPlot = 0;		/* clip when plotting in zoom window */
int Mode_decimPlot = 0;		/* plotting to screen with internal
				   decimation */

void setmode_autoscroll_on()
{
    Mode_autoScroll=1;
}

void setmode_autoscroll_off()
{
    Mode_autoScroll=0;
}

void setmode_ztrack()
{
    /* toggle clipping for now */
    Mode_clipPlot = (Mode_clipPlot==0)?1:0;
    toggleClipButton();
    RedrawZoomWindow("setmode_ztrack");
}
