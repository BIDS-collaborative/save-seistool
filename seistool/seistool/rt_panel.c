#ifndef lint
static char id[] = "$Id: rt_panel.c,v 1.4 2013/02/28 21:24:58 lombard Exp $";
#endif

/*
 * rt_panel.c--
 *    rotations panel
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
#include "mathlib.h"
#include "proto.h"
#include "xv_proto.h"

Frame rt_frame= (Frame)NULL;

Dial *az_dial;
static Panel_item theta_txt, rt_but0, rt_but1, rt_but2, rt_but3;
static Panel_item rt_but4, rt_but5, rt_but6, rt_but7;
Menu rot_pan_menu;
extern Trace **traces;
extern int do_z_win_only;


static float rt_inc_val=10.0;

/* set event azimuth marker and turn it on */
void set_evt_azi(float az)
{
    az_dial->show_marked=1;
    az_dial->marked_value=az;
}

/* set event azimuth marker and turn it off */
void unset_evt_azi(float az)
{
    az_dial->show_marked=0;
    az_dial->marked_value=az;
}


/* manual rotation via text window of rt_panel */
static void theta_txt_notify(Panel_item item, Event *event)
{
    char *thetaS=(char *)xv_get(item,PANEL_VALUE);
    float theta= atof(thetaS);

    SetDialVal(az_dial, theta);
    /* redraw */
    rt_setflag(1);
    rt_setAngle(theta);
    RedrawZoomWindow("theta_txt_notify");
}

void open_rotate_panel()
{
    extern int LastTrack;
    if(LastTrack<0)
	return;
    if(!rt_frame)
	InitRTPanel(tracesFrame);
    if (Mode_triplet==0)
	StartTripletMode();	
    Mode_rotate=1;
    Rot_ZoomContentChanged();
    xv_set(rt_frame, XV_SHOW, TRUE, NULL);
}

void close_rotate_panel()
{
    Mode_rotate=0;
    /*    EndTripletMode();	    /* do Mode_rotate=0 first */
    xv_set(rt_frame, XV_SHOW, FALSE, NULL);
}

static void PermanentRotate()
     /* NOT READY AT ALL !!! */
{
    char *azS=(char *)xv_get(theta_txt,PANEL_VALUE);
    float az= atof(azS);
    /*    RotateTrace();	-- rotate the whole trace */
    rt_setAngle(0.0);
    xv_set(theta_txt, PANEL_VALUE, "0.0", NULL);
}

/* set the event azimuth */
void rt_setAzimuth()
{
    Trace *trc=traces[lowZTrkIndex];
    float rlat,rlon,del,azi,bazi;
    float lat,lon;
     
    if (trc->wave->info.event_latitude != (float) F4FLAG) {
     	lat=trc->wave->info.event_latitude;
     	lon=trc->wave->info.event_longitude;
     	rlat=trc->wave->info.latitude;
     	rlon=trc->wave->info.longitude;
     	if (rlat!=0.0 && rlon !=0.0) {
     	    azimuth(lat,lon,rlat,rlon,&del,&azi,&bazi);
     	    set_evt_azi(bazi);
     	}
    }
    return;
}


void rt_setNewTheta(float theta)
{
    char buf[30];

    while (theta >= 360.0)
	theta -= 360.0;
    while (theta < 0)
	theta += 360.0;
    
    SetDialVal(az_dial, theta);
    sprintf(buf,"%.2f",theta);
    xv_set(theta_txt, PANEL_VALUE, buf, NULL);
    refresh_dial(az_dial);

    rt_setAngle(theta);
    RedrawZoomWindow("updateTheta");
}

/* Set the triplet back to its natural theta */
static void ResetRotate()
{
    rt_setNewTheta(traces[lowZTrkIndex]->trip->sta_rotation);
    rt_setflag(0);
    RedrawZoomWindow("resetRotate");
}


static void incRotate()
{
    char *thetaS=(char *)xv_get(theta_txt,PANEL_VALUE);
    float theta= atof(thetaS)+rt_inc_val;
    rt_setflag(1);
    rt_setNewTheta(theta);
}

static void decRotate()
{
    char *thetaS=(char *)xv_get(theta_txt,PANEL_VALUE);
    float theta= atof(thetaS)-rt_inc_val;
    rt_setflag(1);
    rt_setNewTheta(theta);
}
  
static void inc_180()
{
    char *thetaS=(char *)xv_get(theta_txt,PANEL_VALUE);
    float theta= atof(thetaS)+180;
    rt_setflag(1);
    rt_setNewTheta(theta);
}

/* update rotate dial with new dial value; used when clicking on dial */
static void rt_dial_rot_update(float dial_val)
{
    rt_setNewTheta(dial_val);
    rt_setflag(1);
}

static void changeinc()
{
    if (rt_inc_val==10.0) {
	rt_inc_val=1.0;
	xv_set(rt_but5,PANEL_LABEL_STRING,	"Fine Increment",NULL);
    } else {
	rt_inc_val=10.0;
	xv_set(rt_but5,PANEL_LABEL_STRING,	"Coarse Increment",NULL);
    }
}
    
