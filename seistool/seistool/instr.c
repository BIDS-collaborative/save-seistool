#ifndef lint
static char id[] = "$Id: instr.c,v 1.5 2013/02/28 21:24:59 lombard Exp $";
#endif

/*
 * instr.c--
 *    reading of instrument response database
 *
 * Copyright (c) 1992-3 Andrew K. Yu, Lawrence Berkeley Laboratory
 *      and University of California, Berkeley.
 * All rights reserved.
 */

/* Completely rewritten to use the resplib interface; PNL, 1 Nov 2001 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <resplib.h>
#include "proto.h"
#include "instr.h"

extern Trace **traces;
extern int LastTrack;

#ifdef SITE_BDSN
char *default_instr_file= "/usr/contrib/data/bdsn/instr.db.resp.current";
char *alt_instr_file= "/usr/contrib/data/bdsn/instr.db.resp";
#else
/* LBL needs to provide a file here */
char *default_instr_file= "/m/scr2/u/andrew/seistool/instr.db.resp.current";
char *alt_instr_file= NULL; /* alternate instrument file */
#endif

/* There's a problem in resplib.h: functions declared to return RL_DESC *
 * are being skipped by the compiler: PNL, 2013/01/30                   */
void *ChaOpen(char* Filename);

static char *last_resp_fname;
static int match_instr_resp(char *fname);


int reset_Resp()
{
    int i, needed = 0;
    
    if (last_resp_fname != NULL) {
	free(last_resp_fname);
	last_resp_fname = NULL;
    }
    for (i = 0; i <= LastTrack; i++) {
	switch (traces[i]->wave->info.response) {
	case RESP_NOTFOUND:
	    traces[i]->wave->info.response = RESP_UNKNOWN;
	    /* fall through */
	case RESP_UNKNOWN:
	    needed++;
	    break;
	}
    }
    return needed;
}

void GetResponses(char *fname)
{
    int ret;
    
    /* Has anything changed since the last time we were called? */
    if (fname == NULL ) {
	/* Use the default instr file */
	if (last_resp_fname == NULL || 
	    strcmp(last_resp_fname, default_instr_file) != 0) {
	    /* different file from last time */
	    ret = reset_Resp();
	    last_resp_fname = strdup(default_instr_file);
	    if (ret == 0) return; /* no unknowns; all done */
	}
    }
    else if (fname != NULL ) {
	/* use the specified instr file */
	if (last_resp_fname == NULL || strcmp(fname,last_resp_fname) != 0) {
	    /* different file from last time */
	    ret = reset_Resp();
	    last_resp_fname = strdup(fname);
	    if ( ret == 0) return;
	}
    }
    
    if ( (ret = match_instr_resp(last_resp_fname)) != 0)
    {
	if (fname == NULL && alt_instr_file != NULL) {
	    reset_Resp();
	    last_resp_fname = strdup(default_instr_file);
	    ret = match_instr_resp(alt_instr_file);
	}
    }
    
    return;
}


