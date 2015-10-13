#ifndef lint
static char id[] = "$Id: print.c,v 1.2 2013/02/28 21:24:58 lombard Exp $";
#endif

/*
 * print.c--
 *    the "print" panel
 *
 * Copyright (c) 1992-3 Andrew K. Yu, Lawrence Berkeley Laboratory
 *      and University of California, Berkeley.
 * All rights reserved.
 */
#include <stdio.h>
#include <xview/xview.h>
#include <xview/panel.h>
#include "proto.h"
#include "xv_proto.h"

extern Trace **traces;
extern int LastTrack;
extern int TotalTracks;

char *defaultPrinter= NULL;

static Frame prn_frame= 0;
static Panel panel;
static Panel_item choice, choice2, slider, text;
static char printer_name[200];
static char file_name[200];
static int maxTraces;
static int prn_or_file;
static int prn_selected;

static void choice_notify_proc(Panel_item item, int value, Event *event);
static void choice2_notify_proc(Panel_item item, int value, Event *event);
static void slider_notify_proc(Panel_item item, int value, Event *event);
static void text_notify_proc(Panel_item item, int value, Event *event);
static void InitPrnPanel(Frame frame, int x, int y);
static void setup_prn_panel_vals();
static void prn_activate();
static void prn_cancel();
static int count_selected();
static void selected_to_prn();
static void all_to_prn();
static void selected_to_file();
static void all_to_file();


static void choice_notify_proc(Panel_item item, int value, Event *event)
{
    if (value!=prn_or_file) {
	if (value==0) { /* printer */
	    strcpy(file_name,(char*)xv_get(text, PANEL_VALUE));
	    xv_set(text, PANEL_VALUE, printer_name, NULL);
	}else {
	    strcpy(printer_name,(char *)xv_get(text, PANEL_VALUE));
	    xv_set(text, PANEL_VALUE, file_name, NULL);
	}
    }
    prn_or_file=value;
}

static void choice2_notify_proc(Panel_item item, int value, Event *event)
{
    prn_selected= value;
}

static void slider_notify_proc(Panel_item item, int value, Event *event)
{
    maxTraces=value;
}

static void text_notify_proc(Panel_item item, int value, Event *event)
{
    if (prn_or_file==0) {
	strcpy(printer_name, (char *)xv_get(text, PANEL_VALUE));
    }else {
	strcpy(file_name, (char *)xv_get(text, PANEL_VALUE));
    }
}

static void InitPrnPanel(Frame frame, int x, int y)
{    
    prn_frame = (Frame)xv_create(frame, FRAME_CMD,
	FRAME_LABEL, "Print",
	XV_X, x, XV_Y, y,
	XV_HEIGHT, 300, XV_WIDTH, 370, NULL);
    panel = (Panel)xv_get(prn_frame, FRAME_CMD_PANEL);

    choice= (Panel_item)xv_create(panel, PANEL_CHOICE,
	XV_X, 140, XV_Y, 20,			  
	PANEL_CHOICE_STRINGS, "Printer","File",NULL,
	PANEL_VALUE, 0,
	PANEL_NOTIFY_PROC, choice_notify_proc,
	NULL);
    choice2= (Panel_item)xv_create(panel, PANEL_CHOICE,
	XV_X, 100, XV_Y, 60,			  
	PANEL_CHOICE_STRINGS, "Print all","Print selected",NULL,
	PANEL_VALUE, 0,
	PANEL_NOTIFY_PROC, choice2_notify_proc,
	NULL);
    text= (Panel_item)xv_create(panel, PANEL_TEXT,
	XV_X, 30, XV_Y, 120,
	PANEL_VALUE_DISPLAY_WIDTH, 250,
	PANEL_LABEL_STRING, "Name: ",
	NULL);
    slider= xv_create(panel, PANEL_SLIDER,
	XV_X, 30, XV_Y, 180,			  
	PANEL_LABEL_STRING, "traces on a page ",
	PANEL_MIN_VALUE, 1,
	PANEL_MAX_VALUE, TotalTracks,
	PANEL_SHOW_RANGE, TRUE,
	PANEL_NOTIFY_PROC, slider_notify_proc,
	NULL);
    (void) xv_create(panel, PANEL_BUTTON,
	XV_X, 120, XV_Y, 250,	     
	PANEL_LABEL_STRING, "Print",
	PANEL_NOTIFY_PROC, prn_activate, NULL);
    (void) xv_create(panel, PANEL_BUTTON,
	XV_X, 200, XV_Y, 250,
	PANEL_LABEL_STRING, "Cancel",
	PANEL_NOTIFY_PROC, prn_cancel, NULL);
    maxTraces= 12;
    if (defaultPrinter) strcpy(printer_name,defaultPrinter);
    *file_name='\0';
    prn_or_file=0;
    prn_selected=0;
}

