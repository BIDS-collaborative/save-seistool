#ifndef lint
static char id[] = "$Id: xspctrm.c,v 1.2 2013/02/28 21:24:56 lombard Exp $";
#endif

/*
 * xspctrm.c--
 *    "Cross-spectrum"
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

#define SPWIN_X_OFF 50
#define SPWIN_Y_OFF 50
#define MARGIN	10

/* globals */
static Frame	xspctrm_frame= NULL;
static Canvas	xspctrm_cvs= NULL;
static Window	xspc_win;
static GC	xspc_gc, xspc_dot_gc, xspc_smdot_gc;
static Panel_item itrc_txt, itrc2_txt;

static int xspctrmwin_h= 600;
static int xspctrmwin_w= 800;
static int use_loglog= 1;	    /* use log-log plot */
static Reg_select *used_r1=NULL;    /* region1 */
static Reg_select *used_r2=NULL;    /* region2 */

static int xspc_gridOn= 1;

/* computed data for the current spectrum*/
static float *xspc_y= 0;		
static float xspc_srate;		
static int xspc_n2, xspc_nt2h;	
static int cur_itrc=-1, cur_itrc2=-1;
static float *xspc_a=NULL, *xspc_b=NULL;

static int xspc_soph= 1;		    /* demean, tapering */

extern Trace **traces;
extern int LastTrack;

/* internal function prototypes */
static void InitXSpctrmFrame();
static void options_menu_proc(Menu menu, Menu_item menu_item);
static void scale_menu_proc(Menu menu, Menu_item menu_item);
static void open_xspctrm_win(int itrc);
static void close_xspctrm_win();
static void PlotXSpectrum(int itrc, int ix1, int ix2);
static void redrawXSpctrm();
static void resizeXSpctrm(Canvas canvas, int width, int height);
static void PrintDecade(int i, int x, int y);
static void PlotXAxis(float freqk, int hmin, int hmax, float hs, int nt2h);
static void PlotYAxis(float ymax, float ymin, float vs);
static void DrawXSpectrum(float *y, float srate, int n2, int nt2h);
static void redo_xspctrm();
static void next_xspctrm();


/**********************************************************************
 *   set up spectrum window                                           *
 **********************************************************************/

static void InitXSpctrmFrame()
{
    Panel panel;
    Menu menu;
    char dash_list[2];
    
    xspctrm_frame= xv_create(tracesFrame, FRAME,
	XV_HEIGHT, xspctrmwin_h+40, XV_WIDTH, xspctrmwin_w,
	NULL);		  
    xspctrm_cvs= xv_create(xspctrm_frame, CANVAS,
	XV_X, 0, XV_Y, 40,
	CANVAS_RETAINED, TRUE,
	CANVAS_REPAINT_PROC, redrawXSpctrm,		  
	CANVAS_RESIZE_PROC, resizeXSpctrm,		  
	NULL);		  
    xspc_win= xv_get(canvas_paint_window(xspctrm_cvs), XV_XID);
    createGC( xspc_win, &xspc_gc);
    createGC( xspc_win, &xspc_dot_gc);
    dash_list[0]= 1;
    dash_list[1]= 5;
    XSetLineAttributes(theDisp, xspc_dot_gc, 1, LineOnOffDash,
		       CapRound, JoinRound);
    createGC( xspc_win, &xspc_smdot_gc);
    XSetLineAttributes(theDisp, xspc_smdot_gc, 1, LineOnOffDash,
		       CapRound, JoinRound);
    XSetDashes(theDisp, xspc_smdot_gc,2,dash_list,2);
    panel= (Panel)xv_create(xspctrm_frame, PANEL,
	XV_X, 0, XV_Y, 0, XV_HEIGHT, 40,
	NULL);
    (void)xv_create(panel, PANEL_BUTTON,
	PANEL_LABEL_STRING, "Quit",
	PANEL_NOTIFY_PROC, close_xspctrm_win,
	NULL);
    (void)xv_create(panel, PANEL_BUTTON,
	PANEL_LABEL_STRING, "Redo",
	PANEL_NOTIFY_PROC, redo_xspctrm,
	NULL);
    (void)xv_create(panel, PANEL_BUTTON,
	PANEL_LABEL_STRING, "Next",
	PANEL_NOTIFY_PROC, next_xspctrm,
	NULL);
    menu= (Menu)xv_create(NULL,MENU,
	MENU_STRINGS,
		"Linear Scale", "Log-Linear Sc", "Log Scale",
		NULL,
	MENU_NOTIFY_PROC,   scale_menu_proc,
	NULL);
    (void)xv_create(panel, PANEL_BUTTON,
	PANEL_LABEL_STRING, "Scale",
	PANEL_ITEM_MENU, menu,
	NULL);
    menu= (Menu)xv_create(NULL,MENU,
	MENU_STRINGS,
		"trace portion", "no trace",
		"taper etc.", "no taper", "grid on", "grid off",
		NULL,
	MENU_NOTIFY_PROC,   options_menu_proc,
	NULL);
    (void)xv_create(panel, PANEL_BUTTON,
	PANEL_LABEL_STRING, "Options",
	PANEL_ITEM_MENU, menu,
	NULL);
    itrc_txt= (Panel_item)xv_create(panel, PANEL_NUMERIC_TEXT,
	PANEL_LABEL_STRING, "Trc No.",
	PANEL_VALUE_DISPLAY_WIDTH, 60,			  
	NULL);
    itrc2_txt= (Panel_item)xv_create(panel, PANEL_NUMERIC_TEXT,
	PANEL_LABEL_STRING, "Trc No.",
	PANEL_VALUE_DISPLAY_WIDTH, 60,			  
	NULL);
}

