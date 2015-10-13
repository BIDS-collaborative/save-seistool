#ifndef lint
static char id[] = "$Id: ztrackmgr.c,v 1.3 2013/02/28 21:24:55 lombard Exp $";
#endif

/*
 * ztrackmgr.c--
 *    trace management in zoom window
 *
 * Copyright (c) 1992-3 Andrew K. Yu, Lawrence Berkeley Laboratory
 *      and University of California, Berkeley.
 * All rights reserved.
 */
#include <math.h>
#include <stdio.h>
#include "proto.h"
#include "xv_proto.h"

/**********************************************************************
 *   Variables tracking position of Zoom Tracks                       *
 **********************************************************************/

extern Track **tracks;
extern Trace **traces;
extern int NumTracks;
extern int LastTrack;
extern int lowTrkIndex;
extern int highTrkIndex;


int NumZTracks = NumInitZTracks;
int lowZTrkIndex = 0;
int highZTrkIndex = NumInitZTracks - 1;

/* internal function prototypes */
static void UnstretchZTrk(int iztrk);
void StretchZTrk(int iztrk);


/**********************************************************************
 *   Scrolling Zoom Tracks                                            *
 **********************************************************************/

#define checkScrollable() \
    if (LastTrack==-1 || (LastTrack < NumZTracks)) return;

void ztrk_scroll_var(int amount, int dir)
{
    int i, oldl, oldh;

    checkScrollable();
    oldl= lowZTrkIndex;
    oldh= highZTrkIndex;
    CleanMarks();
    if (dir==SCRL_UP) {
	lowZTrkIndex-=amount;
	highZTrkIndex-=amount;
	if (lowZTrkIndex<0) {
	    lowZTrkIndex= 0;
	    highZTrkIndex= NumZTracks-1;
	}
	if (Mode_triplet) {  /* triplet mode */
	    if(amount<=NumZTracks) {
		Trip_ZoomContentChanged((oldl>1)?oldl-1:0);
	    }else {
		Trip_ZoomContentChanged(lowZTrkIndex);
	    }
	}
	for(i=lowZTrkIndex; i <= highZTrkIndex; i++) {
	    ScaleZTrack(traces[i], traces[oldl]);
	    if (Zoom_Mode_sameVertScale) traces[i]->zaxis_needScale= 1;
	    UpdateMarks(i);
	}
	if (Zoom_Mode_sameVertScale)  newVertScale(); /* same vert scale */
    }else { /* down */
	lowZTrkIndex+= amount;
	highZTrkIndex+= amount;
	if (highZTrkIndex>LastTrack) {
	    lowZTrkIndex= LastTrack-NumZTracks+1;
	    highZTrkIndex= LastTrack;
	}
	if (Mode_triplet) { /* triplet mode */
	    if(amount<=NumZTracks) {
		Trip_ZoomContentChanged((oldh<LastTrack-1)?oldh+1:LastTrack);
	    }else {
		Trip_ZoomContentChanged(lowZTrkIndex);
	    }
	}
        for(i=lowZTrkIndex; i<=highZTrkIndex; i++) {
	    ScaleZTrack(traces[i],traces[oldh]);
	    if (Zoom_Mode_sameVertScale) traces[i]->zaxis_needScale= 1;
	    UpdateMarks(i);
	}
	if (Zoom_Mode_sameVertScale)  newVertScale();
    }
    if (Mode_trvlTime) Trvl_ZoomContentChanged();
    if (Mode_rotate) Rot_ZoomContentChanged();
    RedrawZoomWindow("ztrk_scroll_var");
    if (Mode_autoScroll) {
	AutoScroll(dir);
	SetSbarPosition();
    }
}

void ztrk_scroll_up()
{
    int i, oldl= lowZTrkIndex;

    if (lowZTrkIndex==0)
	return;
    ztrk_scroll_var(1, SCRL_UP);
}

void ztrk_scroll_down()
{
    int i,oldh= highZTrkIndex;

    if (highZTrkIndex>LastTrack)	/* should depend on totalTracks ?? */
	return;
    ztrk_scroll_var(1, SCRL_DOWN);
}

