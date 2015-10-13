#ifndef lint
static char id[] = "$Id: triplet.c,v 1.2 2013/02/28 21:24:56 lombard Exp $";
#endif

/*
 * triplet.c--
 *    handles the grouping of Triplets
 *
 * Copyright (c) 1992-3 Andrew K. Yu, Lawrence Berkeley Laboratory
 *      and University of California, Berkeley.
 * All rights reserved.
 */
#include <math.h>
#include <stdio.h>
#include <strings.h>
#include "proto.h"
#include "xv_proto.h"

int	Mode_triplet= 0;	/* triplet mode */
extern Trace **traces;
extern int LastTrack;

/* internal function prototypes */
static int trp_cmp (Triplet *trp1, Triplet *trp2);
static void Trip_GroupTraces();
static void arrange_triplet(Triplet *trip);



/**********************************************************************
 *                                                                    *
 **********************************************************************/

Triplet *makeTriplet()
{
    Triplet *t= (Triplet *)Malloc(sizeof(Triplet));
    bzero(t,sizeof(Triplet));
    return t;
}

void destroyTriplet(Trace *trc)
{
    int i;
    Triplet *trip= trc->trip;

    if(trip) {
	if(trip->numtrc==1) {	/* last one referenced */
	    free(trip);
	    return;
	}
	/* find the referenced one */
	for (i = 0; i < trip->numtrc; i++) {
	    if (trip->trc[i] == trc) {
		trip->trc[i] = NULL;
		trip->dir[i] = '\0';
	    }
	}
	trip->numtrc--;
    }
}

/*
 * trc_cmp: compare traces. The idea is to sort traces by station, network,
 *    location and first 2 letters of channel names. Those traces that are
 *    `equal' are assumed to be part of the same triplet. Because seistool
 *    is sometimes used to read several traces from one SNCL for different
 *    time periods, we also require that the start times be the same for
 *    traces in a triplet. Currently this requires identical start times;
 *    maybe it would be good enough to match to nearest second.
 *    Finally, we require that traces in the triplet have the same
 *    samplerate; anything else would make triplet rotations too hard.
 */
int trc_cmp(Trace *trc1, Trace *trc2)
{
    BIS3_HEADER *bh1, *bh2;
    int cmp;
    
    bh1 = &trc1->wave->info;
    bh2 = &trc2->wave->info;
    if ( (cmp = strcmp(bh1->station, bh2->station)) != 0)
	return cmp;
    if ( (cmp = strcmp(bh1->network, bh2->network)) != 0)
	return cmp;
    if ( (cmp = strcmp(bh1->location, bh2->location)) != 0)
	return cmp;
    if ( (cmp = strncmp(bh1->channel, bh2->channel, 2)) != 0)
	return cmp;
    if (bh1->sample_rate > bh2->sample_rate)
	return 1;
    else if (bh1->sample_rate < bh2->sample_rate)
	return -1;
    else
	return 0;
}

/* Triple comparison routine for btree ordering.			*/
/* Compare 2 triples, and order them by the lowest loadOrder trace	*/
/* in the triple.  This routine will preserve traces in loadOrder as	*/
/* much as possible during triple creation and reordering.		*/
static int trp_cmp (Triplet *trp1, Triplet *trp2)
{
    int min_1 = 65536;	    /* some VERY large value.	*/
    int min_2 = 65536;	    /* some VERY large value.	*/
    int i;
    
    for (i = 0; i < 3; i++) {
	min_1 = (trp1->trc[i] && (trp1->trc[i]->loadOrder < min_1)) ?
	    trp1->trc[i]->loadOrder : min_1;
	min_2 = (trp2->trc[i] && (trp2->trc[i]->loadOrder < min_2)) ?
	    trp2->trc[i]->loadOrder : min_2;
    }
    
    return (min_1-min_2);
}
    


