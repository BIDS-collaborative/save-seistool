#ifndef lint
static char id[] = "$Id: trackmgr.c,v 1.3 2013/02/28 21:24:56 lombard Exp $";
#endif

/*
 * trackmgr.c--
 *    the Trace Manager of the main window
 *
 * Copyright (c) 1992-3 Andrew K. Yu, Lawrence Berkeley Laboratory
 *      and University of California, Berkeley.
 * All rights reserved.
 */
#include <math.h>
#include <stdio.h>
#include <strings.h>
#include "proto.h"
#include "xv_proto.h"

/**********************************************************************
 *   Tracks Management                                                *
 **********************************************************************/

/* TotalTracks is the number of trace structures allocated */
int	TotalTracks = TotalInitTracks;

/* NumTracks  is the number of track structures allocated */
int	NumTracks = NumInitTracks;

/* LastTrack is the index of the last trace structure used; 1 less than 
 *   the number of trace structures used */
int	LastTrack= 0;

/* lowTrkIndex is the index of the track at the top of the main display */
int	lowTrkIndex = 0;

/* highTrkIndex is the index of the track at the bottom of the main display */
int	highTrkIndex = NumInitTracks - 1;

Trace **traces;
Track **tracks;

extern int TotalWaifTraces;
extern int NumWaifTraces;
extern Trace **waifTraces;

extern char *ExecScriptFname;

static int x1=-1;

/* internal function prototypes */
static void DrawBoundary(int x);
static int InsertTrack(Trace *trc, int itrc);
Trace *DupTrace(Trace *trc, int dupWave);



void InitTrackMgr()
{
    LastTrack= -1;
}

/* mode: 0-- silent mode (don't plot data) 1-- plot mode */
int AssociateTrack(char *fname, FileFormat format, int mode)
{
    Wave **waves, **oldwv;
    Pick **picks, **oldpk;
    Amp **amps, **oldamps;
    int numWave, i;
    float sps;
    char pfname[131], *filename;
    FILE *fp_pfile;

    numWave= LoadWave(fname, format, &waves, &picks, &amps);
    oldwv= waves; oldpk= picks; oldamps = amps;
    i=LastTrack;
    if (numWave+LastTrack>TotalTracks-1) {
	int j;
	int n= TotalTracks * 2;
	if (numWave+LastTrack > n-1)
	    n=numWave+LastTrack+1;    
	printf("tracks full: expanding to %d tracks...\n", n);
	traces= (Trace **)Realloc(traces, sizeof(Trace *)*n);
	if(traces==NULL) return 0; /* not enough mem */
	for(j= TotalTracks; j < n; j++) {
	    traces[j]= (Trace *)newTrace();
	    if(traces[j]==NULL) return 0;
	}
	TotalTracks= n;
    }

    if(numWave > 0) {
	filename= strcpy(Malloc(strlen(fname)+1),fname);
    }
    while(numWave > 0) {
	Trace *trc;
	int height, width;
	    
	trc= traces[++i];

	/* need to discard? */
	if(Mode_autoFilterNames && !matchSeleSNCL(&trc->wave->info)) {
	    waves++;
	    if(oldpk)picks++;
	    if (oldamps)amps++;
	    numWave--;
	    i--;
	    continue;	/* next trace */
	}

	trc->wave= *waves;
	trc->filename= filename;

	InitAxis(&(*waves)->info, &trc->axis);

	/* give a chance to do something to the data first */
	init_trace_hook(*waves, &trc->axis);

	/* if we have a exec script */
	if (ExecScriptFname) exec_script(ExecScriptFname, i);
	
	/* start from middle */
	trc->zaxis= trc->axis;	/* copy the axis */
	trc->zaxis.vs= 0;   /* no scale yet */
	trc->zaxis.hs= 0;   /* no scale yet */
	if (i<NumTracks) {
	    height= tracks[i]->height;
	    width= tracks[i]->width;
	}else {
	    height= tracks[NumTracks-1]->height;
	    width= tracks[NumTracks-1]->width;
	}
	ScaleYAxis(&trc->axis, height);
	sps= (*waves)->info.sample_rate;
	ScaleTAxis(&trc->axis, sps, width);

	/* plot track if necessary*/
	if (!Mode_CLI && mode
	    && i>=lowTrkIndex && i<=highTrkIndex) {
	    UpdatePlotTrack(i, tracks[i-lowTrkIndex]);
	    UpdateLabTrack(traces[i],tracks[i-lowTrkIndex]);
	    XFlush(theDisp);
	}
	trc->loadOrder= i;
	trc->itrc= i;

	waves++;
	if(oldpk)picks++;
	if (oldamps) amps++;
	numWave--;
    }
    if(oldwv)free(oldwv);    /* the array of (wave **) returned by load */
    if(oldpk)free(oldpk);    /* the array of (Pick **) returned by load */
    if(oldamps)free(oldamps);/* the array of (Amp **) returned by load */
    if (i>LastTrack) {
	LastTrack= i;
	if (Mode_autoRegroup) match_and_regroup();
	if (Mode_autoGroupComp) Group_ZNE();
	AdjustSbar();	 /* total number changed */
	AdjustZSbar(ZCvsHeight); /* total number changed */
	return 1;   /* successful */
    }
    
    return 0;	/* failed */
}

