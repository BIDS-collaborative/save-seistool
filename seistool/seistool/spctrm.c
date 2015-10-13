#ifndef lint
static char id[] = "$Id: spctrm.c,v 1.3 2013/02/28 21:24:57 lombard Exp $";
#endif

/*
 * spctrm.c--
 *    implements spectrum
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
#include "spctrm.h"
#include "xv_proto.h"

/**********************************************************************
 *   Spectrum window                                                  *
 **********************************************************************/

/* globals */
static Frame	spctrm_frame= NULL;
static Canvas	spctrm_cvs= NULL;
static Window	spc_win;
static GC	spc_gc, spc_dot_gc, spc_smdot_gc;

static int spctrmwin_h= 600;
static int spctrmwin_w= 800;
static int use_loglog= 1;	    /* use log-log plot */
static int spctrm_plot_portion= 1;  /* plot wave portion */
static Reg_select *used_reg=NULL;    /* the region for the spectra */
static Select_Info *sel_hold;       /* the selection regions of interest */
static hold_spct **spcts;           /* spec array pointer */
static int spc_gridOn= 0;           /* draw grid on display? */
static int spc_fre_pre=0;           /* plot frequency or period */

/* computed data for the current spectrum*/
static int cur_itrc=-1;
static int spc_soph= 1;		    /* demean, tapering */

extern  Xv_Font text_font;
extern  GC glob_colour_gc[9];
extern Trace **traces;
extern int num_glob_gc;

/* internal function prototypes */
static void handle_spc_gc(Window spc_win);
static void InitSpctrmFrame();
static void toggle_data(Panel_item item,Event *event) ;
static void options_menu_proc(Menu menu,Menu_item menu_item);
static void scale_menu_proc(Menu menu,Menu_item menu_item);
static void open_spctrm_win(int itrc);
static void dump_spctrm(void);
static void redrawSpctrm(void);
static void resizeSpctrm(Canvas canvas, int width,int height);
static void PrintDecade(int i, int x, int y);
static void PlotXAxis(double hmin,double hmax,float hs,float inc);
static void PlotYAxis(float ymax,float ymin,float vs, float inc);
static void drawspectrum ();
static void calc_hold_spctrm();
static void redo_spctrm();
static void next_spctrm();
static void toggle_grid();

/**********************************************************************
 *   set up spectrum window                                           *
 **********************************************************************/

static void handle_spc_gc(Window spc_win)
{
    int i;
    char dash_list[2];
    unsigned long *pix_tab;

    handle_colours(spc_win);

    createGC( spc_win, &spc_gc);
    createGC( spc_win, &spc_dot_gc);

    dash_list[0]= 1;
    dash_list[1]= 5;

    XSetLineAttributes(theDisp, spc_dot_gc, 1, LineOnOffDash,
		       CapRound, JoinRound);
    createGC( spc_win, &spc_smdot_gc);

    XSetLineAttributes(theDisp, spc_smdot_gc, 1, LineOnOffDash,
		       CapRound, JoinRound);
    XSetDashes(theDisp, spc_smdot_gc,2,dash_list,2);
}

