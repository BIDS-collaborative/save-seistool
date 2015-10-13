#ifndef lint
static char id[] = "$Id: bcat.c,v 1.2 2013/02/28 21:25:01 lombard Exp $";
#endif

/*
 * bcat.c--
 *    printing of a BIS3_HEADER header; modified from bcat.c in the
 *    BIS package.
 *
 * Copyright (c) 1992-3 Andrew K. Yu, Lawrence Berkeley Laboratory
 *      and University of California, Berkeley.
 * All rights reserved.
 */

#include <stdio.h>
#include <strings.h>
#include "bis3_header.h"
#include "proto.h"

static void printUVAL(int f, UVAL *u);
static void printCOMPLEX(COMPLEX *c);
static void printMAGNITUDE(MAGNITUDE *m);


void printhead(BIS3_HEADER *bp, int level)
{
    int i;
    char *s, tmestr[50];
    float test_f=F4FLAG;
    
    Printf(":: Magic 0x%x (ver %d)\n",bp->magic, bp->version);

    Printf("Station and channel (or stream) identification.\n");
    Printf(":: station name: %s  ", bp->station);
    Printf("network: %s  ", bp->network);
    Printf("channel name: %s\n", bp->channel[0] == '\0' ? "--" : bp->channel);
    Printf("location: %s  ", bp->location[0] == '\0' ? "--" : bp->location);
    Printf(":: dip= %f  azimuth= %f\n", bp->dip, bp->azimuth);
    Printf(":: lat= %f  long= %f  elev= %f  depth= %f\n",
	   bp->latitude, bp->longitude, bp->elevation, bp->depth);

    Printf("Data information.\n");
    sprintTime( tmestr, bp->start_it);
    Printf(":: %s\n", tmestr);
    Printf(":: time correction= %d\n",bp->time_correction);
    switch(bp->format) {
    case I16_FORMAT: s= "I16_FORMAT"; break;
    case I32_FORMAT: s= "I32_FORMAT"; break;
    case R32_FORMAT: s= "R32_FORMAT"; break;
    case R64_FORMAT: s= "R64_FORMAT"; break;
    default: s= "unknown";
    }
    Printf(":: format: %s\n",s);
    Printf(":: number of data values: %d\n", bp->n_values);
    Printf(":: n_flagged: %d\n",bp->n_flagged);
    Printf(":: sampling rate= %f samples per sec\n", bp->sample_rate);
    
    Printf("Data type information.\n");
    Printf(":: flag:");
    printUVAL(bp->format, &bp->flag);
    Printf("\n:: min value: ");
    printUVAL(bp->format,&bp->min_value);
    Printf(" max value: ");
    printUVAL(bp->format,&bp->max_value);
    Printf("\n");

    Printf("Instrument Responses.\n");
    switch(bp->response){
    case 0:  s="displacement"; break;
    case 1:  s="velocity"; break;
    case 2:  s="acceleration"; break;
    case -1:
    default:
	s="unknown"; break;
    }
    Printf(":: response= %s\n",s);
    Printf(":: digital sens= %g  ", bp->digital_sens);
    Printf(":: gain factor= %g\n", bp->gain_factor);
    Printf(":: %d instrument poles:\n", bp->n_poles);
    for(i=0; i< bp->n_poles; i++) {
	Printf(":: pole[%d]  ",i);
	printCOMPLEX(&bp->poles_zeros[i]);
	Printf("\n");
    }
    Printf(":: %d instrument zeros:\n", bp->n_zeros);
    for(i=0; i< bp->n_zeros; i++) {
	Printf(":: zero[%d] ",i);
	printCOMPLEX(&bp->poles_zeros[i+bp->n_poles]);
	Printf("\n");
    }

    if(level>0) return;

    if (bp->event_latitude!=test_f) {
	Printf("Event information.\n");
	Printf(":: lat: %f  long: %f  ",bp->event_latitude,
	       bp->event_longitude);
	if ( bp->event_depth != test_f) Printf("depth: %f", bp->event_depth);
	Printf("\n");

	if (bp->event_origin_it.year!=I4FLAG) {
	    sprintTime( tmestr, bp->event_origin_it);
	    Printf(":: OT:  %s\n",tmestr);
	}

	if (strlen(bp->event_agency)>0) Printf(":: event agency: %s\n", bp->event_agency);
	Printf(":: delta= %f  azimuth %f\n", bp->event_delta,bp->event_azimuth);

	if (test_f!=bp->magnitude[0].value) {
	    Printf(":: magnitude:\n");
	    for(i=0; i<MAGSIZE; i++) {
		if (bp->magnitude[i].value!=test_f) {
		    Printf(":: [%d] ",i);
		    printMAGNITUDE(&bp->magnitude[i]);
		    Printf("\n");
		}
	    }
	}

	if (bp->flinn_engdahl_region!=I4FLAG) {
	    Printf(":: flinn engdahl region= %d\n",bp->flinn_engdahl_region);
	    Printf(":: source loc description: %s\n", bp->source_location_description);
	}

	if (bp->moment[0]!=test_f) {
	    Printf("Moment tensor info.	\n");
	    Printf(":: moment tensor coord\n");
	    for(i=0; i<NUMOMENT; i++) {
		Printf(":: [%d] %f\n",i, bp->moment[i]);
	    }
	    Printf(":: fault plane solution:\n");
	    Printf(":: strike= %d   dip= %d   rake= %d\n",
		   bp->fault_plane.strike, bp->fault_plane.dip,
		   bp->fault_plane.rake);
	    Printf(":: moment= %f\n", bp->fault_plane.m0);
	    Printf(":: source duration= %d\n", bp->source_duration);
	    Printf(":: centroid time offset= %d\n", bp->centroid_time_offset);
	    Printf(":: moment agency= %s\n", bp->moment_agency);
	}
    }
}


static void printUVAL(int f, UVAL *u)
{
    switch(f) {
    case I16_FORMAT:
	Printf("%d",(int)u->sflag);
	break;
    case I32_FORMAT:
	Printf("%d",u->iflag);
	break;
    case R32_FORMAT:
	Printf("%f",u->fflag);
	break;
    case R64_FORMAT:
	Printf("%lf",u->dflag);
	break;
    default:
	break;
    }
}

static void printCOMPLEX(COMPLEX *c)
{
    Printf("(%f, %f)", c->real, c->imag);
}

static void printMAGNITUDE(MAGNITUDE *m)
{
    Printf("val= %f  type= %s",m->value, m->type);
}