static int match_instr_resp(char *fname)
{
    RL_DESC cd;
    response chanResp;
    channel *ch;
    pz_resp *pz;
    BIS3_HEADER *bh;
    STE_TIME req_et;
    int numToGet = 0;
    int itrc, i, ret;
    char reqDate[RL_DATE_LENGTH];
    
    for (i = 0; i <= LastTrack; i++)
	if (traces[i]->wave->info.response == RESP_UNKNOWN)
	    numToGet++;
    if (numToGet == 0) return 0;
    
    if ( (cd = ChaOpen(fname)) == (RL_DESC)NULL) {
	fprintf(stderr, "Error opening %s\n", fname);
	return -1;
    }
    fprintf(stderr, "searching %s for instrument data\n", fname);
    
    chanResp.version = RL_VERSION;

    while ( numToGet && (ret = ChaNext( cd, &chanResp ) ) == RL_OK) {
	ch = &chanResp.resp_chan;

	for (itrc = 0; itrc <= LastTrack; itrc++) {
	    bh = &traces[itrc]->wave->info;
	    if (bh->response != RESP_UNKNOWN)
		continue;
	    if ( strcmp(bh->station, ch->sta) != 0 ||
		 strcmp(bh->network, ch->net) != 0 ||
		 strcmp(bh->channel, ch->seedchan) != 0 ||
		 strcmp(bh->location, ch->location) != 0)
		continue;
	    req_et = sti_to_ste(bh->start_it);
	    sprintf(reqDate, "%4d.%03d.%02d%02d", req_et.year, req_et.doy,
		    req_et.hour, req_et.minute);
	    if (strcmp(reqDate, ch->ondate) < 0 || 
		strcmp(reqDate, ch->offdate) >= 0)
		continue;

	    /* We have a match; use it! */
	    if (strcmp(ch->units,"DU/M/S**2") == 0) {
		bh->response = RESP_ACC;
	    }else if (strcmp(ch->units,"DU/M/S") == 0) {
		bh->response= RESP_VEL;
	    }else if (strcmp(ch->units, "DU/M")         == 0 ||
		      strcmp(ch->units, "DU/M**3/M**3") == 0 ||
		      strcmp(ch->units, "DU/PA")        == 0 ||
		      strcmp(ch->units, "DU/V/M")       == 0 ||
		      strcmp(ch->units, "DU/C")         == 0 ||
		      strcmp(ch->units, "DU/T")         == 0 ||
		      strcmp(ch->units, "DU/COUNTS")    == 0 ||
		      strcmp(ch->units, "DU/microstrain") == 0)
		bh->response= RESP_DISP;
	    else{
		fprintf(stderr,"ERROR: unexpected response units %s for %s %s %s %s\n",
			ch->units, ch->sta, ch->net, ch->seedchan, ch->location);
		continue;
	    }
	    if (strcmp(chanResp.resp_type, "PZ") != 0) {
		fprintf(stderr,"ERROR: unexpected response type %s for %s %s %s %s\n",
			chanResp.resp_type, ch->sta, ch->net, ch->seedchan, 
			ch->location);
		bh->response = RESP_UNKNOWN;
		continue;
	    }
	    numToGet--;

	    bh->latitude = (float)ch->lat;
	    bh->longitude = (float)ch->lon;
	    bh->elevation = (float)ch->elev;
	    bh->depth = (float)ch->depth;
	    bh->dip = (float)ch->dip;
	    bh->azimuth = (float)ch->azimuth;

	    pz = &chanResp.resp_pz;
	    
	    /* Even if the gain isn't greater than 1.0e38, some product *
	     * using it might be, so give a little extra in this test */
	    if (fabs(pz->gain) < 1.0e20) {
		bh->digital_sens= 1.0;
		bh->gain_factor = (float)pz->gain;
	    } else {
		float s= (float)sqrt(fabs(pz->gain));
		bh->gain_factor = bh->digital_sens = s;
		if (pz->gain < 0.0) 
		    bh->gain_factor = -bh->gain_factor;
	    }
	    bh->n_poles = pz->nbpole;
	    bh->n_zeros = pz->nbzero;
	    for( i=0; i < pz->nbpole; i++) { /* poles */
		bh->poles_zeros[i].real = (float)pz->pole[i].real;
		bh->poles_zeros[i].imag = (float)pz->pole[i].imag;
	    }
	    for(i=0; i < pz->nbzero; i++) {
		bh->poles_zeros[i+pz->nbpole].real = (float)pz->zero[i].real;
		bh->poles_zeros[i+pz->nbpole].imag = (float)pz->zero[i].imag;
	    }
	    
	    fprintf(stderr, "- %d poles, %d zeros for %s %s %s %s\n",
		    pz->nbpole, pz->nbzero,ch->sta, ch->net, ch->seedchan,
		    (ch->location[0] == '\0' ? "--" : ch->location));
	}
    }
    ChaClose(cd);
    if (numToGet) {
	for (itrc = 0; itrc <= LastTrack; itrc++) {
	    bh = &traces[itrc]->wave->info;
	    if (bh->response == RESP_UNKNOWN) {
		fprintf(stderr, "no info for %s %s %s %s\n", bh->station,
			bh->network, bh->channel, (bh->location[0] == '\0' ?
						   "--" : bh->location));
		traces[itrc]->wave->info.response == RESP_NOTFOUND;
	    }
	}
    } 
    else {
	/* we found them all; who cares if we had an error */
	return 0;
    }
    
    switch (ret) {
    case RL_IO_ERROR:
	fprintf(stderr, "Error reading %s\n", fname);
	return ret;
	break;
    case RL_VERSION_ERROR:
	fprintf(stderr, "resplib version mismatch\n");
	return ret;
	break;
    case RL_FORMAT_ERROR:
	fprintf(stderr, "resplib format error\n");
	return ret;
	break;
    }
    return numToGet;
}