static void Trip_GroupTraces()
{
    Btree *bt= empty_btree(trc_cmp);
    BtreeNode **trcGp, **trpGp;
    Triplet **trips, **trips_sort;
    int i, j;
    int num_trips, s_trips, num_trips_sort;
    /* btree for triples.   *

       /* group traces with same SNCL (except component direction), start time
	  and sampling rate */
    for(i=0; i<=LastTrack; i++) {
	btree_insert(bt, traces[i]);
    }
    trcGp= (BtreeNode **)btree_linearize(bt);

    /* an array to hold the triplets */
    trips= (Triplet **)Malloc(sizeof(Triplet *)*(LastTrack+1));
    bzero(trips,sizeof(Triplet*)*(LastTrack+1));
    s_trips= num_trips= 0;

    /* create triplets */
    for(i=0; i< bt->numElt; i++) {
	Triplet *t;

	s_trips= num_trips;
	trips[num_trips++]= makeTriplet();
	trips[num_trips-1]->rotated=0;

	/* Loop through all the traces that compare equally in trc_cmp() */
	for(j=0; j< trcGp[i]->numVal; j++) {
	    int k, ok;
	    Trace *trc= (Trace *)trcGp[i]->val[j];
	    /* if successfully associates with the triplet,
	       move on to the next trace, otherwise create a new
	       triplet */
	    ok= 0;
#ifdef DEBUG
	    fprintf(stderr, "j %d i %d %s %s %d\n",j, i, 
		    trc->wave->info.station, trc->wave->info.channel,
		    trc->wave->info.start_it.second);
#endif
	    for(k=s_trips; k<num_trips; k++) {
		t= trips[k];
		if(associate_triplet(t, trc)) {
#ifdef DEBUF
		    fprintf(stderr, "assoc %d %d\n", k, t->numtrc);
#endif		    
		    ok= 1;
		    break;
		}
	    }
	    /* trace didn't associate with existing triplet; make a new one */
	    if(!ok) {
#ifdef DEBUG
		fprintf(stderr, "no assoc %d\n", num_trips);
#endif
		trips[num_trips++]= t= makeTriplet();
		associate_triplet(t,trc);
	    }
	}
    }
    for (i = 0; i < num_trips; i++)
	arrange_triplet(trips[i]);

    /* cleanup initial btree. */
    btree_destroy(bt);
    free(trcGp);

    /* reorder the triples so that they are ordered by the earliest	*/
    /* original trace order in the triple.				*/
    bt= empty_btree(trp_cmp);
    for(i=0; i< num_trips; i++) {
	btree_insert(bt, trips[i]);
    }
    trpGp= (BtreeNode **)btree_linearize(bt);

    /* an array to hold the triplets */
    trips_sort= (Triplet **)Malloc(sizeof(Triplet *)*(num_trips+1));
    bzero(trips_sort,sizeof(Triplet*)*(num_trips+1));
    num_trips_sort = 0;
    for (i=0; i< bt->numElt; i++) {
	for (j=0; j< trpGp[i]->numVal; j++) {
	    trips_sort[num_trips_sort++] = (Triplet *)trpGp[i]->val[j];
	}
    }

    reorg_trcs(num_trips_sort, trips_sort);

    /* clean up. */
    btree_destroy(bt);
    free(trpGp);
    free(trips);
    free(trips_sort);
}