static void setup_prn_panel_vals()
{
    xv_set(choice, PANEL_VALUE, prn_or_file, NULL);
    if (prn_or_file==0) {
	xv_set(text, PANEL_VALUE, printer_name, NULL);
    }else {
	xv_set(text, PANEL_VALUE, file_name, NULL);
    }
    xv_set(choice2, PANEL_VALUE, prn_selected, NULL);
    xv_set(slider, PANEL_VALUE, maxTraces, NULL);
}

void show_print_panel()
{
    if(!prn_frame) {
	int x, y;
	x= (int)xv_get(tracesFrame, XV_X);
	y= (int)xv_get(tracesFrame, XV_Y);
        InitPrnPanel(tracesFrame, x+FrmWidth-400,y+30);
    }

    /* save old values */
    setup_prn_panel_vals();
    xv_set(prn_frame, XV_SHOW, TRUE, NULL);
}

static void prn_activate()
{
    if (prn_or_file==0) { /* print to printer */
	strcpy(printer_name,(char *)xv_get(text, PANEL_VALUE));
	if (prn_selected) { /* print only selected */
	    selected_to_prn();
	}else {	/* print all */
	    all_to_prn();
	}
    }else { /* print to file */
	strcpy(file_name,(char*)xv_get(text, PANEL_VALUE));
	if (prn_selected) { /* print only selected */
	    selected_to_file();
	}else {	/* print all */
	    all_to_file();
	}
    }
    xv_set(prn_frame, XV_SHOW, FALSE, NULL);	/* done */
}

static void prn_cancel()
{
    xv_set(prn_frame, XV_SHOW, FALSE, NULL);
}

/********** real printing procedures ************/

static int count_selected()
{
    int i,count;
    count=0;
    for(i=0; i <= LastTrack; i++) {
	if (traces[i]->selected) count++;
    }
    return count;
}

static void selected_to_prn()
{
    int num, itrc, printed;
    char *title, cmd[100];

    num= count_selected();
    if (num>0) {
	/* plot to the tmp file first */
	sprintf(cmd, "lpr -P%s", printer_name);
	InitPlot(cmd, (num<maxTraces)?num:maxTraces, 1 /*printer*/);
	printed=0;
	title=(char*)xv_get(tracesFrame, FRAME_LEFT_FOOTER);
	for(itrc=0; itrc<=LastTrack; itrc++) {
	    if(traces[itrc]->selected) {
		if (printed>=maxTraces) {
		    ShowPage(title);
		    printed=0;
		}
		PS_PlotWave(itrc);
		printed++;
	    }
	}
	EndPlot(title);
    }
}

static void all_to_prn()
{
    int num=LastTrack+1, itrc,printed;
    char cmd[100], *title;

    if (num>0) {
	/* plot to the tmp file first */
	sprintf(cmd, "lpr -P%s",printer_name);
	InitPlot(cmd, (num<maxTraces)?num:maxTraces, 1 /* printer */);
	printed=0;
	title=(char*)xv_get(tracesFrame, FRAME_LEFT_FOOTER);
	for(itrc=0; itrc<=LastTrack; itrc++) {
	    if (printed>=maxTraces) {
		ShowPage(title);
		printed=0;
	    }
	    PS_PlotWave(itrc);
	    printed++;
	}
    	EndPlot(title);
    }

}

static void selected_to_file()
{
    int num, itrc, printed;
    char *title=NULL;
    
    num= count_selected();
    if (num>0) {
	InitPlot(file_name, (num<maxTraces)?num:maxTraces, 0);
	printed=0;
	title=(char*)xv_get(tracesFrame, FRAME_LEFT_FOOTER);
	for(itrc=0; itrc<=LastTrack; itrc++) {
	    if(traces[itrc]->selected) {
		if (printed>=maxTraces) {
		    ShowPage(title);
		    printed=0;
		}
		PS_PlotWave(itrc);
		printed++;
	    }
	}
	EndPlot(title);
    }
}

static void all_to_file()
{
    int num=LastTrack+1, itrc, printed;
    char *title;

    if (num>0) {
	/* plot to the tmp file first */
	InitPlot(file_name, (num<maxTraces)?num:maxTraces, 0);
	printed=0;
	title=(char*)xv_get(tracesFrame, FRAME_LEFT_FOOTER);
	for(itrc=0; itrc<=LastTrack; itrc++) {
	    if (printed>=maxTraces) {
		ShowPage(title);
		printed=0;
	    }
	    PS_PlotWave(itrc);
	    printed++;
	}
    	EndPlot(title);
    }
}

