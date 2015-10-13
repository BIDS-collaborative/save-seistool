#ifndef lint
static char id[] = "$Id: trc_wave.c,v 1.3 2013/02/28 21:24:56 lombard Exp $";
#endif

/*
 * trc_wave.c--
 *    loading of TRC data files
 *
 * Copyright (c) 1992-3 Andrew K. Yu, Lawrence Berkeley Laboratory
 *      and University of California, Berkeley.
 * All rights reserved.
 */
#include <math.h>
#include <stdio.h>
#include <strings.h>
#include "instr.h"
#include "proto.h"
#include "xv_proto.h"

/*  Modifications:
 *  [1] 10/8 92-- take trc files with station names in header
 */

/**********************************************************************
 *   Extra state variables                                            *
 **********************************************************************/

/*
 *  EXTRA INPUT NECESSARY:
 *	statfile-- a station list file
 *	srate--	   sampling rate
 *	datatype-- float or int or short?
 */
 
/* some housekeeping info */
#define TRC_dataFloat	1
#define TRC_dataInt	2
#define TRC_dataShort	3
#define INIT_MAX_STAT   128

static int oldStat=0;
static int numStat=0;
static int statfile_Specified=0;
static int old_statfile_Specified=0;
static char **oldStation;   /* quick fix */
static char **theStation;
static int TRC_dataType= TRC_dataFloat;
static float old_srate=0.0;
static float TRC_srate=0.0;

/* prototypes */
static void readTRCfloat(FILE *fp, int *nsamp_ptr, float **data);
static void readTRCint(FILE *fp, int *nsamp_ptr, int **data);
static void readTRCshort(FILE *fp, int *nsamp_ptr, short **data);
static void trc2bis(int itrace, int nsamp, BIS3_HEADER *bh);
static void Demand_callback(char *fname, FileFormat format);
static int check_TRC_setup();
static void parseStatFile(char *fname);
static int parseHeader(FILE *fp, int ntrace, int nbyte);


/**********************************************************************
 *   Reading in data (of various types)                               *
 **********************************************************************/
/* cf. /m/home/u/barstow/xtest/5kest26s.d:   short data.
       /m/home/u/barstow/progs/uw/trctouw.c: format */

/* Reading a Trace from the TRC file */
static void readTRCfloat(FILE *fp, int *nsamp_ptr, float **data)
{
    int nbyte,nsamp, itmp;
    char ndum[1000];

    /* note: only the crucial fread is check for error */
    fread(&itmp,sizeof(int),1,fp);
    nbyte=itmp-4;
    if (fread(&nsamp,sizeof(int),1,fp)==0) 
	return;
    if (nbyte) fread(ndum,sizeof(char),nbyte,fp);
    fread((char *)&itmp, sizeof(int), 1, fp);
    *nsamp_ptr= nsamp;
    
    *data= (float *)Malloc(sizeof(float) * nsamp);
    if (*data==NULL) return;	/* not enuff mem */
#ifdef DEBUG
printf("Reading in a trace: %d samples\n",nsamp);
#endif

    fread(&nbyte,sizeof(int),1,fp);
    if (fread(*data, sizeof(float), nsamp, fp)==0)
	return;
    fread(&nbyte,sizeof(int),1,fp);
}

static void readTRCint(FILE *fp, int *nsamp_ptr, int **data)
{
    int nbyte,nsamp,itmp;
    char ndum[1000];

    /* note: only the crucial fread is check for error */
    fread(&itmp,sizeof(int),1,fp);
    nbyte=itmp-4;
    if (fread(&nsamp,sizeof(int),1,fp)==0) 
	return;
    if (nbyte) fread(ndum,sizeof(char),nbyte,fp);
    fread((char *)&itmp, sizeof(int), 1, fp);
    *nsamp_ptr= nsamp;

    *data= (int *)Malloc(sizeof(int) * nsamp);
    if (*data==NULL) return;	/* not enuff mem */
#ifdef DEBUG
printf("Reading in a trace: %d samples\n",nsamp);
#endif

    fread(&nbyte,sizeof(int),1,fp);
    if (fread(*data, sizeof(int), nsamp, fp)==0)
	return;
    fread(&nbyte,sizeof(int),1,fp);
}

