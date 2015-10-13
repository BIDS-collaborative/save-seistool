#ifndef lint
static char id[] = "$Id: mseed_wave.c,v 1.4 2013/02/28 21:24:59 lombard Exp $";
#endif

/*
 * sdr_wave.c--
 *    loading of SDR data files
 *
 * Copyright (c) 1992-3 Andrew K. Yu, Lawrence Berkeley Laboratory
 *      and University of California, Berkeley.
 * All rights reserved.
 */
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <strings.h>
#include <sys/param.h>
#include "qlib2.h"

#include "instr.h"
#include "proto.h"
#include "xv_proto.h"

/*
 * major revision to use the QLIB mseed utilities in place of the
 * old seistool sdr stuff. PNL 2/20/03
 */

/* extern STI_TIME sdr_t_2_sti(); HERE: still used? */

/* #define DEBUG  */
/**********************************************************************
 *   Prototypes & structures                                          *
 **********************************************************************/

static void sdr2bis(DATA_HDR *hdr, int nsamp, BIS3_HEADER *bh);
static int read_fill_mseed_data(FILE *fp, char *fname, float **pdata, 
				BIS3_HEADER *bh);


/**********************************************************************
 *   Load Wave                                                        *
 **********************************************************************/

int LoadSDRWave(char *fname, Wave ***waves_ptr)
{
    FILE *fp;
    Wave *wv;
    BIS3_HEADER *bh;
    DATA_HDR *hdr;
    int npts;
    float *pdata;
	
    fp=fopen(fname,"r");
    if(fp==NULL) {
	ReportFileNotFound(fname, 1);
	return 0;
    }

    wv= (Wave *)Malloc(sizeof(Wave));
    bh=&wv->info;

    npts = read_fill_mseed_data(fp, fname, &pdata, bh);
    fclose(fp);
    if(npts <= 0) {
	free(wv);
	return 0;
    }

    wv->data = pdata;

    *waves_ptr= (Wave **)Malloc(sizeof(Wave *));
    **waves_ptr= wv;

    return (1);
}

#define INCR	262144

static int read_fill_mseed_data(FILE *fp, char *fname, float **pdata, 
				BIS3_HEADER *bh)
{
    DATA_HDR *ihdr = NULL;
    DATA_HDR *hdr = NULL;
    INT_TIME last_end;
    double gap, interval, thresh;
    float *arr, *tofloat;
    int *pint, *data = NULL;
    int n, npts = 0;
    int n_alloc = 0;
    int seconds, usecs, new_samples, need_pad;
    int min, max, i;
    int status, blksize;
    int	blknum = 0;
    char *mseed = NULL;

    while ((status = blksize = read_ms_record (&hdr, &mseed, fp)) > 0) {
	++blknum;
	new_samples = hdr->num_samples;
	need_pad = 0;
	if (!ihdr) {
	    ihdr = dup_data_hdr (hdr);
	    time_interval2 (1, hdr->sample_rate, hdr->sample_rate_mult, 
			    &seconds, &usecs);
	    /* thresh is 1.5 samples, in usecs */
	    interval = (((double)seconds) * USECS_PER_SEC + usecs);
	    thresh = 1.5 * interval;
	}
	else {
	    /* The normal gap is one sample interval */
	    gap = tdiff(hdr->begtime, last_end);
	    if (gap > thresh) {
		/* Round down to the whole sample */
		need_pad = (int)(-0.5 + dsamples_in_time2(hdr->sample_rate,
							  hdr->sample_rate_mult, 
							  gap));
		new_samples += need_pad;
	    }
	    else if (gap < 0.0) {
		fprintf(stderr, "Data overlap in %s\n", fname);
		return -1;
	    }
	}
	last_end = hdr->endtime;
	
	/* Ensure we have enough space for the data plus the gap*/
	while (npts + new_samples > n_alloc) {
	    data = (int *)Realloc (data, (n_alloc+INCR) * sizeof(int));
	    
	    if (data) 
		n_alloc += INCR;
	    else 
		return -1;
	}
	
	if (need_pad) {
	    printf("-- [%.5s%.3s] %f secs padded in.\n",
		   hdr->station_id, hdr->channel_id, 
		   need_pad * interval / USECS_PER_SEC);
	    
	    pint= &data[npts];
	    for(i=0; i < (need_pad); i++) {
		*pint++= 0;
	    }
	    npts += need_pad;
	}
	
	n = ms_unpack (hdr, hdr->num_samples, mseed, data+npts);
	if (n != hdr->num_samples) {
	    fprintf(stderr, "Error unpacking MiniSEED data.\n");
	    return(-1);
	}
	npts += n;
	free_data_hdr(hdr);
	hdr = NULL;
    }
    if (mseed) free (mseed);
    if (status != EOF) {
	fprintf (stderr, "Error reading %s block %d.\n",fname, ++blknum);
	return -1;
    }
    if (ihdr == NULL) {
	fprintf( stderr, "Nothing read from %s\n", fname);
	return -1;
    }
    
    sdr2bis(ihdr, npts, bh);
    free_data_hdr(ihdr);
    
    /* determine data range, convert to float */
    *pdata = tofloat = (float *)data;
    pint = data;
    min = max = data[0];
    *tofloat++ = (float)*pint++;
    for(i = 1; i < npts; i++) {
	if (*pint < min)
	    min = *pint;
	else if (*pint > max)
	    max = *pint;
	*tofloat++ = (float)*pint++;
    }
    bh->min_value.iflag = min;
    bh->max_value.iflag = max;
    bh->format = I32_FORMAT;
    
    return (npts);
}
    

