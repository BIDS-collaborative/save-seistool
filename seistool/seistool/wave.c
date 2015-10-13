#ifndef lint
static char id[] = "$Id: wave.c,v 1.3 2013/02/28 21:24:56 lombard Exp $";
#endif

/*
 * wave.c--
 *    handles the abstract data type Wave
 *
 * Copyright (c) 1992-3 Andrew K. Yu, Lawrence Berkeley Laboratory
 *      and University of California, Berkeley.
 * All rights reserved.
 */
#include <stdio.h>
#include <xview/xview.h>
#include <xview/canvas.h>
#include "proto.h"
#include "xv_proto.h"

extern Trace **traces;
extern Track **tracks;
extern int lowTrkIndex;
extern int highTrkIndex;

int auto_demean=1;


FileFormat atofilefmt(char *str)
{
    FileFormat fmt;
    if(!strncasecmp(str,"BIS", 3)) {
	fmt= BIS_Format;
    }else if(!strncasecmp(str,"TRC", 3)) {
	fmt= TRC_Format;
    }else if(!strncasecmp(str,"MSEED", 4)) {
	fmt= SDR_Format;
    }else if(!strncasecmp(str,"SDR", 3)) {
	fmt= SDR_Format;
    }else if(!strncasecmp(str,"SEGY", 4) ||
	     !strncasecmp(str,"SGY", 3)) {
	fmt= SEGY_Format;
    }else if(!strncasecmp(str,"SAC", 3)) {
	fmt= SAC_Format;
    }else {
	fmt= -1;
    }
    return fmt;
}

char *formatName(FileFormat format)
{
    switch(format) {
    case BIS_Format: return "BIS";
    case TRC_Format: return "TRC";
    case SDR_Format: return "MSEED";
    case SEGY_Format: return "SEGY";
    case SAC_Format: return "SAC";
    }
    return NULL;
}

/* Load all the waves (traces) from fname. Some formats (not present anymore)*
 * had picks in with the trace data, so pick_ptr is here for that purpose    */
int LoadWave(char *fname, FileFormat format, Wave ***waves_ptr,
	     Pick ***picks_ptr, Amp ***amps_ptr)
{
    int i = 0;
    extern int Mode_abs_vscale;
    
    *waves_ptr= NULL;
    *picks_ptr= NULL;
    *amps_ptr = NULL;
    switch(format) {
    case BIS_Format: 
	i= LoadBISWave(fname, waves_ptr);
	break;
    case TRC_Format:
	i= LoadTRCWave(fname, waves_ptr);
	break;
    case SDR_Format:
	i= LoadSDRWave(fname, waves_ptr);
	break;
    case SEGY_Format:
	i= LoadSEGYWave(fname, waves_ptr);
	break;
    case SAC_Format:
	i= LoadSACWave(fname, waves_ptr);
	break;
    default:
	break;
    }

    /* yeah not a logical place but the best one */
    /* to calculate the absolute scaling         */
    if (Mode_abs_vscale==1) newAbsVertScale();

    return i;
}

int LoadWfm(wfmTuple *wfm, Wave **waveptr)
{
    int ok= 0;
    printf("-> %s %s\n",wfm->trcName,wfm->evt->name);
    switch(wfm->evt->format) {
    case BIS_Format: 
	fprintf(stderr,"LoadWfm: BIS format not ready.\n");
	break;
    case TRC_Format:
	fprintf(stderr,"LoadWfm: TRC format not ready.\n");
	break;
    case SDR_Format:
	ok= LoadSDRWfm(wfm, &waveptr);
	break;
    case SEGY_Format:
	ok= LoadSEGYWfm(wfm, &waveptr);
	break;
    case SAC_Format:
	fprintf(stderr,"LoadWfm: SAC format not ready.\n");
	break;
    default:
	break;
    }
    return ok;
}


