#ifndef lint
static char id[] = "$Id: segy_wave.c,v 1.3 2013/02/28 21:24:57 lombard Exp $";
#endif

/*
 * segy_wave.c--
 *    loading/saving of SEG-Y data files
 *
 * Copyright (c) 1992-3 Andrew K. Yu, Lawrence Berkeley Laboratory
 *      and University of California, Berkeley.
 * All rights reserved.
 */
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#include "instr.h"
#include "proto.h"
#include "segy_head.h"
#include "xv_proto.h"

#define SEGY_Float  1
#define SEGY_Long   2
#define	SEGY_Short  3
#define SEGY_Float2 5	/* same as SEGY_Float */

/*  non-export prototypes  */
static int segy_read_trc(FILE *fp, int kform, struct trace *t_hdr, 
			 float **dataptr, int *total_size, BIS3_HEADER *bh);
static void getStationChannel(struct trace *segyh, char *sname, char *cname);
static void segy2bis(struct reel *reelh, struct trace *segyh, BIS3_HEADER *bh);
static int segy_write_trc(FILE *fp, struct trace *t_hdr, float *data);
static void prepare_reel_header(struct reel *reelh, BIS3_HEADER *bh, int kntr);
static void prepare_trace_header(struct reel *reelh, struct trace *segyh, 
				 BIS3_HEADER *bh);


static int segy_read_trc(FILE *fp, int kform, struct trace *t_hdr, 
			 float **dataptr, int *total_size, BIS3_HEADER *bh)
{
    float *data;
    int nsamp, j;

    if (fread(t_hdr, sizeof(struct trace), 1,fp) != 1) {
	fprintf(stderr,
	    "Error reading in SEG-Y trace header from ");
	return 0;
    }
    *total_size += sizeof(struct trace);	
    nsamp= t_hdr->knsamp;

    *dataptr= data= (float *)Malloc(sizeof(float)*nsamp);
    if (data==NULL) return 2;    /* not enuff mem */

    switch(kform) {
    case SEGY_Float:
    case SEGY_Float2: {
	float min, max;
	int ok;

	ok=fread((char *)data,sizeof(float),nsamp,fp);
	*total_size += sizeof(float)*ok;
	if (ok!=nsamp) {
	    fprintf(stderr,
		"Error reading in SEG-Y trace (float) data from ");
	    return 0;
	}
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
	break;
    }
    case SEGY_Long: {
	int *iarray= (int *)Malloc(sizeof(int)*nsamp);
	int min, max, ok;

	if (iarray==NULL) return 2;	/* not enuff mem */
	ok= fread((char *)iarray, sizeof(int),nsamp,fp);
	*total_size+= sizeof(int)*ok;
	if (ok!=nsamp) {
	    fprintf(stderr,
		"Error reading in SEG-Y trace (int) data from ");
	    return 0;
	}
	min= max= iarray[0];
	data[0]=(float)iarray[0];
	for(j=1; j < nsamp; j++) {
	    int i;
	    i=iarray[j];
	    data[j]=(float)i;
	    if (i < min) {
		min= i;
	    }else if (i > max) {
		max= i;
	    }
	}
	free(iarray);
	bh->min_value.iflag= min;
	bh->max_value.iflag= max;
	bh->format= I32_FORMAT;
	break;
    }
    case SEGY_Short: {
	short *sarray= (short *)Malloc(sizeof(short)*nsamp);
	short min, max;
	int ok;
	if (sarray==NULL) return 2;	/* not enuff mem */
	ok= fread((char *)sarray, sizeof(short),nsamp,fp);
	*total_size+= sizeof(float)*ok;
	if (ok!=nsamp) {
	    fprintf(stderr,
		    "Error reading in SEG-Y trace (short) data from");
	    return 0;
	}
	min= max= sarray[0];
	data[0]=(float)sarray[0];
	for(j=1; j < nsamp; j++) {
	    short s;
	    s=sarray[j];
	    data[j]=(float)s;
	    if (s < min) {
		min= s;
	    }else if (s > max) {
		max= s;
	    }
	}
	free(sarray);
	bh->min_value.sflag= min;
	bh->max_value.sflag= max;
	bh->format= I16_FORMAT;
	break;
    }
    default:
	fprintf(stderr,"Unknown SEG-Y data type %d from \n",
		    kform);
	return 0;
    }
    return 1;
}