static void options_menu_proc(Menu menu, Menu_item menu_item)
{
    char *s= (char *)xv_get(menu_item, MENU_STRING);
    if(!strcmp(s, "taper etc.")) {
	xspc_soph= 1;
    }else if(!strncmp(s, "no ta", 5)) {
	xspc_soph= 0;
    }else if(!strcmp(s, "grid on")) {
	xspc_gridOn= 1;
    }else if(!strncmp(s, "grid off", 5)) {
	xspc_gridOn= 0;
    }
}

static void scale_menu_proc(Menu menu, Menu_item menu_item)
{
    char *s= (char *)xv_get(menu_item, MENU_STRING);
    if (!strcmp(s, "Log Scale")) {
	use_loglog= 1;
	redo_xspctrm();
    }else if(!strcmp(s, "Linear Scale")) {
	use_loglog= 0;
	redo_xspctrm();
    }else if(!strcmp(s, "Log-Linear Sc")) {
	use_loglog= 2;
	redo_xspctrm();
    }
}

static void open_xspctrm_win(int itrc)
{
    char buf[200];
    char *t;
    BIS3_HEADER *bh= &traces[itrc]->wave->info;

    cur_itrc= itrc;
    if(!xspctrm_frame) InitXSpctrmFrame();
    t=(char *)rindex(traces[itrc]->filename,'/');
    t= (t!=NULL)? t+1:traces[itrc]->filename;
    sprintf(buf,"[%d] %s %s %s %s Cross Spectrum (%s)", itrc, bh->station,
	    bh->network, bh->channel, 
	    (strlen(bh->location) > 0) ? bh->location : "--", t);
    printf("working on x-spectru\n");
    xv_set(xspctrm_frame, XV_SHOW, TRUE,
	FRAME_LABEL, buf,
	NULL);
}

static void close_xspctrm_win()
{
    xv_set(xspctrm_frame, XV_SHOW, FALSE, NULL);
}
    
/**********************************************************************
 *   Plotting the Spectrum                                            *
 **********************************************************************/

static void PlotXSpectrum(int itrc, int ix1, int ix2)
{
    Trace *trc= traces[itrc];
    float *y, srate;
    Trace *trc2= traces[cur_itrc2];
    float *y2, srate2;
    float *a, *b;
    int nt2h, n2;
    int i;

    open_xspctrm_win(itrc);

    srate= trc->wave->info.sample_rate;
    srate2= trc2->wave->info.sample_rate;
    if(srate!=srate2) {
	fprintf(stderr,"Error: SAMPLE RATE NOT EQUAL!\n");
	return;
    }
    if (ix1<0) ix1=0;
    if (ix2>=trc->wave->info.n_values) {
	ix2= trc->wave->info.n_values-1;
    }
    CalcSpectrum(srate, trc->wave->data, ix1, ix2, &y, &a, &b,
		 &n2, &nt2h, xspc_soph);
    free(a); free(b);
    CalcSpectrum(srate, trc2->wave->data, ix1, ix2, &y2, &a, &b,
		 &n2, &nt2h, xspc_soph);
    for(i=0; i<nt2h-1; i++) {
	y[i]/=y2[i];
    }
    free(y2);
    if(use_loglog) {
	for(i=0;i<nt2h-1;i++) {
	    y[i]= (float)log10((double)y[i]);
	}
    }
    xspc_y=y;
    xspc_a= a; xspc_b= b;
    xspc_srate=srate;
    xspc_n2=n2;
    xspc_nt2h= nt2h;
    redrawXSpctrm();
}

