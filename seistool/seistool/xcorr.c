#ifndef lint
static char id[] = "$Id: xcorr.c,v 1.2 2013/02/28 21:24:56 lombard Exp $";
#endif

/*
 * xcorr.c--
 *    implements cross correlation code
 *
 * Copyright (c) 1992-3 Andrew K. Yu, Lawrence Berkeley Laboratory
 *      and University of California, Berkeley.
 * All rights reserved.
 */
#include <math.h>
#include <stdio.h>
#include <xview/xview.h>
#include <xview/frame.h>
#include <xview/canvas.h>
#include <xview/panel.h>
#include <xview/cms.h>
#include <xview/font.h>
#include "mathlib.h"
#include "proto.h"
#include "xv_proto.h"

#define MARGIN 30
#define SPWIN_X_OFF 40
#define SPWIN_Y_OFF 40

/**********************************************************************
 *   Cross Correlation window                                         *
 **********************************************************************/

/* globals */
static Frame	xcorr_frame= NULL;
static Canvas	xcorr_cvs= NULL;
static Window	xcorr_win;
static GC	xcorr_gc;

static int xcorrwin_h= 600;
static int xcorrwin_w= 800;
static int xcorr_plot_portion= 1;   /* plot wave portion */
static Reg_select *used_reg=NULL;   /* the region for the spectra */
static Select_Info *sel_hold;       /* the selection regions of interest */
static int open_xcor=0;             /* is the window open */
static float *xcorr_data=NULL;      /* store the data for the event */

static int xcr_gridOn= 0;

/* computed data for the current spectrum*/
static Cms colours;

static int xcr_soph= 1;		    /* demean, tapering */

extern  Xv_Font text_font;

/* internal function prototypes */
static void InitXcorrFrame(void);
static void open_xcorr_win();
static void redo_xcorr();
static void resizeXcorr (Canvas canvas, int width,int height);
static void Calc_xcor(Select_Info *reg);
static void Plot_xcor(float *data,int n_data, Select_Info *reg);
static void PlotXAxis(double hmin,double hmax,float hs,float inc);
static void PlotYAxis(double ymin,double ymax,double vs,double inc);
static void close_xcorr_win();
static void next_xcorr();
static void dump_xcorr();
static void options_menu_proc();
static void scale_menu_proc();
static void toggle_grid();


static void InitXcorrFrame(void)
{
    Panel panel;
    Menu  menu;

    xcorr_frame= xv_create(tracesFrame, FRAME,
			   XV_HEIGHT, xcorrwin_h+40, XV_WIDTH, xcorrwin_w,
			   NULL);		  
    xcorr_cvs= xv_create(xcorr_frame, CANVAS,
			 XV_X, 0, XV_Y, 40,
			 CANVAS_RETAINED, TRUE,
			 CANVAS_REPAINT_PROC, redrawXcorr,		  
			 CANVAS_RESIZE_PROC, resizeXcorr,		  
			 NULL);		  
    xcorr_win= xv_get(canvas_paint_window(xcorr_cvs), XV_XID);

    createGC( xcorr_win, &xcorr_gc);

    panel= (Panel)xv_create(xcorr_frame, PANEL,
			    XV_X, 0, XV_Y, 0, XV_HEIGHT, 40,
			    NULL);
    (void)xv_create(panel, PANEL_BUTTON,
		    PANEL_LABEL_STRING, "Quit",
		    PANEL_NOTIFY_PROC, close_xcorr_win,
		    NULL);
    (void)xv_create(panel, PANEL_BUTTON,
		    PANEL_LABEL_STRING, "Redo",
		    PANEL_NOTIFY_PROC, redo_xcorr,
		    NULL);
    (void)xv_create(panel, PANEL_BUTTON,
		    PANEL_LABEL_STRING, "Next",
		    PANEL_NOTIFY_PROC, next_xcorr,
		    NULL);
    menu= (Menu)xv_create(NULL,MENU,
			  MENU_STRINGS,
			  "Linear Scale", "Log Scale",
			  NULL,
			  MENU_NOTIFY_PROC,   scale_menu_proc,
			  NULL);
    (void)xv_create(panel, PANEL_BUTTON,
		    PANEL_LABEL_STRING, "Scale",
		    PANEL_ITEM_MENU, menu,
		    NULL);
    (void)xv_create(panel, PANEL_BUTTON,	PANEL_LABEL_STRING, "Dump",
		    PANEL_NOTIFY_PROC, dump_xcorr,
		    NULL);
    menu= (Menu)xv_create(NULL,MENU,
			  MENU_STRINGS,
			  "trace portion", "no trace",
			  "taper etc.", "no taper",
			  NULL,
			  MENU_NOTIFY_PROC,   options_menu_proc,
			  NULL);
    (void)xv_create(panel, PANEL_BUTTON,
		    PANEL_LABEL_STRING, "Options",
		    PANEL_ITEM_MENU, menu,
		    NULL);
    (void)xv_create(panel, PANEL_BUTTON,
		    PANEL_LABEL_STRING, "Grid",
		    PANEL_NOTIFY_PROC, toggle_grid,
		    NULL);
}

