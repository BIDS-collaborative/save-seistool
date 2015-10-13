#ifndef lint
static char id[] = "$Id: tt_panel.c,v 1.2 2013/02/28 21:24:56 lombard Exp $";
#endif

/*
 * tt_panel.c--
 *    travel time curves panel
 *
 * Copyright (c) 1992-3 Andrew K. Yu, Lawrence Berkeley Laboratory
 *      and University of California, Berkeley.
 * All rights reserved.
 */
#include <math.h>
#include <stdlib.h>
#include <xview/xview.h>
#include <xview/frame.h>
#include <xview/canvas.h>
#include <xview/panel.h>
#include "proto.h"
#include "xv_proto.h"


#define DEPTH_DIAL  0
#define DELTA_DIAL  1

Frame tt_frame= (Frame)NULL;
Panel_item tt_phs_list;
Panel_item tt_dep_txt, tt_del_txt;
int tt_button_picking= 0; 

static Dial *dial, *dial2;
static Panel_item lat_txt, lon_txt, pick_but, ot_txt;

/* internal function prototypes */
static Panel_setting tt_dep_txt_notify(Panel_item item, Event *event);
static Panel_setting tt_del_txt_notify(Panel_item item, Event *event);
static Panel_setting ot_txt_notify(Panel_item item, Event *event);
static Panel_setting lon_txt_notify(Panel_item item, Event *event);
static void which(Panel_item item, char *string, int client_data, 
		  Panel_list_op op, Event *event);


void tt_SetStartDepth(float dep)
{
  if (dep>=0 && dep <=2000) {
    SetDialVal(dial,dep);
  }
}

void tt_SetStartDelta(float del)
{
    SetDialVal(dial2,del);
}


static Panel_setting tt_dep_txt_notify(Panel_item item, Event *event)
{
    char *depthS=(char *)xv_get(item,PANEL_VALUE);
    float depth= atof(depthS);

    if (depth<0) depth =0;
    if (depth>2000) depth =2000;
    SetDialVal(dial, depth);
    tt_ChangeDepth(depth);
}

static Panel_setting tt_del_txt_notify(Panel_item item, Event *event)
{
    char *delS=(char *)xv_get(item,PANEL_VALUE);
    float del= atof(delS);

    if (del<0) del=0;
    if (del>180) del=180;
    SetDialVal(dial2, del);
    tt_ChangeDelta(del);
}

static Panel_setting ot_txt_notify(Panel_item item, Event *event)
{
    char *otS=(char *)xv_get(item,PANEL_VALUE);

    tt_ChangeOT(otS);
    RedrawZoomWindow("ot_txt_notify");
}

static Panel_setting lon_txt_notify(Panel_item item, Event *event)
{
    char *lonS=(char *)xv_get(item,PANEL_VALUE);
    char *latS=(char *)xv_get(lat_txt, PANEL_VALUE);
    float lat= atof(latS);
    float lon= atof(lonS);

    tt_ChangeEvtLoc(lat,lon);
}

void tt_updateOTitem(STI_TIME it)
{
    STE_TIME et = sti_to_ste(it);
    char buf[30];
    float sec = (float)et.second + (float)et.usec / USECS_PER_SEC;
    
    sprintf(buf,"%d:%d:%.2f",et.hour,et.minute,sec);
    xv_set(ot_txt, PANEL_VALUE, buf, NULL);
}

void tt_updateDelTxtitem(float del)
{
    char buf[30];
    sprintf(buf,"%.2f",del);
    xv_set(tt_del_txt, PANEL_VALUE, buf, NULL);
}

/* this is an unfortunate consequence of having 2 event procs handling the
 * same canvas. Since we mapped the left mouse button twice, the user has
 * to decide which one to use:
 */
void tt_togglePicking()
{
    if(tt_button_picking) {
	/* switch back to trvltime event proc */
	SwitchTTZevtProc();
	tt_button_picking= 0;
	xv_set(pick_but, PANEL_LABEL_STRING, "To Pick ", NULL);
    }else {
	/* switch back to original event proc */
	RestoreZevtProc();
	tt_button_picking= 1;
	xv_set(pick_but, PANEL_LABEL_STRING, "TrvlTme", NULL);
    }
}