int associate_triplet(Triplet *trip, Trace *trc)
{
    int ok=1;
    int i;
    STI_TIME ts, te;
    char comp_dir;
    BIS3_HEADER *bh = &trc->wave->info;
    
    ts = bh->start_it;
    te = st_add_dtime(ts, (double)(bh->n_values-1) * 
		      USECS_PER_SEC / bh->sample_rate);
    comp_dir = bh->channel[2];

    if(trip->numtrc) {
	if (trip->numtrc == 3)
	    return 0;
	if (trip->trc[0]->wave->info.channel[2] == bh->channel[2] ||
	    (trip->numtrc == 2 && 
	     trip->trc[1]->wave->info.channel[2] == bh->channel[2]) )
	    return 0;
	/* make sure traces in the triplet overlap for some time interval */
	if (TimeCmp(ts,trip->stime) <=0) {   
	    /* trace starts before trip */

	    if (TimeCmp(te,trip->stime)<=0) 
		/* trace ends before triplet starts */
		return 0;
	} 
	else if (TimeCmp(ts,trip->etime)>0) 
	    /* trace starts after triplet ends */
	    return 0;
    }
    

    /* okay, this trace associates with this triplet */
    if(trip->numtrc==0) {	/* first trace */
	trip->stime= bh->start_it;
	trip->etime= st_add_dtime(trip->stime, (double)bh->n_values * 
				  USECS_PER_SEC / bh->sample_rate);
	trip->sovrlap= trip->stime;
	trip->eovrlap= trip->etime;
	trip->trc[0] = trc;  /* fill the first slot for now */
    }else {
	/* trip->stime is the earliest start time of any trace;
	 * trip->sovrlap is the latest start time of any trace.
	 * trip->eovrlap is the earliest end time of any trace;
	 * trip->etime is the latest end time of any trace. */
	if(TimeCmp(ts,trip->stime) < 0) {  
	    trip->stime = ts;     /* trace starts before triplet starts */
	}else if (TimeCmp(ts,trip->sovrlap) > 0){ 
	    trip->sovrlap = ts;   /* trace starts after overlap starts */
	}
	if(TimeCmp(te,trip->etime)>0) {
	    trip->etime= te;      /* trace ends after triplet ends */
	}else if (TimeCmp(te,trip->eovrlap) < 0){
	    trip->eovrlap = te;   /* trace ends before overlap ends */
	}
	if (trip->numtrc == 1)
	    trip->trc[1] = trc;
	else if (trip->numtrc == 2)
	    trip->trc[2] = trc;
    }
    trip->numtrc++;

    return 1;
}

void reorg_trcs(int num_trips, Triplet **trips)
{
    int itrc, i, j, first_itrc;
    /* reorganize the traces */
    itrc= 0;
    for(i=0; i<num_trips; i++) {
	Triplet *t= trips[i];
	first_itrc = itrc;
	for (j = 0; j < 3; j++ ) {
	    if(t->trc[j]) {
		traces[itrc]= t->trc[j];
		traces[itrc]->itrc= itrc;
		itrc++;
		t->trc[j]->trip= t;
	    }
	}
	if (itrc > first_itrc)
	    t->first_trc = traces[first_itrc];
    }
}

/**********************************************************************
 *                                                                    *
 **********************************************************************/

/*
 * The triplet displayed in zoom window has changed to include Trace ref_itrc,
 * so update the low and high zomm track indices and 
 * possibly the number of traces in zoom window.
 */
void Trip_ZoomContentChanged(int ref_itrc)
{
    Trace *trc= traces[ref_itrc];
    Triplet *trip;
    int i, num;
    if(trc && trc->trip) {
	trip= trc->trip;
	trc = trip->first_trc; /* use first trace in the group */
	num = trip->numtrc;
	if(num!=NumZTracks) internal_ChangeNumZTracks(num);
	lowZTrkIndex= trc->itrc;
	highZTrkIndex= lowZTrkIndex+num-1;
    }
}

void StartTripletMode()
{
    BIS3_HEADER *bh= &traces[0]->wave->info;
    int i, oldl;
    if(Mode_triplet==1)return;	/* already started */ 
    Mode_triplet= 1;
    if( LastTrack >= 0) {
	/* read in default responses */
	GetResponses(NULL);
	Trip_GroupTraces();
	if (ZoomWindowMapped) {
	    oldl=lowZTrkIndex;
	    Trip_ZoomContentChanged(lowZTrkIndex);
	    /* rescale */
	    ScaleZTrack(traces[lowZTrkIndex],traces[oldl]);
	    if (Zoom_Mode_sameVertScale) traces[lowZTrkIndex]->zaxis_needScale= 1;
	    for(i=lowZTrkIndex+1; i<=highZTrkIndex; i++) {
		ScaleZTrack(traces[i],traces[oldl]);
		if (Zoom_Mode_sameVertScale) traces[i]->zaxis_needScale= 1;
	    }
	    RedrawZoomWindow("StartTripletMode"); /* just in case */
	    if(Zoom_Mode_sameVertScale) newVertScale();
	}
	RedrawScreen();
    }
}