static void open_xcorr_win()
{

    if (open_xcor==0) {
	open_xcor=1;
	InitXcorrFrame();
    } else {
	XClearWindow(theDisp, xcorr_win);
    }
    xv_set(xcorr_frame,XV_SHOW, TRUE,
	   FRAME_LABEL, "Cross Correlation",
	   NULL);
}


void handle_xcorr ()
{
    sel_hold=creating_gsr_win(2);
    open_xcorr_win();
    redo_xcorr();
}

void redrawXcorr()
{
    XClearWindow(theDisp, xcorr_win);
    redo_xcorr();
}

static void redo_xcorr()
{
    Calc_xcor (sel_hold);
}

static void resizeXcorr(Canvas canvas, int width,int height)
{
    int needRedraw= 0;

    if(height<=xcorrwin_h && width<=xcorrwin_w)
	needRedraw=1;
    xcorrwin_h= height;
    xcorrwin_w= width;
    /* this is a kludge to get around XView's reluctance to redraw 
       when a canvas is resized to a smaller size. */
    if(needRedraw)
	redrawXcorr();
}

static void Calc_xcor(Select_Info *reg)
{
    float *new_x, *new_y, *out_d;
    int tr_len,n2_len,ierr=0,nt2h;
    int i,j;
    int sel_zer=reg->sel_reg[0];
    int sel_one=reg->sel_reg[1];
    float s_rate=reg->regs[sel_zer].info.sample_rate;

    /* 0. see if same sampling rate */
    if (reg->regs[sel_zer].info.sample_rate != reg->regs[sel_one].info.sample_rate) {return;}

    /* 1. find the lenght of the two time series */
    tr_len=reg->regs[sel_zer].info.n_values>reg->regs[sel_one].info.n_values?reg->regs[sel_zer].info.n_values:reg->regs[sel_one].info.n_values;
    tr_len+=1;
    n2_len=pow(2,ceil(log(tr_len)/log(2))); 
    nt2h=n2_len;

    /* 2. put both in regions with equal length time series */
    /*    padding with zeros                                */
    new_x=(float *)malloc(sizeof(float)*(n2_len+1)*2);
    new_y=(float *)malloc(sizeof(float)*(n2_len+1)*2);
    out_d=(float *)malloc(sizeof(float)*(n2_len+1)*2);

    for (j=0;j<reg->regs[sel_zer].info.n_values;j++) {
	new_x[j]=reg->regs[sel_zer].data[j];
    }
    for (j=reg->regs[sel_zer].info.n_values;j<n2_len*2;j++) {
	new_x[j]=0;
    }

    for (j=0;j<reg->regs[sel_one].info.n_values;j++) {
	new_y[j]=reg->regs[sel_one].data[j];
    }
    for (j=reg->regs[sel_one].info.n_values;j<n2_len*2;j++) {
	new_y[j]=0;
    }

    /* 3. call Lind's modified xcor routine                 */
    CalcXcorr(s_rate,new_x,new_y,&out_d,&n2_len,&tr_len,ierr);

    /* 4. update display (find max's and min's etc          */
    xcorr_data=out_d;
    Plot_xcor(out_d,n2_len,reg);
    free(new_x);
    free(new_y);
}