static void readTRCshort(FILE *fp, int *nsamp_ptr, short **data)
{
    int nbyte,nsamp,itmp;
    char ndum[1000];

    /* note: only the crucial fread is check for error */
    fread(&itmp,sizeof(int),1,fp);
    nbyte=itmp-4;
    if (fread(&nsamp,sizeof(int),1,fp)==0) 
	return;
    if (nbyte) fread(ndum, sizeof(char), nbyte, fp);
    fread((char *)&itmp, sizeof(int), 1, fp);
    *nsamp_ptr= nsamp;

    *data= (short *)Malloc(sizeof(short) * nsamp);
#ifdef DEBUG
printf("Reading in a trace: %d samples\n",nsamp);
#endif

    fread(&nbyte,sizeof(int),1,fp);
    if (fread(*data, sizeof(short), nsamp, fp)==0)
	return;
    fread(&nbyte,sizeof(int),1,fp);
}

/**********************************************************************
 *   Load Wave                                                        *
 **********************************************************************/

int LoadTRCWave(char *fname, Wave ***waves_ptr)
{
    FILE *fp;
    Wave *wv, **waves;
    BIS3_HEADER *bh;
    int nsamp;
	
    int nbyte, itmp, ntrace, i;
    int has_hdr=0;

    int n, numWaves=0;
    int n_read;

    fp=fopen(fname,"rb");
    if(fp==NULL) {
	ReportFileNotFound(fname, 1);
	return 0;
    }

    /* start reading a TRC file */
    fread(&itmp,sizeof(int),1,fp);
    nbyte= itmp-4;
    fread(&ntrace,sizeof(int),1,fp);
    /* swap them: just in case-- quick fix, obviously */
    old_srate= TRC_srate;
    oldStation= theStation;
    oldStat= numStat;
    if (nbyte) {
	has_hdr= parseHeader(fp, ntrace, nbyte);
    }
    fread((char *)&itmp,sizeof(int),1,fp);

    if(!has_hdr) {
	/* guess we're a little paranoid */
	TRC_srate= old_srate;
	theStation= oldStation;
	numStat= oldStat;
    }else {
	old_statfile_Specified= statfile_Specified;
	statfile_Specified= 1;
    }
    
    /* can only tell whether we have the hdr at this pt: */
    if (!has_hdr && !check_TRC_setup())
	return 0;	/* didn't have enuff info */
#ifdef DEBUG
printf("*** Reading a Trc File: %d trace(s).\n",ntrace);
#endif

    if (!has_hdr && statfile_Specified && ntrace!=numStat) {
	fprintf(stderr, "Alert: there are %d traces in file %s\n",
		ntrace, fname);
	fprintf(stderr, "    but there are %d stations in the station list\n",
		numStat);
	if (ntrace > numStat) {
	    fprintf(stderr, "    traces ignored.\n");
	    return 0;	/* otherwise, we'll tolerate */
	}
    }

    /* allocate array to hold waves */
    *waves_ptr= waves= (Wave **)Malloc(sizeof(Wave *)*ntrace);

    for(i=0; i < ntrace; i++) {
	wv= (Wave *)Malloc(sizeof(Wave));
	bh=&wv->info;

	/* prepare data */
	switch(TRC_dataType) {
	case TRC_dataFloat: {
	    float *data, min, max;
	    int j;
	
	    readTRCfloat(fp, &nsamp, &data);
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
	    wv->data= data;
	    break;
	}
	case TRC_dataInt: {
	    int *iarray; float *data;
	    int j, min, max;
	    
	    readTRCint(fp, &nsamp, &iarray);
	    data= (float *)Malloc(sizeof(float)*nsamp);
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
	    wv->data= data;
	    break;
	}
	case TRC_dataShort: {
	    short *sarray; float *data;
	    int j; short min, max;
	    
	    readTRCshort(fp, &nsamp, &sarray);
	    data= (float *)Malloc(sizeof(float)*nsamp);
	    if (data==NULL) return numWaves;	/* not enuff mem */
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
	    wv->data= data;
	    break;
	}}

	/* the header ... */
	trc2bis(i, nsamp, bh);
	
	*waves= wv;
	waves++;
	numWaves++;
    }
    if (has_hdr) {
	for(i=0; i < numStat; i++) {
	    if(theStation[i])free(theStation[i]);
	}
	free(theStation);
	/* restore the stuff */
	TRC_srate= old_srate;
	theStation= oldStation;
	numStat= oldStat;
	statfile_Specified=old_statfile_Specified;
    }
    fclose(fp);
    return numWaves;
}