/* mode: 0-- silent mode (don't plot data) 1-- plot mode */
int AssociateWfmTrack(wfmTuple *wfm, int mode)
{
    Wave *wave;
    int ok, itrc;
    float sps;

    ok= LoadWfm( wfm, &wave);
    if (LastTrack==TotalTracks-1) {
	printf("tracks full... no more room\n");
	return 0;   /* failed */
    }
    if(ok) {
	Trace *trc;
	int height, width;
	    
	itrc= ++LastTrack;
	trc= traces[itrc];
	trc->wave= wave;
	trc->filename= wfm->evt->name;

	InitAxis(&wave->info, &trc->axis);

	/* give a chance to do something to the data first */
	init_trace_hook(wave, &trc->axis);

	/* if we have a exec script */
	if (ExecScriptFname) exec_script(ExecScriptFname, itrc);
	
	/* start from middle */
	trc->zaxis= trc->axis;	/* copy the axis */
	trc->zaxis.vs= 0;   /* no scale yet */
	trc->zaxis.hs= 0;   /* no scale yet */
	if (itrc<NumTracks) {
	    height= tracks[itrc]->height;
	    width= tracks[itrc]->width;
	}else {
	    height= tracks[NumTracks-1]->height;
	    width= tracks[NumTracks-1]->width;
	}
	ScaleYAxis(&trc->axis, height);
	sps= wave->info.sample_rate;
	ScaleTAxis(&trc->axis, sps, width);

	/* plot track if necessary*/
	if (mode && itrc>=lowTrkIndex && itrc<=highTrkIndex) {
	    UpdatePlotTrack(itrc, tracks[itrc-lowTrkIndex]);
	    UpdateLabTrack(traces[itrc],tracks[itrc-lowTrkIndex]);
	    XFlush(theDisp);
	}
	trc->loadOrder= itrc;
	trc->itrc= itrc;

	if (Mode_autoRegroup) match_and_regroup();
	AdjustSbar();	 /* total number changed */
	AdjustZSbar(ZCvsHeight); /* total number changed */
	return 1;   /* successful */
    }
    return 0;
}