void EndTripletMode()
{
    /* either one of these need to retain triplet mode */
    if(!Mode_trvlTime && !Mode_rotate)
	Mode_triplet= 0;
}


/**********************************************************************
 *                                                                    *
 **********************************************************************/

void checkTripBound(Trace *trc)
{
    Triplet *trip= trc->trip;
    Axis *zaxis= &trc->zaxis;
    int s_ix, e_ix, len;

    len = zaxis->ix2 - zaxis->ix1;
    s_ix= timeToIndex(trc, trip->sovrlap);
    e_ix= timeToIndex(trc, trip->eovrlap);

    if(zaxis->ix1<s_ix) {
	zaxis->ix1=s_ix;
	if (s_ix + len > e_ix)
	    zaxis->ix2 = e_ix;
	else
	    zaxis->ix2 = s_ix + len;
    }
    else if(zaxis->ix2>e_ix) {
	zaxis->ix2 = e_ix;
	if (e_ix - len < s_ix)
	    zaxis->ix1 = s_ix;
	else
	    zaxis->ix1 = e_ix - len;
    }
}

/* note that the triplets have the same sampling rate. */
void getTripBound(Trace *trc, int *pS_ix, int *pE_ix)
{
    Triplet *trip= trc->trip;
    *pS_ix= timeToIndex(trc, trip->sovrlap);
    *pE_ix= timeToIndex(trc, trip->eovrlap);
}

/*
 * arrange_triplet: figure out which traces should be Z, N and E or
 *   1, 2, and 3. Choice is based on their dips and azimuths if we
 *   know them, otherwise on the trace direction labels.
 *   Also decide if triplet can be rotated about its vertical component.
 *   Once we have decided which trace is which, then put them in that
 *   order in the Traces array.
 */
