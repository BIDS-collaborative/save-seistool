#ifndef lint
static char id[] = "$Id: fq_panel.c,v 1.2 2013/02/28 21:25:00 lombard Exp $";
#endif

/*
 * fq_panel.c--
 *    filters panel
 *
 * Copyright (c) 1992-3 Andrew K. Yu, Lawrence Berkeley Laboratory
 *      and University of California, Berkeley.
 * All rights reserved.
 */
#include <xview/xview.h>
#include <xview/frame.h>
#include <xview/canvas.h>
#include <xview/panel.h>
#include <math.h>
#include "proto.h"
#include "xv_proto.h"

static Frame fq_frame= (Frame)NULL;
static Panel_item fq_list_i, inst_list_i;
static Dial *fl_dial, *fh_dial;
static Panel_item fl_txt, fh_txt;
static int inst_cho=0;

float fq_fl_value=0.01,fq_fh_value=0.1;

static void InitFQPanel();

void fq_updateLowFreq(float fl)
{
    char buf[30];
    sprintf(buf,"%.2e",fl);
    xv_set(fl_txt, PANEL_VALUE, buf, NULL);
    fq_fl_value=fl;
}

void fq_updateHighFreq(float  fh)
{
    char buf[30];
    sprintf(buf,"%.2e",fh);
    xv_set(fh_txt, PANEL_VALUE, buf, NULL);
    fq_fh_value=fh;
}

static Panel_setting fl_txt_notify(Panel_item item, Event *event)
{
    char *flS=(char *)xv_get(item,PANEL_VALUE);
    float fl= atof(flS);

    fq_fl_value=fl;
    SetDialVal(fl_dial, fl);
}

static Panel_setting fh_txt_notify(Panel_item item, Event *event)
{
    char *fhS=(char *)xv_get(item,PANEL_VALUE);
    float fh= atof(fhS);

    fq_fh_value=fh;
    SetDialVal(fh_dial, fh);
}

void open_freq_panel()
{
    if(!fq_frame) InitFQPanel(tracesFrame);
    xv_set(fq_frame, XV_SHOW, TRUE, NULL);
}

static void close_freq_panel()
{
    xv_set(fq_frame, XV_SHOW, FALSE, NULL);
}

static void cho_fil_type() 
{
    int which=(int)xv_get(fq_list_i, PANEL_LIST_FIRST_SELECTED);

    switch(which) {
    case 0 : xv_set(fh_txt,PANEL_INACTIVE,TRUE,NULL);
	xv_set(fl_txt,PANEL_INACTIVE,TRUE,NULL);
	disable_dial(fh_dial);
	disable_dial(fl_dial);
	break;
    case 1 :
    case 2 : xv_set(fh_txt,PANEL_INACTIVE,FALSE,NULL);
	xv_set(fl_txt,PANEL_INACTIVE,FALSE,NULL);
	enable_dial(fh_dial);
	enable_dial(fl_dial);
	break;
    case 3 :
    case 4 : xv_set(fl_txt,PANEL_INACTIVE,TRUE,NULL);
	xv_set(fh_txt,PANEL_INACTIVE,FALSE,NULL);
	enable_dial(fh_dial);
	disable_dial(fl_dial);
	break;
    case 5 :
    case 6 : xv_set(fh_txt,PANEL_INACTIVE,TRUE,NULL);
	xv_set(fl_txt,PANEL_INACTIVE,FALSE,NULL);
	enable_dial(fl_dial);
	disable_dial(fh_dial);
	break;
    }
}

#define FHIGH_DIAL  0
#define FLOW_DIAL  1

static void FilterAll()
{
    char *str;
    int which=(int)xv_get(fq_list_i, PANEL_LIST_FIRST_SELECTED);
    int inst=(int)xv_get(inst_list_i,PANEL_LIST_FIRST_SELECTED);
    float fh, fl;

    str= (char *)xv_get(fl_txt, PANEL_VALUE);
    fl= atof(str);
    str= (char *)xv_get(fh_txt, PANEL_VALUE);
    fh= atof(str);
    /* just fake out the filtering routine */
    /* for the hi and lo pass cases        */
    if (which>4) {
	fh=fl;
    } else if (which>2) {
	fl=fh;
    }

    if(fl>fh) {
	fprintf(stderr,"Error: low freq > high freq\n");
	return;
    }

    /*  val=xv_get(); */
    ApplyFilter(which, inst_cho, 1, fl, fh);
}

static void FilterSelected()
{
    char *str;
    int which= (int)xv_get(fq_list_i, PANEL_LIST_FIRST_SELECTED);
    int inst=(int)xv_get(inst_list_i,PANEL_LIST_FIRST_SELECTED);
    float fh, fl;

    str= (char *)xv_get(fl_txt, PANEL_VALUE);
    fl= atof(str);
    str= (char *)xv_get(fh_txt, PANEL_VALUE);
    fh= atof(str);
    /* just fake out the filtering routine */
    /* for the hi and lo pass cases        */
    if (which >4) {
	fh=fl;
    } else if (which>2) {
	fl=fh;
    }
    if(fl>fh) {
	fprintf(stderr,"Error: low freq > high freq\n");
	return;
    }
    ApplyFilter(which, inst_cho, 0, fl, fh);
}