static void redrawXSpctrm()
{
    XClearWindow(theDisp, xspc_win);
    DrawXSpectrum(xspc_y, xspc_srate, xspc_n2, xspc_nt2h);
}

static void resizeXSpctrm(Canvas canvas, int width, int height)
{
    int needRedraw= 0;
    if(height<xspctrmwin_h && width<xspctrmwin_w)
	needRedraw=1;
    xspctrmwin_h= height;
    xspctrmwin_w= width;
    /* this is a kludge to get around XView's reluctance to redraw 
       when a canvas is resized to a smaller size. */
    if(needRedraw)
	redrawXSpctrm();
    
}

static void PrintDecade(int i, int x, int y)
{
    char buf[30];
    sprintf(buf,"%d",i);
    XDrawString(theDisp, xspc_win, xspc_gc, x, y+5, "10", 2); 
    XDrawString(theDisp, xspc_win, xspc_gc, x+14,	y, buf, strlen(buf));
}

static void PlotXAxis(float freqk, int hmin, int hmax, float hs, int nt2h)
{
    char buf[30];
    int i;

    /* axis */
    XDrawLine(theDisp, xspc_win, xspc_gc, SPWIN_X_OFF, xspctrmwin_h-SPWIN_Y_OFF, 
	      xspctrmwin_w-MARGIN, xspctrmwin_h-SPWIN_Y_OFF);

    /* labels */
    if (use_loglog==1) {    /* log-log */
	int x, y;
	
	XDrawString(theDisp, xspc_win, xspc_gc, xspctrmwin_w/2-50,
	    xspctrmwin_h-SPWIN_Y_OFF+35, "F R E Q U E N C Y", 17);
	PrintDecade(hmin, SPWIN_X_OFF+5, xspctrmwin_h-SPWIN_Y_OFF+15);
	PrintDecade(hmax, xspctrmwin_w-MARGIN-20,
	    xspctrmwin_h-SPWIN_Y_OFF+15);
	for(i=hmin+1; i<= hmax-1; i++) {
	    x= (i-hmin)*hs+ SPWIN_X_OFF;
	    y= xspctrmwin_h-SPWIN_Y_OFF;
	    XDrawLine(theDisp, xspc_win, xspc_gc, x, y, x, y-8);
	    if(xspc_gridOn)
		XDrawLine(theDisp, xspc_win, xspc_dot_gc, x, y-8, x, MARGIN);
	    PrintDecade(i, x, y+15);
	}
	/* smaller marks */
	for(i=hmin; i<= hmax-1; i++) {
	    int k, xx;
	    x= (i-hmin)*hs + SPWIN_X_OFF;
	    y= xspctrmwin_h-SPWIN_Y_OFF;
	    for(k=2; k<10; k++) {
		xx= x+log10((double)k)*hs;
		XDrawLine(theDisp, xspc_win, xspc_gc, xx, y, xx, y-4);
		if(xspc_gridOn)
		    XDrawLine(theDisp, xspc_win, xspc_smdot_gc,
			      xx, y-4, xx, MARGIN);
	    }
	}
    }else {	/* linear and log-linear */
	XDrawString(theDisp, xspc_win, xspc_gc, xspctrmwin_w/2+50,
	    xspctrmwin_h-SPWIN_Y_OFF+25, "F R E Q U E N C Y", 17);
	sprintf(buf, "%g", (double)freqk);
	XDrawString(theDisp, xspc_win, xspc_gc, SPWIN_X_OFF+5,
	    xspctrmwin_h-SPWIN_Y_OFF+15, buf, strlen(buf));
	sprintf(buf, "%g", (double)(freqk*(nt2h-1)));
	XDrawString(theDisp, xspc_win, xspc_gc, xspctrmwin_w-MARGIN-40,
	    xspctrmwin_h-SPWIN_Y_OFF+15, buf, strlen(buf));
    }
}

