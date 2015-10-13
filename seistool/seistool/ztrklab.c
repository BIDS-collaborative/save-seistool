#ifndef lint
static char id[] = "$Id: ztrklab.c,v 1.4 2013/02/28 21:24:55 lombard Exp $";
#endif

/*
 * ztrklab.c--
 *    manages the trace "label" boxes in zoom window
 *
 * Copyright (c) 1992-3 Andrew K. Yu, Lawrence Berkeley Laboratory
 *      and University of California, Berkeley.
 * All rights reserved.
 */
#include <xview/xview.h>
#include <xview/frame.h>
#include <xview/canvas.h>
#include <xview/panel.h>
#include <xview/svrimage.h>
#include <xview/xv_xrect.h>

#include "proto.h"
#include "xv_proto.h"

/* the following are bitmaps created for button images */
#include "bitmap/compress.bmp"
#include "bitmap/expand.bmp"
#include "bitmap/raise.bmp"
#include "bitmap/lower.bmp"
#include "bitmap/center.bmp"
#include "bitmap/zero.bmp"

/**********************************************************************
 *   Variables & prototypes                                           *
 **********************************************************************/

static Canvas *ZLab_canvas=NULL;
static Canvas *ZLab_canvas2;
static Window *ZLab_win;
static Panel *ZLab_panel;

extern Trace **traces;
extern int LastTrack;

/* internal function prototypes */
static int Zlab_notify_raise(Panel_item item, Event *event);
static int Zlab_notify_lower(Panel_item item, Event *event);
static int Zlab_notify_center(Panel_item item, Event *event);
static int Zlab_notify_zero(Panel_item item, Event *event);
static int Zlab_notify_compress(Panel_item item, Event *event);
static int Zlab_notify_expand(Panel_item item, Event *event);
static void Zlab_event_proc(Xv_Window window, Event *event);
static void Zlab_repaint_proc(Canvas canvas, Xv_Window paint_window, 
			      Display *dpy, Window xwin, Xv_xrectlist xrects);


/**********************************************************************
 *   Zoom Track Labels                                                *
 **********************************************************************/