int LoadSEGYWave(char *fname, Wave ***waves_ptr)
{
    FILE *fp;
    int n, numWaves=0;
    Wave *wv, **waves;

    int total_size; /* for reporting error */
    
    struct reel reel_header;
    struct trace trace_header;
    char char_header[3200];
    BIS3_HEADER *bh;

    fp=fopen(fname,"rb");
    if(fp==NULL) {
	ReportFileNotFound(fname, 1);
	return 0;
    }

    if (fread(char_header,1,3200,fp)!=3200){
	fprintf(stderr, "Error reading in a SEG-Y header from %s.\n",fname);
	return 0;
    }
    fread(&reel_header, sizeof(struct reel), 1, fp);
    total_size= sizeof(char_header) + sizeof(struct reel);

    *waves_ptr= waves= (Wave **)Malloc(sizeof(Wave *)*reel_header.kntr);
    for(n=0; n<reel_header.kntr; n++) {
	int ok;
	float *data;

	wv= (Wave *)Malloc(sizeof(Wave));
	if (wv==NULL) return numWaves; /* not enuff mem */
	bh=&wv->info;

	/* read header and data for the trace */
	ok= segy_read_trc(fp,reel_header.kform,&trace_header,
			   &data,&total_size,bh);
	if (ok!=1) {
	    if(ok==0) {
		fprintf(stderr,
			"%s\n   total of %d bytes read.\n", fname,total_size);
	    }
	    return numWaves;
	}

	/* prepare header */
	segy2bis(&reel_header,&trace_header,bh);

	wv->data= data;

	*waves= wv;
	waves++;
	numWaves++;
    }
    fclose(fp);
    return numWaves;
}

static void getStationChannel(struct trace *segyh, char *sname, char *cname)
{    
    char *stn;

    /* 93/1/19-- J.P. wants to have 8 chars for station
       names. So the original dum181 (short) before stn
       is taken to be part of the name. The following
       is for compatibility: */
    if (segyh->stn[0]=='\0' && segyh->stn[1]=='\0' &&
	segyh->stn[2]!='\0') {
	/* seems like stn[0] and stn[1] is really "dum181" */
	stn= &segyh->stn[2];
    }else {
	stn= segyh->stn;
    }
    /* if stn[] blank, use trace number as station name */
    if (stn[0]==' ') {
	sprintf(sname,"%d",segyh->kfldtn);
	strcpy(cname, "???");
    }else {
	strncpy(sname, stn, 3);
	sname[3] = '\0';
	strcpy(cname, stn+3);
    }
}

