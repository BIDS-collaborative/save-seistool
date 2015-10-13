#ifndef lint
static char id[] = "$Id: bis_wave.c,v 1.3 2013/02/28 21:25:00 lombard Exp $";
#endif

/*
 * bis_wave.c--
 *    loading of BIS and BIS3 data files
 *
 * Copyright (c) 1992-3 Andrew K. Yu, Lawrence Berkeley Laboratory
 *      and University of California, Berkeley.
 * All rights reserved.
 */

#include <strings.h>
#include <stdio.h>
#include <stdlib.h>
#include <rpc/types.h>
#include <rpc/xdr.h>
#include "bis_proto.h"
#include "proto.h"
#include "xv_proto.h"

char default_network[NETSIZE];
static void bis1_to_3(BIS_HEADER *bh, BIS3_HEADER *bh3);

int strib(char *string) /* strip trailing blanks or newline */
{
     int i,length;
     length = strlen(string);
     if (length == 0) {
         return 0;
     } else {
         for(i = length-1; i >= 0; i--) {
             if (string[i] == ' ' || string[i] == '\n') {
                 string[i] = '\0';
             } else {
                 return 0;
             }
         }
         return 0;
    }
} /* strib */

int LoadBISWave(char *fname, Wave ***waves_ptr)
{
    FILE *fp;
    XDR xdrs;
    DATE start_date, event_date;
    bool_t ok=TRUE;
    int n, num= 0;
    int maxWaves= 128;	    /* arbitrary */
    Wave *wv, **waves;

    /* create XDR stream */
    fp=fopen(fname,"r");
    if(fp==NULL) {
	ReportFileNotFound(fname, 1);
	return 0;
    }
    xdrstdio_create(&xdrs, fp, XDR_DECODE);
    /* create waves */
    *waves_ptr= waves= (Wave **)Malloc(sizeof(Wave *)*maxWaves);
    if (waves==NULL) return 0;	/* not enuff mem */
    while(1) {
	wv= (Wave *)Malloc(sizeof(Wave));
	
	/* try to read in the magic cookie and version number */
        if (xdr_MAGIC(&xdrs, &wv->info)==FALSE ) {
	    free(wv);
	    goto Cleanup;  /* no more BIS_HEADER */
	}
	if (wv->info.magic != BMAGIC) {
	    fprintf(stderr, "%s is not a BIS file!\n", fname);
	    free(wv);
	    goto Cleanup;
	}
	
	switch (wv->info.version) {
	case BVERSION3:
	    if (xdr_BIS3_HEADER_before(&xdrs, &wv->info)==FALSE ) {
		free(wv);
		goto Cleanup;  /* no more BIS_HEADER */
	    }
	    if (xdr_DATE(&xdrs, &start_date)==FALSE ) {
		free(wv);
		goto Cleanup;
	    }
	    if (xdr_BIS3_HEADER_middle(&xdrs, &wv->info)==FALSE ) {
		free(wv);
		goto Cleanup;
	    }
	    if (xdr_DATE(&xdrs, &event_date)==FALSE ) {
		free(wv);
		goto Cleanup;
	    }
	    if (xdr_BIS3_HEADER_after(&xdrs, &wv->info)==FALSE ) {
		free(wv);
		goto Cleanup;
	    }
	    break;
	    
	case BVERSION1:
	    {
		BIS_HEADER bh;
		if (xdr_BIS_HEADER_before(&xdrs, &bh)==FALSE ) {
		    free(wv);
		    goto Cleanup;  /* no more BIS_HEADER */
		}
		if (xdr_DATE(&xdrs, &start_date)==FALSE ) {
		    free(wv);
		    goto Cleanup;
		}
		if (xdr_BIS_HEADER_middle(&xdrs, &bh)==FALSE ) {
		    free(wv);
		    goto Cleanup;
		}
		if (xdr_DATE(&xdrs, &event_date)==FALSE ) {
		    free(wv);
		    goto Cleanup;
		}
		if (xdr_BIS_HEADER_after(&xdrs, &bh)==FALSE ) {
		    free(wv);
		    goto Cleanup;
		}
		fprintf(stderr, "%s is BIS version %d; assuming network %s\n",
			fname, wv->info.version, default_network);
		bis1_to_3( &bh, &wv->info );
	    }
	    break;
	default:
	    fprintf(stderr, "%s is unknown version BIS: %d\n", fname,
		    wv->info.version);
	    free(wv);
	    goto Cleanup;
	}
	strib(wv->info.location);
	if (wv->info.location[0] == '-') wv->info.location[0] = '\0';
	wv->info.start_it = date_to_sti( start_date );
	if (event_date.day == 0 || event_date.day == I4FLAG) { /* valid days are 1 - 366 */
	    wv->info.event_origin_it.year = I4FLAG;
	    wv->info.event_origin_it.second = I4FLAG;
	    wv->info.event_origin_it.usec = I4FLAG;
	}
	else
	    wv->info.event_origin_it = date_to_sti( event_date );
	
	/* read in sampled values */
	if (wv->info.n_values!=0) {
	    int j;
	    int npts=wv->info.n_values;
	    float *data, *array;

	    data= array= (float *)Malloc(sizeof(float)*npts);
	    if (data==NULL) return num;   /* not enuff mem */
	    ok= TRUE;
	    switch(wv->info.format) {
	    case I16_FORMAT: {
		short s,min,max;

		ok &= xdr_short(&xdrs, &s);
		min= max= s;
		*array++=(float)s;
		for(j=1; j < npts; j++) {
		    ok &= xdr_short(&xdrs, &s);
		    if (s < min) {
			min= s;
		    }else if (s > max) {
			max= s;
		    }
		    *array= (float)s;
		    array++;
		}
		wv->info.min_value.sflag= min;
		wv->info.max_value.sflag= max;
		wv->data= data;
		break;
	    }
	    case I32_FORMAT: {
		int i,min,max;

		ok &= xdr_int(&xdrs, &i);
		min= max= i;
		*array++=(float)i;
		for(j=1; j < npts; j++) {
		    ok &= xdr_int(&xdrs, &i);
		    if (i < min) {
			min= i;
		    }else if (i > max) {
			max= i;
		    }
		    *array= (float) i;
		    array++;
		}
		wv->info.min_value.iflag= min;
		wv->info.max_value.iflag= max;
		wv->data= data;
		break;
	    }
	    case R32_FORMAT: {
		float f,min,max;

		ok &= xdr_float(&xdrs, &f);
		min= max= f;
		*array++=f;
		for(j=1; j < npts; j++) {
		    ok &= xdr_float(&xdrs, &f);
		    if (f < min) {
			min= f;
		    }else if (f > max) {
			max= f;
		    }
		    *array= f;
		    array++;
		}
		wv->info.min_value.fflag= min;
		wv->info.max_value.fflag= max;
		wv->data= data;
		break;
	    }
	    case R64_FORMAT: {
		double d,min,max;

		ok &= xdr_double(&xdrs, &d);
		min= max= d;
		*array++=(float)d;
		for(j=1; j < npts; j++) {
		    ok &= xdr_double(&xdrs, &d);
		    if (d < min) {
			min= d;
		    }else if (d > max) {
			max= d;
		    }
		    *array= (float)d;
		    array++;
		}
		wv->info.min_value.dflag= min;
		wv->info.max_value.dflag= max;
		wv->data= data;
		break;
	    }
	    default: /* unknown format */
		fprintf(stderr, "ReadBISData: unknown data format.\n");
		break;
	    }
	    if (ok==FALSE) {
		fprintf(stderr, "Warning: error reading in data from %s.\n",
			fname);
	    }
	}
	waves[num++]= wv;
	if(num==maxWaves) {
	    maxWaves*= 2;
	    waves= (Wave **)Realloc((char*)waves, sizeof(Wave *)*maxWaves);
	    if(waves==NULL) return num;
	    *waves_ptr= waves;
	}
    }

 Cleanup:
    xdr_destroy(&xdrs);
    fclose(fp);
    return num;
}