static void cho_inst_type (Panel_item item, char *name, Xv_opaque data,
			   Panel_list_op oper, Event *evt, int row)
{
    if (data==-1) {
	data=0;
    }
    inst_cho = data;
}

static void InitFQPanel(Frame frame)
{    

    Panel panel;
    char t_frq_str[1000];

    fq_frame= (Frame)xv_create(frame, FRAME,
			       FRAME_LABEL, "Filters",
			       XV_HEIGHT, 210, XV_WIDTH,630,
			       NULL);
    panel = (Panel)xv_create(fq_frame,PANEL, NULL);

    fl_dial= (Dial *)newDial(20, 50, 120, 120, DIAL_KNOB_TYPE);
    SetDialLabel(fl_dial, "0", "5", "10");
    fl_dial->scale=(float)1/30;
    InitDial(fq_frame, fl_dial);
    SetDialChangeFunc(fl_dial, FLOW_DIAL, fq_updateLowFreq);
    SetDialVal(fl_dial, fq_fl_value);
    sprintf(t_frq_str,"%.2e",fq_fl_value);

    (void)xv_create(panel, PANEL_MESSAGE,
		    XV_X, 30, XV_Y, 35,			  
		    PANEL_LABEL_STRING, "Low Freq (Hz)",
		    PANEL_LABEL_BOLD, TRUE,
		    NULL);
    fl_txt=(Panel_item)xv_create(panel, PANEL_TEXT,
				 XV_X, 50, XV_Y, 180,			  
				 PANEL_VALUE_DISPLAY_WIDTH, 70,
				 PANEL_NOTIFY_PROC, fl_txt_notify,
				 PANEL_VALUE,t_frq_str ,
				 NULL);
	
    sprintf(t_frq_str,"%.2e",fq_fh_value);
    fh_dial= (Dial *)newDial(150, 50, 120, 120, DIAL_KNOB_TYPE);
    SetDialLabel(fh_dial, "0", "500", "1000");
    fh_dial->scale=(float)10/3;
    InitDial(fq_frame, fh_dial);
    SetDialChangeFunc(fh_dial, FHIGH_DIAL, fq_updateHighFreq);
    SetDialVal(fh_dial,fq_fh_value);

    (void)xv_create(panel, PANEL_MESSAGE,
		    XV_X, 180, XV_Y, 35,			  
		    PANEL_LABEL_STRING, "High Freq (Hz)",
		    PANEL_LABEL_BOLD, TRUE,
		    NULL);
    fh_txt=(Panel_item)xv_create(panel, PANEL_TEXT,
				 XV_X, 180, XV_Y, 180,			  
				 PANEL_VALUE_DISPLAY_WIDTH, 70,			  
				 PANEL_NOTIFY_PROC, fh_txt_notify,
				 PANEL_VALUE, t_frq_str ,
				 NULL);

    (void)xv_create(panel, PANEL_BUTTON,
		    XV_X, 20, XV_Y, 5,
		    PANEL_LABEL_STRING,	"Quit",
		    PANEL_NOTIFY_PROC,	close_freq_panel,
		    NULL);

    (void)xv_create(panel, PANEL_BUTTON,
		    XV_X, 80, XV_Y, 5,
		    PANEL_LABEL_STRING,	"Filter All",
		    PANEL_NOTIFY_PROC,	FilterAll,
		    NULL);

    (void)xv_create(panel, PANEL_BUTTON,
		    XV_X, 170, XV_Y, 5,
		    PANEL_LABEL_STRING,	"Filter Selected",
		    PANEL_NOTIFY_PROC,	FilterSelected,
		    NULL);

    fq_list_i= (Panel_item)xv_create(panel, PANEL_LIST,
				     XV_X, 310, XV_Y, 50,	    
				     PANEL_LIST_TITLE, "Filters",			  
				     PANEL_LIST_DISPLAY_ROWS, 6,
				     /*	PANEL_LIST_WIDTH, 80,*/
				     PANEL_NOTIFY_PROC,cho_fil_type,
				     PANEL_LIST_STRINGS, "No filter", "bandps bwth","bandps bwth 0 phase", "lopass bwth",
				     "lopass bwth 0 phase","hipass bwth","hipass bwth 0 phase",
				     NULL,
				     PANEL_LIST_CLIENT_DATAS,0,1,2,3,4,5,6, NULL,
				     PANEL_LIST_SELECT,1,
				     NULL);

    inst_list_i=(Panel_item)xv_create(panel, PANEL_LIST,
				      XV_X, 500, XV_Y, 50,
				      PANEL_LIST_TITLE, "Instrument",
				      PANEL_LIST_DISPLAY_ROWS, 6,
				      PANEL_NOTIFY_PROC, cho_inst_type,
				      PANEL_LIST_STRINGS, "No deconv", "Displacement", "Velocity",
				      "Acceleration","Benioff 100 kg",
				      "20s Butterworth", "Wood Anderson",
				      "ULP instrument","STS2 LP response",
				      NULL,
				      PANEL_LIST_CLIENT_DATAS,-1,20,21,22,1,2,3,4,11,NULL,
				      NULL);
    xv_set(fq_list_i,PANEL_LIST_SELECT,1,TRUE,NULL);

}

	