/**********************************************************************
 *   Header conversion                                                *
 **********************************************************************/

static void sdr2bis(DATA_HDR *hdr, int nsamp, BIS3_HEADER *bh)
{
    int j;
    float temp;

    /* no BMAGIC insert; done in DumpWave */
    bzero(bh->station, STATSIZE);
    strncpy(bh->station, hdr->station_id, STATSIZE);
    bh->station[STATSIZE-1] = '\0';

    bzero(bh->network, NETSIZE);
    strncpy(bh->network, hdr->network_id, SDR_NETWORK_LEN);
    bzero(bh->channel, CHSIZE);
    strncpy(bh->channel, hdr->channel_id, SDR_CHANNEL_LEN);
    bzero(bh->location, LOCSIZE);
    strncpy(bh->location, hdr->location_id, SDR_LOCATION_LEN);
    strib(bh->location);
    bh->dip= 0;
    bh->azimuth= -1.0;
    bh->latitude= 0;
    bh->longitude= 0;
    bh->elevation= 0;
    bh->depth= 0;

    /***   Data info   ***/
    
    bh->start_it.year = hdr->begtime.year;
    bh->start_it.second = hdr->begtime.second;
    bh->start_it.usec = hdr->begtime.usec;
    
    bh->time_correction= 0;
    /* format set already */

    bh->n_values= nsamp;
    bh->n_flagged= 0;

    /* sample rate is a little tricky */
    temp = hdr->sample_rate;
    bh->sample_rate = (temp>0)? temp: -1/temp;
    temp = hdr->sample_rate_mult;
    bh->sample_rate = (temp>0)?
	bh->sample_rate * temp: -bh->sample_rate/temp;

    /***   Data_type info   ***/

    bh->flag.iflag= I4FLAG;

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
 *   Previewing                                                       *
 **********************************************************************/

int PreviewSDR(char *fname, wfmTuple **wfmtup, void **fsi, EvtFile *evt)
{
    FILE *fp;
    DATA_HDR hdr;
    wfmTuple *wfm= NULL;
    char *name;
    int j;
	
    fp=fopen(fname,"r");
    if(fp==NULL) {
	ReportFileNotFound(fname, 1);
	return 0;
    }
    if (fread(&hdr, sizeof(DATA_HDR), 1, fp)!=1) {
	fprintf(stderr,"Can't read DATA_HDR frame 0 from %s\n", fname);
	return 0;
    }

    /* fill in the tuple */
    *wfmtup= wfm= (wfmTuple *)Malloc(sizeof(wfmTuple));
    wfm->next= NULL;
    wfm->evt= evt;
    wfm->hdr_offset= wfm->trc_offset= -1;    /* not important */
    name= wfm->trcName;

    for(j=0; j<STATSIZE; j++) {
	if(hdr.station_id[j]==' ') 
	    hdr.station_id[j]='\0';	/* remove blanks */
    }
    sprintf(name, "%s.%s.%s.%s", hdr.station_id, hdr.network_id,
	    hdr.channel_id, hdr.location_id);

    *fsi= NULL;	/* no specific stuff */
    fclose(fp);

    return (1);	    /* always */
}

int LoadSDRWfm(wfmTuple *wfm, Wave ***waveptr)
{
    return LoadSDRWave(wfm->evt->name, waveptr);
}