/**********************************************************************
 *   Previewing                                                       *
 **********************************************************************/

int PreviewBIS(char *fname,  wfmTuple **wfmtup, void **fsi, EvtFile *evt )
{
    FILE *fp;
    long hdrOff, trcOff;
    XDR xdrs;
    bool_t ok=TRUE;
    int n, numWaves=0;
    BIS3_HEADER bh3;
    DATE start_date, event_date;
    
    wfmTuple *wfm= NULL, *prevwfm= NULL;

    fp=fopen(fname,"r");
    if(fp==NULL) {
	ReportFileNotFound(fname, 1);
	return 0;
    }
    xdrstdio_create(&xdrs, fp, XDR_DECODE);

    *fsi= NULL;
    while(1) {
	hdrOff= xdr_getpos(&xdrs);	/* header position */

	/* try to read in the magic cookie and version number */
        if (xdr_MAGIC(&xdrs, &bh3)==FALSE ) {
	    goto Cleanup;  /* no more BIS_HEADER */
	}
	if (bh3.magic != BMAGIC) {
	    fprintf(stderr, "%s is not a BIS file!\n", fname);
	    goto Cleanup;
	}
	
	switch (bh3.version) {
	case BVERSION3:
	    if (xdr_BIS3_HEADER_before(&xdrs, &bh3)==FALSE ) {
		goto Cleanup;  /* no more BIS_HEADER */
	    }
	    if (xdr_DATE(&xdrs, &start_date)==FALSE ) {
		goto Cleanup;
	    }
	    if (xdr_BIS3_HEADER_middle(&xdrs, &bh3)==FALSE ) {
		goto Cleanup;
	    }
	    if (xdr_DATE(&xdrs, &event_date)==FALSE ) {
		goto Cleanup;
	    }
	    if (xdr_BIS3_HEADER_after(&xdrs, &bh3)==FALSE ) {
		goto Cleanup;
	    }
	    break;
	case BVERSION1:
	    {
		BIS_HEADER bh;
		if (xdr_BIS_HEADER_before(&xdrs, &bh)==FALSE ) {
		    goto Cleanup;  /* no more BIS_HEADER */
		}
		if (xdr_DATE(&xdrs, &start_date)==FALSE ) {
		    goto Cleanup;
		}
		if (xdr_BIS_HEADER_middle(&xdrs, &bh)==FALSE ) {
		    goto Cleanup;
		}
		if (xdr_DATE(&xdrs, &event_date)==FALSE ) {
		    goto Cleanup;
		}
		if (xdr_BIS_HEADER_after(&xdrs, &bh)==FALSE ) {
		    goto Cleanup;
		}
		fprintf(stderr, "%s is BIS version %d; assuming network %s\n",
			fname, bh.version, default_network);
		bis1_to_3( &bh, &bh3 );
	    }
	    break;

	default:
	    fprintf(stderr, "%s is unknown version BIS: %d\n", fname,
		    bh3.version);
	    goto Cleanup;
	}

	trcOff= xdr_getpos(&xdrs);

	/* looks like we're going to have to read the
	   sample values in-- this is gonna be painful! */

	if (bh3.n_values!=0) {
	    int j;
	    int npts=bh3.n_values;

	    ok= TRUE;
	    switch(bh3.format) {
	    case I16_FORMAT: {
		short s;
		for(j=0; j < npts; j++) {
		    ok &= xdr_short(&xdrs, &s);
		}
		break;
	    }
	    case I32_FORMAT: {
		int i;
		for(j=0; j < npts; j++) {
		    ok &= xdr_int(&xdrs, &i);
		}
		break;
	    }
	    case R32_FORMAT: {
		float f;
		for(j=0; j < npts; j++) {
		    ok &= xdr_float(&xdrs, &f);
		}
		break;
	    }
	    case R64_FORMAT: {
		double d;
		for(j=0; j < npts; j++) {
		    ok &= xdr_double(&xdrs, &d);
		}
		break;
	    }
	    default: /* unknown format */
		fprintf(stderr, "ReadBISData: unknown data format.\n");
		break;
	    }
	    if (ok==FALSE) {
		fprintf(stderr, "Warning: error reading in data from %s.\n",
			fname);
	    }
	}
	
	/* fill in the tuple */
	wfm= (wfmTuple *)Malloc(sizeof(wfmTuple));
	if (prevwfm) {
	    prevwfm->next= wfm;
	    prevwfm= wfm;
	}else {
	    *wfmtup= prevwfm= wfm;
	}
	wfm->next= NULL;
	sprintf(wfm->trcName, "%s %s %s %s", bh3.station, bh3.network,
		bh3.channel, bh3.location);
	wfm->evt= evt;
	wfm->hdr_offset= hdrOff;
	wfm->trc_offset= trcOff;

	numWaves++;
    }

 Cleanup:
    fclose(fp);
    return numWaves;
}