void CloseAllTracks()
{
    int i;
    extern char *DEFAULT_LEFT_FOOTER;
    extern float TotalSeconds;

    if (!CheckChangesFlag())
	return;	    /* forget it ! */
    if(Mode_trvlTime) ExitTTMode();
    if(Mode_rotate)close_rotate_panel();
    LastTrack= -1;
    for(i=0; i<TotalTracks; i++) {
	cleanTrace(traces[i]);
    }
    for(i=0; i < TotalWaifTraces; i++) 
	cleanTrace(waifTraces[i]);
    NumWaifTraces = 0;

    if (Mode_CLI) return;
    
    /* reset indices */
    lowTrkIndex= 0;
    highTrkIndex= NumTracks-1;
    lowZTrkIndex= 0;
    highZTrkIndex= NumZTracks-1;
    TotalSeconds= 0.0;	    /* so the time scale will not be drawn */
    RedrawScreen();
    xv_set(tracesFrame, FRAME_LEFT_FOOTER, DEFAULT_LEFT_FOOTER,
	   NULL);
    AdjustSbar();
    SetSbarPosition();
    AdjustZSbar(ZCvsHeight);
    if (ZoomWindowMapped) {
	CleanZStatus();		/* if any */
	RedrawZoomWindow("closeAllTracks");
    }
}

/**********************************************************************
 *   Tracks Scrolling                                                 *
 **********************************************************************/

void trk_scroll_up()
{
    if (lowTrkIndex==0)
	return;
    lowTrkIndex--; highTrkIndex--;
    RedrawScreen();
}

void trk_scroll_down()
{
    if (highTrkIndex==TotalTracks-1)
	return;
    highTrkIndex++; lowTrkIndex++;
    RedrawScreen();
}

void trk_scroll_pgup()
{
    if (lowTrkIndex<NumTracks) {
	lowTrkIndex= 0;
	highTrkIndex= NumTracks-1;
    }else {
	lowTrkIndex-= NumTracks;
	highTrkIndex-= NumTracks;
    }
    SetSbarPosition();
    RedrawScreen();
}

void trk_scroll_pgdown()
{
    if (LastTrack+1<NumTracks)
	return;
    if (highTrkIndex>=(LastTrack+1-NumTracks)) {
	lowTrkIndex= LastTrack+1-NumTracks;
	highTrkIndex= LastTrack;
    }else {
	lowTrkIndex+= NumTracks;
	highTrkIndex+= NumTracks;
    }
    SetSbarPosition();
    RedrawScreen();
}

void trk_scroll_var(int amount, int dir)
{
    if (dir==SCRL_UP) {
	lowTrkIndex-=amount;
	highTrkIndex-=amount;
	if (lowTrkIndex<0) {
	    lowTrkIndex=0;
	    highTrkIndex=NumTracks-1;
	}
    }else { /* down */
	lowTrkIndex+=amount;
	highTrkIndex+=amount;
	if (highTrkIndex>=(LastTrack+1-NumTracks)) {
	    highTrkIndex= LastTrack;
	    lowTrkIndex= LastTrack+1-NumTracks;
	}
    }
    RedrawScreen();
}

/**********************************************************************
 *   Tracks Scaling                                                   *
 **********************************************************************/

static void DrawBoundary(int x)
{
    int i;
    Track *trk;
    for(i=0; i < NumTracks; i++) {
	trk= tracks[i];
	drawBrokenLine(trk->xwin, trk->ovrGC, x, trk->height);
    }
    XFlush(theDisp);
}

void ClearBound()
{
    x1=-1;
}

int HasBoundary()
{
    return (x1!=-1);
}

void BoundTracks(int x, Event *event)
{
    if (event_is_down(event)) {
	/* logs the boundary */
	DrawBoundary(x);
	x1=x;
    }else {
	/* note that x1 might be cleared by ClearBound() already;
	   eg. when the tracks are being clipped */
	if (x1!=-1) DrawBoundary(x1);
	x1=-1;
    }
}

void BoundTracks_drag(int x, Event *event)
{
    if (event_is_down(event)) {
	/* clear old, logs the new boundary */
	if (x1!=-1) {
	    DrawBoundary(x1);
	    DrawBoundary(x);
	    x1=x;
	}
    }else {
	if (x1!=-1) {
	    DrawBoundary(x1);
	    x1=-1;
	}
    }
}