void demean_trace(Wave *wave, Axis *axis, int ix1, int ix2)
{
    int n_val,i;
    float mean, *data, max, min;

    n_val= wave->info.n_values;
    if(!fixIxBounds(&ix1,&ix2,n_val))
	return;
    /*  find the mean (or D.C. offset) in the given range */
    data=wave->data;
    max=min=mean= data[ix1];
    for(i=ix1+1; i<=ix2; i++) {
	mean+=data[i];
	if(data[i]>max)
	    max= data[i];
	else if (data[i]<min)
	    min= data[i];
    }
    mean/= (ix2-ix1)+1;
    /*  remove the mean  */

    if (auto_demean) {
      for(i=0, data=wave->data; i<n_val; i++, data++) {
	*data-= mean;
      }
      axis->ymax= axis->y1= max-mean;
      axis->ymin= axis->y2= min-mean;
      wave->dcOffset= mean;
    }
}

void init_trace_hook(Wave *wave, Axis *axis)
{
    if(auto_demean) {
	int n_val,i;
	float mean, *data;

	n_val= wave->info.n_values;
	/*  find the mean (or D.C. offset)  */
	demean_trace(wave,axis,0,n_val-1);
    }
}

void CompleteSingleLoad(char *fname, FileFormat format)
{
    if(!Mode_CLI)
	xv_set(tracesFrame, FRAME_LEFT_FOOTER, fname, NULL);
}

void CompleteRest(char *fname, FileFormat format)
{
    int i;
    FILE *fp_pfile;
    
    FullScale();
/*    if (Mode_align) {
	UnifyTimeScale();   /* rescale time axis */
/*    }
    /* attempt to load in the picks */
    if (Mode_autoLoadPicks) {
	char pfname[131];
	char afname[131];

	cvtToPFname(pfname, fname, format);
	/* don't scream, forget it if it's not there */
	ReadPicks(pfname);

	cvtToAFname(afname, fname, format);
	ReadAmps(afname);
    }

    if (Mode_CLI) return;

    if (Mode_align) RedrawTScale();
    
    if (ZoomWindowMapped) {
	CleanMarks();
	if (Mode_triplet)
	    Trip_ZoomContentChanged(lowZTrkIndex);
	for(i=lowZTrkIndex; i <= highZTrkIndex; i++) {
	    ScaleZTrack(traces[i], traces[lowZTrkIndex]);
	    if (Zoom_Mode_sameVertScale) traces[i]->zaxis_needScale= 1;
	    UpdateMarks(i);
	}
	if(Zoom_Mode_sameVertScale) newVertScale();
	if (Mode_trvlTime) Trvl_ZoomContentChanged();
	if (Mode_rotate) Rot_ZoomContentChanged();
	RedrawZoomWindow("CompleteRest"); /* just in case */
    }
}

void CompleteLoad(char *fname, FileFormat format)
{

    CompleteSingleLoad( fname, format );
    CompleteRest(fname, format);
}

