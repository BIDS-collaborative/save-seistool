#ifndef lint
static char id[] = "$Id: mark.c,v 1.2 2013/02/28 21:24:59 lombard Exp $";
#endif

/*
 * mark.c--
 *    conversion between coordinates/time/indices
 *
 * Copyright (c) 1992-3 Andrew K. Yu, Lawrence Berkeley Laboratory
 *      and University of California, Berkeley.
 * All rights reserved.
 */
#include <stdio.h>
#include "proto.h"

extern int subsample_rate;

/* WARNING: it is the responsibility of caller to make sure trc is not NULL */

int iround(double x)
{
    if (x > 0.0)
	return (int)(x + 0.5);
    else
	return (int)(x - 0.5);
}


/*  time to index  */
int timeToIndex(Trace *trc, STI_TIME time)
{
    BIS3_HEADER *bh = &trc->wave->info;
    double samples;

    samples = st_tdiff(time, bh->start_it) * bh->sample_rate;
    return iround(samples);
}

/*  (sub)sample index to time  */
STI_TIME indexToTime(Trace *trc, int index, int subrate)
{
    double secs;

    secs = index / (trc->wave->info.sample_rate * subrate);
    return st_add_dtime(trc->wave->info.start_it, secs * USECS_PER_SEC);
}

/*
 * timeToCoord: find the x-coordinate (pixel units) for the given time
 *    on the given axis (main window or zoom window).
 */
int timeToCoord(Axis *axis, Trace *trc, double off_set)
{
    BIS3_HEADER *bh = &trc->wave->info;
    double first_off, last_off, win_off;
    
    first_off = st_tdiff(indexToTime(trc, axis->ix1, 1), bh->start_it);
    win_off = off_set - first_off;
  
    /* is this in the window */
    /* last_off = st_tdiff(indexToTime(trc, axis->ix2, 1), bh->start_it);
     *  if (first_off>off_set || last_off<off_set) {
     *    return -1;
     *  }
     */

    return iround(win_off * bh->sample_rate / axis->hs);
}


/*  coordinate to (sub)index; if subrate is 1 result is index; 
    otherwise result is subindex  */
int coordToIndex(Axis *axis, int x, int subrate)
{
    if (axis->hs==0)
	return 0;
    return iround( (axis->ix1 + axis->hs * x) * subrate);
}

/*  (sub) index to coordinate  */
int indexToCoord(Axis *axis, int idx, int subrate)
{
    float x;
    if (axis->hs==0 || subrate == 0)
	return 0;
    x =  (idx - axis->ix1) / (axis->hs * subrate);
    return iround(x);
}

