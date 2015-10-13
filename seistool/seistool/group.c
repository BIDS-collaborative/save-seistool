#ifndef lint
static char id[] = "$Id: group.c,v 1.2 2013/02/28 21:24:59 lombard Exp $";
#endif

/*
 * group.c--
 *    grouping traces
 *
 * Copyright (c) 1992-3 Andrew K. Yu, Lawrence Berkeley Laboratory
 *      and University of California, Berkeley.
 * All rights reserved.
 */
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include "proto.h"
#include "xv_proto.h"

extern Trace **traces;
extern int LastTrack;
extern int Mode_triplet;

/* The following are defined but not used in this file; see trackmgr.c */
int Mode_autoRegroup= 0;
int Mode_autoGroupComp= 0;  /* note this one is totally different
			       from Mode_triplet */
/*
 * match_and_regroup: if and only if traces are already grouped so that
 *   station name and first two letters of channel name of adjacent traces
 *   are matching, then put the Z trace first in that group, then the N trace
 *   and finally the E trace. Otherwise, do nothing.
 */
void match_and_regroup()
{
    int itrc;
    Trace *trc;
    char gpname[10];
    int gp=0, len, i;
    BIS3_HEADER *bh1, *bh2, *bh3;
    Trace *trc_z, *trc_n, *trc_e;
    
    if (Mode_triplet) return;
    
    itrc=0;
    while(itrc<=LastTrack-2) {
	bh1 = &traces[itrc]->wave->info;
	bh2 = &traces[itrc+1]->wave->info;
	bh3 = &traces[itrc+2]->wave->info;
	/* match the next two */
	if ( strcmp(bh1->station, bh2->station) != 0 ||
	     strcmp(bh1->network, bh2->network) != 0 ||
	     strcmp(bh1->location, bh2->location) != 0 ||
	     strncmp(bh1->channel, bh2->channel, 2) != 0) {
	    itrc++;
	    continue;
	}
	if ( strcmp(bh1->station, bh3->station) != 0 ||
	     strcmp(bh1->network, bh3->network) != 0 ||
	     strcmp(bh1->location, bh3->location) != 0 ||
	     strncmp(bh1->channel, bh3->channel, 2) != 0) {
	    itrc += 2;
	    continue;
	}
	
	/* next two matches, only differ by channel direction */
	trc_z=trc_n=trc_e= NULL;
	for(i=0; i<3; i++) {
	    switch(traces[itrc+i]->wave->info.channel[2]) {
	    case 'Z': case 'z': 
		trc_z=traces[itrc+i];
		break;
	    case 'N': case 'n':
		trc_n=traces[itrc+i];
		break;
	    case 'E': case 'e':
		trc_e=traces[itrc+i];
		break;
	    default:
		/* Oops; don't know what to do with weird directions */
		itrc++;
		goto Next;
	    }
	}
	if (trc_z && trc_n && trc_e) {
	    traces[itrc]= trc_z;
	    trc_z->itrc= itrc;
	    traces[itrc+1]= trc_n;
	    trc_n->itrc= itrc+1;
	    traces[itrc+2]= trc_e;
	    trc_e->itrc= itrc+2;
	}
	itrc+=3;
    Next:
	continue;
    }
    return;
}

void regroup_3comp()
{
    match_and_regroup();
    RedrawScreen();
    if(ZoomWindowMapped)
	RedrawZoomWindow("regroup_3comp");
}


/*
 * Group_ZNE: order the traces so all the Z components are first, then
 *  all the North components, then all the East, and then all the others.
 */
void Group_ZNE()
{
    Trace **zs, **ns, **es, **garb;
    int nzs, nns, nes, ngarb;
    int idx, i;

    if (Mode_triplet)
	EndTripletMode();
    
    /* well, we have space! */
    zs= (Trace **)Malloc(sizeof(Trace *)*LastTrack);
    ns= (Trace **)Malloc(sizeof(Trace *)*LastTrack);
    es= (Trace **)Malloc(sizeof(Trace *)*LastTrack);
    garb= (Trace **)Malloc(sizeof(Trace *)*LastTrack);
    if (!zs||!ns||!es||!garb) return;	/* not enuff mem */
    nzs= nns= nes= ngarb= 0;
    /* classify the trace pointers */
    for(i=0; i<= LastTrack; i++) {
	Trace *trc= traces[i];
	BIS3_HEADER *bh= &traces[i]->wave->info;
	char s;

	if(!trc) continue;
	s = bh->channel[2];
	switch (s) {
	case 'Z': case 'V': case 'v': case '1':
	    zs[nzs++]= trc;
	    break;
	case 'N': case 'n': case '2':
	    ns[nns++]= trc;
	    break;
	case 'E': case 'e': case '3':
	    es[nes++]= trc;
	    break;
	default:
	    garb[ngarb++]= trc;
	}
    }
    /* rearrange */
    for(i=0; i< nzs; i++) {
	traces[i]= zs[i];
	traces[i]->itrc= i;
    }
    for(i=0; i< nns; i++) {
	traces[nzs+i]= ns[i];
	traces[nzs+i]->itrc= nzs+i;
    }
    for(i=0; i< nes; i++) {
	traces[nzs+nns+i]= es[i];
	traces[nzs+nns+i]->itrc= nzs+nns+i;
    }
    for(i=0; i< ngarb; i++) {
	traces[nzs+nns+nes+i]= garb[i];
	traces[nzs+nns+nes+i]->itrc= nzs+nns+nes+i;
    }
    free(zs);
    free(ns);
    free(es);
    free(garb);

    RedrawScreen();
    if (ZoomWindowMapped) {
	RedrawZoomWindow("Group_ZNE");
    }
}
