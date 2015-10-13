#ifndef lint
static char id[] = "$Id: freqplot.c,v 1.2 2013/02/28 21:25:00 lombard Exp $";
#endif

/*
 * freqplot.c--
 *    frequencies vs. times plot
 *
 * Copyright (c) 1992-3 Andrew K. Yu, Lawrence Berkeley Laboratory
 *      and University of California, Berkeley.
 * All rights reserved.
 */
#include <math.h>
#include <stdio.h>
#include <strings.h>
#include <xview/xview.h>
#include <xview/frame.h>
#include <xview/canvas.h>
#include <xview/panel.h>

#include "mathlib.h"
#include "proto.h"
#include "xv_proto.h"

/**********************************************************************
 *   Spectrum window                                                  *
 **********************************************************************/

/* globals */
static Frame	fplt_frame= NULL;
static Canvas	fplt_cvs= NULL;
static Window	fplt_win;
static GC	fplt_gc;
static Panel_item len_txt, step_txt, itrc_txt, scale_txt;

extern GC shade_gc[8];
extern Trace **traces;
extern int LastTrack;

static int fpltwin_h= 600;
static int fpltwin_w= 800;

/* computed data for the current spectrum*/
static float **fplt_y= 0;		
static float fplt_srate;		
static int fplt_n2, fplt_nt2h;	
static int cur_itrc=0;
static float **fplt_a=NULL, **fplt_b=NULL;
static float global_ymax, global_ymin;

static int fplt_steps= 0;
static int fplt_sf= 3;
static int fplt_twin_len=0, fplt_twin_step=0;

static int fplt_soph= 1;

static int fplt_shades= 1;

/* prototypes */
static void InitFpltFrame();
static void options_menu_proc(Menu menu, Menu_item menu_item);
static void open_fplt_win(int itrc);
static void close_fplt_win();
static void PlotFreqplot(int itrc);
static void redrawFplt();
static void PrintDecade(int i, int x, int y);
static void PlotXAxis(float ymax, float ymin, float vs);
static void PlotYAxis(float freqk, int hmin, int hmax, float hs, int nt2h);
static void DrawFreqplot(float **y_arr, float srate, int n2, int nt2h);
static void go_fplt();
static void PlotLegend(float ymax, float ymin, int start_shade);


/**********************************************************************
 *   set up spectrum window                                           *
 **********************************************************************/

static void InitFpltFrame()
{
    Panel panel;
    Menu menu;
    
    fplt_frame= xv_create(tracesFrame, FRAME,
			  XV_HEIGHT, fpltwin_h+40, XV_WIDTH, fpltwin_w,
			  NULL);		  
    fplt_cvs= xv_create(fplt_frame, CANVAS,
			XV_X, 0, XV_Y, 40,		  
			CANVAS_REPAINT_PROC, redrawFplt,		  
			NULL);		  
    fplt_win= xv_get(canvas_paint_window(fplt_cvs), XV_XID);
    createGC( fplt_win, &fplt_gc);
    panel= (Panel)xv_create(fplt_frame, PANEL,
			    XV_X, 0, XV_Y, 0, XV_HEIGHT, 40,
			    NULL);
    (void)xv_create(panel, PANEL_BUTTON,
		    PANEL_LABEL_STRING, "Quit",
		    PANEL_NOTIFY_PROC, close_fplt_win,
		    NULL);
    menu= (Menu)xv_create(NULL,MENU,
			  MENU_STRINGS,
			  "taper etc.", "no taper", "shades", "no shades",
			  NULL,
			  MENU_NOTIFY_PROC,   options_menu_proc,
			  NULL);
    (void)xv_create(panel, PANEL_BUTTON,
		    PANEL_LABEL_STRING, "Options",
		    PANEL_ITEM_MENU, menu,
		    NULL);
    len_txt= (Panel_item)xv_create(panel, PANEL_TEXT,
				   PANEL_LABEL_STRING, "Length",
				   PANEL_VALUE_DISPLAY_WIDTH, 60,			  
				   NULL);
    step_txt= (Panel_item)xv_create(panel, PANEL_TEXT,
				    PANEL_LABEL_STRING, "Step",
				    PANEL_VALUE_DISPLAY_WIDTH, 60,			  
				    NULL);
    scale_txt= (Panel_item)xv_create(panel, PANEL_TEXT,
				     PANEL_LABEL_STRING, "Scale",
				     PANEL_VALUE_DISPLAY_WIDTH, 60,			  
				     NULL);
    itrc_txt= (Panel_item)xv_create(panel, PANEL_NUMERIC_TEXT,
				    PANEL_LABEL_STRING, "Trc No.",
				    PANEL_VALUE_DISPLAY_WIDTH, 60,			  
				    NULL);
    (void)xv_create(panel, PANEL_BUTTON,
		    PANEL_LABEL_STRING, "Go",
		    PANEL_NOTIFY_PROC, go_fplt,
		    NULL);
    InitStipples();
}