static void PlotYAxis(float ymax, float ymin, float vs)
{
    char buf[30];

    XDrawLine(theDisp, xspc_win, xspc_gc, SPWIN_X_OFF, MARGIN,
	      SPWIN_X_OFF, xspctrmwin_h-SPWIN_Y_OFF);
    /* label */
    DrawVertString(xspc_win, xspc_gc, 10, xspctrmwin_h/2-100, "AMPLITUDE");

    if (use_loglog) {
	int i,iymax,iymin;
	int y;

	iymax=(int)ymax;
	iymin=(int)ymin;
	PrintDecade(iymin, SPWIN_X_OFF/2, xspctrmwin_h-SPWIN_Y_OFF-5);
	PrintDecade(iymax, SPWIN_X_OFF/2, MARGIN+10);
	for(i=iymin+1; i<= iymax-1; i++) {
	    y= (iymax-i)*vs+ MARGIN;
	    XDrawLine(theDisp, xspc_win, xspc_gc, SPWIN_X_OFF, y,
		SPWIN_X_OFF+8, y);
	    if(xspc_gridOn)
		XDrawLine(theDisp, xspc_win, xspc_dot_gc, SPWIN_X_OFF+8, y,
			xspctrmwin_w-MARGIN, y);
	    PrintDecade(i, SPWIN_X_OFF/2, y);
	}
	/* smaller marks */
	for(i=iymin; i<= iymax-1; i++) {
	    int k, yy;
	    y= (iymax-i)*vs + MARGIN;
	    for(k=2; k<10; k++) {
		yy= y-log10((double)k)*vs;
		XDrawLine(theDisp, xspc_win, xspc_gc, SPWIN_X_OFF, yy,
			SPWIN_X_OFF+4, yy);
		if(xspc_gridOn)	
		    XDrawLine(theDisp, xspc_win, xspc_smdot_gc, SPWIN_X_OFF+4, yy,
			  xspctrmwin_w-MARGIN, yy);
	    }
	}
    }else {
	sprintf(buf, "%g", (double)ymax);
	XDrawString(theDisp, xspc_win, xspc_gc, SPWIN_X_OFF/2, MARGIN+5,
		buf, strlen(buf));
	sprintf(buf, "%g", (double)ymin);
	XDrawString(theDisp, xspc_win, xspc_gc, SPWIN_X_OFF/2,
		xspctrmwin_h-SPWIN_Y_OFF-5, buf, strlen(buf));
    }
}