void InitZTrkLab(Frame frame)
{
    Server_image raise, lower, compress, expand, center, zero;
    int i, y;
    int show= ZTrkHeight>=105;

    ZLab_win= (Window *)Malloc(sizeof(Window)*NumZTracks);
    ZLab_canvas= (Canvas *)Malloc(sizeof(Canvas)*NumZTracks);
    ZLab_canvas2= (Canvas *)Malloc(sizeof(Canvas)*NumZTracks);
    ZLab_panel= (Panel *)Malloc(sizeof(Panel)*NumZTracks);

    if (ZLab_win==NULL || ZLab_canvas==NULL || ZLab_panel==NULL ||
	ZLab_canvas2==NULL )
	return;	    /* not enuff memory */
    /* creates button images */
    raise = (Server_image)xv_create(NULL, SERVER_IMAGE,
				    XV_WIDTH, raise_width,
				    XV_HEIGHT, raise_height,
				    SERVER_IMAGE_DEPTH, 1,
				    SERVER_IMAGE_X_BITS, raise_bits,
				    NULL);
    lower = (Server_image)xv_create(NULL, SERVER_IMAGE,
				    XV_WIDTH, lower_width,
				    XV_HEIGHT, lower_height,
				    SERVER_IMAGE_DEPTH, 1,
				    SERVER_IMAGE_X_BITS, lower_bits,
				    NULL);
    center = (Server_image)xv_create(NULL, SERVER_IMAGE,
				     XV_WIDTH, center_width,
				     XV_HEIGHT, center_height,
				     SERVER_IMAGE_DEPTH, 1,
				     SERVER_IMAGE_X_BITS, center_bits,
				     NULL);
    zero = (Server_image)xv_create(NULL, SERVER_IMAGE,
				   XV_WIDTH, zero_width,
				   XV_HEIGHT, zero_height,
				   SERVER_IMAGE_DEPTH, 1,
				   SERVER_IMAGE_X_BITS, zero_bits,
				   NULL);
    expand = (Server_image)xv_create(NULL, SERVER_IMAGE,
				     XV_WIDTH, expand_width,
				     XV_HEIGHT, expand_height,
				     SERVER_IMAGE_DEPTH, 1,
				     SERVER_IMAGE_X_BITS, expand_bits,
				     NULL);
    compress = (Server_image)xv_create(NULL, SERVER_IMAGE,
				       XV_WIDTH, compress_width,
				       XV_HEIGHT, compress_height,
				       SERVER_IMAGE_DEPTH, 1,
				       SERVER_IMAGE_X_BITS, compress_bits,
				       NULL);

    /* set up the label canvas */
    y= (Mode_align && Mode_ZDisplayTScale)? ZTSHEIGHT : 0;
    for(i=0; i < NumZTracks; i++) {
	Panel panel;
	Canvas canvas;
	Panel_item item0, item1, item2;
	int ht;

	ht= (i!=NumZTracks-1)? ZTrkHeight :
	    ZCvsHeight-ZTrkHeight*(NumZTracks-1);
	canvas = (Canvas)xv_create(frame, CANVAS,
				   XV_X, 0, XV_Y, y,		   
				   XV_WIDTH, ZTrkLabWidth,
				   XV_HEIGHT, ht,
				   /* CANVAS_WIDTH, ZTrkLabWidth,
				      CANVAS_HEIGHT, ZTrkHeight,
				      CANVAS_AUTO_EXPAND, FALSE,
				      CANVAS_AUTO_SHRINK, FALSE, */
				   CANVAS_X_PAINT_WINDOW, TRUE,
				   CANVAS_REPAINT_PROC, Zlab_repaint_proc,
				   XV_KEY_DATA, TRACK_INDEX, i,	/* for identification */
				   NULL);
	ZLab_canvas[i]= canvas;       

	panel = (Panel)xv_create(canvas, PANEL,
				 XV_X, 1,		 
				 XV_HEIGHT, ht-1,
				 XV_WIDTH, ZTrkLabWidth,
				 /* PANEL_LAYOUT, PANEL_HORIZONTAL,
				 PANEL_ITEM_X_GAP, 3,
				 PANEL_ITEM_Y_GAP, 3, */
				 NULL);
	ZLab_panel[i]= panel;

	/* raise trace */
	item0= (Panel_item)xv_create(panel, PANEL_BUTTON,
				     XV_X, 5, XV_Y, ZTrkHeight - 60,
				     PANEL_LABEL_IMAGE,	raise,
				     PANEL_NOTIFY_PROC,	Zlab_notify_raise,
				     XV_SHOW, (show? TRUE:FALSE),
				     XV_KEY_DATA, TRACK_INDEX, i,
				     NULL);

	/* lower trace */
	item1= (Panel_item)xv_create(panel, PANEL_BUTTON,
				     XV_X, 45, XV_Y, ZTrkHeight - 60,
				     PANEL_LABEL_IMAGE,	lower,
				     PANEL_NOTIFY_PROC,	Zlab_notify_lower,
				     XV_SHOW, (show? TRUE:FALSE),
				     XV_KEY_DATA, TRACK_INDEX, i,
				     NULL);

	/* center trace */
	item2= (Panel_item)xv_create(panel, PANEL_BUTTON,
				     XV_X, 65, XV_Y, ZTrkHeight - 60,
				     PANEL_LABEL_IMAGE,	center,
				     PANEL_NOTIFY_PROC,	Zlab_notify_center,
				     XV_SHOW, (show? TRUE:FALSE),
				     XV_KEY_DATA, TRACK_INDEX, i,
				     NULL);

	/* expand vertically */
	(void)xv_create(panel, PANEL_BUTTON,
			XV_X, 5, XV_Y, ZTrkHeight - 30,
			PANEL_LABEL_IMAGE,	expand,
			PANEL_NOTIFY_PROC,	Zlab_notify_expand,
			PANEL_NEXT_ROW, -1,
			XV_SHOW, (show? TRUE:FALSE),
			XV_KEY_DATA, TRACK_INDEX, i,
			NULL);

	/* compress vertically */
	(void)xv_create(panel, PANEL_BUTTON,
			XV_X, 35, XV_Y, ZTrkHeight - 30,
			PANEL_LABEL_IMAGE,	compress,
			PANEL_NOTIFY_PROC,	Zlab_notify_compress,
			XV_SHOW, (show? TRUE:FALSE),
			XV_KEY_DATA, TRACK_INDEX, i,
			NULL);

	/* zero bias and unity gain for trace */
	(void)xv_create(panel, PANEL_BUTTON,
			XV_X, 65, XV_Y, ZTrkHeight - 30,
			PANEL_LABEL_IMAGE,	zero,
			PANEL_NOTIFY_PROC,	Zlab_notify_zero,
			XV_SHOW, (show? TRUE:FALSE),
			XV_KEY_DATA, TRACK_INDEX, i,
			NULL);

	canvas = (Canvas)xv_create(canvas, CANVAS,
				   XV_X, 5, XV_Y, 10,		   
				   XV_WIDTH, ZTrkLabWidth-10,
				   XV_HEIGHT, 30, /* ZTrkHeight,*/
				   CANVAS_X_PAINT_WINDOW, TRUE,
				   CANVAS_REPAINT_PROC, Zlab_repaint_proc,
				   OPENWIN_SHOW_BORDERS, FALSE,
				   XV_KEY_DATA, TRACK_INDEX, i,	/* for identification */
				   NULL);
	xv_set(canvas_paint_window(canvas),
	       WIN_EVENT_PROC, Zlab_event_proc,
	       WIN_CONSUME_EVENTS,
	       LOC_DRAG, LOC_MOVE, WIN_ASCII_EVENTS, WIN_MOUSE_BUTTONS,
	       LOC_WINENTER, LOC_WINEXIT,
	       NULL,
	       XV_KEY_DATA, TRACK_INDEX, i,	/* for identification */
	       NULL);
	ZLab_canvas2[i]= canvas;       
	ZLab_win[i]= (Window)xv_get(canvas_paint_window(canvas),XV_XID);

	y+=ZTrkHeight;
    }
}