static void options_menu_proc(Menu menu, Menu_item menu_item)
{
    char *s= (char *)xv_get(menu_item, MENU_STRING);
    if(!strcmp(s, "taper etc.")) {
	fplt_soph= 1;
    }else if(!strncmp(s, "no ta", 5)) {
	fplt_soph= 0;
    }else if(!strncmp(s, "shade", 5)) {
	fplt_shades= 1;
    }else if(!strncmp(s, "no sh", 5)) {
	fplt_shades= 0;
    }
}

static void open_fplt_win(int itrc)
{
    char buf[200];
    char *t;
    BIS3_HEADER *bh= &traces[itrc]->wave->info;

    cur_itrc= itrc;
    if(!fplt_frame) InitFpltFrame();
    t=(char *)rindex(traces[itrc]->filename,'/');
    t= (t!=NULL)? t+1:traces[itrc]->filename;
    sprintf(buf,"[%d] %s %s %s %s Freq-Time Plot (%s)", itrc, bh->station,
	    bh->network, bh->channel, 
	    (strlen(bh->location) > 0) ? bh->location : "--", t);
    xv_set(fplt_frame, XV_SHOW, TRUE,
	   FRAME_LABEL, buf,
	   NULL);
}

static void close_fplt_win()
{
    xv_set(fplt_frame, XV_SHOW, FALSE, NULL);
}
    
/**********************************************************************
 *   Plotting the Freq plot                                           *
 **********************************************************************/

static int plotted= 0;