void ztrk_scroll_pgup()
{
    ztrk_scroll_var(NumZTracks, SCRL_UP);
}

void ztrk_scroll_pgdown()
{
    ztrk_scroll_var(NumZTracks, SCRL_DOWN);
}

void ztrk_scroll_right()
{
    /* do this for all tracks on zoom window */
    int i, len;
    Trace *trc;

    if (Mode_triplet) {
	int s_idx, e_idx, ix2, ix1, len, len2;
	CleanMarks();
	trc= traces[lowZTrkIndex];
	getTripBound(trc,&s_idx,&e_idx);
	ix2= trc->zaxis.ix2; ix1= trc->zaxis.ix1;
	len= ix2-ix1;
	ix1= ix2; ix2= ix1+len;
	if(ix2>e_idx) {
	    ix2=e_idx;
	    ix1=e_idx-len;
	}
	if(ix1<s_idx)ix1=s_idx;
	len=ix1-trc->zaxis.ix1;
	len2=ix2-trc->zaxis.ix2;
	if(trc->wave) {
	    trc->zaxis.ix1= ix1; trc->zaxis.ix2=ix2;
	}
	for(i=lowZTrkIndex+1; i<= highZTrkIndex; i++) {
	    trc= traces[i];
	    if(trc->wave) {
		trc->zaxis.ix1+= len;
		trc->zaxis.ix2+= len2;
	    }
	}
	UpdateAllMarks();
    }else {
	for(i=lowZTrkIndex; i<= highZTrkIndex; i++) {
	    /* same horizontal scale */
	    trc= traces[i];
	    if (trc->wave) {
		UpdateMarks(i); /* clean up */
		len= trc->zaxis.ix2 - trc->zaxis.ix1;
		trc->zaxis.ix1= trc->zaxis.ix2;
		trc->zaxis.ix2= trc->zaxis.ix1 + len;
		fixZtrkBounds(trc, len);
		UpdateMarks(i);
	    }
	}
    }
    if (Mode_trvlTime) Trvl_Rescale();
    RedrawZoomWindow("ztrk_scroll_right");
}

void ztrk_scroll_halfright()
{
    /* do this for all tracks on zoom window */
    int i, len;
    Trace *trc;
    if (Mode_triplet) {
	int s_idx, e_idx, ix2, ix1, len, len2;
	CleanMarks();
	trc= traces[lowZTrkIndex];
	getTripBound(trc,&s_idx,&e_idx);
	ix2= trc->zaxis.ix2; ix1= trc->zaxis.ix1;
	len= ix2-ix1;
	ix1= ix1+len/2; ix2= ix1+len;
	if(ix2>e_idx) {
	    ix2=e_idx;
	    ix1=e_idx-len;
	}
	if(ix1<s_idx)ix1=s_idx;
	len=ix1-trc->zaxis.ix1;
	len2=ix2-trc->zaxis.ix2;
	if(trc->wave) {
	    trc->zaxis.ix1= ix1; trc->zaxis.ix2=ix2;
	}
	for(i=lowZTrkIndex+1; i<= highZTrkIndex; i++) {
	    trc= traces[i];
	    if(trc->wave) {
		trc->zaxis.ix1+= len;
		trc->zaxis.ix2+= len2;
	    }
	}
	UpdateAllMarks();
    }else {
	for(i=lowZTrkIndex; i<= highZTrkIndex; i++) {
	    /* same horiztal scale */
	    trc= traces[i];
	    if (trc->wave) {
		UpdateMarks(i);
		len= trc->zaxis.ix2 - trc->zaxis.ix1;
		trc->zaxis.ix1= trc->zaxis.ix1 + len/2;
		trc->zaxis.ix2= trc->zaxis.ix1 + len;
		fixZtrkBounds(trc, len);
		UpdateMarks(i);
	    }
	}
    }
    if (Mode_trvlTime) Trvl_Rescale();
    RedrawZoomWindow("ztrk_scroll_half_right");
}

