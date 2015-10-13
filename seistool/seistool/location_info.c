#ifndef lint
static char id[] = "$Id: location_info.c,v 1.2 2013/02/28 21:24:59 lombard Exp $";
#endif

#include <math.h>
#include <xview/xview.h>
#include <xview/frame.h>
#include <xview/canvas.h>
#include <xview/panel.h>

#include "mathlib.h"
#include "proto.h"
#include "xv_proto.h"

extern Trace **traces;
extern int LastTrack;
extern char *default_coord_file;
static Frame LLoc_frame;
static Panel_item lat_txt,lon_txt,depth_txt;

void get_location_info()
{
    xv_set(LLoc_frame,XV_SHOW,TRUE,NULL);
}

void Exit_lloc()
{
    xv_set(LLoc_frame,XV_SHOW,FALSE,NULL);
}

static Panel_setting Storeloc() 
{
    char *lonS=(char *)xv_get(lon_txt, PANEL_VALUE);
    char *latS=(char *)xv_get(lat_txt, PANEL_VALUE);
    char *depS=(char *)xv_get(depth_txt, PANEL_VALUE);
    /*  char *time=(char *)xv_get(time_txt,PANEL_VALUE); */

    float lat= atof(latS);
    float lon= atof(lonS);
    float dep= atof(depS);
    float rlat,rlon,del,azi,bazi;
    int i;
    Trace *trc;

    if (strcmp(latS,"") && strcmp(lonS,"")) {
	/* load station locations */
	GetResponses(NULL);

	for (i=0;i<=LastTrack; i++) {
	    trc=traces[i];
	    trc->wave->info.event_latitude=lat;
	    trc->wave->info.event_longitude=lon;
	    if (strcmp(depS,""))  trc->wave->info.event_depth=dep;
	    rlat=trc->wave->info.latitude;
	    rlon=trc->wave->info.longitude;
	    if (rlat!=0.0 && rlon !=0.0) {
		azimuth(lat,lon,rlat,rlon,&del,&azi,&bazi);
		trc->wave->info.event_delta=del;
		trc->wave->info.event_azimuth = bazi;
	    }
	}
    }
    xv_set(LLoc_frame,XV_SHOW,FALSE,NULL); 
    activate_sort_event( 1 );
    rt_setAzimuth();
    if (Mode_rotate) { /* show the event azimuth pointer */
	Rot_ZoomContentChanged();
    }
}

  
void InitLoadLocPanel(Frame frame)
{
    Panel panel;

    LLoc_frame=(Frame) xv_create(frame,FRAME,FRAME_LABEL,"Event Location",
				 XV_HEIGHT,100,XV_WIDTH,300,NULL);
    panel=(Panel) xv_create(LLoc_frame,PANEL,NULL);
    xv_set(LLoc_frame,XV_SHOW,FALSE,NULL);

    lat_txt=(Panel_item)xv_create(panel, PANEL_TEXT,
				  XV_X, 30, XV_Y, 20,			  
				  PANEL_LABEL_STRING, "Event lat:",
				  PANEL_VALUE_DISPLAY_WIDTH, 150,
				  PANEL_VALUE, NULL,
				  NULL);
    lon_txt=(Panel_item)xv_create(panel, PANEL_TEXT,
				  XV_X, 30, XV_Y, 40,			  
				  PANEL_LABEL_STRING, "Event long:",
				  PANEL_VALUE_DISPLAY_WIDTH, 150,	
				  PANEL_VALUE, NULL,
				  NULL);
    depth_txt=(Panel_item)xv_create(panel, PANEL_TEXT,
				    XV_X, 30, XV_Y, 60,			  
				    PANEL_LABEL_STRING, "Event depth:",
				    PANEL_VALUE_DISPLAY_WIDTH, 150,	
				    PANEL_VALUE, NULL,
				    NULL);
    (void)xv_create(panel, PANEL_BUTTON, XV_X, 30, XV_Y,80,
		    PANEL_NOTIFY_PROC, Storeloc,
		    PANEL_LABEL_STRING, "Store",
		    NULL);
    (void)xv_create(panel, PANEL_BUTTON, XV_X, 130, XV_Y,80,
		    PANEL_NOTIFY_PROC, Exit_lloc,
		    PANEL_LABEL_STRING, "Cancel",
		    NULL);

}