static void PlotFreqplot(int itrc)
{
    Trace *trc= traces[itrc];
    float *y;                 /* Spectral power?             */
    float srate;              /* sample rate of trace        */
    float *a, *b;             /* amp and phase of trace      */
    int nt2h;                 /* number of bins in the spect */
    int n2;
    int i, j, k;              /* counters....                */
    int len;                  /* length of spectra           */
    int steps;                /* number of traces to show    */
    int steplen;              /* length in time bins of steps*/ 
    float ymax, ymin;         /* max and min values of Spect */

    srate= trc->wave->info.sample_rate;
    if (fplt_twin_len>0 && fplt_twin_step>0) {
	len= fplt_twin_len;
	steplen= fplt_twin_step;
    }else {
	return;
    }
    steps= trc->wave->info.n_values/steplen;
    if(fplt_y) {
	/* clean up */
	free(fplt_y);
	free(fplt_a);
	free(fplt_b);
    }
    fplt_y= (float **)Malloc(sizeof(float *)*steps);
    bzero(fplt_y, sizeof(float*)*steps);
    /* actually we don't need the (a,b) right now but CalcSpectrum
       expects the array... */
    fplt_a= (float **)Malloc(sizeof(float *)*steps);
    bzero(fplt_a, sizeof(float*)*steps);
    fplt_b= (float **)Malloc(sizeof(float *)*steps);
    bzero(fplt_b, sizeof(float*)*steps);

    /* calc spectrum of whole trace */
    CalcSpectrum(srate, trc->wave->data, 0, len, &y, &a, &b,
		 &n2, &nt2h, fplt_soph);
    if (y[0]>0) {
	ymax=ymin=y[0]= (float)log10((double)y[0]);
    } else {
	ymax=-1;
	ymin=1e8;
	y[0]=-1;
    }
    for(k=1;k<nt2h-1;k++) {
	if (y[k]<=0) {
	    y[k]=-1;
	} else {
	    y[k]= (float)log10((double)y[k]);
	    if(y[k]>ymax)
		ymax=y[k];
	    else if(y[k]<ymin)
		ymin=y[k];
	}
    }
    for(k=0;k<nt2h-1;k++) {
	if (y[k]==-1) {
	    y[k]=ymin;
	}
    }

    fplt_y[0]=y;
    fplt_a[0]= a; fplt_b[0]= b;
    fplt_srate=srate;
    fplt_n2=n2;
    fplt_nt2h= nt2h;

    /* do the individual pieces */
    /* i is the position in the trace to be done */
    for(i=1,j=1; i < trc->wave->info.n_values-len-1; i+= steplen, j++) {
	CalcSpectrum(srate, trc->wave->data, i, i+len, &y, &a, &b,
		     &n2, &nt2h, fplt_soph);
	for(k=0;k<nt2h-1;k++) {
	    if (y[k]<=0) {
		y[k]=-1;
	    } else {
		y[k]= (float)log10((double)y[k]);
		if(y[k]>ymax)
		    ymax=y[k];
		else if(y[k]<ymin)
		    ymin=y[k];
	    }
	}
	for(k=0;k<nt2h-1;k++) {
	    if (y[k]==-1) {
		y[k]=ymin;
	    }
	}

	/* store the values in the global arrays */
	fplt_y[j]=y;              
	fplt_a[j]= a; fplt_b[j]= b;
    }
    global_ymax= ymax;
    global_ymin= ymin;
    fplt_steps= j;
    plotted= 1;
    redrawFplt();
}

static void redrawFplt()
{
    XClearWindow(theDisp, fplt_win);
    DrawFreqplot(fplt_y, fplt_srate, fplt_n2, fplt_nt2h);
}

#define FPWIN_X_OFF 50
#define FPWIN_Y_OFF 50
#define MARGIN	10

/* This function seems to be static in several different files. *
 * Can they be consolidated into one file?                      */
static void PrintDecade(int i, int x, int y)
{
    char buf[30];
    sprintf(buf,"%d",i);
    XDrawString(theDisp, fplt_win, fplt_gc, x, y+5, "10", 2); 
    XDrawString(theDisp, fplt_win, fplt_gc, x+14, y, buf, strlen(buf));   
}

static void PlotXAxis(float ymax, float ymin, float vs)
{
    char buf[30];
    int x, y;
    int i;

    /* axis */
    XDrawLine(theDisp, fplt_win, fplt_gc, FPWIN_X_OFF-7, fpltwin_h-FPWIN_Y_OFF, 
	      fpltwin_w-MARGIN, fpltwin_h-FPWIN_Y_OFF);

    /* draw a time axis */

    /* labels */
    XDrawString(theDisp, fplt_win, fplt_gc, 380,
		fpltwin_h-FPWIN_Y_OFF+35, "T I M E", 7);
}