static void arrange_triplet(Triplet *trip)
{
    BIS3_HEADER *bh;
    Trace *tmp_trc;
    int i, nZ_dip, nN_az, nE_az, nN, nE, nZ;
    int no_dip, no_rotate;
    float max_dip = -1.0, az;
    
    /* Find the component closest to vertical */
    nN = nE = nZ = -1;  /* direction by name */
    nN_az = nE_az = -1; /* direction by azimuth */
    no_dip = no_rotate = 0;
    for (i = 0; i < trip->numtrc; i++) {
	bh = &trip->trc[i]->wave->info;
	if (bh->response < 0) {
	    no_dip = 1;
	}
	if ((float)fabs(bh->dip) > max_dip) {
	    max_dip = (float)fabs(bh->dip);
	    nZ_dip = i;
	}
	switch(bh->channel[2]) {
	case 'Z':
	    if (nZ == -1)
		nZ = i;
	    else {
		no_rotate = 1;
		fprintf(stderr, "duplicate 'Z' component %s %s %s %s\n",
			bh->station, bh->network, bh->channel,
			bh->location[0] == '\0' ? "--" : bh->location);
	    }
	    break;
	case 'N':
	    if (nN = -1)
		nN = i;
	    else {
		no_rotate = 1;
		fprintf(stderr, "duplicate 'N' component %s %s %s %s\n",
			bh->station, bh->network, bh->channel,
			bh->location[0] == '\0' ? "--" : bh->location);
	    }
	    break;
	case 'E':
	    if (nE = -1)
		nE = i;
	    else {
		no_rotate = 1;
		fprintf(stderr, "duplicate 'E' component %s %s %s %s\n",
			bh->station, bh->network, bh->channel,
			bh->location[0] == '\0' ? "--" : bh->location);
	    }
	    break;
	default:
	    break;
	}
    }
    if (no_dip == 1 && no_rotate == 1) {
	/* some dip/az not found, directions indeterminated, so give
	 * arbitrary direction labels and disable rotation */
	trip->rotated = -1;
	for (i = 0; i < trip->numtrc; i++)
	    trip->dir[i] = '1' + i;
    }
    else if (no_dip == 1) {
	/* some dip/az not found, but we might be able to fake directions */
	if (nZ != -1) {
	    if (nZ != TRC_Z) {
		tmp_trc = trip->trc[TRC_Z];
		trip->trc[TRC_Z] = trip->trc[nZ];
		trip->trc[nZ] = tmp_trc;
		if (nZ == TRC_Y)
		    nN = TRC_Y;
		else if (nZ = TRC_X)
		    nE = TRC_X;
	    }
	    trip->dir[TRC_Z] = 'Z';
	}
	if (nN != -1) {
	    if (nN != TRC_Y) {
		tmp_trc = trip->trc[TRC_Y];
		trip->trc[TRC_Y] = trip->trc[nN];
		trip->trc[nN] = tmp_trc;
		if (nN = TRC_X)
		    nE = TRC_X;
	    }
	    trip->dir[TRC_Y] = 'N';
	}
	else   /* no North; can't rotate */
	    trip->rotated = -1;
	if (nE != -1) {
	    if (nE != TRC_X) {
		tmp_trc = trip->trc[TRC_X];
		trip->trc[TRC_X] = trip->trc[nE];
		trip->trc[nE] = tmp_trc;
	    }
	    trip->dir[TRC_X] = 'E';
	}
	else   /* no East; can't rotate */
	    trip->rotated = -1;
    } 
    else {    /* all dip/az known */
	if (max_dip > 80) {
	    if ( nZ_dip != TRC_Z ) {  /* need to move some traces */
		tmp_trc = trip->trc[TRC_Z];
		trip->trc[TRC_Z] = trip->trc[nZ_dip];
		trip->trc[nZ_dip] = tmp_trc;
	    }
	    trip->dir[TRC_Z] = 'Z';
	    nZ_dip = TRC_Z;
	}
	if (max_dip < 10 || max_dip > 80) { 
	    /* now look for N and E traces */
	    for (i = 0; i < 3; i++) {
		if (i != nZ_dip && trip->trc[i] != NULL) {
		    az = trip->trc[i]->wave->info.azimuth;
		    if ((az >= 45 && az < 135) || (az >= 225 && az < 315)) {
			if ( nE_az == -1)
			    nE_az = i;
			else
			    no_rotate = 1;  /* two Easts! */
		    } 
		    else {
			if ( nN_az == -1)
			    nN_az = i;
			else
			    no_rotate = 1;  /* two Norths! */
		    }
		}
	    }
	} 
	else {
	    /* no 'vertical' or 'horizontals' */
	    no_rotate = 1;
	}
	if (no_rotate == 1) {
	    for (i = 0; i < 3; i++) {
		if (trip->trc[i] != NULL && trip->dir[i] != 'Z')
		    trip->dir[i] = '1' + i;
	    }
	    trip->rotated = -1;
	    return;
	}
	if (nN_az != -1) {
	    if (nN_az != TRC_Y) {
		tmp_trc = trip->trc[TRC_Y];
		trip->trc[TRC_Y] = trip->trc[nN_az];
		trip->trc[nN_az] = tmp_trc;
		if (nN_az = TRC_X)
		    nE_az = TRC_X;
	    }
	    trip->dir[TRC_Y] = 'N';
	    trip->sta_rotation = trip->trc[TRC_Y]->wave->info.azimuth;
	    trip->rot_theta = trip->sta_rotation;
	}
	else  /* no North; can't rotate */
	    trip->rotated = -1;
	if (nE_az != -1) {
	    if (nE_az != TRC_X) {
		tmp_trc = trip->trc[TRC_X];
		trip->trc[TRC_X] = trip->trc[nE_az];
		trip->trc[nE_az] = tmp_trc;
	    }
	    trip->dir[TRC_X] = 'E';
	}
	else   /* no East; can't rotate */
	    trip->rotated = -1;
    }

    return;
}

