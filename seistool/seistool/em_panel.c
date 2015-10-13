#ifndef lint
static char id[] = "$Id: em_panel.c,v 1.2 2013/02/28 21:25:00 lombard Exp $";
#endif

/*
 * em_panel.c--
 *    the event queue manager
 *
 * Copyright (c) 1992-3 Andrew K. Yu, Lawrence Berkeley Laboratory
 *      and University of California, Berkeley.
 * All rights reserved.
 */
#include <stdio.h>
#include <xview/xview.h>
#include <xview/frame.h>
#include <xview/canvas.h>
#include <xview/panel.h>
#include <xview/notice.h>
#include <xview/scrollbar.h>

#include "xv_proto.h"

/**********************************************************************
 *   EmPanel                                                         *
 **********************************************************************/

static Frame em_frame= (Frame)NULL;
static Panel_item em_evt_i;

#define NUM_ITEMS   11

static Panel_setting em_dismiss(Panel_item item, Event *event)
{
    xv_set(em_frame, XV_SHOW, FALSE, NULL);
}

static void em_reset(), em_goto();

static void InitEmPanel(Frame frame)
{
    Panel panel;
    extern int Mode_NCD;
    int frame_width=350;
    
    em_frame= (Frame)xv_create(frame, FRAME,
			       FRAME_LABEL, "Event Queue Manager",
			       XV_HEIGHT, 520, XV_WIDTH,frame_width,
			       NULL);
    panel = (Panel)xv_create(em_frame,PANEL, NULL);

    (void)xv_create(panel, PANEL_BUTTON,
		    XV_X, 50, XV_Y, 490,
		    PANEL_LABEL_STRING,	"Dismiss",
		    PANEL_NOTIFY_PROC,	em_dismiss,
		    NULL);
    (void)xv_create(panel, PANEL_BUTTON,
		    XV_X, 150, XV_Y, 490,
		    PANEL_LABEL_STRING,	"Reset",
		    PANEL_NOTIFY_PROC,	em_reset,
		    NULL);
    (void)xv_create(panel, PANEL_BUTTON,
		    XV_X, 250, XV_Y, 490,
		    PANEL_LABEL_STRING,	"Goto",
		    PANEL_NOTIFY_PROC,	em_goto,
		    NULL);
    if(Mode_NCD) {
	em_evt_i= (Panel_item)xv_create(panel, PANEL_LIST,
					XV_X, 20, XV_Y, 15,
					PANEL_LIST_TITLE, "Events",
					PANEL_LIST_DISPLAY_ROWS, 20,
					PANEL_LIST_WIDTH, frame_width-50,
					PANEL_CHOOSE_ONE, TRUE,
					NULL);
    }else {
	em_evt_i= (Panel_item)xv_create(panel, PANEL_LIST,
					XV_X, 20, XV_Y, 15,
					PANEL_LIST_TITLE, "Events",
					PANEL_LIST_DISPLAY_ROWS, 23,
					PANEL_LIST_WIDTH, 0,
					PANEL_CHOOSE_ONE, TRUE,
					NULL);
    }    
}

static void update_em_evt_list()
{
    EqEvent *cur= getCurrentEvt();
#if 0
    EqEvent *evt= getEventHead();
#endif
    EqEvent *evt= getEventTail();
    int idx= 0;
    char buf[1000];

    while (evt) {
	if (evt->evt && evt->evt->next!=NULL) {
	    sprintf(buf,"%s ...", evt->evt->name);
	}else {
	    sprintf(buf,"%s", evt->evt->name);
	}
#if 0
	if (evt==cur) {
	    xv_set(em_evt_i, PANEL_LIST_INSERT, idx,
		   PANEL_LIST_STRING, idx, buf,
		   PANEL_LIST_SELECT, idx, TRUE,
		   NULL);
	}else {
	    xv_set(em_evt_i, PANEL_LIST_INSERT, idx,
		   PANEL_LIST_STRING, idx, buf,
		   PANEL_LIST_SELECT, idx, FALSE,
		   NULL);
	}
	evt=evt->next;
#endif
	if (evt==cur) {
	    xv_set(em_evt_i, PANEL_LIST_INSERT, 0,
		   PANEL_LIST_STRING, 0, buf,
		   PANEL_LIST_SELECT, 0, TRUE,
		   NULL);
	}else {
	    xv_set(em_evt_i, PANEL_LIST_INSERT, 0,
		   PANEL_LIST_STRING, 0, buf,
		   PANEL_LIST_SELECT, 0, FALSE,
		   NULL);
	}
	evt=evt->prev;
	idx++;
    }

    xv_set(em_evt_i, XV_KEY_DATA, NUM_ITEMS, idx, NULL);
}

/**********************************************************************
 *                                                                    *
 **********************************************************************/

static void em_reset()
{
    int num= (int)xv_get(em_evt_i, XV_KEY_DATA, NUM_ITEMS);
    if(num>0) {
	xv_set(em_evt_i, PANEL_LIST_DELETE_ROWS, 0, num, NULL);
    }
    update_em_evt_list();
}

static void em_goto()
{
    EqEvent *evt= getEventHead();
    int idx= 0;

    while (evt) {
	if (xv_get(em_evt_i, PANEL_LIST_SELECTED, idx)==TRUE)
	    break;
	evt=evt->next;
	idx++;
    }
    if(evt)goto_specified_event(evt);
}

/**********************************************************************
 *                                                                    *
 **********************************************************************/

void handle_eqevtmgr_event()
{
    if(!em_frame)InitEmPanel(tracesFrame);
    xv_set(em_frame, XV_SHOW, TRUE, NULL);
    update_em_evt_list();
}