static void PlotYAxis(float freqk, int hmin, int hmax, float hs, int nt2h)
{
    char buf[30];
    int i, x, y;

    /* axis */
    XDrawLine(theDisp, fplt_win, fplt_gc, FPWIN_X_OFF-7, MARGIN,
	      FPWIN_X_OFF-7, fpltwin_h-FPWIN_Y_OFF);

    /* labels */
    DrawVertString(fplt_win, fplt_gc, 10-3, 200, "FREQUENCY");

    PrintDecade(hmin, FPWIN_X_OFF/2-7, fpltwin_h-FPWIN_Y_OFF-7);
    PrintDecade(hmax, FPWIN_X_OFF/2-7, MARGIN+10);
    for(i=hmin+1; i<= hmax-1; i++) {
	y= (hmax-i)*hs+ MARGIN;
	XDrawLine(theDisp, fplt_win, fplt_gc, FPWIN_X_OFF-7, y,
		  FPWIN_X_OFF+8-7, y);
	PrintDecade(i, FPWIN_X_OFF/2-7, y);
    }
    /* smaller marks */
    for(i=hmin; i<= hmax-1; i++) {
	int k, yy;
	y= (hmax-i)*hs + MARGIN;
	for(k=1; k<10; k++) {
	    yy= y-log10((double)k)*hs;
	    XDrawLine(theDisp, fplt_win, fplt_gc, FPWIN_X_OFF-7, yy,
		      FPWIN_X_OFF+4-7, yy);
	}
    }
}

static void DrawFreqplot(float **y_arr, float srate, int n2, int nt2h)
{
    float hs, vs;
    float ymax, ymin;
    int hmin, hmax;
    XPoint *points, *plot;
    int npts, i, x;
    char buf[30];
    float freqk;

    int j;
    float *y;

    Trace *trc=traces[cur_itrc];

    if(!plotted)return;
    
    freqk= srate/n2; 
    hmax= ceil(log10((double)(freqk*(nt2h-1))));
    hmin= floor(log10((double)freqk));
    hs= (float)(fpltwin_h-FPWIN_Y_OFF-MARGIN)/(hmax-hmin);

    PlotYAxis(freqk, hmin, hmax, hs, nt2h);
    PlotXAxis(0, 0, vs);

    if(fplt_shades==0) {

	float vertlen= (float)(fpltwin_w-FPWIN_X_OFF-MARGIN)/(fplt_steps+fplt_sf);

	for(j=0; j<fplt_steps; j++) {

	    y= y_arr[j];

	    ymax= ceil((double)global_ymax);
	    ymin= floor((double)global_ymin);
	    vs= (float)(vertlen*fplt_sf)/(ymax-ymin);

	    plot= points= (XPoint *)Malloc(sizeof(XPoint)* (nt2h+1));

	    npts=1;
	    points->x= FPWIN_X_OFF+vertlen*(j+fplt_sf)-(y[0]-ymin)*vs;
	    points->y= fpltwin_h-FPWIN_Y_OFF-(log10((double)freqk)-hmin)*hs;
	    for(i=1; i < nt2h-1; i++) {
		x= fpltwin_h-FPWIN_Y_OFF-
		    (log10((double)(freqk*(i+1))) - hmin)*hs;
		points++;
		points->x= FPWIN_X_OFF+vertlen*(j+fplt_sf)-(y[i]-ymin)*vs;
		points->y= x;
		npts++;
	    }
	    XDrawLines(theDisp, fplt_win, fplt_gc, plot, npts, CoordModeOrigin);
	    free(plot);
	}

    }else {			/* use shades */

	int vertlen= (fpltwin_w-FPWIN_X_OFF-MARGIN)/fplt_steps;

	for(j=0; j<fplt_steps; j++) {
	    int start_shade;
	    int x, y1, y2;
	    int sh[8],ss;
	    int s;
	
	    y= y_arr[j];
	    ymax= ceil((double)global_ymax);
	    ymin= floor((double)global_ymin);
	    if ((ymax-ymin)>9) {
		start_shade= 0;
	    }else {
		start_shade= 9-(int)(ymax-ymin);
	    }
	    plot= points= (XPoint *)Malloc(sizeof(XPoint)* (nt2h+1));

	    x= FPWIN_X_OFF+vertlen*(j);
	    for(ss=0; ss<8; ss++) {
		sh[ss]=0;
	    }
	    y1= fpltwin_h-FPWIN_Y_OFF- 
		(log10((double)(freqk))-hmin)*hs;
	    s= (int)floor((double)(y[1]-ymin))+start_shade-1;
	    if(s>7)s=7;
	    for(i=2; i < nt2h-1; i++) {
		y2= fpltwin_h-FPWIN_Y_OFF- 
		    (log10((double)(freqk*(i))) - hmin)*hs;
		if(y2!=y1) {
		    if(s>=0) {
			sh[s]++;
			XFillRectangle(theDisp,fplt_win, shade_gc[s],
				       x, y2, (int)vertlen,y1-y2);
		    }
		    y1= y2;
		    s= (int)floor((double)(y[i]-ymin))+start_shade-1;
		    if(s>7)s=7;
		}
	    }
	    free(plot);
	    PlotLegend(ymax, ymin, start_shade);
	}
    }

    XFlush(theDisp);

}