static void bis1_to_3(BIS_HEADER *bh, BIS3_HEADER *bh3)
{
    int i;

    strncpy(bh3->station, bh->station, STATSIZE);
    bh3->station[STATSIZE-1] = '0';
    strcpy(bh3->network, default_network);
    strncpy(bh3->channel, bh->channel, CHSIZE);
    bh3->channel[CHSIZE-1] = '\0';
    strncpy(bh3->location, bh->location, LOCSIZE);
    bh3->location[LOCSIZE-1] = '\0';
    bh3->dip = bh->dip;
    bh3->azimuth = bh->azimuth;
    bh3->latitude = bh->latitude;
    bh3->longitude = bh->longitude;
    bh3->elevation = bh->elevation;
    bh3->depth = bh->depth;
    bh3->time_correction = bh->time_correction;
    bh3->format = bh->format;
    bh3->n_values = bh->n_values;
    bh3->n_flagged = bh->n_flagged;
    bh3->sample_rate = bh->sample_rate;
    bh3->flag = bh->flag;
    bh3->min_value = bh->min_value;
    bh3->max_value = bh->max_value;
    bh3->response = bh->response;
    bh3->digital_sens = bh->digital_sens;
    bh3->gain_factor = bh->gain_factor;
    bh3->n_poles = bh->n_poles;
    bh3->n_zeros = bh->n_zeros;
    for (i = 0; i < NUMPOLES; i++)
	bh3->poles_zeros[i] = bh->poles_zeros[i]; /* struct copy */
    bh3->event_latitude = bh->event_latitude;
    bh3->event_longitude = bh->event_longitude;
    bh3->event_depth = bh->event_depth;
    strncpy(bh3->event_agency, bh->event_agency, EVAGYSIZE);
    bh3->event_delta = bh->event_delta;
    bh3->event_azimuth = bh->event_azimuth;
    for (i = 0; i < MAGSIZE; i++) {
	bh3->magnitude[i].value = bh->magnitude[i].value;
	strncpy(bh3->magnitude[i].type, bh->magnitude[i].type,
		MAGTYPESIZE);
    }
    bh3->flinn_engdahl_region = bh->flinn_engdahl_region;
    strncpy(bh3->source_location_description,
	    bh->source_location_description, SRCDSIZE);
    for (i = 0; i < NUMOMENT; i++)
	bh3->moment[i] = bh->moment[i];
    bh3->fault_plane = bh->fault_plane;  /* structure copy */
    bh3->source_duration = bh->source_duration;
    bh3->centroid_time_offset = bh->centroid_time_offset;
    strncpy(bh3->moment_agency, bh->moment_agency, NMOMAGY);
    
    return;
}