/* itrc for identification only */
void PlotWave(Trace *trc, Axis *axis, int width, int height, Window xwin,
	      GC gc, int toClip, int yoffset, int itrc)
{
    int i, idx;
    XPoint *points, *plot;
    float *data= trc->wave->data;
    float hs, vs;
    int k, k2, y0;
    int offset, n_val;

    k= axis->ix1, k2=axis->ix2;
    hs= axis->hs, vs= axis->vs;
    y0= axis->y0;

    offset= (k<0)? -k : 0;
    n_val= trc->wave->info.n_values;
    if(!fixIxBounds(&k,&k2,n_val))
	return;

    /* lots of points */
    if (k2-k> 10000) {
	int idx_inc;
	points= plot= (XPoint *)Malloc(sizeof(XPoint) * width * 8);
	if (points==NULL) return;	/* not enuff mem */

	idx_inc= (hs<1)? 1: hs;
	if(hs> (k2-k)/(width*8)+1)idx_inc= (k2-k)/(width*8)+1;
	if(toClip) {
	    for(idx=k, i=0; i<width*8 && idx<k2; idx+= idx_inc, i++) {
		int y;
		plot->x= (idx - k + offset)/hs;
		y= y0 - data[idx] / vs;
		if (y<0) {
		    y=0;
		}else if(y>=height) {
		    y=height-1;
		}
		plot->y= yoffset + y;
		plot++;
	    }
	}else {
	    yoffset+=y0;    /* yoffset + y0 */
	    for(idx=k, i=0; i<width*8 && idx<k2; idx+= idx_inc, i++) {
		plot->x= (idx - k + offset)/hs;
		plot->y= yoffset - data[idx] / vs;
		plot++;
	    }
	}
	XDrawLines(theDisp, xwin, gc, points, i, CoordModeOrigin);
    }else { /* just plot everything */
	int max= (XMaxRequestSize(theDisp)-3)/2;
	int m;

	points= plot= (XPoint *)Malloc(sizeof(XPoint) * max);
	if (points==NULL) return;	/* not enuff mem */

	if (toClip) {
	    for(m=k; m<=k2; m+=max) {
		plot= points;
		for(idx=m,i=0; idx<=k2 && i<max; idx++, i++) {
		    int y;
		    plot->x= (i + offset)/hs;
		    y= y0 - data[idx] / vs;
		    if (y < 0) {
			y = 0;
		    }else if (y >= height) {
			y = height -1;
		    }
		    plot->y= yoffset + y;
		    plot++;
		}
		XDrawLines(theDisp, xwin, gc, points, i, CoordModeOrigin);
		offset+=i;
	    }
	}else {
	    yoffset+=y0;	/* yoffset + y0 */
	    for(m=k; m<=k2; m+=max) {
		plot= points;
		for(idx=m, i=0; idx<=k2 && i<max; idx++, i++) {
		    plot->x= (i + offset)/hs;
		    plot->y= yoffset - data[idx] / vs;
		    plot++;
		}
		XDrawLines(theDisp, xwin, gc, points, i, CoordModeOrigin);
		offset+=i;
	    }
	}
    }
    free(points);
}

void PlotWave_decim(Trace *trc, Axis *axis, int width, int height, Window xwin,
		    GC gc, int toClip, int yoffset, int itrc)
{
    int i, idx;
    XPoint *points, *plot;
    float *data= trc->wave->data;
    float hs, vs;
    int k, k2, y0;
    int offset, n_val;

    k= axis->ix1, k2=axis->ix2;
    hs= axis->hs, vs= axis->vs;
    y0= axis->y0;

    offset= (k<0)? -k : 0;
    n_val= trc->wave->info.n_values;
    if(!fixIxBounds(&k,&k2,n_val))
	return;

    points= plot= (XPoint *)Malloc(sizeof(XPoint) * width);
    if (points==NULL) return;	/* not enuff mem */
    if (hs > 1) {

	offset/= hs;
	i=0; idx=k;
	if (toClip) {
	    while(i<width && idx<n_val) {
		int y;
		plot->x= i + offset;
		y= y0 - data[idx] / vs;
		if (y<0) {
		    y=0;
		}else if(y>=height) {
		    y=height-1;
		}
		plot->y= yoffset + y;
		i++; idx= k+i*(hs);
		plot++;
	    }
	}else {
	    yoffset+=y0;    /* yoffset + y0 */
	    while(i<width && idx<n_val) {
		plot->x= i + offset;
		plot->y= yoffset - data[idx] / vs;
		i++;
		/* note we do this instead of idx+= trc->axis.hs
		so that trucation won't get worse */ 
		idx= k+i*(hs);
		plot++;
	    }
	}
    }else {  /* trc->axis.hs < 1 */
	if (toClip) {
	    for(idx=k, i=0; idx<=k2; idx++, i++) {
		int y;
		plot->x= (i + offset)/hs;
		y= y0 - data[idx] / vs;
		if (y < 0) {
		    y = 0;
		}else if (y >= height) {
		    y = height -1;
		}
		plot->y= yoffset + y;
		plot++;
	    }
	}else {
	    yoffset+=y0;	/* yoffset + y0 */
	    for(idx=k, i=0; idx<=k2; idx++, i++) {
		plot->x= (i + offset)/hs;
		plot->y= yoffset - data[idx] / vs;
		plot++;
	    }
	}
    }

    XDrawLines(theDisp, xwin, gc, points, i, CoordModeOrigin);
    free(points);
}