/**********************************************************************
 *   handle events for spectrum operations                            *
 **********************************************************************/

void handle_freqplot()
{
    cur_itrc= 0;
    go_fplt();
}


static void go_fplt()
{
    Trace *trc;
    char *s;
    int val;
    
    /* check the inputs */
    if(len_txt) {
	s= (char *)xv_get(len_txt, PANEL_VALUE);
	val= atoi(s);
	if(val>0) fplt_twin_len= val;
	s= (char *)xv_get(step_txt, PANEL_VALUE);
	val= atoi(s);
	if(val>0) fplt_twin_step= val;
	s= (char *)xv_get(scale_txt, PANEL_VALUE);
	val= atoi(s);
	if(val>0) fplt_sf= val;
	val= (int)xv_get(itrc_txt, PANEL_VALUE);
	cur_itrc= (val>=0 && val<=LastTrack)? val:0;
    }
    
    trc= traces[cur_itrc];
    /* plot again -- might be different */

    if (trc) {
	open_fplt_win(cur_itrc);

	if(fplt_twin_len>0 && fplt_twin_step>0)
	    /* Why extra arguments in following?  PNL 2013/01/30
	       PlotFreqplot(cur_itrc, trc->mark1_idx, trc->mark2_idx); */
	    PlotFreqplot(cur_itrc);
    }
}

static void PlotLegend(float ymax, float ymin, int start_shade)
{
    int ly= fpltwin_h-30;
    int lx= fpltwin_w-300;
    int i,l_yval,shw;
    char syval[50];

    if(start_shade>0) {
	shw= 200/(8-start_shade);

	XDrawRectangle(theDisp,fplt_win,fplt_gc,lx,ly,shw*(8-start_shade),20);
	for(i=0; i< (8-start_shade); i++) {
	    XFillRectangle(theDisp,fplt_win, shade_gc[start_shade+i],
			   lx+i*shw, ly, shw, 20);
	}
    }else {
	shw= 200/(9-start_shade);

	XDrawRectangle(theDisp,fplt_win,fplt_gc,lx,ly,shw*9,20);
	for(i=0; i< 8; i++) {
	    XFillRectangle(theDisp,fplt_win, shade_gc[i],
			   lx+(i+1)*shw, ly, shw, 20);
	}
    }

    /* added label to scale */
    l_yval=floor(ymin);
    sprintf(syval,"%d",l_yval);
    l_yval=strlen(syval);
    XDrawString(theDisp,fplt_win,fplt_gc,lx,ly-2,syval,l_yval);
    l_yval=ceil(ymax);
    sprintf(syval,"%d",l_yval);
    l_yval=strlen(syval);
    XDrawString(theDisp,fplt_win,fplt_gc,lx+(i+1)*shw,ly-2,syval,l_yval);
    XFlush(theDisp);
}
