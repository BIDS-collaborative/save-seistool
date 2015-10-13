#ifndef lint
static char id[] = "$Id: sp_panel.c,v 1.2 2013/02/28 21:24:57 lombard Exp $";
#endif

/*
 * sp_panel.c--
 *    panel for selecting a window on a trace
 *
 * Copyright (c) 1992-3 Andrew K. Yu, Lawrence Berkeley Laboratory
 *      and University of California, Berkeley.
 * All rights reserved.
 */
#include <stdio.h>
#include <math.h>
#include <xview/xview.h>
#include <xview/panel.h>
#include <xview/scrollbar.h>
#include "xv_proto.h"

static Frame sp_frame= (Frame)0;
static Canvas sp_canvas;
static Panel sp_panel;
static Panel_item choice, len_txt, off_txt;

static int nsamp_or_sec= 0; /* nsamp */
static int sp_nsamp;
static float sp_sec;

static void choice_notify_proc(Panel_item item, int value, Event *event);
static void InitSpPanel(Frame frame);

static void close_sp_panel();


static void choice_notify_proc(Panel_item item, int value, Event *event)
{
    char *val, buf[100];
    if (value!=nsamp_or_sec) {
	val= (char *)xv_get(len_txt, PANEL_VALUE);
	if (value==0) {
	    /* nsamp: save seconds and set nsamp */
	    sp_sec= atof(val);
	    sprintf(buf,"%d",sp_nsamp);
	    xv_set(len_txt, PANEL_VALUE, buf, NULL);
	}else {	
	    /* sec: save nsamp and set secs */
	    sp_nsamp= atoi(val);
	    sprintf(buf,"%f",sp_sec);
	    xv_set(len_txt, PANEL_VALUE, buf, NULL);
	}
    }
    nsamp_or_sec=value;
}

int get_window_length(int *nsamp, float *sec)
{
    char *val;
    if(!sp_frame) {
	*nsamp=0;
	*sec=0.0;
	return 0;
    }
    if(nsamp_or_sec==0) {
	val= (char *)xv_get(len_txt, PANEL_VALUE);
	*nsamp= atoi(val);
	*sec=0.0;
    }else {
	val= (char *)xv_get(len_txt, PANEL_VALUE);
	*nsamp= 0;
	*sec= (float)atof(val);
    }
    return nsamp_or_sec;
}

static void InitSpPanel(Frame frame)
{    
    sp_frame = (Frame)xv_create(frame, FRAME,
	FRAME_LABEL, "Window Parameters",
	XV_X, FrmWidth/2+xv_get(tracesFrame,XV_X)-150,
	XV_Y, FrmHeight/2+xv_get(tracesFrame,XV_Y)-50,
	XV_HEIGHT, 150, XV_WIDTH, 370,
	NULL);
    sp_canvas= (Canvas)xv_create(sp_frame, CANVAS,
	XV_HEIGHT, 150, XV_WIDTH, 370, NULL);
    sp_panel = (Panel)xv_create(sp_canvas, PANEL,
	XV_HEIGHT, 150, XV_WIDTH, 370, NULL);

    choice= (Panel_item)xv_create(sp_panel, PANEL_CHOICE,
	XV_X, 80, XV_Y, 40,			  
	PANEL_CHOICE_STRINGS, "samples","seconds",NULL,
	PANEL_VALUE, 0,
	PANEL_LABEL_STRING, "Units:",
	PANEL_NOTIFY_PROC, choice_notify_proc,
	NULL);
    len_txt = (Panel_item)xv_create(sp_panel, PANEL_TEXT,
	XV_X, 30, XV_Y, 80,
	PANEL_VALUE_DISPLAY_WIDTH, 200,
	PANEL_LABEL_STRING, "Length: ",
	NULL);
    (void) xv_create(sp_panel, PANEL_BUTTON,
	XV_X, 300, XV_Y, 8,	     
	PANEL_LABEL_STRING, "Close",
	PANEL_NOTIFY_PROC, close_sp_panel, NULL);
}

void open_sp_panel()
{
    if(!sp_frame)
	InitSpPanel(tracesFrame);
    xv_set(sp_frame, XV_SHOW, TRUE, NULL);
}

static void close_sp_panel()
{
    xv_set(sp_frame, XV_SHOW, FALSE, NULL);
}