static void segy2bis(struct reel *reelh, struct trace *segyh, BIS3_HEADER *bh)
{
    int i,j;
    STE_TIME ste;
    extern char default_network[NETSIZE];
    
    /* no BMAGIC insert; DumpWave does it */

    getStationChannel(segyh, bh->station, bh->channel);
    strcpy(bh->network,default_network);
    *bh->location='\0';

    bh->dip= 0;
    bh->azimuth= -1.0;
    bh->latitude= 0;
    bh->longitude= 0;
    bh->elevation= 0;
    bh->depth= 0;	/* segy doesn't have depth */

    /***   Data info   ***/
    ste.year = reelh->nyear;
    ste.doy = reelh->nday;
    ste.hour= reelh->nhour;
    ste.minute= reelh->nmin;
    ste.second= reelh->nsec;
    ste.usec= reelh->nmsec * USECS_PER_MSEC;
    bh->start_it = ste_to_sti( ste );

    bh->time_correction= 0;	

    /* format taken care of */

    bh->n_values= segyh->knsamp;
    bh->n_flagged= 0;
    bh->sample_rate= 1000000.0/reelh->sr;	/* reelh->sr in microsecs */

    /***   Data_type info   ***/

    switch (reelh->kform) {
	case SEGY_Float:
	case SEGY_Float2:
	    bh->flag.fflag= F4FLAG;
	    break;
	case SEGY_Long:
	    bh->flag.iflag= I4FLAG;
	    break;
	case SEGY_Short:
	    bh->flag.sflag= I2FLAG;
	    break;
    }

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
    bh->event_latitude=F4FLAG;
    bh->event_longitude= F4FLAG;
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

/**********************************************************************
 *   Previewing                                                       *
 **********************************************************************/

int PreviewSEGY(char *fname, wfmTuple **wfmtup, void **fsi, EvtFile *evt)
{
    FILE *fp;
    long hdrOff, trcOff;
    int n, numWaves=0;

    int total_size; /* for reporting error */
    struct reel reel_header;
    struct trace trace_header;
    char char_header[3200];
    char dummy[10];
    
    wfmTuple *wfm= NULL, *prevwfm= NULL;
    Segy_FSI *segy_fsi;

    fp=fopen(fname,"rb");
    if(fp==NULL) {
	ReportFileNotFound(fname, 1);
	return 0;
    }

    if (fread(char_header,1,3200,fp)!=3200){
	fprintf(stderr, "Error reading in a SEG-Y header from %s.\n",fname);
	return 0;
    }
    fread(&reel_header, sizeof(struct reel), 1, fp);
    total_size= sizeof(char_header) + sizeof(struct reel);

    segy_fsi= (Segy_FSI *)Malloc(sizeof(Segy_FSI));
    *fsi= (void *)segy_fsi;
    segy_fsi->kform= reel_header.kform;
    
    for(n=0; n<reel_header.kntr; n++) {
	int j, nsamp;

	hdrOff= ftell(fp);
        if (fread(&trace_header, sizeof(struct trace), 1,fp) != 1) {
	    fprintf(stderr,
		    "Error reading in SEG-Y trace header from %s\n",fname);
	    fprintf(stderr,
		    "   total of %d bytes read.\n", total_size);
	    return numWaves-1;
	}

	total_size+= sizeof(struct trace);	
	nsamp= trace_header.knsamp;

	/* skipping the data */

	trcOff= ftell(fp);
	switch(reel_header.kform) {
	case SEGY_Float: 
	case SEGY_Float2: {
	    int ok;

	    ok= fseek(fp, sizeof(float)*nsamp, 1);  /* seek forward */
	    total_size+= sizeof(float)*nsamp;
	    if (ok==-1) {
		fprintf(stderr,
		    "Error reading in SEG-Y trace (float) data from %s\n",fname);
		fprintf(stderr,
		    "   total of %d bytes read.\n", total_size);
		return numWaves-1;
	    }
	    break;
	}
	case SEGY_Long: {
	    int ok;

	    ok= fseek(fp, sizeof(int)*nsamp, 1);    /* seek forward */
	    total_size+= sizeof(int)*nsamp;
	    if (ok==-1) {
		fprintf(stderr,
		    "Error reading in SEG-Y trace (int) data from %s\n",fname);
		fprintf(stderr,
		    "   total of %d bytes read.\n", total_size);
		return numWaves-1;
	    }
	    break;
	}
	case SEGY_Short: {
	    int ok;

	    ok= fseek(fp, sizeof(short)*nsamp, 1);    /* seek forward */
	    total_size+= sizeof(float)*nsamp;
	    if (ok==-1) {
		fprintf(stderr,
		    "Error reading in SEG-Y trace (short) data from %s\n",fname);
		fprintf(stderr,
		    "   total of %d bytes read.\n", total_size);
		return numWaves-1;
	    }
	    break;
	}
	default:
	    fprintf(stderr,"Unknown SEG-Y data type %d\n",
		    reel_header.kform);
	    return numWaves;
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
	getStationChannel(&trace_header, wfm->trcName, dummy);
	wfm->evt= evt;
	wfm->hdr_offset= hdrOff;
	wfm->trc_offset= trcOff;

	numWaves++;
    }
    fclose(fp);
    return numWaves;
}

int LoadSEGYWfm(wfmTuple  *wfm, Wave ***waveptr)
{
    FILE *fp;
    Wave *wv;
    int total_size; /* for reporting error */
    struct reel reel_header;
    struct trace trace_header;
    char char_header[3200];
    BIS3_HEADER *bh;
    char *fname= wfm->evt->name;
    int ok;
    float *data;

    fp=fopen(fname,"rb");
    if(fp==NULL) {
	ReportFileNotFound(fname, 1);
	return 0;
    }

    if (fread(char_header,1,3200,fp)!=3200){
	fprintf(stderr, "Error reading in a SEG-Y header from %s.\n",fname);
	return 0;
    }
    fread(&reel_header, sizeof(struct reel), 1, fp);
    total_size= sizeof(char_header) + sizeof(struct reel);

    *waveptr= (Wave **)Malloc(sizeof(Wave *));
    **waveptr = wv= (Wave *)Malloc(sizeof(Wave));
    if (wv==NULL) return 0; /* not enuff mem */
    bh=&wv->info;

    fseek(fp, wfm->hdr_offset, 0);  /* find the right one */
    
    /* read header and data for the trace */
    ok= segy_read_trc(fp,reel_header.kform,&trace_header,
		   &data,&total_size,bh);
    if (ok!=1) {
	if(ok==0) {
	    fprintf(stderr,
		"%s\n   total of %d bytes read.\n", fname,total_size);
	}
	return 0;
    }
    /* prepare header */
    segy2bis(&reel_header,&trace_header,bh);
    wv->data= data;
    fclose(fp);
    return 1;
}

/**********************************************************************
 *   Saving                                                           *
 **********************************************************************/

static int segy_write_trc(FILE *fp, struct trace *t_hdr, float *data)
{
    int nsamp;

    if (fwrite(t_hdr, sizeof(struct trace), 1,fp) != 1) {
	fprintf(stderr,
	    "Error writing out SEG-Y trace header to ");
	return 0;
    }
    nsamp= t_hdr->knsamp;
    if (fwrite((char *)data,sizeof(float),nsamp,fp)!=nsamp) {
	fprintf(stderr,
		"Error writing out SEG-Y trace (float) data to ");
	return 0;
    }
    return 1;
}

/* returns 1 if succeed, 0 if fails */
int SaveSEGYWave(char *fname, Wave **waves, int numWaves)
{
    FILE *fp;
    int n;
    Wave *wv;
    
    struct reel reel_header;
    struct trace trace_header;
    char char_header[3200];
    BIS3_HEADER *bh;

    if (numWaves<=0) return 0;	
    
    fp=fopen(fname,"wb");
    if(fp==NULL) {
	ReportFileNotFound(fname, 0);
	return 0;
    }

    bzero(char_header, 3200);
    if (fwrite(char_header,1,3200,fp)!=3200){
	fprintf(stderr, "Error writing out a SEG-Y header to %s.\n",fname);
	return 0;
    }

    bh= &waves[0]->info;
    prepare_reel_header(&reel_header, bh, numWaves);
    fwrite(&reel_header, sizeof(struct reel), 1, fp);

    for(n=0; n<reel_header.kntr; n++) {
	int ok;

	wv= waves[n];
	bh=&wv->info;

	/* prepare header */
	prepare_trace_header(&reel_header, &trace_header, bh);

	/* write header and data for the trace */
	if(segy_write_trc(fp,&trace_header,wv->data)!=1) {
	    fprintf(stderr,"%s\n", fname);
	    return 0;
	}
    }
    fclose(fp);
    return 1;
}

static void prepare_reel_header(struct reel *reelh, BIS3_HEADER *bh, int kntr)
{
    STE_TIME ste;

    bzero(reelh, sizeof(struct reel));
    ste = sti_to_ste( bh->start_it );
    reelh->nyear= ste.year;
    reelh->nday= ste.day;
    reelh->nhour= ste.hour;
    reelh->nmin= ste.minute;
    reelh->nsec= ste.second;
    reelh->nmsec= ste.usec/USECS_PER_MSEC;
    if(1000000.0/bh->sample_rate > 16384)
	fprintf(stderr,
		"Warning: sampling rate too big to fit into segy header.\n");
    reelh->sr= 1000000.0/bh->sample_rate;
    reelh->kform= SEGY_Float;
    reelh->kntr= kntr;
    reelh->knsamp= bh->n_values;    /* WARNING: might not match the trace's knsamp */
}

static void prepare_trace_header(struct reel *reelh, struct trace *segyh, 
				 BIS3_HEADER *bh)
{
/* make sure the header and reel header matches?? */
    bzero(segyh, sizeof(struct trace));
    strncpy(segyh->stn, bh->station, 4);
    strncpy( (segyh->stn + 3), bh->channel, 4);
    segyh->knsamp= bh->n_values;
}
