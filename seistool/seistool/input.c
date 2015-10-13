#ifndef lint
static char id[] = "$Id: input.c,v 1.2 2013/02/28 21:24:59 lombard Exp $";
#endif

/*
 * input.c--
 *    implements a generic input from X terminal routine
 *
 * Copyright (c) 1992-3 Andrew K. Yu, Lawrence Berkeley Laboratory
 *      and University of California, Berkeley.
 * All rights reserved.
 */
#include <xview/xview.h>
#include <xview/panel.h>
#include "proto.h"
#include "xv_proto.h"

#define MAXBUFLEN   1024

static Frame in_frame;
static char *in_buf;
static int  in_numch;
static Panel_item in_text;
static int in_loop=0;

static void in_done_proc()
{
    if (in_loop) xv_window_return(0);
}

static void in_notify_proc()
{
    char *s= (char *)xv_get(in_text, PANEL_VALUE);
    strncpy(in_buf, s, in_numch);
    if (in_loop) xv_window_return(0);
}

void GetString(char *label, char *str, int numch)
{
    Panel in_panel;
    int x, y;

    in_buf=(char *)Malloc(numch+1);
    if (in_buf==NULL) return;	/* not enuff mem */
    in_numch= numch;
    *in_buf='\0';
    x= (int)xv_get(tracesFrame,XV_X);
    y= (int)xv_get(tracesFrame,XV_Y);
    in_frame=(Frame)xv_create(tracesFrame, FRAME_CMD,
	FRAME_LABEL, "",
	FRAME_DONE_PROC, in_done_proc,
	XV_X, FrmWidth/2+x-150, XV_Y, FrmHeight/2+y-50,
	XV_HEIGHT, 100, XV_WIDTH, 300,
	NULL);
    in_panel=(Panel)xv_get(in_frame, FRAME_CMD_PANEL);
    in_text=(Panel_item)xv_create(in_panel, PANEL_TEXT,
	XV_X, 10, XV_Y, 40,
	PANEL_LAYOUT, PANEL_VERTICAL,
	PANEL_LABEL_STRING, label,
	PANEL_VALUE_DISPLAY_WIDTH, 280,
	PANEL_NOTIFY_PROC, in_notify_proc,
	NULL);
    xv_set(in_frame, XV_SHOW, TRUE, NULL);
    in_loop=1;
    (void)xv_window_loop(in_frame);
    in_loop=0;
    xv_destroy_safe(in_frame);
    strncpy(str, in_buf, numch);
    free(in_buf);
}
