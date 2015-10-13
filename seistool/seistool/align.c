#ifndef lint
static char id[] = "$Id: align.c,v 1.4 2013/02/28 21:25:01 lombard Exp $";
#endif

/*
 * align.c--
 *    handles time alignment
 *
 * Copyright (c) 1992-3 Andrew K. Yu, Lawrence Berkeley Laboratory
 *      and University of California, Berkeley.
 * All rights reserved.
 */
#include <stdio.h>
#include <strings.h>
#include "proto.h"
#include "xv_proto.h"

/*  Global  */
STI_TIME earl_time, late_time;
double TotalSeconds = 0.0;
char *Align_phase_n=NULL;
extern int LastTrack;
extern Trace **traces;
extern Track **tracks;

extern Trace **waifTraces;
extern int NumWaifTraces;

/*
 * getEarliestSample--
 *    find earliest times amongst all traces
 */
STI_TIME getEarliestSample(int includeWaifs)
{
    STI_TIME *earl, *fst;
    int i;
    
    if (LastTrack>=0) {	/* be safe */
	earl = &traces[0]->wave->info.start_it;
    }
    for(i=1; i<=LastTrack; i++) {
	fst = &traces[i]->wave->info.start_it;
	if (TimeCmp(*earl, *fst) > 0) {
	    earl = fst;
	}
    }
    if (includeWaifs) {
	for(i=0; i<NumWaifTraces; i++) {
	    fst = &waifTraces[i]->wave->info.start_it;
	    if (TimeCmp(*earl, *fst) > 0) {
		earl = fst;
	    }
	}
    }
    
    return *earl;
}

/*
 * UnifyTimeScale--
 *    used for "time align" mode.
 *    actual for mode_align && mode_timescale
 */
void UnifyTimeScale()
{
    BIS3_HEADER *bh;
    STI_TIME  fst, lst;
    double sec_len;
    float srate;
    int i;

    /* find earliest and latest times amongst all traces */
    bzero(&earl_time, sizeof(STI_TIME));
    bzero(&late_time, sizeof(STI_TIME));
    if (LastTrack >= 0) {	/* be safe */
	earl_time= traces[0]->wave->info.start_it;
    }else {
	TotalSeconds = 0.0;
	return;
    }

    for(i=0; i<=LastTrack; i++) {
	bh = &(traces[i]->wave->info);
	fst = bh->start_it;
	if (TimeCmp(earl_time, fst) > 0) {
	    earl_time = fst;
	}
	lst = st_add_dtime(fst, (double)bh->n_values * USECS_PER_SEC 
			   / bh->sample_rate);
	if (TimeCmp(lst, late_time) > 0) {
	    late_time = lst;
	}
    }

    /* now, we want a unified horizontal scale */
    TotalSeconds = sec_len = st_tdiff(late_time, earl_time);

    for(i=0; i <=LastTrack; i++) {
	Trace *trc=traces[i];
	bh = &(trc->wave->info);
	srate = bh->sample_rate;

	/* tracks all have the same width! */
	trc->axis.hs = (float)(sec_len * srate / (tracks[0]->width-1));
	trc->axis.ix1= timeToIndex(trc, earl_time);
	trc->axis.ix2= timeToIndex(trc, late_time);
	/* what about zaxis ??? */
    }
    /* forget about the stepInterval and stuff ... */
}


/* fill in earl_time and late_time in unaligned modes */
void Fill_glob_Times(double earl_sec,double late_sec)
{
    bzero(&earl_time, sizeof(STI_TIME));
  
    late_time = st_add_dtime(earl_time, late_sec * USECS_PER_SEC);
    earl_time = st_add_dtime(earl_time, earl_sec * USECS_PER_SEC);
    /* Does this work? These STI_TIME structs are filled with *
     *  some small time; so leapseconds won't be used...      */
}  

/*
 * timescale_noalign --
 * 
 */
void timescale_noalign  ()
{
    BIS3_HEADER *bh;
    Trace *trc;
    int i;
    float srate, dur, hs, max_len=-1.0;

    /* find max time length amongst all traces */
 
    for(i=0; i<=LastTrack; i++) {
	bh= &(traces[i]->wave->info);
	dur= (float)bh->n_values / bh->sample_rate;

	if (dur > max_len) {
	    max_len=dur;
	}
    }
    Fill_glob_Times(0,max_len);

    /* now, we want a unified horizontal scale */
    TotalSeconds = (double)max_len;

    for(i=0; i <=LastTrack; i++) {
	trc = traces[i];
	srate= traces[i]->wave->info.sample_rate;

	/* tracks all have the same width! :) */
	trc->axis.hs = max_len*srate / (tracks[0]->width-1);
	trc->axis.ix1 = 0;
	trc->axis.ix2 = max_len * srate;
	/* what about zaxis ??? */
    }
    return;
}