void ztrk_scroll_left()
{
    /* do this for all tracks on zoom window */
    int i, len;
    Trace *trc;
    if (Mode_triplet) {
	int s_idx, e_idx, ix2, ix1, len, len2;
	CleanMarks();
	trc= traces[lowZTrkIndex];
	getTripBound(trc,&s_idx,&e_idx);
	ix2= trc->zaxis.ix2; ix1= trc->zaxis.ix1;
	len= ix2-ix1;
	ix2= ix1; ix1= ix2-len;
	if(ix1<s_idx) {
	    ix1=s_idx;
	    ix2=s_idx+len;
	}
	if(ix2>e_idx)ix2=e_idx;
	len=trc->zaxis.ix1-ix1;
	len2=trc->zaxis.ix2-ix2;
	if(trc->wave) {
	    trc->zaxis.ix1= ix1; trc->zaxis.ix2=ix2;
	}
	for(i=lowZTrkIndex+1; i<= highZTrkIndex; i++) {
	    trc= traces[i];
	    if(trc->wave) {
		trc->zaxis.ix1-= len;
		trc->zaxis.ix2-= len2;
	    }
	}
	UpdateAllMarks();
    }else {
	for(i=lowZTrkIndex; i<= highZTrkIndex; i++) {
	    /* same horiztal scale */
	    trc= traces[i];
	    if (trc->wave) {
		UpdateMarks(i);
		len= trc->zaxis.ix2 - trc->zaxis.ix1;
		trc->zaxis.ix2= trc->zaxis.ix1;
		trc->zaxis.ix1= trc->zaxis.ix2-len;
		fixZtrkBounds(trc, len);
		UpdateMarks(i);
	    }
	}
    }
    if (Mode_trvlTime) Trvl_Rescale();
    RedrawZoomWindow("ztrk_scroll_left");
}

void ztrk_scroll_halfleft()
{
    /* do this for all tracks on zoom window */
    int i, len;
    Trace *trc;
    if (Mode_triplet) {
	int s_idx, e_idx, ix2, ix1, len, len2;
	CleanMarks();
	trc= traces[lowZTrkIndex];
	getTripBound(trc,&s_idx,&e_idx);
	ix2= trc->zaxis.ix2; ix1= trc->zaxis.ix1;
	len= ix2-ix1;
	ix2= ix1+len/2; ix1= ix2-len;
	if(ix1<s_idx) {
	    ix1=s_idx;
	    ix2=s_idx+len;
	}
	if(ix2>e_idx)ix2=e_idx;
	len=trc->zaxis.ix1-ix1;
	len2=trc->zaxis.ix2-ix2;
	if(trc->wave) {
	    trc->zaxis.ix1= ix1; trc->zaxis.ix2=ix2;
	}
	for(i=lowZTrkIndex+1; i<= highZTrkIndex; i++) {
	    trc= traces[i];
	    if(trc->wave) {
		trc->zaxis.ix1-= len;
		trc->zaxis.ix2-= len2;
	    }
	}
	UpdateAllMarks();
    }else {
	for(i=lowZTrkIndex; i<= highZTrkIndex; i++) {
	    /* same horiztal scale */
	    trc= traces[i];
	    if (trc->wave) {
		UpdateMarks(i);
		len= trc->zaxis.ix2 - trc->zaxis.ix1;
		trc->zaxis.ix2= trc->zaxis.ix1 + len/2;
		trc->zaxis.ix1= trc->zaxis.ix2-len;
		fixZtrkBounds(trc, len);
		UpdateMarks(i);
	    }
	}
    }
    if (Mode_trvlTime) Trvl_Rescale();
    RedrawZoomWindow("ztrk_scroll_halfleft");
}

/* direction: 0 is up, 1 down */
void AutoScroll(int direction)
{
    if (direction==SCRL_UP) {
	if (lowZTrkIndex<lowTrkIndex) {
	    /* needs to scroll main window also */
	    highTrkIndex= highZTrkIndex;
	    lowTrkIndex= highTrkIndex - NumTracks + 1;
	    if (lowTrkIndex<0) {
		lowTrkIndex=0;
		highTrkIndex= NumTracks-1;
	    }
	    RedrawScreen();
	}
    }else {
	if (highZTrkIndex>highTrkIndex) {
	    /* needs to scroll main window also */
	    lowTrkIndex= lowZTrkIndex;
	    highTrkIndex= lowTrkIndex + NumTracks - 1;
	    if (highTrkIndex>LastTrack) {
		highTrkIndex= LastTrack;
		lowTrkIndex= highTrkIndex - NumTracks + 1;
	    }
	    RedrawScreen();
	}
    }
}