void DestroyZTrkLab()
{
    int i;

    if(ZLab_canvas==NULL)return;
    for(i=0; i < NumZTracks; i++) {
	xv_destroy_safe(ZLab_canvas[i]);
    }
    free(ZLab_win);
    free(ZLab_canvas);
    free(ZLab_panel);
}


/**********************************************************************
 *   Zoom Track Label Panel Button Events Notification                *
 **********************************************************************/

static int Zlab_notify_raise(Panel_item item, Event *event)
{
    int iztrk,i;
    extern void ztrk_LowerDC();

    iztrk= (int)xv_get(item, XV_KEY_DATA, TRACK_INDEX)+lowZTrkIndex;
    ztrk_LowerDC(iztrk);    /* lower DC to raise trace */
    RedrawZoomWindow("zlab_notify_raise");
    return XV_OK;
}

static int Zlab_notify_lower(Panel_item item, Event *event)
{
    int iztrk,i;
    extern void ztrk_RaiseDC();
    
    iztrk= (int)xv_get(item, XV_KEY_DATA, TRACK_INDEX)+lowZTrkIndex;
    ztrk_RaiseDC(iztrk);    /* raise DC to lower trace */
    RedrawZoomWindow("zlab_notify_lower");
    return XV_OK;
}

static int Zlab_notify_center(Panel_item item, Event *event)
{
    int iztrk,i;
    extern void ztrk_RaiseDC();
    
    iztrk= (int)xv_get(item, XV_KEY_DATA, TRACK_INDEX)+lowZTrkIndex;
    ztrk_CenterDC(iztrk);    /* adjust DC to center trace */
    RedrawZoomWindow("zlab_notify_center");
    return XV_OK;
}
    