static void InitSpctrmFrame()
{
    Panel panel;
    Menu menu;
    
    spctrm_frame= xv_create(tracesFrame, FRAME,
			    XV_HEIGHT, spctrmwin_h+40, XV_WIDTH, spctrmwin_w,
			    NULL);		  
    spctrm_cvs= xv_create(spctrm_frame, CANVAS,
			  XV_X, 0, XV_Y, 40,
			  CANVAS_RETAINED, TRUE,
			  CANVAS_REPAINT_PROC, redrawSpctrm,		  
			  CANVAS_RESIZE_PROC, resizeSpctrm,		  
			  NULL);		  
    spc_win= xv_get(canvas_paint_window(spctrm_cvs), XV_XID);

    handle_spc_gc(spc_win);

    panel= (Panel)xv_create(spctrm_frame, PANEL,
			    XV_X, 0, XV_Y, 0, XV_HEIGHT, 40,
			    NULL);
    (void)xv_create(panel, PANEL_BUTTON,
		    PANEL_LABEL_STRING, "Quit",
		    PANEL_NOTIFY_PROC, close_spctrm_win,
		    NULL);
    (void)xv_create(panel, PANEL_BUTTON,
		    PANEL_LABEL_STRING, "Redo",
		    PANEL_NOTIFY_PROC, redo_spctrm,
		    NULL);
    (void)xv_create(panel, PANEL_BUTTON,
		    PANEL_LABEL_STRING, "Next",
		    PANEL_NOTIFY_PROC, next_spctrm,
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
    (void)xv_create(panel, PANEL_BUTTON,	PANEL_LABEL_STRING, "Dump",
		    PANEL_NOTIFY_PROC, dump_spctrm,
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
    (void) xv_create(panel,PANEL_BUTTON,
		     PANEL_LABEL_STRING,"Period",
		     PANEL_NOTIFY_PROC, toggle_data,
		     NULL);   
}

static void toggle_data(Panel_item item,Event *event)
{
    spc_fre_pre=spc_fre_pre?0:1;
    if (spc_fre_pre==0) {
	xv_set(item,PANEL_LABEL_STRING,"Period",NULL);
    } else {
	xv_set(item,PANEL_LABEL_STRING,"Frequency",NULL);
    }
    /* redraw window with new axis */
    redo_spctrm(); 
}

static void options_menu_proc(Menu menu,Menu_item menu_item)
{
    char *s= (char *)xv_get(menu_item, MENU_STRING);
    if (!strncmp(s, "trace", 5)) {
	spctrm_plot_portion= 1;
	redo_spctrm();
    }else if(!strncmp(s, "no tr",5)) {
	spctrm_plot_portion= 0;
	redo_spctrm();
    }else if(!strcmp(s, "taper etc.")) {
	spc_soph= 1;
    }else if(!strncmp(s, "no ta", 5)) {
	spc_soph= 0;
    }
}

static void scale_menu_proc(Menu menu,Menu_item menu_item)
{
    char *s= (char *)xv_get(menu_item, MENU_STRING);
    if (!strcmp(s, "Log Scale")) {
	use_loglog= 1;
	redo_spctrm();
    }else if(!strcmp(s, "Linear Scale")) {
	use_loglog= 0;
	redo_spctrm();
    }else if(!strcmp(s, "Log-Linear Sc")) {
	use_loglog= 2;
	redo_spctrm();
    }
}

static void open_spctrm_win(int itrc)
{
    char buf[200];
    char *t;
    BIS3_HEADER *bh= &traces[itrc]->wave->info;

    cur_itrc= itrc;
    if(!spctrm_frame) InitSpctrmFrame();
    /*  t=(char *)rindex(traces[itrc]->filename,'/');
	t= (t!=NULL)? t+1:traces[itrc]->filename;
	sprintf(buf,"[%d] %s %s %s %s Spectrum (%s)", itrc, bh->station, 
	bh->network, bh->channel, 
	(strlen(bh->location) > 0) ? bh->location : "--", t); */
    xv_set(spctrm_frame, XV_SHOW, TRUE,
	   FRAME_LABEL, "Spectrum",
	   NULL);
}

void close_spctrm_win()
{
    xv_set(spctrm_frame, XV_SHOW, FALSE, NULL);
}
    
/**********************************************************************
 *   Output Spectrum (to File)                                        *
 **********************************************************************/

/* dump current spectrum to file */
static void dump_spctrm() 
{
    FILE *fp;
    /*    float freqk= 2.0*3.14159265*spc_srate/spc_n2; */
    /*    float freqk= spc_srate/spc_n2; */
    int i,j,k;
    char buf[500];

    strcpy(buf,traces[cur_itrc]->filename);
    if(!strcmp(&buf[strlen(buf)-3],"sgy")) {
	sprintf(&buf[strlen(buf)-3], "%d.spec", cur_itrc);
    }else {
	sprintf(buf,"%s.%d.spec",buf,cur_itrc);
    }
    if ((fp= fopen(buf,"w"))==NULL) {
	fprintf(stderr, "Cannot open \"%s\"\n", buf);
	return;
    }
    printf("Writing spectrum to %s.\n",buf);

    for (i=0;i<sel_hold->num_sel;i++) {
	float srate,freqk;
	k=sel_hold->sel_reg[i];
	srate=sel_hold->regs[k].info.sample_rate;
	freqk=srate/spcts[k]->n_pnts;

	for (j=0;j<spcts[k]->n2_pnts-1;j++) {
	    fprintf(fp, " %g\t%g\t%g\t%g\n", freqk*(j+1),spcts[k]->amp[j] , spcts[k]->phase[j], spcts[k]->power[j]);
	}
	fprintf(fp,"\n");
    }

    fclose(fp);
}

/**********************************************************************
 *   Plotting the Spectrum                                            *
 **********************************************************************/

static void redrawSpctrm(void)
{
    XClearWindow(theDisp, spc_win);
    drawspectrum();
}

static void resizeSpctrm(Canvas canvas, int width,int height)
{
    int needRedraw= 0;
    if(height<=spctrmwin_h && width<=spctrmwin_w)
	needRedraw=1;
    spctrmwin_h= height;
    spctrmwin_w= width;
    /* this is a kludge to get around XView's reluctance to redraw 
       when a canvas is resized to a smaller size. */
    if(needRedraw)
	redrawSpctrm();
    
}

#define SPWIN_X_OFF 50
#define SPWIN_Y_OFF 50
#define MARGIN	10

void DrawVertString(Window win, GC gc, int x,int y, char *str)
{
    int i;
    for(i=0; i<strlen(str); i++) {
	XDrawString(theDisp, win, gc, x, y, str+i, 1);
	y+= 20;
    }
}

static void PrintDecade(int i,int x,int y)
{
    char buf[30];
    sprintf(buf,"%d",i);
    XDrawString(theDisp, spc_win, spc_gc, x, y+5, "10", 2); 
    XDrawString(theDisp, spc_win, spc_gc, x+14,	y, buf, strlen(buf));   
}

static void PlotXAxis(double hmin, double hmax, float hs, float inc)
{
    char buf[30];
    char xtitle[50];
    int i;

    /* axis */
    XDrawLine(theDisp, spc_win, spc_gc, SPWIN_X_OFF, spctrmwin_h-SPWIN_Y_OFF, 
	      spctrmwin_w-MARGIN, spctrmwin_h-SPWIN_Y_OFF);

    /* labels */
    if (!spc_fre_pre) {
	strcpy(xtitle,"F R E Q U E N C Y");
    } else {
	strcpy(xtitle,"P E R I O D");
    }

    XDrawString(theDisp, spc_win, spc_gc, spctrmwin_w/2-50,
		spctrmwin_h-SPWIN_Y_OFF+35, xtitle, strlen(xtitle));
    if (use_loglog==1) {    /* log-log */
	int x, y;
	
	PrintDecade(hmin, SPWIN_X_OFF+5, spctrmwin_h-SPWIN_Y_OFF+15);
	PrintDecade(hmax, spctrmwin_w-MARGIN-20,
		    spctrmwin_h-SPWIN_Y_OFF+15);
	for(i=hmin+1; i<= hmax-1; i++) {
	    x= (i-hmin)*hs+ SPWIN_X_OFF;
	    y= spctrmwin_h-SPWIN_Y_OFF;
	    XDrawLine(theDisp, spc_win, spc_gc, x, y, x, y-8);
	    if(spc_gridOn)
		XDrawLine(theDisp, spc_win, spc_dot_gc, x, y-8, x, MARGIN);
	    PrintDecade(i, x, y+15);
	}
	/* smaller marks */
	for(i=hmin; i<= hmax-1; i++) {
	    int k, xx;
	    x= (i-hmin)*hs + SPWIN_X_OFF;
	    y= spctrmwin_h-SPWIN_Y_OFF;
	    for(k=2; k<10; k++) {
		xx= x+log10((double)k)*hs;
		XDrawLine(theDisp, spc_win, spc_gc, xx, y, xx, y-4);
		if(spc_gridOn)
		    XDrawLine(theDisp, spc_win, spc_smdot_gc,
			      xx, y-4, xx, MARGIN);
	    }
	}
    }else {	/* linear and log-linear */
	int x,y;
	float q,p;
	Font_string_dims dims;

	y=spctrmwin_h-SPWIN_Y_OFF;
	for (q=hmin;q<hmax+inc;q+=inc){
	    x=(q-hmin)*hs+SPWIN_X_OFF;
	    XDrawLine(theDisp, spc_win, spc_gc, x, y, x, y-8);
	    if(spc_gridOn)
		XDrawLine(theDisp, spc_win, spc_dot_gc, x, y-8, x, MARGIN);
	    sprintf(buf, "%g", q);
	    (void) xv_get(text_font,FONT_STRING_DIMS,buf,&dims);

	    XDrawString(theDisp, spc_win, spc_gc, x-dims.width/2, y+2+dims.height, buf, strlen(buf));
	    for(p=q;p<q+inc;p+=inc/10) {
		x=(p-hmin)*hs+SPWIN_X_OFF;
		XDrawLine(theDisp, spc_win, spc_gc, x, y-4, x, y);
		if(spc_gridOn)
		    XDrawLine(theDisp, spc_win, spc_smdot_gc, x, y-4, x, MARGIN);

	    }

	}
    }
}

static void PlotYAxis(float ymax, float ymin, float vs, float inc)
{
    char buf[30];

    XDrawLine(theDisp, spc_win, spc_gc, SPWIN_X_OFF, MARGIN,
	      SPWIN_X_OFF, spctrmwin_h-SPWIN_Y_OFF);
    /* label */
    DrawVertString(spc_win, spc_gc, 10, spctrmwin_h/2-100, "AMPLITUDE");

    if (use_loglog) {
	int i,iymax,iymin;
	int y;

	iymax=(int)ymax;
	iymin=(int)ymin;
	PrintDecade(iymin, SPWIN_X_OFF/2, spctrmwin_h-SPWIN_Y_OFF-5);
	PrintDecade(iymax, SPWIN_X_OFF/2, MARGIN+10);
	for(i=iymin+1; i<= iymax-1; i++) {
	    y= (iymax-i)*vs+ MARGIN;
	    XDrawLine(theDisp, spc_win, spc_gc, SPWIN_X_OFF, y,
		      SPWIN_X_OFF+8, y);
	    if(spc_gridOn)
		XDrawLine(theDisp, spc_win, spc_dot_gc, SPWIN_X_OFF+8, y,
			  spctrmwin_w-MARGIN, y);
	    PrintDecade(i, SPWIN_X_OFF/2, y);
	}
	/* smaller marks */
	for(i=iymin; i<= iymax-1; i++) {
	    int k, yy;
	    y= (iymax-i)*vs + MARGIN;
	    for(k=2; k<10; k++) {
		yy= y-log10((double)k)*vs;
		XDrawLine(theDisp, spc_win, spc_gc, SPWIN_X_OFF, yy,
			  SPWIN_X_OFF+4, yy);
		if(spc_gridOn)	
		    XDrawLine(theDisp, spc_win, spc_smdot_gc, SPWIN_X_OFF+4, yy,
			      spctrmwin_w-MARGIN, yy);
	    }
	}
    }else {
	int x,y;
	float q,p;
	Font_string_dims dims;

	x=SPWIN_X_OFF+8;
	for (q=ymin;q<ymax+inc;q+=inc){
	    y=(ymax-q)*vs+MARGIN;
	    XDrawLine(theDisp, spc_win, spc_gc, x, y, x-8, y);
	    if(spc_gridOn)
		XDrawLine(theDisp, spc_win, spc_dot_gc, SPWIN_X_OFF+8, y,
			  spctrmwin_w-MARGIN, y);

	    sprintf(buf, "%.3g", q);
	    (void) xv_get(text_font,FONT_STRING_DIMS,buf,&dims);
	
	    XDrawString(theDisp, spc_win, spc_gc, x-9-dims.width, y+dims.height/2, buf, strlen(buf));

	    for(p=q;p<q+inc;p+=inc/10) {
		y=(ymax-p)*vs+MARGIN;
		XDrawLine(theDisp, spc_win, spc_gc, x-4, y, x-8, y);
		if(spc_gridOn)
		    XDrawLine(theDisp, spc_win, spc_smdot_gc, SPWIN_X_OFF+4, y,
			      spctrmwin_w-MARGIN, y);

	    }
	}
    }
}

/**********************************************************************
 *   handle events for spectrum operations                            *
 **********************************************************************/

void drawspectrum ()
{
    float hs,vs;
    double ymax=-1,ymin=-1;
    double ystep,hstep;
    double hmax,hmin;
    double tmax,tmin;
    char  dash[2]={8,4};
    char  dots[2]={4,4};

    int i,j,k;

    ystep = 1.0;
    hstep = 1.0;
  
    /* if no selection region return */
    if (sel_hold->num_sel==0) {return;}

    /* calc max and min values (ver and hor) */
    for (i=0;i<sel_hold->num_sel;i++) {
	float srate,freqk;
	k=sel_hold->sel_reg[i];
	srate=sel_hold->regs[k].info.sample_rate;
	freqk=srate/spcts[k]->n_pnts;

	/* first time through */
	if (ymin==-1) {
	    ymin=ymax=spcts[k]->power[0];
	    if (!spc_fre_pre) {
		hmin=freqk;
		hmax=freqk*spcts[k]->n2_pnts;
	    } else {
		hmax=1/freqk;
		hmin=1/(freqk*spcts[k]->n2_pnts);
	    }
	}

	for (j=0;j<spcts[k]->n2_pnts-1;j++) {
	    if (spcts[k]->power[j]>ymax) {
		ymax=spcts[k]->power[j];
	    }else if (spcts[k]->power[j]<ymin) {
		ymin=spcts[k]->power[j];
	    }
	}

	if (!spc_fre_pre) {
	    if (hmin>freqk)  hmin=freqk;
	    if (hmax<freqk*spcts[k]->n2_pnts) hmax=freqk*spcts[k]->n2_pnts;
	} else {
	    if (hmax<1/freqk)  hmax=1/freqk;
	    if (hmin>1/(freqk*spcts[k]->n2_pnts)) hmin=1/(freqk*spcts[k]->n2_pnts);
	}
    }

    /* calc vertical and horizontal scales */
    if (use_loglog) {
	ymax= ceil(log10((double)ymax));
	ymin= floor(log10((double)ymin));
    } else {
	scale_line_axis(ymin,ymax,&ystep,&tmin,&tmax);
	ymin=tmin;
	ymax=tmax;
    }
    vs=(float)(spctrmwin_h-SPWIN_Y_OFF-MARGIN)/(ymax-ymin);


    if (use_loglog==1) {    /* log-log */
	hmax= ceil(log10((double)hmax));
	hmin= floor(log10((double)hmin));
    }else {	/* linear and log-linear */
	scale_line_axis(hmin,hmax,&hstep,&tmin,&tmax);
	hmin=tmin;
	hmax=tmax; 
    }
    hs= (float)(spctrmwin_w-SPWIN_X_OFF-3*MARGIN)/(hmax-hmin);

    /* plot & label Y-axis */
    PlotYAxis(ymax, ymin, vs, ystep);

    /* plot X-axis */
    PlotXAxis(hmin, hmax, hs, hstep);

    /* plot the spectra */
    /* and plot legend  */

    for (i=0;i<sel_hold->num_sel;i++) {
	int s,max_plot=(XMaxRequestSize(theDisp)-3)/2, num_plot_p;
	XPoint *points=(XPoint *)malloc(sizeof(XPoint)* (max_plot+4));
	float srate,freqk;
	int txval,t_prev,t_max,t_min,num_pt=0;
	int w, num_time=1,tyval;
	int pl_pos=0;
	double x_val,ftxval,ftyval;

	s=update_glob_line_at(i);
	k=sel_hold->sel_reg[i];
	srate=sel_hold->regs[k].info.sample_rate;
	freqk=srate/spcts[k]->n_pnts;
	num_plot_p=spcts[k]->n2_pnts-1;
    
	for (j=0;j<num_plot_p;j++) {
	    if (!spc_fre_pre) {
		x_val=freqk*(j+1);
	    } else {
		x_val=1/(freqk*(j+1));
	    }

	    if (use_loglog==1) { /* log horizontal scale */
		ftxval=log10((double)x_val);
	    }else {
		ftxval=x_val;
	    }
      
	    if (use_loglog) { /* log verticle scale */
		ftyval=log10(spcts[k]->power[j]);
	    } else { /* linear verticle scale */
		ftyval=spcts[k]->power[j];
	    }
      
	    txval=SPWIN_X_OFF+(ftxval-hmin)*hs;
	    tyval=spctrmwin_h-SPWIN_Y_OFF-(ftyval-ymin)*vs;

	    if (txval!=t_prev) {
		/* plot old points */
		if (num_pt!=0) {
		    points[pl_pos].x=t_prev;
		    points[pl_pos++].y=t_min;
		    if (num_pt>1) {
			points[pl_pos].x=t_prev;
			points[pl_pos++].y=t_max;
		    }
		}
	
		/* new point */
		t_prev=txval;
		t_max=t_min=tyval;
		num_pt=1;
	    } else {
		num_pt+=1;
		if (tyval>=t_max) {
		    t_max=tyval;
		} else if (tyval<t_min) {
		    t_min=tyval;
		}
	    }
      
	}
	XDrawLines(theDisp, spc_win, glob_colour_gc[s], points,pl_pos-2 , CoordModeOrigin);
	free(points);
    }

    XFlush(theDisp);

    if (sel_hold->num_sel>num_glob_gc) {reset_glob_line_at();}
}


static void calc_hold_spctrm()
{
    int i,j=0;
    float *y, srate;
    float *a, *b;
    int nt2h, n2;
    hold_spct *t_hold;

    /* free old memory */
    if (spcts!=NULL) {
	while(spcts[j]!=NULL) {
	    if (spcts[j]->amp!=NULL) free(spcts[j]->amp);
	    if (spcts[j]->phase!=NULL) free(spcts[j]->phase);
	    if (spcts[j]->power!=NULL) free(spcts[j]->power);
	    free(spcts[j]);
	    j+=1;
	}
	free(spcts);
    }

    /* fill new memory */
    spcts=(hold_spct**)malloc(sizeof(hold_spct*)*(sel_hold->num_wind+1));
    spcts[sel_hold->num_sel]=NULL;

    for (i=0;i<sel_hold->num_wind;i++) {
	t_hold=(hold_spct*)malloc(sizeof(hold_spct));
	spcts[i]=t_hold;

	CalcSpectrum(sel_hold->regs[i].info.sample_rate, sel_hold->regs[i].data
		     , 0, sel_hold->regs[i].info.n_values , &y, &a, &b,
		     &n2, &nt2h, spc_soph); 

	spcts[i]->amp=a;
	spcts[i]->phase=b;
	spcts[i]->power=y;
	spcts[i]->n_pnts=n2;
	spcts[i]->n2_pnts=nt2h;
    }
    spcts[sel_hold->num_wind]=NULL;
}

void handle_spectrum()
{
    sel_hold=creating_gsr_win(0);
    if (sel_hold->num_wind == 0) return;
  
    calc_hold_spctrm();

    open_spctrm_win(0);

    redo_spctrm();
}

void event_hand_spectrum(Select_Info *new_event)
{
    sel_hold=new_event;
    /*  calc_hold_spctrm(); */
    open_spctrm_win(0);
    redo_spctrm();

}

static void redo_spctrm()
{
    /* Trace *trc= traces[cur_itrc];
       /* plot again -- might be different */
    /* if(used_reg!=NULL) {
       /*	PlotSpectrum(cur_itrc,used_reg->left_index, used_reg->right_index );
       /* }*/
    redrawSpctrm();
}

static void next_spctrm()
{
    int move=sel_hold->num_sel-1;
    extern void redraw_gsrwin(Canvas reg);

    sel_hold->sel_reg[move]+=1;

    if (sel_hold->sel_reg[move]>=sel_hold->num_wind)
	sel_hold->sel_reg[move]=0;

    redraw_gsrwin(sel_hold->can);
    redrawSpctrm();
}

static void toggle_grid()
{
    if(spc_gridOn) {
	/* turn off */
	spc_gridOn= 0;
    }else {
	/* turn on */
	spc_gridOn= 1;
    }
    redo_spctrm();
}

/**********************************************************************
 *   Plotting wave portion                                            *
 **********************************************************************/

void PlotWavePortion(int itrc, int ix1, int ix2, int box_x, int box_y,
		     int box_h, int box_w)
{
    float vs, hs;
    float ymax, ymin, *data;
    int i;
    Trace *trc= traces[itrc];
    XPoint *points, *plot;

    if(!spctrm_plot_portion) return;

    if(!fixIxBounds(&ix1,&ix2,trc->wave->info.n_values))
	return;

    /* clear the region */
    XClearArea(theDisp, spc_win, box_x, box_y, box_w, box_h, False);
    /* draw the box */
    XDrawRectangle(theDisp, spc_win, spc_gc, box_x, box_y, box_w, box_h);
    
    data= trc->wave->data;
    /* ok, have to do all this again */
    hs= (float)box_w/(ix2-ix1+1);

    ymax= ymin= data[ix1];
    for(i=ix1+1; i<= ix2; i++) {
	if (data[i]>ymax) {
	    ymax= data[i];
	}else if (data[i]<ymin){
	    ymin= data[i];
	}
    }
    
    vs= (float)(box_h-20)/(ymax-ymin);

    plot= points= (XPoint *)Malloc(sizeof(XPoint)*(ix2-ix1+1));
    box_y+=box_h-10;

    if (hs<1.0) {
	int dec= 1, n;
	i=n= 0;
	if(hs<0.02) dec= (int)(((float)(ix2-ix1+1))/(box_w/0.02));
	while(n*hs < box_w) {
	    points->x= box_x + n*hs;
	    points->y= box_y - (data[n+ix1]-ymin)*vs;
	    points++; n+=dec; i++;
	}
    } else {
	for(i=0; i< ix2-ix1+1; i++) {
	    points->x= box_x + i*hs;
	    points->y= box_y - (data[i+ix1]-ymin)*vs;
	    points++;
	}
    }

    XDrawLines(theDisp, spc_win, spc_gc, plot, i, CoordModeOrigin);
    XFlush(theDisp);
    free(plot);
}    