/**********************************************************************
 *   Scaling Zoom Tracks                                              *
 **********************************************************************/

/* historical! I gave up: naming the following one way or the other
   doesn't help the strange feelings
   -- problem now gone with the stretch/unstretch buttons in zoom window
      gone. AY (5/93) */

static void UnstretchZTrk(int iztrk)
{
    Trace *trc;
    int len, midpt;

    trc= traces[iztrk];
    if (trc->wave) {
	len= trc->zaxis.ix2-trc->zaxis.ix1;
	midpt= trc->zaxis.ix1+len/2;
	trc->zaxis.ix1= midpt-len;
	trc->zaxis.ix2= midpt+len;
	fixZtrkBounds(trc, len*2);
	if(Mode_triplet)
	    checkTripBound(trc);
	ScaleTAxis(&trc->zaxis, trc->wave->info.sample_rate, ZCvsWidth);
    }
}

void StretchZTrk(int iztrk)
{
    Trace *trc;
    int len, midpt;

    trc= traces[iztrk];
    if (trc->wave) {
	len= trc->zaxis.ix2-trc->zaxis.ix1;
	if (len<4) return;  /* cannot unstretch further */
	midpt= trc->zaxis.ix1+len/2;
	trc->zaxis.ix1= midpt-len/4;
	trc->zaxis.ix2= midpt+len/4;
	fixZtrkBounds(trc, len/2);
	if(Mode_triplet)
	    checkTripBound(trc);
	ScaleTAxis(&trc->zaxis, trc->wave->info.sample_rate, ZCvsWidth);
    }
}

void ztrk_stretch()
{
    int i;
    
    CleanMarks();
    for(i=lowZTrkIndex; i <=highZTrkIndex; i++) {
	StretchZTrk(i);
    }
    UpdateAllMarks();
    if (Mode_trvlTime) Trvl_Rescale();
    RedrawZoomWindow("ztrk_stretch");
}


void ztrk_unstretch()
{
    int i;
    
    CleanMarks();
    for(i=lowZTrkIndex; i <=highZTrkIndex; i++) {
	UnstretchZTrk(i);
    }
    UpdateAllMarks();
    if (Mode_trvlTime) Trvl_Rescale();
    RedrawZoomWindow("ztrk_unstretch");
}

void CompressZTrk(int iztrk)
{
    Trace *trc;
    float vs;

    trc= traces[iztrk];
    if (trc->wave) {
	vs= trc->zaxis.vs * 2;
	trc->zaxis.vsmag/= 2;
	trc->zaxis.vs= vs;
    }
}

void ztrk_RaiseDC(int itrc)
{
    Trace *trc= traces[itrc];
    float adj;

    adj= (trc->zaxis.y1 - trc->zaxis.y2)/10.0;
    /*trc->zaxis.y0 += adj/(trc->zaxis.vs*trc->zaxis.vsmag);*/
    trc->zaxis.y0 += adj/(trc->zaxis.vs);
}

void ztrk_LowerDC(int itrc)
{
    Trace *trc= traces[itrc];
    float adj;

    adj= (trc->zaxis.y1 - trc->zaxis.y2)/10.0;
    /*trc->zaxis.y0 -= adj/(trc->zaxis.vs*trc->zaxis.vsmag);*/
      trc->zaxis.y0 -= adj/(trc->zaxis.vs);
}

void ztrk_CenterDC(int itrc)
{
    Trace *trc = traces[itrc];
    Axis *ax = &trc->zaxis;
    float adj = 0.0;
    float y0;
    int i,n = 0;

    for (i = ax->ix1; i < ax->ix2; i++, n++)
	adj += trc->wave->data[i];
    adj /= (float)n;

    y0 = ZTrkHeight/2;
    ax->y0 = y0 + adj/(trc->zaxis.vs);
}

