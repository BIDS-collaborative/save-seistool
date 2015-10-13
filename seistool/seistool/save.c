#ifndef lint
static char id[] = "$Id: save.c,v 1.2 2013/02/28 21:24:58 lombard Exp $";
#endif

/*
 * save.c--
 *    call backs for saving of data files
 *
 * Copyright (c) 1992-3 Andrew K. Yu, Lawrence Berkeley Laboratory
 *      and University of California, Berkeley.
 * All rights reserved.
 */
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/stat.h>
#include <rpc/types.h>
#include <rpc/xdr.h>

#include "bis_proto.h"
#include "proto.h"
#include "xv_proto.h"

extern Trace **traces;
extern int LastTrack;

static FILE *open_dumpfile(XDR *xdrs);
static void getSACfname(char *dir, char *fname, int itrc);


void DumpWave(XDR *xdrs, int itrc)
{
    BIS3_HEADER bh;
    DATE start_date, event_origin_date;
    bool_t ok= TRUE;
    int i;
    float *f;

    /* prepare header */
    bcopy(&traces[itrc]->wave->info, &bh, sizeof(bh));
    f= traces[itrc]->wave->data;
    start_date = sti_to_date(bh.start_it);
    if (bh.event_origin_it.year == I4FLAG)
	bzero((void*)&event_origin_date, sizeof(DATE));
    else
	event_origin_date = sti_to_date(bh.event_origin_it);

    bh.magic= BMAGIC;
    bh.version= BVERSION3;  /* we only write BIS version 3 */
    bh.format= R32_FORMAT;
    bh.min_value.fflag= traces[itrc]->axis.ymin;
    bh.max_value.fflag= traces[itrc]->axis.ymax;

    /* write out header, in pieces */
    if (xdr_MAGIC(xdrs, &bh)==FALSE) {
	fprintf(stderr, "Cannot write BIS header magic.\n");
	return;
    }
    if (xdr_BIS3_HEADER_before(xdrs, &bh)==FALSE) {
	fprintf(stderr, "Cannot write out BIS header.\n");
	return;
    }
    if (xdr_DATE(xdrs, &start_date)==FALSE) {
	fprintf(stderr, "Cannot write out BIS header start date.\n");
	return;
    }
    if (xdr_BIS3_HEADER_middle(xdrs, &bh)==FALSE) {
	fprintf(stderr, "Cannot write out BIS header.\n");
	return;
    }
    if (xdr_DATE(xdrs, &event_origin_date)==FALSE) {
	fprintf(stderr, "Cannot write out BIS header event origin date.\n");
	return;
    }
    if (xdr_BIS3_HEADER_after(xdrs, &bh)==FALSE) {
	fprintf(stderr, "Cannot write out BIS header.\n");
	return;
    }
    /* write out data */
    for(i=0; i< bh.n_values; i++) {
	ok &= xdr_float(xdrs, f);
	f++;
    }
    if (ok==FALSE) {
	fprintf(stderr, "Cannot write out BIS data.\n");
    }

    return;
}

static FILE *open_dumpfile(XDR *xdrs)
{
    FILE *fp;
    struct stat s_buf;
    char fname[1000];

    GetString("Dump file: ",fname,1000);
    fp= fopen(fname, "w");
    if (fp) 
	xdrstdio_create(xdrs, fp, XDR_ENCODE);
    else
	fprintf(stderr, "Error opening %s\n", fname);

    return fp;
}

static void getSACfname(char *dir, char *fname, int itrc)
{
    BIS3_HEADER *bh= &traces[itrc]->wave->info;
    STE_TIME et;
    int yr, mo, dy;
    et = sti_to_ste(bh->start_it);
    sprintf(fname,"%s/%d%02d%02d%02d%02d%02d.%s.%s.%s.%s",
	    dir, 
	    et.year,et.month,et.day,et.hour,et.minute,et.second,
	    bh->station, bh->network, bh->channel, bh->location);
}

void DumpBIS()
{
    int i;
    FILE *fp;
    XDR xdrs;

    if ((fp= open_dumpfile(&xdrs))==NULL) return;
    
    if (firstSelectedTrc()!=-1) { /* just write out the selected ones */
	for(i=0; i<=LastTrack; i++) {
	    if(traces[i]->selected) {
		DumpWave(&xdrs,i);
	    }
	}
    }else {
	for(i=0; i<=LastTrack; i++) {
	    DumpWave(&xdrs,i);
	}
    }

    xdr_destroy(&xdrs);
    fclose(fp);
}

void DumpSAC()
{
    char fname[1000], dir[1000];
    int i;

    *dir='\0';
    GetString("Write to directory", dir, 1000);

    if(*dir=='\0') {
    	/* write to current directory */
	strcpy(dir,".");
    }
    
    if (firstSelectedTrc()!=-1) { /* just write out the selected ones */
	printf("Write selected files to %s\n",dir);
	for(i=0; i<=LastTrack; i++) {
	    if(traces[i]->selected) {
		getSACfname(dir,fname,i);
		printf("Writing SAC file %s\n",fname);
		WriteSAC(fname,i);
	    }
	}
    }else {
	for(i=0; i<=LastTrack; i++) {
	    getSACfname(dir,fname,i);
	    printf("Writing SAC file %s\n",fname);
	    WriteSAC(fname,i);
	}
    }
}

void DumpSEGY()
{
    char fname[1000];
    Wave **waves;
    int i, num=0;

    *fname='\0';
    GetString("Write to file", fname, 1000);
    if(*fname=='\0')return;

    waves= (Wave **)Malloc(sizeof(Wave *)*(LastTrack+1));
    if (firstSelectedTrc()!=-1) { /* just write out the selected ones */
	for(i=0; i<=LastTrack; i++) {
	    if(traces[i]->selected) {
		waves[num++]= traces[i]->wave;
	    }
	}
    }else {
	for(i=0; i<=LastTrack; i++) {
	    waves[num++]= traces[i]->wave;
	}
    }
    printf("Writing %d traces to %s\n", num, fname);
    SaveSEGYWave(fname, waves, num);
    free(waves);
}