static void DrawXSpectrum(float *y, float srate, int n2, int nt2h)
{
    float hs, vs;
    float ymax, ymin;
    int hmin, hmax;
    XPoint *points, *plot;
    int npts, i;
    char buf[30];
    float freqk;

    Trace *trc=traces[cur_itrc];
    
    /* compute scale */

    ymax=ymin=y[0];
    for(i=1; i < nt2h-1; i++) {
	if(y[i]>ymax) {
	    ymax=y[i];
	}else if (y[i]<ymin) {
	    ymin=y[i];
	}
    }

/*    freqk= 2.0*3.14159265*srate/n2; */
    freqk= srate/n2; 

    if (use_loglog) {
	ymax= ceil((double)ymax);
	ymin= floor((double)ymin);
	vs= (float)(xspctrmwin_h-SPWIN_Y_OFF-MARGIN)/(ymax-ymin);
    }else {
	vs= (float)(xspctrmwin_h-SPWIN_Y_OFF-MARGIN)/(ymax-ymin);
    }

    if (use_loglog==1) {    /* log-log */
	hmax= ceil(log10((double)(freqk*(nt2h-1))));
	hmin= floor(log10((double)freqk));
	hs= (float)(xspctrmwin_w-SPWIN_X_OFF-MARGIN)/(hmax-hmin);
    }else {	/* linear and log-linear */
	hs= (float)(xspctrmwin_w-SPWIN_X_OFF-MARGIN)/(nt2h-1);
    }

    /* plot & label Y-axis */
    PlotYAxis(ymax, ymin, vs);

    /* plot X-axis */
    PlotXAxis(freqk, hmin, hmax, hs, nt2h);
	
    plot= points= (XPoint *)Malloc(sizeof(XPoint)* (nt2h+1));

    if (use_loglog==1) {    /* log-log */
	int x;
	npts=1;
	points->x= (log10((double)freqk) - hmin)*hs + SPWIN_X_OFF;
	points->y= xspctrmwin_h-SPWIN_Y_OFF-(y[0]-ymin)*vs;
	for(i=1; i < nt2h-1; i++) {
	    x= (log10((double)(freqk*(i+1))) - hmin)*hs + SPWIN_X_OFF;
	    if(x!=points->x) {
		points++;
		points->x= x;
		points->y= xspctrmwin_h-SPWIN_Y_OFF-(y[i]-ymin)*vs;
		npts++;
	    }
	}
    }else {	/* linear and log-linear */
	if(hs > 1) {
	    i=0;
	    while(i*hs < xspctrmwin_w-SPWIN_X_OFF-MARGIN) {
		points->x= i*hs+SPWIN_X_OFF;
		points->y= xspctrmwin_h-SPWIN_Y_OFF-
		    (y[i]-ymin)*vs;
		points++; i++;
	    }
	    npts= i;
	}else {
	    for(i=0; i < nt2h-1; i++) {
		points->x= i * hs + SPWIN_X_OFF;
		points->y= xspctrmwin_h-SPWIN_Y_OFF - (y[i]-ymin)*vs;
		points++;
	    }
	    npts= nt2h-1;
	}
    }

    XDrawLines(theDisp, xspc_win, xspc_gc, plot, npts, CoordModeOrigin);
    XFlush(theDisp);
    free(plot);

}

/**********************************************************************
 *   handle events for spectrum operations                            *
 **********************************************************************/

void handle_xspectrum()
{
    char buff[30];
    int itrc, ix1, ix2;
    int i;

    for(i=0; i <= LastTrack; i++) {
	Trace *trc= traces[i];
	if(trc->sel_reg!=NULL) {
	    cur_itrc=i;
	    cur_itrc2=i;
	    used_r1=trc->sel_reg;
	    used_r2=trc->sel_reg;	    
	    redo_xspctrm();
	    break;
	}
    }

}


static void redo_xspctrm()
{
    Trace *trc;

    /* which traces to do if specified */
    printf("working on this\n");
    if(itrc_txt) {
	int val;
	val= (int)xv_get(itrc_txt,PANEL_VALUE);
	if(val>=0 && val<=LastTrack) {
	  cur_itrc=val;
	  used_r1=traces[val]->sel_reg;
	}

	val= (int)xv_get(itrc2_txt,PANEL_VALUE);
	if(val>=0 && val<=LastTrack) {
	  cur_itrc2=val;
	  used_r2=traces[val]->sel_reg;
	}
    }

    if (used_r1==NULL) next_xspctrm();
    
    trc= traces[cur_itrc];
    /* plot again -- might be different */
    if(used_r1!=NULL) {
      printf("reploting X Spectr\n");
	PlotXSpectrum(cur_itrc, used_r1->left_index,used_r1->right_index);
    }
}

static void next_xspctrm()
{
  int i, itrc;

  Reg_select *curr=used_r1->next;
  /* next region part of same trace */
  if (curr!=NULL) {
    used_r1=curr;
    itrc=cur_itrc;
  } else {
      
    itrc=-1;
    /* forward */
    for(i=cur_itrc+1; i <= LastTrack; i++) {
      curr=traces[i]->sel_reg;
      if (curr!=NULL) {
	itrc=i;
	used_r1=curr;
	break;
      }      
    }
    if (itrc==-1) {		/* wrap around */
      for(i=0; i<=cur_itrc; i++ ) {
	curr=traces[i]->sel_reg;
	if (curr!=NULL) {
	  itrc=i;
	  used_r1=curr;
	  break;
	}
      }
    }
  }

  if (itrc==-1) {		/* no more to plot -- resigns */
    close_xspctrm_win();
  }else {
    cur_itrc= itrc;
    PlotXSpectrum(itrc,used_r1->left_index,used_r1->right_index);
  }
}