static int Zlab_notify_zero(Panel_item item, Event *event)
{
    int iztrk,i;
    extern void ztrk_ZeroDC();
    
    iztrk= (int)xv_get(item, XV_KEY_DATA, TRACK_INDEX)+lowZTrkIndex;
    ztrk_ZeroDC(iztrk);    /* adjust DC to center trace */
    RedrawZoomWindow("zlab_notify_zero");
    return XV_OK;
}
    

static int Zlab_notify_compress(Panel_item item, Event *event)
{
    int iztrk;
    extern void CompressZTrk();
    
    iztrk= (int)xv_get(item, XV_KEY_DATA, TRACK_INDEX)+lowZTrkIndex;
    CompressZTrk(iztrk);
    RedrawZoomWindow("zlab_notify_compress");
    return XV_OK;
}

static int Zlab_notify_expand(Panel_item item, Event *event)
{
    int iztrk;
    extern void ExandZTrk();
    
    iztrk= (int)xv_get(item, XV_KEY_DATA, TRACK_INDEX)+lowZTrkIndex;
    ExpandZTrk(iztrk);
    RedrawZoomWindow("zlab_notify_expand");
    return XV_OK;
}

static void Zlab_event_proc(Xv_Window window, Event *event)
{
    int itrc;
    
    itrc= (int)xv_get(window, XV_KEY_DATA, TRACK_INDEX)+lowZTrkIndex;
    if (itrc>LastTrack || traces[itrc]->wave==NULL)
	return;	/* ignore */
    switch(event_action(event)) {
    case ACTION_MENU:
    case MS_RIGHT: {
	if(event_is_up(event)) {
	    open_info_win();
	    DisplayTraceInfo(itrc);
	}
	break;
    }
    default:
	break;
    }
}

/**********************************************************************
 *   Repaint & resize Zoom Track Labels                               *
 **********************************************************************/

static void Zlab_repaint_proc(Canvas canvas, Xv_Window paint_window, 
			      Display *dpy, Window xwin, Xv_xrectlist xrects)
{
    int iztrk;

    iztrk= (int)xv_get(canvas, XV_KEY_DATA, TRACK_INDEX);
    UpdateZlab(iztrk);
}