void InitTTPanel(Frame frame)
{    
    Panel panel;

    tt_frame= (Frame)xv_create(frame, FRAME,
	FRAME_LABEL, "Travel Times",
	XV_HEIGHT, 280, XV_WIDTH,450,
	NULL);
    panel = (Panel)xv_create(tt_frame,PANEL, NULL);
    xv_set(tt_frame, XV_SHOW, TRUE, NULL);

    dial= (Dial *)newDial(30, 50, 120, 120, DIAL_KNOB_TYPE);
    SetDialLabel(dial, "0", "350", "700");
    dial->scale=(float)7/3;
    InitDial(tt_frame, dial);
    SetDialChangeFunc(dial, DEPTH_DIAL, tt_ChangeDepth);

    (void)xv_create(panel, PANEL_MESSAGE,
	XV_X, 30, XV_Y, 35,			  
	PANEL_LABEL_STRING, "Depth (km)",
	PANEL_LABEL_BOLD, TRUE,
	NULL);
    tt_dep_txt=(Panel_item)xv_create(panel, PANEL_TEXT,
	XV_X, 60, XV_Y, 180,			  
	PANEL_VALUE_DISPLAY_WIDTH, 60,			  
	PANEL_NOTIFY_PROC, tt_dep_txt_notify,
	PANEL_VALUE, NULL,
	NULL);
	
    dial2= (Dial *)newDial(180, 50, 120, 120, DIAL_KNOB_TYPE);
    SetDialLabel(dial2, "0", "90", "180");
    dial2->scale=(float)18/30;
    InitDial(tt_frame, dial2);
    SetDialChangeFunc(dial2, DELTA_DIAL, tt_ChangeDelta);

    (void)xv_create(panel, PANEL_MESSAGE,
	XV_X, 180, XV_Y, 35,			  
	PANEL_LABEL_STRING, "Delta (deg)",
	PANEL_LABEL_BOLD, TRUE,
	NULL);
    tt_del_txt=(Panel_item)xv_create(panel, PANEL_TEXT,
	XV_X, 210, XV_Y, 180,			  
	PANEL_VALUE_DISPLAY_WIDTH, 60,			  
	PANEL_NOTIFY_PROC, tt_del_txt_notify,
	PANEL_VALUE, NULL,
	NULL);

    ot_txt=(Panel_item)xv_create(panel, PANEL_TEXT,
	XV_X, 30, XV_Y, 220,			  
	PANEL_LABEL_STRING, "Origin Time:",			 
	PANEL_NOTIFY_PROC, ot_txt_notify,
	PANEL_VALUE_DISPLAY_WIDTH, 150,			  
	PANEL_VALUE, NULL,
	NULL);
    lat_txt=(Panel_item)xv_create(panel, PANEL_TEXT,
	XV_X, 30, XV_Y, 240,			  
	PANEL_LABEL_STRING, "Event lat:",
	PANEL_VALUE_DISPLAY_WIDTH, 150,			  
	PANEL_VALUE, NULL,
	NULL);
    lon_txt=(Panel_item)xv_create(panel, PANEL_TEXT,
	XV_X, 30, XV_Y, 260,			  
	PANEL_LABEL_STRING, "Event long:",
	PANEL_NOTIFY_PROC, lon_txt_notify,
	PANEL_VALUE_DISPLAY_WIDTH, 150,			  
	PANEL_VALUE, NULL,
	NULL);

    (void)xv_create(panel, PANEL_BUTTON,
	XV_X, 20, XV_Y, 5,
	PANEL_LABEL_STRING,	"Quit",
	PANEL_NOTIFY_PROC,	ExitTTMode,
	NULL);
    (void)xv_create(panel, PANEL_BUTTON,
	XV_X, 80, XV_Y, 5,
	PANEL_LABEL_STRING,	"All Ph",
	PANEL_NOTIFY_PROC,	AllPhases,
	NULL);
    (void)xv_create(panel, PANEL_BUTTON,
	XV_X, 150, XV_Y, 5,
	PANEL_LABEL_STRING,	"Basic Ph",
	PANEL_NOTIFY_PROC,	BasicPhases,
	NULL);
    pick_but=(Panel_item)xv_create(panel, PANEL_BUTTON,
	XV_X, 240, XV_Y, 5,
	PANEL_LABEL_STRING,	"Pick",
	PANEL_NOTIFY_PROC,	tt_togglePicking,
	NULL);

    tt_phs_list= (Panel_item)xv_create(panel, PANEL_LIST,
	XV_X, 330, XV_Y, 30,	    
	PANEL_LIST_TITLE, "Phases",			  
	PANEL_LIST_DISPLAY_ROWS, 11,
	PANEL_LIST_WIDTH, 80,
	PANEL_CHOOSE_ONE, FALSE,	    
	XV_KEY_DATA, 33, 0,			  
	PANEL_NOTIFY_PROC,	which,
	NULL);
}

static void which(Panel_item item, char *string, int client_data, 
		  Panel_list_op op, Event *event)
{
    if(op) {
	ShowPhase(client_data);
    }else {
	UnshowPhase(client_data);
    }
}
	
