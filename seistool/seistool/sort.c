#ifndef lint
static char id[] = "$Id: sort.c,v 1.2 2013/02/28 21:24:57 lombard Exp $";
#endif

/*
 * sort.c--
 *    reordering of traces
 *
 * Copyright (c) 1992-3 Andrew K. Yu, Lawrence Berkeley Laboratory
 *      and University of California, Berkeley.
 * All rights reserved.
 */
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include "sort.h"
#include "proto.h"
#include "xv_proto.h"

extern Trace **traces;
extern int LastTrack;
extern int Mode_triplet;
extern int Zoom_Mode_sameVertScale;
int sort_type;                    /* global variable for the sort type */

static int comparLoadOrder(const void *si, const void *sj)
{
    Trace **ti = (Trace **)si;
    Trace **tj = (Trace **)sj;
    return ((*ti)->loadOrder - (*tj)->loadOrder);
}

int RestoreLoadOrder()
{
    int i;
    qsort((char *)traces, LastTrack+1, sizeof(Trace *), comparLoadOrder);
    for(i=0; i<=LastTrack; i++)
	traces[i]->itrc= i;
}

static int sort_trace(Trace *trc1, Trace *trc2)
{
    int cmp;
    float cmpf;
    double cmpd;
  
    switch(sort_type) {
    case SORT_STATION: 
	cmp=strcmp(trc1->wave->info.station,trc2->wave->info.station);
	break;
    case SORT_CHANNEL:
	cmp=strcmp(trc1->wave->info.channel,trc2->wave->info.channel);
	break;
    case SORT_DELTA: 
	cmpf=trc1->wave->info.event_delta-trc2->wave->info.event_delta;
	if (cmpf>0.0) 
	    cmp=1;
	else if (cmpf<0.0) 
	    cmp=-1;
	else
	    cmp=0;
	break;
    case SORT_AZIMUTH:
	cmpf=trc1->wave->info.event_azimuth-trc2->wave->info.event_azimuth;
	if (cmpf>0.0) 
	    cmp=1;
	else if (cmpf<0.0) 
	    cmp=-1;
	else
	    cmp=0;
	break;
    case SORT_LAT:
	cmpf=trc1->wave->info.latitude-trc2->wave->info.latitude;
	if (cmpf>0.0) 
	    cmp=1;
	else if (cmpf<0.0)
	    cmp=-1;
	else
	    cmp=0;
	break;
    case SORT_LON:
	cmpf=trc1->wave->info.longitude-trc2->wave->info.longitude;
	if (cmpf>0.0)
	    cmp=1;
	else if (cmpf<0.0)
	    cmp=-1;
	else
	    cmp=0;
	break;
    case SORT_PICK:
	cmp=pick_cmp_trace(trc1,trc2);
	break;
    default:
	cmp=0;
    }


    /* if the delta is the same then do name comparisons */
    return ((cmp!=0) ? cmp : trc_cmp(trc1,trc2));
}

static int sort_trip(Triplet *trp1, Triplet *trp2)
{
    int cmp;
    float cmpf;
    double cmpd;
  
    switch(sort_type) {
    case SORT_STATION: 
	cmp=strcmp(trp1->first_trc->wave->info.station,
		   trp2->first_trc->wave->info.station);
	break;
    case SORT_CHANNEL:
	cmp=strcmp(trp1->first_trc->wave->info.channel,
		   trp2->first_trc->wave->info.channel);
	break;
    case SORT_DELTA:
	cmpf=trp1->first_trc->wave->info.event_delta -
	    trp2->first_trc->wave->info.event_delta;
    if (cmpf>0)
	cmp=1;
    else if (cmpf<0)
	cmp=-1;
    else
	cmp=0;
    break;
    case SORT_AZIMUTH:
	cmpf=trp1->first_trc->wave->info.event_azimuth -
	    trp2->first_trc->wave->info.event_azimuth;
	if (cmpf>0) 
	    cmp=1;
	else if (cmpf<0)
	    cmp=-1;
	else
	    cmp=0;
	break;
    case SORT_LAT:
	cmpf=trp1->first_trc->wave->info.latitude -
	    trp2->first_trc->wave->info.latitude;
	if (cmpf>0)
	    cmp=1;
	else if (cmpf<0)
	    cmp=-1;
	else
	    cmp=0;
	break;
    case SORT_LON: 
	cmpf=trp1->first_trc->wave->info.longitude -
	    trp2->first_trc->wave->info.longitude;
	if (cmpf>0)
	    cmp=1;
	else if (cmpf<0)
	    cmp=-1;
	else
	    cmp=0;
	break;
    case SORT_PICK:
	cmp=pick_cmp_trip(trp1,trp2);
	break;
    default:
	cmp=0;
    }
  
    return cmp;
}

/* to make this work more generally we need */
/* to change sort_dis into a varaiable comparrison */
/* function brought into this routine      */
/* what about triplet mode????             */
void sort_traces()
{
    Btree *bt;
    BtreeNode **trcGp;
    int i,j,count=0;


    /* if not in triplet mode reorder the traces */
    if (Mode_triplet==0) { 
	bt=empty_btree(sort_trace);

	/* load btree with traces */
	for (i=0;i<=LastTrack;i++) {
	    btree_insert(bt,traces[i]);
	}

	trcGp=(BtreeNode **)btree_linearize(bt);
    
	/* now move the list into a traces */

	for (i=0;i<bt->numElt;i++) {
    
	    for (j=0;j<trcGp[i]->numVal; j++) {
		traces[count++]=(Trace *)trcGp[i]->val[j];
	    }

	}

	/* if Zoom - zoom contents changed */
	if (ZoomWindowMapped) {
	    RedrawZoomWindow("sort_traces"); /* just in case */
	    if(Zoom_Mode_sameVertScale) newVertScale();
	}

	/* reorder the triplets */
    } else {
	int num_trips=0;
	Triplet **trips;

	trips= (Triplet **)Malloc(sizeof(Triplet *)*(LastTrack+1));
	bzero(trips,sizeof(Triplet*)*(LastTrack+1));

	bt=empty_btree(sort_trip);

	/* fill trips with triplet info from traces */
	for(i=0;i<=LastTrack; i++) {
	    int seen=0;
	    Trace *trc=traces[i];

	    for(j=0;j< num_trips; j++) {
		if (trips[j]->first_trc == trc->trip->first_trc) {
		    seen=1;
		    break;
		}
	    }

	    if (seen==0) {
		trips[num_trips++]=trc->trip;
	    }
	}

	/* load triplets into btree */
	for(i=0;i< num_trips; i++) {
	    btree_insert(bt,trips[i]);
	}

	trcGp=(BtreeNode **)btree_linearize(bt);
   
	/* now move the list into a traces */
	for (i=0;i<bt->numElt;i++) {
    
	    for (j=0;j<trcGp[i]->numVal; j++) {
		trips[count++]=(Triplet *)trcGp[i]->val[j];
	    }
	}

	reorg_trcs(num_trips,trips);
	free (trips);
    
	if (ZoomWindowMapped) {
	    Trip_ZoomContentChanged(lowZTrkIndex);
	    RedrawZoomWindow("Sort_trips"); /* just in case */
	    if(Zoom_Mode_sameVertScale) newVertScale();
	}
    }


    RedrawScreen ();
    btree_destroy(bt);
    free(trcGp);
}