void InitRTPanel(Frame frame)
{    
    Canvas cvs;
    Panel panel;

    rt_frame= (Frame)xv_create(frame, FRAME,
			       FRAME_LABEL, "Rotate",
			       XV_HEIGHT, 180, XV_WIDTH,300,
			       NULL);
    panel = (Panel)xv_create(rt_frame,PANEL, NULL);

    az_dial= (Dial *)newDial(20, 30, 120, 120, DIAL_ROT_TYPE);
    az_dial->scale=1.0;
    az_dial->value=90.0;
    InitDial(rt_frame, az_dial);
    SetDialChangeFunc(az_dial, 0, rt_dial_rot_update);

    (void)xv_create(panel, PANEL_MESSAGE,
		    XV_X, 30, XV_Y, 10,			  
		    PANEL_LABEL_STRING, "Azimuth (deg)",
		    PANEL_LABEL_BOLD, TRUE,
		    NULL);
    theta_txt=(Panel_item)xv_create(panel, PANEL_TEXT,
				 XV_X, 50, XV_Y, 150,			  
				 PANEL_VALUE_DISPLAY_WIDTH, 70,			  
				 PANEL_VALUE, "0.0",
				 PANEL_NOTIFY_PROC, theta_txt_notify,
				 NULL);
	
    rt_but0= (Panel_item)xv_create(panel, PANEL_BUTTON,
				   XV_X, 160, XV_Y, 20,
				   PANEL_LABEL_STRING,	"Quit",
				   PANEL_NOTIFY_PROC,	close_rotate_panel,
				   NULL);

    rt_but1= (Panel_item)xv_create(panel, PANEL_BUTTON,
				   XV_X, 160, XV_Y, 50,
				   PANEL_LABEL_STRING,	"Permanent",
				   PANEL_INACTIVE, TRUE,
				   PANEL_NOTIFY_PROC,	PermanentRotate,
				   NULL);

    rt_but2= (Panel_item)xv_create(panel, PANEL_BUTTON,
				   XV_X, 160, XV_Y, 80,
				   PANEL_LABEL_STRING,	"Reset",
				   PANEL_NOTIFY_PROC,	ResetRotate,
				   NULL);

    rt_but3= (Panel_item)xv_create(panel, PANEL_BUTTON,
				   XV_X, 150, XV_Y, 125,
				   PANEL_LABEL_STRING,	"+",
				   PANEL_NOTIFY_PROC,	incRotate,
				   NULL);

    rt_but4= (Panel_item)xv_create(panel, PANEL_BUTTON,
				   XV_X, 150, XV_Y, 150,
				   PANEL_LABEL_STRING,	"-",
				   PANEL_NOTIFY_PROC,	decRotate,
				   NULL);

    rt_but5= (Panel_item)xv_create(panel, PANEL_BUTTON,
				   XV_X, 180, XV_Y, 125,
				   PANEL_LABEL_STRING,	"Coarse Increment",
				   PANEL_NOTIFY_PROC,	changeinc,
				   NULL);

    rot_pan_menu=(Menu)xv_create(NULL,MENU,
				 MENU_STRINGS,"rotate rms zoom",
				 "rotate event zoom", NULL,
				 MENU_NOTIFY_PROC,autor_events ,
				 NULL);

    rt_but6= (Panel_item)xv_create(panel, PANEL_BUTTON,
				   XV_X, 150, XV_Y, 100,
				   PANEL_LABEL_STRING,	"Auto Rotate",
				   PANEL_ITEM_MENU,	rot_pan_menu,
				   NULL);

    rt_but7= (Panel_item)xv_create(panel, PANEL_BUTTON,
				   XV_X, 180, XV_Y, 150,
				   PANEL_LABEL_STRING,	"+ 180",
				   PANEL_NOTIFY_PROC,	inc_180,
				   NULL);

}
	
void rt_activatePanel()
{
    az_dial->inactive= 0;
    xv_set(theta_txt, PANEL_INACTIVE, FALSE, NULL);
    xv_set(rt_but0, PANEL_INACTIVE, FALSE, NULL);    /* quit */
    /*    xv_set(rt_but1, PANEL_INACTIVE, FALSE, NULL); -- perm. inactive */
    xv_set(rt_but2, PANEL_INACTIVE, FALSE, NULL);    /* reset */
    xv_set(rt_but3, PANEL_INACTIVE, FALSE, NULL);    /* incRotate */
    xv_set(rt_but4, PANEL_INACTIVE, FALSE, NULL);    /* decRotate */
    xv_set(rt_but5, PANEL_INACTIVE, FALSE, NULL);    /* changeinc */
    xv_set(rt_but6, PANEL_INACTIVE, FALSE, NULL);    /* Auto Rotate */
    xv_set(rt_but7, PANEL_INACTIVE, FALSE, NULL);    /* inc_180 */
}

void rt_inactivatePanel()
{
    az_dial->inactive= 1;
    xv_set(theta_txt, PANEL_INACTIVE, TRUE, NULL);
    /*    xv_set(rt_but0, PANEL_INACTIVE, TRUE, NULL); -- can always quit! */
    /*    xv_set(rt_but1, PANEL_INACTIVE, TRUE, NULL); -- already inactive */
    xv_set(rt_but2, PANEL_INACTIVE, TRUE, NULL);
    xv_set(rt_but3, PANEL_INACTIVE, TRUE, NULL);
    xv_set(rt_but4, PANEL_INACTIVE, TRUE, NULL);
    xv_set(rt_but5, PANEL_INACTIVE, TRUE, NULL);
    xv_set(rt_but6, PANEL_INACTIVE, TRUE, NULL);
    xv_set(rt_but7, PANEL_INACTIVE, TRUE, NULL);
}