/* do it by ResizeZTRk-- so that ZTrkHeight is set properly */
void ResizeZlab(int shrink)
{
    int iztrk, y;
    int show= ZTrkHeight>=105;
    
    y= (Mode_align && Mode_ZDisplayTScale)? ZTSHEIGHT : 0;
    for(iztrk=0; iztrk < NumZTracks; iztrk++) {
	Panel_item item0, item1, item2, item3, item4, item5;
	if (ZLab_canvas[iztrk]) {
	    int ht= (iztrk!=NumZTracks-1)? ZTrkHeight :
		ZCvsHeight-ZTrkHeight*(NumZTracks-1);
	    xv_set(ZLab_canvas[iztrk],
		   XV_Y, y, /* iztrk*ZTrkHeight */
		   XV_WIDTH, ZTrkLabWidth,
		   XV_HEIGHT, ht,
		   /*		   CANVAS_WIDTH, ZTrkLabWidth, 
				   CANVAS_HEIGHT, ZTrkHeight, */
		   NULL);
	    item0= (Panel_item)xv_get(ZLab_panel[iztrk],PANEL_FIRST_ITEM);
	    item1= (Panel_item)xv_get(item0,PANEL_NEXT_ITEM);
	    item2= (Panel_item)xv_get(item1,PANEL_NEXT_ITEM);
	    item3= (Panel_item)xv_get(item2,PANEL_NEXT_ITEM);
	    item4= (Panel_item)xv_get(item3,PANEL_NEXT_ITEM);
	    item5= (Panel_item)xv_get(item4,PANEL_NEXT_ITEM);
	    if (shrink) {
		xv_set(item0, XV_X, 5, XV_Y, ZTrkHeight - 60,
		       XV_SHOW, (show? TRUE:FALSE),
		       NULL);
		xv_set(item1, XV_X, 35, XV_Y, ZTrkHeight - 60,
		       XV_SHOW, (show? TRUE:FALSE),
		       NULL);
		xv_set(item2, XV_X, 65, XV_Y, ZTrkHeight - 60,
		       XV_SHOW, (show? TRUE:FALSE),
		       NULL);
		xv_set(item3, XV_X, 5, XV_Y, ZTrkHeight - 30,
		       XV_SHOW, (show? TRUE:FALSE),
		       NULL);
		xv_set(item4, XV_X, 35, XV_Y, ZTrkHeight - 30,
		       XV_SHOW, (show? TRUE:FALSE),
		       NULL);
		xv_set(item5, XV_X, 65, XV_Y, ZTrkHeight - 30,
		       XV_SHOW, (show? TRUE:FALSE),
		       NULL);
	    }else {
		xv_set(item5, XV_X, 65, XV_Y, ZTrkHeight - 30,
		       XV_SHOW, (show? TRUE:FALSE),
		       NULL);
		xv_set(item4, XV_X, 35, XV_Y, ZTrkHeight - 30,
		       XV_SHOW, (show? TRUE:FALSE),
		       NULL);
		xv_set(item3, XV_X, 5, XV_Y, ZTrkHeight - 30,
		       XV_SHOW, (show? TRUE:FALSE),
		       NULL);
		xv_set(item2, XV_X, 65, XV_Y, ZTrkHeight - 60,
		       XV_SHOW, (show? TRUE:FALSE),
		       NULL);
		xv_set(item1, XV_X, 35, XV_Y, ZTrkHeight - 60,
		       XV_SHOW, (show? TRUE:FALSE),
		       NULL);
		xv_set(item0, XV_X, 5, XV_Y, ZTrkHeight - 60,
		       XV_SHOW, (show? TRUE:FALSE),
		       NULL);
	    }
	    xv_set(ZLab_panel[iztrk],
		   XV_X, 1,
		   XV_HEIGHT, ht-1, NULL); /* make sure it fills the area */

	    y+= ZTrkHeight;
	}
    }
}

/**********************************************************************
 *   Updating  Zoom Track Labels                                      *
 **********************************************************************/

void UpdateZlab(int iztrk)
{
    GC gc= DefaultGC(theDisp, DefaultScreen(theDisp));
    Trace *trc= traces[iztrk+lowZTrkIndex];
    char orientation[20], sncl[20];
    float off_set;
    BIS3_HEADER *bh= &trc->wave->info;
    
    if (Mode_align && Mode_ZDisplayTScale)
	XClearWindow(theDisp,ZLab_win[iztrk]);

    if (trc && trc->wave && ZoomWindowMapped
	&& trc->zaxis.hs!=0) {
	sprintf(sncl, "%s %s %s %s", bh->station, bh->network, bh->channel, 
		(strlen(bh->location) > 0) ? bh->location : "--");
	XDrawString(theDisp, ZLab_win[iztrk], gc, 5, 10, sncl, strlen(sncl));
	if (trc->trip!=NULL) {

	    if (trc->trip->trc[TRC_Z]==trc) {
		sprintf(orientation,"Dip = %02.0f ",trc->wave->info.dip);
	    } else {
		float ang = trc->wave->info.azimuth + trc->trip->rot_theta -
		    trc->trip->sta_rotation;
		while ( ang > 360.0) 
		    ang -= 360.0;
		while (ang < 0.0 )
		    ang += 360.0;
		sprintf(orientation,"Azi = %03.0f ",ang);
	    }

	    XDrawString(theDisp, ZLab_win[iztrk], gc, 5, 25, orientation,
			strlen(orientation));
	}
    }
}

void CleanZLab()
{
    int i;
    for(i=0; i < NumZTracks; i++) {
	XClearWindow(theDisp,ZLab_win[i]);
    }
}