void ztrk_ZeroDC(int itrc)
{
    Trace *trc = traces[itrc];
    ScaleYAxis(&trc->zaxis, ZTrkHeight);
}


void ztrk_compress()
{
    int i;

    /* no need to mess with the marks on zoom window */
    for(i=lowZTrkIndex; i <=highZTrkIndex; i++) {
	CompressZTrk(i);
    }
    RedrawZoomWindow("ztrk_compress");
}

void ExpandZTrk(int iztrk)
{
    Trace *trc;
    float vs;

    trc= traces[iztrk];
    if (trc->wave) {
	vs= trc->zaxis.vs / 2;
	trc->zaxis.vsmag*= 2;
	if (vs > 0) trc->zaxis.vs= vs;
    }
}

void ztrk_expand()
{
    int i;

    /* no need to mess with the marks on zoom window */
    for(i=lowZTrkIndex; i <=highZTrkIndex; i++) {
	ExpandZTrk(i);
    }
    RedrawZoomWindow("ztrk_expand");
}

/**********************************************************************
 *   Marking Intervals                                                *
 **********************************************************************/

void Zoom_UpdateInterval(int itrc)
{
  extern GC shade_gc[8];
  int x1, x2;
  int i=0;
  float srate;
  Trace *trc=traces[itrc];
  Reg_select *curr=trc->sel_reg;
  GC gc_temp;

  /* if the trace is in the window */
  if (itrc<=highZTrkIndex && itrc>=lowZTrkIndex && trc !=NULL && trc->wave !=NULL) {
    srate= trc->wave->info.sample_rate;
    InitStipples(); 
    while (curr!=NULL) {
      gc_temp=shade_gc[i++];
      XSetFunction(theDisp, gc_temp, GXxor);
      XSetPlaneMask(theDisp, gc_temp,AllPlanes);
      x1= indexToCoord(&trc->zaxis, curr->right_index, 1);
      x2= indexToCoord(&trc->zaxis, curr->left_index, 1);
      Zoom_DrawInterval(itrc, x1, x2,gc_temp);
      curr=curr->next;
      if (i>7) {i=0;}
    }
  }
}

void Zoom_DrawInterval(int itrc, int x1, int x2, GC gc_val)
{
  int bx1,bx2;  /* bounderies of the Zoom window */
  int yoffset;  /* y location of the trace in the Zoom window */

  /* if the trace is in the window */
  if (itrc<=highZTrkIndex && itrc>=lowZTrkIndex) {
    Trace *trc= traces[itrc];

    yoffset=(itrc-lowZTrkIndex) * ZTrkHeight;
    bx1=indexToCoord(&trc->zaxis, trc->zaxis.ix1, 1 );
    bx2=indexToCoord(&trc->zaxis, trc->zaxis.ix2, 1 );

    if(x2<x1) {int t=x1; x1=x2; x2=t; }
    if(bx2<bx1) {int t=bx1; bx1=bx2; bx2=t; }

    /* if in the region */
    if(x1>bx2||x2<bx1) {return;}
    if(x1<bx1) {x1=bx1;}
    if(x2>bx2) {x2=bx2;}

    XFillRectangle(theDisp, ZCvsWin, gc_val,
		   x1, yoffset, x2-x1+1, ZTrkHeight);
  }
}


/**********************************************************************
 *                                                                    *
 **********************************************************************/

void fixZtrkBounds(Trace *trc, int len)
{
    int tot_len= trc->axis.ix2-trc->axis.ix1;
    if(len>tot_len)
	len= tot_len;
    if(trc->zaxis.ix1<trc->axis.ix1) {
	trc->zaxis.ix1= trc->axis.ix1;
	trc->zaxis.ix2= trc->axis.ix1 + len;
    }else if(trc->zaxis.ix2>trc->axis.ix2) {
	trc->zaxis.ix2= trc->axis.ix2;
	trc->zaxis.ix1= trc->axis.ix2 - len;
    }
}
