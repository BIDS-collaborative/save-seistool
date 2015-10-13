#ifndef lint
static char id[] = "$Id: sac_wave.c,v 1.4 2013/02/28 21:24:58 lombard Exp $";
#endif

/*
 * sac_wave.c--
 *    loading/saving of SAC data files
 *
 * Copyright (c) 1992-3 Andrew K. Yu, Lawrence Berkeley Laboratory
 *      and University of California, Berkeley.
 * All rights reserved.
 */
#include <stdio.h>
#include <strings.h>
#include "qlib2.h"

#include "instr.h"
#include "proto.h"
#include "sachead.h"
#include "xv_proto.h"

extern Trace **traces;

/* from Doug Neuhauser: */
/* a SAC structure containing all null values */
static struct SAChead sac_null = {
-12345., -12345., -12345., -12345., -12345.,
-12345., -12345., -12345., -12345., -12345.,
-12345., -12345., -12345., -12345., -12345.,
-12345., -12345., -12345., -12345., -12345.,
-12345., -12345., -12345., -12345., -12345.,
-12345., -12345., -12345., -12345., -12345.,
-12345., -12345., -12345., -12345., -12345.,
-12345., -12345., -12345., -12345., -12345.,
-12345., -12345., -12345., -12345., -12345.,
-12345., -12345., -12345., -12345., -12345.,
-12345., -12345., -12345., -12345., -12345.,
-12345., -12345., -12345., -12345., -12345.,
-12345., -12345., -12345., -12345., -12345.,
-12345., -12345., -12345., -12345., -12345.,
-12345, -12345, -12345, -12345, -12345,
-12345, -12345, -12345, -12345, -12345,
-12345, -12345, -12345, -12345, -12345,
-12345, -12345, -12345, -12345, -12345,
-12345, -12345, -12345, -12345, -12345,
-12345, -12345, -12345, -12345, -12345,
-12345, -12345, -12345, -12345, -12345,
(unsigned)-12345, (unsigned)-12345, (unsigned)-12345, (unsigned)-12345, (unsigned)-12345,
{ '-','1','2','3','4','5',' ',' ' },
{ '-','1','2','3','4','5',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ' },
{ '-','1','2','3','4','5',' ',' ' }, { '-','1','2','3','4','5',' ',' ' },
{ '-','1','2','3','4','5',' ',' ' }, { '-','1','2','3','4','5',' ',' ' },
{ '-','1','2','3','4','5',' ',' ' }, { '-','1','2','3','4','5',' ',' ' },
{ '-','1','2','3','4','5',' ',' ' }, { '-','1','2','3','4','5',' ',' ' },
{ '-','1','2','3','4','5',' ',' ' }, { '-','1','2','3','4','5',' ',' ' },
{ '-','1','2','3','4','5',' ',' ' }, { '-','1','2','3','4','5',' ',' ' },
{ '-','1','2','3','4','5',' ',' ' }, { '-','1','2','3','4','5',' ',' ' },
{ '-','1','2','3','4','5',' ',' ' }, { '-','1','2','3','4','5',' ',' ' },
{ '-','1','2','3','4','5',' ',' ' }, { '-','1','2','3','4','5',' ',' ' },
{ '-','1','2','3','4','5',' ',' ' }, { '-','1','2','3','4','5',' ',' ' },
{ '-','1','2','3','4','5',' ',' ' }
};

/*  non-export prototypes  */
static void sac2bis(struct SAChead *sach, BIS3_HEADER *bh);


int LoadSACWave(char *fname, Wave ***waves_ptr)
{
    FILE *fp;
    Wave *wv;
    struct SAChead sach;
    BIS3_HEADER *bh;

    int nsamp, j;
    float min,max, *data;

    if ((fp=fopen(fname,"rb"))==NULL) {
	ReportFileNotFound(fname, 1);
	return 0;
    }

    /* read in header */
    if (fread(&sach, sizeof(sach), 1, fp)!=1) {
	fprintf(stderr,
	    "Error reading in SAC header from %s\n",fname);
	return 0;
    }

    /* check if it's time series & evenly spaced */
    if (sach.iftype!=1) {
	fprintf(stderr,
	    "Error: SAC file not a time series\n");
	return 0;
    }
    if (sach.leven!=1) {
	fprintf(stderr,
	    "Error: I don't handle unevenly spaced SAC file\n");
	return 0;
    }

    wv= (Wave *)Malloc(sizeof(Wave));
    if (wv==NULL) return 0; /* not enuff mem */
    bh=&wv->info;
    bzero(bh, sizeof(BIS3_HEADER));

    nsamp= sach.npts;
    data= (float *)Malloc(sizeof(float)*nsamp);
    if (data==NULL) return 0;    /* not enuff mem */

    fread((char *)data,sizeof(float),nsamp,fp);
    min= max= data[0];
    for(j=1; j < nsamp; j++) {
	if (data[j] < min) {
	    min= data[j];
	}else if (data[j] > max) {
	    max= data[j];
	}
    }
    bh->min_value.fflag= min;
    bh->max_value.fflag= max;	    
    bh->format= R32_FORMAT;

    /* prepare header */
    sac2bis(&sach, bh);

    wv->data= data;
    *waves_ptr= (Wave **)Malloc(sizeof(Wave *));
    **waves_ptr= wv;

    fclose(fp);
    return 1;	/* 1 trace */
}

static void sac2bis(struct SAChead *sach, BIS3_HEADER *bh)
{
    int i,j;
    STE_TIME ste;
    STI_TIME sti;
    
    /* no BMAGIC insert; done in DumpWave */

    /* assume K_LEN >= STATSIZE: */
    strncpy(bh->station, sach->kstnm, STATSIZE);
    bh->station[STATSIZE]='\0';
    trim(bh->station);
    
    if (strncmp(sach->knetwk, "-12345", 6) != 0) {
	strncpy(bh->network, sach->knetwk, NETSIZE);
	bh->network[NETSIZE-1] = '\0';
    }
    else
	bh->network[0] = '\0';
    trim(bh->network);
    
    /* seistool expects kuser0 field to contain the SEED */
    /* channel name; if it's not there, it'll just take the component name */
    if (!strncmp(sach->kuser0,"-12345",6) && !strncmp(sach->kcmpnm,"-12345",6)) {
	sprintf(bh->channel,"XX%c",sach->kcmpnm[0]);
    } else if (!strncmp(sach->kcmpnm,"-12345",6)) {
	strncpy(bh->channel,sach->kuser0,3);
    } else {
      strncpy(bh->channel,sach->kcmpnm,3);
    }
    trim(bh->channel);
    
    if (strncmp(sach->khole, "-12345", 6) != 0) {
	strncpy(bh->location, sach->khole, LOCSIZE);
	bh->location[LOCSIZE-1] = '\0';
    }
    else
	*bh->location='\0';
    trim(bh->location);
    
    if (sach->cmpinc != -12345.0)
	bh->dip= sach->cmpinc;
    if (sach->cmpaz != -12345.0)
	bh->azimuth= sach->cmpaz;
    if (sach->stla != -12345.0)
	bh->latitude= sach->stla;
    if (sach->stlo != -12345.0)
	bh->longitude= sach->stlo;
    bh->elevation= 0;
    bh->depth= 0;	/* sachead doesn't have depth */

    /* *** Data info  ***
     * Due to the SAC B field time offset, I don't have
     * a better way of handling this:
     */
    ste.year = sach->nzyear;
    ste.doy = sach->nzjday;
    ste.hour = sach->nzhour;
    ste.minute = sach->nzmin;
    ste.second = sach->nzsec;
    ste.usec = sach->nzmsec * USECS_PER_MSEC;
    sti = ste_to_sti( ste );
    bh->start_it = st_add_dtime( sti, (double)sach->b * USECS_PER_SEC);
    
    bh->time_correction= 0;	
    /* format taken care of */

    bh->n_values= sach->npts;
    bh->n_flagged= 0;
    bh->sample_rate= 1.0/sach->delta;

    /***   Data_type info   ***/
    bh->flag.fflag= F4FLAG;

    /***   poles & zeros   ***/
    bh->response=RESP_NOTFOUND;
    bh->digital_sens=0;
    bh->gain_factor= 0;
    bh->n_poles= 0;
    bh->n_zeros= 0;
    for(j=0;j < NUMPOLES; j++) {
	bh->poles_zeros[j].real = 0.;
	bh->poles_zeros[j].imag = 0.;
    }

    /***   Event_1   ***/
    if (sach->evla != -12345.0)
	bh->event_latitude= sach->evla;
    else
	bh->event_latitude = F4FLAG;
    if (sach->evlo != -12345.0)
	bh->event_longitude= sach->evlo;
    else
	bh->event_longitude = F4FLAG;
    bh->event_depth= F4FLAG;
    bh->event_origin_it.year= I4FLAG;
    bh->event_origin_it.second= I4FLAG;
    bh->event_origin_it.usec= I4FLAG;
    *bh->event_agency= '\0';
    bh->event_delta= F4FLAG;
    bh->event_azimuth= F4FLAG;
    for(j=0;j < MAGSIZE; j++) {
	bh->magnitude[j].value= F4FLAG;
    }
    for(j=0; j < MAGSIZE; j++) {
	*bh->magnitude[j].type= '\0';
    }
    bh->flinn_engdahl_region= I4FLAG;
    *bh->source_location_description= '\0';

    /***   Moment_1   ***/
    for(j=0; j <NUMOMENT; j++) {
	bh->moment[j]= F4FLAG;
    }
    bh->fault_plane.strike= I4FLAG;
    bh->fault_plane.dip= I4FLAG;
    bh->fault_plane.rake= I4FLAG;
    bh->fault_plane.m0= F4FLAG;
    bh->source_duration= I4FLAG;
    bh->centroid_time_offset= I4FLAG;
    *bh->moment_agency= '\0';
}


void WriteSAC(char *fname, int itrc)
{
    Trace *trc= traces[itrc];
    BIS3_HEADER *bh= &trc->wave->info;
    struct SAChead sach;
    FILE *fp;
    STE_TIME ste;
    
    int i;

    sach= sac_null; /* initialize */

    if((fp=fopen(fname,"wb"))==NULL) {
	fprintf(stderr,
		"Cannot open %s for writing.\n",fname);
	return;
    }
    
    strcpy(sach.kstnm, bh->station);
    for(i=strlen(sach.kstnm); i< K_LEN; i++) {
	sach.kstnm[i]=' '; /* blank pad */
    }
    strcpy(sach.knetwk, bh->network);
    for(i=strlen(sach.knetwk); i< K_LEN; i++) {
	sach.knetwk[i]=' '; /* blank pad */
    }
    strncpy(sach.kuser0,bh->channel,3);
    for(i=3; i< K_LEN; i++) {
	sach.kuser0[i]=' '; /* blank pad */
    }
    sach.kcmpnm[0]= bh->channel[2];
    for(i=1; i< K_LEN; i++) {
	sach.kcmpnm[i]=' '; /* blank pad */
    }
    if (strlen(bh->location) > 0) {
	strcpy(sach.khole, bh->location);
	for(i=strlen(sach.khole); i< K_LEN; i++) {
	    sach.khole[i]=' '; /* blank pad */
	}
    }
    
    if (bh->azimuth >= 0.0) {
	sach.cmpinc= bh->dip;
	sach.cmpaz= bh->azimuth;
    }
    sach.stla= bh->latitude;
    sach.stlo= bh->longitude;

    ste = sti_to_ste(bh->start_it);
    sach.nzyear= ste.year;
    sach.nzjday= ste.doy;
    sach.nzhour= ste.hour;
    sach.nzmin= ste.minute;
    sach.nzsec= ste.second;
    sach.nzmsec= 0;
    sach.npts= bh->n_values;
    sach.delta= 1.0/bh->sample_rate;
    
    /* write out the header */
    sach.b= (float)ste.usec / USECS_PER_SEC;
    sach.e= sach.npts/bh->sample_rate - sach.b;
    sach.iftype= 1; /* time series */
    sach.leven= 1; /* evenly spaced */
    sach.internal4= SACVERSION;	/* header version number */

    fwrite(&sach,sizeof(sach),1,fp);
    fwrite(trc->wave->data, sizeof(float), sach.npts, fp);
    fclose(fp);
}