static void Plot_xcor(float *data,int n_data, Select_Info *reg)
{
    int i;
    double hmax,hmin,vmax,vmin,tmax,tmin;
    double ystep,xstep;
    double v_scale,h_scale;
    float thmin,h_step_size;
    float x_off;
    int x_loc,y_loc;
    int sel_zer=reg->sel_reg[0];
    float s_rate=reg->regs[sel_zer].info.sample_rate;
    XPoint *points=(XPoint *)malloc(sizeof(XPoint)* (n_data+1));

    data=xcorr_data;

    /* 1. find min and max values */
    vmin=vmax=data[0];
    for (i=1;i<n_data;i++) {
	if (vmin>data[i]) vmin=data[i];
	if (vmax<data[i]) vmax=data[i];
    }  

    hmax=(n_data)/(2*s_rate);
    hmin=-hmax;
    x_off=hmin;
    thmin=hmin;
    h_step_size=1/s_rate;

    scale_line_axis(vmin,vmax,&ystep,&tmin,&tmax);
    vmin=tmin;
    vmax=tmax;

    scale_line_axis(hmin,hmax,&xstep,&tmin,&tmax);
    hmin=tmin;
    hmax=tmax;
    x_off=thmin-hmin;
  
    /* 2. calculate the h_scale and v_scale */
    v_scale=(float)(xcorrwin_h-3*MARGIN)/((vmax-vmin));
    h_scale=(float)(xcorrwin_w-2*MARGIN)/((hmax-hmin));
    /* 3. plot x and y axis       */
    PlotXAxis(hmin,hmax,h_scale,xstep);
    PlotYAxis(vmin,vmax,v_scale,ystep);
    /* 4. plo the correlation     */
    for (i=0;i<n_data;i++) {
	x_loc=SPWIN_X_OFF+(x_off+i*h_step_size)*h_scale;
	y_loc=SPWIN_Y_OFF+(vmax-data[i])*v_scale;
	points[i].x=x_loc;
	points[i].y=y_loc;
    }
    XDrawLines(theDisp, xcorr_win, xcorr_gc, points,n_data , CoordModeOrigin);  
    free(points);
    XFlush(theDisp);
}


static void PlotXAxis(double hmin,double hmax,float hs,float inc)
{
    char buf[30];
    int i;
    int x,y;
    float q,p;
    Font_string_dims dims;
  

    /* axis */
    XDrawLine(theDisp, xcorr_win, xcorr_gc, SPWIN_X_OFF, xcorrwin_h-SPWIN_Y_OFF, 
	      xcorrwin_w-MARGIN, xcorrwin_h-SPWIN_Y_OFF);
  
    /* labels */
    XDrawString(theDisp, xcorr_win, xcorr_gc, xcorrwin_w/2-50,
		xcorrwin_h-SPWIN_Y_OFF+35, "Delay Time", 17);

    y=xcorrwin_h-SPWIN_Y_OFF;
    for (q=hmin;q<hmax+inc;q+=inc){
	x=(q-hmin)*hs+SPWIN_X_OFF;
	XDrawLine(theDisp, xcorr_win, xcorr_gc, x, y, x, y-8);
	sprintf(buf, "%g", q);
	(void) xv_get(text_font,FONT_STRING_DIMS,buf,&dims);
    
	XDrawString(theDisp, xcorr_win, xcorr_gc, x-dims.width/2, y+2+dims.height, buf, strlen(buf));
	for(p=q;p<q+inc;p+=inc/10) {
	    x=(p-hmin)*hs+SPWIN_X_OFF;
	    XDrawLine(theDisp, xcorr_win, xcorr_gc, x, y-4, x, y);
	}
    
    }
}

static void PlotYAxis(double ymin,double ymax,double vs,double inc)
{
    char buf[30];
    int x,y;
    float q,p;
    Font_string_dims dims;
    

    XDrawLine(theDisp, xcorr_win, xcorr_gc, SPWIN_X_OFF, MARGIN,
	      SPWIN_X_OFF, xcorrwin_h-SPWIN_Y_OFF);
    /* label */
    DrawVertString(xcorr_win, xcorr_gc, 10, xcorrwin_h/2-100, "Correlation");

    x=SPWIN_X_OFF+8;
    (void) xv_get(text_font,FONT_STRING_DIMS,buf,&dims);

    for (q=ymin;q<ymax+inc;q+=inc){
	y=(ymax-q)*vs+SPWIN_Y_OFF;
	XDrawLine(theDisp, xcorr_win, xcorr_gc, x, y, x-8, y);
	sprintf(buf, "%.3g", q);
    
	XDrawString(theDisp, xcorr_win, xcorr_gc, x-9-dims.width, y+dims.height/2, buf, strlen(buf));
      
	for(p=q;p<q+inc;p+=inc/10) {
	    y=(ymax-p)*vs+MARGIN;
	    XDrawLine(theDisp, xcorr_win, xcorr_gc, x-4, y, x-8, y);
	}
    }
}


static void close_xcorr_win()
{
    open_xcor=0;
    xv_set(xcorr_frame, XV_SHOW, FALSE, NULL);
}

static void next_xcorr()
{
    sel_hold->sel_reg[0]+=1;
    if (sel_hold->sel_reg[0]>=sel_hold->num_wind) {
	sel_hold->sel_reg[0]=0;    
    }
    redraw_gsrwin(sel_hold->can);
    redrawXcorr();
}
static void dump_xcorr(){}
static void options_menu_proc(){}
static void scale_menu_proc(){}

static void toggle_grid()
{     
    xcr_gridOn=xcr_gridOn?0:1;
}