/**********************************************************************
 *   Marking Intervals                                                *
 **********************************************************************/

void UpdateInterval(int itrc)
{
    extern GC shade_gc[8];
    int x1, x2;
    int i=0;
    float srate;
    Trace *trc=traces[itrc];
    Reg_select *curr=trc->sel_reg;
    GC gc_temp;

    InitStipples();  
    while (curr!=NULL) {
	gc_temp=shade_gc[i++];
	XSetFunction(theDisp, gc_temp, GXxor);
	XSetPlaneMask(theDisp, gc_temp,AllPlanes);
	x1= indexToCoord(&trc->axis, curr->right_index, 1);
	x2= indexToCoord(&trc->axis, curr->left_index, 1);
	DrawInterval(itrc, x1, x2,gc_temp);
	curr=curr->next;
	if (i>7) {i=0;}
    }
}

void DrawInterval(int itrc, int x1, int x2, GC gc_val)
{
    Track *trk= tracks[itrc-lowTrkIndex];
    if(x2<x1) {int t=x1; x1=x2; x2=t; }
    XFillRectangle(theDisp, trk->xwin, gc_val,
		   x1, 0, x2-x1+1, trk->height);
}


/**********************************************************************
 *   Misc. info                                                       *
 **********************************************************************/

/*
 * firstSelectedTrc--
 *    returns the first selected trace
 */
int firstSelectedTrc()
{
    int i;
    for(i=0; i<=LastTrack; i++) {
	if(traces[i]->selected) {
	    return i;
	}
    }
    return -1;
}

/**********************************************************************
 *   Select Tracks                                                    *
 **********************************************************************/

void Select_all()
{
    int i;
    for(i=0; i<=LastTrack; i++)
	traces[i]->selected= 1;
    RedrawScreen();    
}

void Deselect_all()
{
    int i;
    for(i=0; i<=LastTrack; i++)
	traces[i]->selected= 0;
    RedrawScreen();    
}

void Discard_Selected()
{
    Trace **tmptrc;
    int idx, i;
    tmptrc= (Trace **)Malloc(sizeof(Trace *)*TotalTracks);
    if (tmptrc==NULL) return;	/* not enuff mem */
    bzero(tmptrc, sizeof(Trace *)*TotalTracks);
    for(i=0, idx=-1; i<= LastTrack; i++) {
	if(!traces[i]->selected) {
	    tmptrc[++idx]=traces[i];
	    traces[i]->itrc= idx;
	}else {
	    cleanTrace(traces[i]);
	    free(traces[i]);
	}
    }
    free(traces);
    traces= tmptrc;
    LastTrack= idx;
    for(i=idx+1; i < TotalTracks; i++) {
	traces[i]= (Trace *)Malloc(sizeof(Trace));
	if (traces[i]==NULL) return;	/* not enuff mem */
	bzero(traces[i],sizeof(Trace));
	traces[i]->wave=NULL;
	traces[i]->loadOrder=-1;
    }
    
    if(LastTrack<NumZTracks) { /* including LastTrack==-1 */
	lowZTrkIndex= 0;
	highZTrkIndex= NumZTracks-1;
    }else {
	if (highZTrkIndex>LastTrack) {
	    highZTrkIndex= LastTrack;
	    lowZTrkIndex= LastTrack-NumZTracks+1;
	}
    }
    if(LastTrack<NumTracks) { /* including LastTrack==-1 */
	lowTrkIndex= 0;
	highTrkIndex= NumTracks-1;
    }else {
	if (highTrkIndex>LastTrack) {
	    highTrkIndex= LastTrack;
	    lowTrkIndex= LastTrack-NumTracks+1;
	}
    }
    /* laziness: just rescale all of them... */
    ScaleZTrack(traces[lowZTrkIndex], NULL);
    for(i=lowZTrkIndex+1; i <= highZTrkIndex; i++) {
	ScaleZTrack(traces[i], traces[lowZTrkIndex]);
    }

    RedrawScreen();
    AdjustSbar();
    SetSbarPosition();
    AdjustZSbar(ZCvsHeight);
    if (ZoomWindowMapped) {
	RedrawZoomWindow("Discard_selected");
	CleanZStatus();		/* if any */
    }
}