/**********************************************************************
 *   Conversion                                                       *
 **********************************************************************/

static void trc2bis(int itrace, int nsamp, BIS3_HEADER *bh)
{
    int j;
    
    /* no BMAGIC insert; DumpWave does it */
    if (statfile_Specified) {
	strncpy(bh->station, theStation[itrace], STATSIZE);
    }else {
	sprintf(bh->station, "%d", itrace);
    }
    bh->station[STATSIZE]='\0';
    *bh->network='\0';
    *bh->location='\0';
    *bh->channel='\0';
    bh->dip= 0;
    bh->azimuth= -1.0;
    bh->latitude= 0;
    bh->longitude= 0;
    bh->elevation= 0;
    bh->depth= 0;

    /***   Data info   ***/
    /* we don't care about starting time */
    bh->start_it.year= 0;	
    bh->start_it.second= 0;
    bh->start_it.usec= 0;

    bh->time_correction= 0;
    /* bh->format set already */

    bh->n_values= nsamp;
    bh->n_flagged= 0;
    bh->sample_rate= TRC_srate;

    /***   Data_type info   ***/

    /* except format==5, interpret as int */
    if (TRC_dataFloat) {
	bh->flag.fflag= F4FLAG;
    }else {
	bh->flag.iflag= I4FLAG;
    }

    /***   poles & zeros   ***/
    /* none of this crap */
    bh->response=RESP_NOTFOUND;
    bh->digital_sens=0;
    bh->gain_factor= 0;
    bh->n_poles= 0;
    bh->n_zeros= 0;

    for(j=0; j < NUMPOLES; j++) {
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
 *   Get the extra states neeeded                                     *
 **********************************************************************/

static void Demand_callback(char *fname, FileFormat format)
{
    if (fname[0]!='\0') parseStatFile(fname);
    GetString("sampling rate: ",fname,100);
    TRC_srate= atof(fname);
}

int Demand_TRC_setup()
{
    ReportError("%s","TRC file has no header\nEnter Station List File\n");
    SelectFile("Choose Station File", 0, Demand_callback, 1 /*return*/);
}

static int check_TRC_setup()
{
    /* check to see if we have everything we need */
    if (!statfile_Specified || TRC_srate==0.0) {
	Demand_TRC_setup();
	return (TRC_srate!=0.0);
    }
    else
	return	1;
}


static void parseStatFile(char *fname)
{
    FILE *fp;
    char buf[100], stat[100];
    int i, max;

    if ((fp=fopen(fname,"r"))==NULL) {
	ReportFileNotFound(fname, 0);
	return;
    }
    if (statfile_Specified) { /* specified before, discard old one */
	int i;
	for(i= 0; i < numStat; i++) {
	    if (theStation[i]) free(theStation[i]);
	}
	free(theStation);
    }
    max=INIT_MAX_STAT;
    theStation= (char **)Malloc(sizeof(char *)*INIT_MAX_STAT);
    if (theStation==NULL) return;   /* not enuff mem */
    i=0;
    while (fgets(buf,100,fp)!=NULL) {
	sscanf(buf,"%s", stat);
	if (*stat=='#') continue;   /* ignore comments */
	theStation[i]= (char *)Malloc(strlen(stat)+1);
	if (theStation[i]==NULL) return;    /* not enuff mem */
	strcpy(theStation[i],stat);
	i++;
	if (i==max) {	/* need more */
	    max*=2;
	    theStation= (char **)Realloc(theStation, sizeof(char *)*max);
	    if(theStation==NULL)return;
	}
    }
    numStat=i;
    statfile_Specified=1;
    fclose(fp);
}

static int parseHeader(FILE *fp, int ntrace, int nbyte)
{
    int name_len;
    int i, max;

    nbyte-=sizeof(int);
    fread(&name_len,sizeof(int),1,fp);
#ifdef DEBUG
printf("name_len: %d\n",name_len);
#endif
    max=INIT_MAX_STAT;
    theStation= (char **)Malloc(sizeof(char *)*INIT_MAX_STAT);
    if (theStation==NULL) return 0;   /* not enuff mem */
    bzero(theStation, sizeof(char*)*max);
    for(i=0; i<ntrace && nbyte>=name_len; i++) {
	theStation[i]= (char *)Malloc(name_len);
	if (theStation[i]==NULL) return 0;    /* not enuff mem */
	fread(theStation[i],sizeof(char),name_len,fp);
	nbyte-=name_len;
#ifdef DEBUG
printf("[%d] %s\n",i,theStation[i]);
#endif
	if (i==max) {	/* need more */
	    max*=2;
	    theStation= (char **)Realloc(theStation, sizeof(char *)*max);
	    if(theStation==NULL) return 0;
	}
    }
    numStat=i;
    /* sampling rate */
    nbyte-=sizeof(float);
#ifdef DEBUG
printf("nbyte left: %d\n",nbyte);
#endif
    if(nbyte>=0) {
	fread(&TRC_srate, sizeof(float), 1, fp);
	TRC_srate= 1/TRC_srate;	/* assume seconds per sample */
#ifdef DEBUG
printf("sampling rate: %f\n",TRC_srate);
#endif
	if (nbyte) {
	    char *ndum= (char *)Malloc(nbyte);
	    fread(ndum, 1, nbyte, fp);
	    free(ndum);
	}
    }else {
	fprintf(stderr,"TRC header does not contain sample rate.\n");
    }
    return (i>0);   /* has_hdr? */
}

/*
 * format: (in any order, free format)
 * # again, '#' denotes comment
 *  statlist	stat_list_file
 *  datatype	[ int | float | short ]
 *  srate	sampling_rate
 */
void parseTRCiniFile(char *fname)
{
    FILE *fp;
    char buf[100], key[100], val[100];

    if ((fp=fopen(fname,"r"))==NULL) {
	fprintf(stderr, "Cannot open TRC init file %s.\n",
		fname); 
	return;
    }
    while(fgets(buf,100,fp)!=NULL) {
	int items_read;

	items_read= sscanf(buf, "%s %s", key, val);
	if (key[0]=='#') continue; /* comments, ignore */
	if (!strncmp(key, "st", 2)) {
	    /* specify station list file */
	    if(items_read==2)parseStatFile(val);
	}else if(!strncmp(key, "da", 2)) {
	    if (val[0]=='i' || val[0]=='I') {
		TRC_dataType= TRC_dataInt;
	    }else if (val[0]=='s' || val[0]=='S') {
		TRC_dataType= TRC_dataShort;
	    }else {
		TRC_dataType= TRC_dataFloat;
	    }
	}else if(!strncmp(key, "sr", 2)) {
	    TRC_srate= atof(val);
#ifdef DEBUG
printf("sampling rate: %f\n",TRC_srate);
#endif
	}
	/* ignore otherwise. */	 
    }	
    fclose(fp);
}
    
/**********************************************************************
 *   Previewing                                                       *
 **********************************************************************/

int PreviewTRC(char *fname, wfmTuple **wfmtup, void **fsi, EvtFile *evt)
{
    FILE *fp;
    long hdrOff, trcOff;

    int nsamp;
    int nbyte, itmp, ntrace,i;
    char ndum[1000];
    int has_hdr=0;
    int n, numWaves=0;
    int n_read;
    wfmTuple *wfm= NULL, *prevwfm= NULL;

    fp=fopen(fname,"rb");
    if(fp==NULL) {
	ReportFileNotFound(fname, 1);
	return 0;
    }

    /* start reading a TRC file */
    fread(&itmp,sizeof(int),1,fp);
    nbyte= itmp-4;
    fread(&ntrace,sizeof(int),1,fp);
    /* swap them: just in case-- quick fix, obviously */
    old_srate= TRC_srate;
    oldStation= theStation;
    oldStat= numStat;
    if (nbyte) {
	has_hdr= parseHeader(fp, ntrace, nbyte);
    }
    fread((char *)&itmp,sizeof(int),1,fp);

    if(!has_hdr) {
	/* guess we're a little paranoid */
	TRC_srate= old_srate;
	theStation= oldStation;
	numStat= oldStat;
    }else {
	old_statfile_Specified= statfile_Specified;
	statfile_Specified= 1;
    }
    
    /* can only tell whether we have the hdr at this pt: */
    if (!has_hdr && !check_TRC_setup())
	return 0;	/* didn't have enuff info */
#ifdef DEBUG
printf("*** Reading a Trc File: %d trace(s).\n",ntrace);
#endif

    if (!has_hdr && statfile_Specified && ntrace!=numStat) {
	fprintf(stderr, "Alert: there are %d traces in file %s\n",
		ntrace, fname);
	fprintf(stderr, "    but there are %d stations in the station list\n",
		numStat);
	if (ntrace > numStat) {
	    fprintf(stderr, "    traces ignored.\n");
	    return 0;	/* otherwise, we'll tolerate */
	}
    }
    for(i=0; i < ntrace; i++) {

	trcOff= 0;
#if 0
	trcOff= ftell(fp);

	/* well, shouldn't have written it with fortran in the
	   first place... */
	fread(&itmp,sizeof(int),1,fp);
	nbyte=itmp-4;
	fread(&itmp,sizeof(int),1,fp);	/* nsamp */
	if (nbyte) fread(ndum, sizeof(char), nbyte, fp);
	fread(&nbyte,sizeof(int),1,fp);
	switch(TRC_dataType) {
	case TRC_dataFloat:
	    fseek(fp,sizeof(float)*nsamp,1);
	    break;
	case TRC_dataInt:
	    fseek(fp,sizeof(int)*nsamp,1);
	    break;
	case TRC_dataShort:
	    fseek(fp,sizeof(short)*nsamp,1);
	    break;
	}
	fread(&nbyte,sizeof(int),1,fp);
#endif

	/* fill in the tuple */
	wfm= (wfmTuple *)Malloc(sizeof(wfmTuple));
	if (prevwfm) {
	    prevwfm->next= wfm;
	    prevwfm= wfm;
	}else {
	    *wfmtup= prevwfm= wfm;
	}
	wfm->next= NULL;
	if (statfile_Specified) {
	    strncpy(wfm->trcName, theStation[i], STATSIZE);
	}else {
	    sprintf(wfm->trcName, "%d", i);
	}
	wfm->evt= evt;
	wfm->hdr_offset= -1;	    /* no header */
	wfm->trc_offset= trcOff;
	
	numWaves++;
    }
    if (has_hdr) {
	for(i=0; i < numStat; i++) {
	    if(theStation[i])free(theStation[i]);
	}
	free(theStation);
	/* restore the stuff */
	TRC_srate= old_srate;
	theStation= oldStation;
	numStat= oldStat;
	statfile_Specified=old_statfile_Specified;
    }
    fclose(fp);
    *fsi= NULL;
    return numWaves;
}