/**********************************************************************
 *   Rearrange Tracks                                                 *
 **********************************************************************/

void Rearrange_Selected_top()
{
    Trace **tmptrc;
    int idx, i;
    tmptrc= (Trace **)Malloc(sizeof(Trace *)*LastTrack);
    if (tmptrc==NULL) return;	/* not enuff mem */
    /* copy the trace pointers */
    for(i=0; i<= LastTrack; i++) {
	tmptrc[i]=traces[i];
    }
    /* fill in selected */
    for(i=0,idx=-1; i<= LastTrack; i++) {
	if(tmptrc[i]->selected) {
	    traces[++idx]=tmptrc[i];
	    traces[idx]->itrc= idx;
	}
    }
    /* fill in not selected */
    for(i=0; i<= LastTrack; i++) {
	if(!tmptrc[i]->selected) {
	    traces[++idx]=tmptrc[i];
	    traces[idx]->itrc= idx;
	}
    }
    if(idx!=LastTrack)	/* just to be sure */
	fprintf(stderr,"Rearr_sel_top: somethings's missing!\n");
    free(tmptrc);

    RedrawScreen();
    if (ZoomWindowMapped) {
	RedrawZoomWindow("Rearrange_selected_top");
    }
}

/**********************************************************************
 *   Arrange Tracks                                                   *
 **********************************************************************/

/* insert track at itrc */
static int InsertTrack(Trace *trc, int itrc)   /* NOT USED */
{
    int i;
    if (LastTrack==TotalTracks-1) {
	fprintf(stderr, "Tracks full.\n");
	return 0;
    }
    free(traces[LastTrack+1]);	/* free the extra one */
    for(i=LastTrack; i>= itrc; i--) {
	traces[i+1]= traces[i];
	traces[i+1]->itrc= i+1;
    }
    traces[itrc]= trc;
    trc->itrc= itrc;
    LastTrack++;
    AdjustSbar();
    AdjustZSbar(ZCvsHeight);

    return 1;
}

Trace *DupTrace(Trace *trc, int dupWave) /* not used */
{
    Trace *ntrc= (Trace *)Malloc(sizeof(Trace));

    (*ntrc)= (*trc);	/* copy bulk */
    if(trc->wave && dupWave) {
	ntrc->wave= (Wave *)Malloc(sizeof(Wave));
	ntrc->wave->info= trc->wave->info;
	ntrc->wave->data= (float *)Malloc(sizeof(float) *
					  trc->wave->info.n_values);
	bcopy(trc->wave->data, ntrc->wave->data, sizeof(float) *
	      trc->wave->info.n_values);
	if(trc->picks) {
	    Pick *p, *t;
	    ntrc->picks= t= (Pick *)Malloc(sizeof(Pick));
	    p=trc->picks;
	    *t= *p; /* copy pick */
	    while(p->next) {
		p= p->next;
		t->next= (Pick *)Malloc(sizeof(Pick));
		*t->next= *p;
		t= t->next;
	    }
	    t->next= NULL;
	}
	if(trc->amps) {
	    Amp *a, *t;
	    ntrc->amps= t= (Amp *)Malloc(sizeof(Amp));
	    a=trc->amps;
	    *t= *a; /* copy amp */
	    while(a->next) {
		a= a->next;
		t->next= (Amp *)Malloc(sizeof(Amp));
		*t->next= *a;
		t= t->next;
	    }
	    t->next= NULL;
	}
	if(trc->trip) {
	    ntrc->trip= (Triplet*)makeTriplet();
	    associate_triplet(ntrc->trip,ntrc);
	}
    }
    return ntrc;
}
