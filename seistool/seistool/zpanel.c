#ifndef lint
static char id[] = "$Id: zpanel.c,v 1.2 2013/02/28 21:24:55 lombard Exp $";
#endif

/*
 * zpanel.c--
 *    set up the zoom window panel 
 *
 * Copyright (c) 1992-3 Andrew K. Yu, Lawrence Berkeley Laboratory
 *      and University of California, Berkeley.
 * All rights reserved.
 */
#include <xview/xview.h>
#include <xview/canvas.h>
#include <xview/panel.h>
#include <xview/xv_xrect.h>

#include "proto.h"
#include "xv_proto.h"

/* the following are bitmaps created for the images */
#include    "bitmap/bp_pirate_20.bmp"
#include    "bitmap/bp_up_20.bmp"
#include    "bitmap/bp_down_20.bmp"
#include    "bitmap/bp_pgup_20.bmp"
#include    "bitmap/bp_pgdown_20.bmp"
#include    "bitmap/bp_advframe_20.bmp"
#include    "bitmap/bp_bacframe_20.bmp"
#include    "bitmap/bp_advhalf_20.bmp"
#include    "bitmap/bp_backhalf_20.bmp"
#include    "bitmap/bp_stretch_20.bmp"
#include    "bitmap/bp_unstretch_20.bmp"
#include    "bitmap/bp_expand_20.bmp"
#include    "bitmap/bp_compress_20.bmp"
#include    "bitmap/bp_clip_20.bmp"
#include    "bitmap/bp_unclip_20.bmp"

#include    "bitmap/bp_ttime_20.bmp"
#include    "bitmap/bp_rotate_20.bmp"

#include    "bitmap/bp_tscale_20.bmp"
#include    "bitmap/bp_redraw_20.bmp"
#include    "bitmap/bp_reserved_20.bmp"

extern Trace **traces;

static Server_image clip, unclip;
static Panel_item clipButton;
Panel_item tscaleButton;

/* internal function prototypes */
static void refreshZoomCanvas( );
static void extra_menu_proc(Menu menu, Menu_item menu_item);


static void refreshZoomCanvas( )
{
    redrawZoomCanvas("refresh zoom");
}


void InitZPanel(Frame frame)
{
    Panel   panel;
    Server_image  close, up, down, pgup, pgdown, advfr, bacfr,
	advhalf, bachalf, stretch, unstretch, expand, compress;
    Server_image ttime, rotate, tscale, redraw;
    Menu menu;

    menu= (Menu)xv_create(NULL, MENU,
			  MENU_STRINGS,
			  "Clipping", "UniScale", NULL,	
			  MENU_NOTIFY_PROC, extra_menu_proc,
			  NULL);
    panel = (Panel)xv_create(frame,PANEL,
			     XV_WIDTH,  72,	XV_HEIGHT, 298,
			     PANEL_ITEM_X_GAP, 4,
			     PANEL_ITEM_Y_GAP, 4,
			     NULL);

    /* setup images */
    close = (Server_image)xv_create(NULL, SERVER_IMAGE,
				    XV_WIDTH, bp_pirate_20_width,
				    XV_HEIGHT, bp_pirate_20_height,
				    SERVER_IMAGE_X_BITS, bp_pirate_20_bits,
				    NULL);
    clip = (Server_image)xv_create(NULL, SERVER_IMAGE,
				   XV_WIDTH, bp_clip_20_width,
				   XV_HEIGHT, bp_clip_20_height,
				   SERVER_IMAGE_X_BITS, bp_clip_20_bits,
				   NULL);
    unclip = (Server_image)xv_create(NULL, SERVER_IMAGE,
				     XV_WIDTH, bp_unclip_20_width,
				     XV_HEIGHT, bp_unclip_20_height,
				     SERVER_IMAGE_X_BITS, bp_unclip_20_bits,
				     NULL);
    up = (Server_image)xv_create(NULL, SERVER_IMAGE,
				 XV_WIDTH, bp_up_20_width,
				 XV_HEIGHT, bp_up_20_height,
				 SERVER_IMAGE_X_BITS, bp_up_20_bits,
				 NULL);
    down = (Server_image)xv_create(NULL, SERVER_IMAGE,
				   XV_WIDTH, bp_down_20_width,
				   XV_HEIGHT, bp_down_20_height,
				   SERVER_IMAGE_X_BITS, bp_down_20_bits,
				   NULL);
    pgup = (Server_image)xv_create(NULL, SERVER_IMAGE,
				   XV_WIDTH, bp_pgup_20_width,
				   XV_HEIGHT, bp_pgup_20_height,
				   SERVER_IMAGE_X_BITS, bp_pgup_20_bits,
				   NULL);
    pgdown = (Server_image)xv_create(NULL, SERVER_IMAGE,
				     XV_WIDTH, bp_pgdown_20_width,
				     XV_HEIGHT, bp_pgdown_20_height,
				     SERVER_IMAGE_X_BITS, bp_pgdown_20_bits,
				     NULL);
    bacfr = (Server_image)xv_create(NULL, SERVER_IMAGE,
				    XV_WIDTH, bp_bacframe_20_width,
				    XV_HEIGHT, bp_bacframe_20_height,
				    SERVER_IMAGE_X_BITS, bp_bacframe_20_bits,
				    NULL);
    advfr = (Server_image)xv_create(NULL, SERVER_IMAGE,
				    XV_WIDTH, bp_advframe_20_width,
				    XV_HEIGHT, bp_advframe_20_height,
				    SERVER_IMAGE_X_BITS, bp_advframe_20_bits,
				    NULL);
    bachalf = (Server_image)xv_create(NULL, SERVER_IMAGE,
				      XV_WIDTH, bp_backhalf_20_width,
				      XV_HEIGHT, bp_backhalf_20_height,
				      SERVER_IMAGE_X_BITS, bp_backhalf_20_bits,
				      NULL);
    advhalf = (Server_image)xv_create(NULL, SERVER_IMAGE,
				      XV_WIDTH, bp_advhalf_20_width,
				      XV_HEIGHT, bp_advhalf_20_height,
				      SERVER_IMAGE_X_BITS, bp_advhalf_20_bits,
				      NULL);
    stretch = (Server_image)xv_create(NULL, SERVER_IMAGE,
				      XV_WIDTH, bp_stretch_20_width,
				      XV_HEIGHT, bp_stretch_20_height,
				      SERVER_IMAGE_DEPTH, 1,
				      SERVER_IMAGE_X_BITS, bp_stretch_20_bits,
				      NULL);
    unstretch = (Server_image)xv_create(NULL, SERVER_IMAGE,
					XV_WIDTH, bp_unstretch_20_width,
					XV_HEIGHT, bp_unstretch_20_height,
					SERVER_IMAGE_DEPTH, 1,
					SERVER_IMAGE_X_BITS, bp_unstretch_20_bits,
					NULL);
    expand = (Server_image)xv_create(NULL, SERVER_IMAGE,
				     XV_WIDTH, bp_expand_20_width,
				     XV_HEIGHT, bp_expand_20_height,
				     SERVER_IMAGE_DEPTH, 1,
				     SERVER_IMAGE_X_BITS, bp_expand_20_bits,
				     NULL);
    compress = (Server_image)xv_create(NULL, SERVER_IMAGE,
				       XV_WIDTH, bp_compress_20_width,
				       XV_HEIGHT, bp_compress_20_height,
				       SERVER_IMAGE_DEPTH, 1,
				       SERVER_IMAGE_X_BITS, bp_compress_20_bits,
				       NULL);
    ttime = (Server_image)xv_create(NULL, SERVER_IMAGE,
				    XV_WIDTH, bp_ttime_20_width,
				    XV_HEIGHT, bp_ttime_20_height,
				    SERVER_IMAGE_DEPTH, 1,
				    SERVER_IMAGE_X_BITS, bp_ttime_20_bits,
				    NULL);
    rotate = (Server_image)xv_create(NULL, SERVER_IMAGE,
				     XV_WIDTH, bp_rotate_20_width,
				     XV_HEIGHT, bp_rotate_20_height,
				     SERVER_IMAGE_DEPTH, 1,
				     SERVER_IMAGE_X_BITS, bp_rotate_20_bits,
				     NULL);
    tscale = (Server_image)xv_create(NULL, SERVER_IMAGE,
				     XV_WIDTH, bp_tscale_20_width,
				     XV_HEIGHT, bp_tscale_20_height,
				     SERVER_IMAGE_DEPTH, 1,
				     SERVER_IMAGE_X_BITS, bp_tscale_20_bits,
				     NULL);
    redraw = (Server_image)xv_create(NULL, SERVER_IMAGE,
				       XV_WIDTH, bp_redraw_20_width,
				       XV_HEIGHT, bp_redraw_20_height,
				       SERVER_IMAGE_DEPTH, 1,
				       SERVER_IMAGE_X_BITS, bp_redraw_20_bits,
				       NULL);

    (void)xv_create(panel, PANEL_BUTTON,
		    PANEL_LABEL_IMAGE,	close,
		    PANEL_NOTIFY_PROC,	close_zoom_window,
		    NULL);
    clipButton= (Panel_item)xv_create(panel, PANEL_BUTTON,
				      PANEL_LABEL_IMAGE,	clip,
				      PANEL_NOTIFY_PROC,	setmode_ztrack,
				      NULL);
    (void)xv_create(panel, PANEL_BUTTON,
		    PANEL_LABEL_IMAGE,	up,
		    PANEL_NOTIFY_PROC,	ztrk_scroll_up,
		    PANEL_NEXT_ROW, -1,
		    NULL);
    (void)xv_create(panel, PANEL_BUTTON,
		    PANEL_LABEL_IMAGE,	down,
		    PANEL_NOTIFY_PROC,	ztrk_scroll_down,
		    NULL);
    (void)xv_create(panel, PANEL_BUTTON,
		    PANEL_LABEL_IMAGE,	pgup,
		    PANEL_NOTIFY_PROC,	ztrk_scroll_pgup,
		    PANEL_NEXT_ROW, -1,
		    NULL);
    (void)xv_create(panel, PANEL_BUTTON,
		    PANEL_LABEL_IMAGE,	pgdown,
		    PANEL_NOTIFY_PROC,	ztrk_scroll_pgdown,
		    NULL);
    (void)xv_create(panel, PANEL_BUTTON,
		    PANEL_LABEL_IMAGE,	bacfr,
		    PANEL_NOTIFY_PROC,	ztrk_scroll_left,
		    PANEL_NEXT_ROW, -1,
		    NULL);
    (void)xv_create(panel, PANEL_BUTTON,
		    PANEL_LABEL_IMAGE,	advfr,
		    PANEL_NOTIFY_PROC,	ztrk_scroll_right,
		    NULL);
    (void)xv_create(panel, PANEL_BUTTON,
		    PANEL_LABEL_IMAGE,	bachalf,
		    PANEL_NOTIFY_PROC,	ztrk_scroll_halfleft,
		    PANEL_NEXT_ROW, -1,
		    NULL);
    (void)xv_create(panel, PANEL_BUTTON,
		    PANEL_LABEL_IMAGE,	advhalf,
		    PANEL_NOTIFY_PROC,	ztrk_scroll_halfright,
		    NULL);
    (void)xv_create(panel, PANEL_BUTTON,
		    PANEL_LABEL_IMAGE,	stretch,
		    PANEL_NOTIFY_PROC,	ztrk_stretch,
		    PANEL_NEXT_ROW, -1,
		    NULL);
    (void)xv_create(panel, PANEL_BUTTON,
		    PANEL_LABEL_IMAGE,	unstretch,
		    PANEL_NOTIFY_PROC,	ztrk_unstretch,
		    NULL);
    (void)xv_create(panel, PANEL_BUTTON,
		    PANEL_LABEL_IMAGE,	expand,
		    PANEL_NOTIFY_PROC,	ztrk_expand,
		    PANEL_NEXT_ROW, -1,
		    NULL);
    (void)xv_create(panel, PANEL_BUTTON,
		    PANEL_LABEL_IMAGE,	compress,
		    PANEL_NOTIFY_PROC,	ztrk_compress,
		    NULL);
    (void)xv_create(panel, PANEL_BUTTON,
		    PANEL_LABEL_IMAGE,	ttime,
		    PANEL_NOTIFY_PROC,	EnterTTMode,
		    NULL);
    (void)xv_create(panel, PANEL_BUTTON,
		    PANEL_LABEL_IMAGE,	rotate,
		    PANEL_NOTIFY_PROC,	open_rotate_panel,
		    NULL);
    tscaleButton= (Panel_item)xv_create(panel, PANEL_BUTTON,
					PANEL_LABEL_IMAGE,	tscale,
					PANEL_NOTIFY_PROC,	ToggleZTScale,
					PANEL_INACTIVE,		(Mode_align?FALSE:TRUE),
					NULL);
    (void)xv_create(panel, PANEL_BUTTON,
		    PANEL_LABEL_IMAGE,	redraw,
		    PANEL_NOTIFY_PROC, refreshZoomCanvas,
		    NULL);
}

static void extra_menu_proc(Menu menu, Menu_item menu_item)
{
    char *s= (char *)xv_get(menu_item, MENU_STRING);

    if (!strcmp(s, "Clipping")) {
	setmode_ztrack();
    }else if(!strcmp(s, "UniScale")) {
	int i;
	Zoom_Mode_sameVertScale= !Zoom_Mode_sameVertScale;
	for(i=lowZTrkIndex; i<= highZTrkIndex; i++) {
	    traces[i]->zaxis_needScale= 1;
	}
	if(Zoom_Mode_sameVertScale) {
	    newVertScale();
	}
	RedrawZoomWindow("extra_menu_proc");
    }
}

void toggleClipButton()
{
    if(Mode_clipPlot) {
	xv_set(clipButton, PANEL_LABEL_IMAGE, unclip, NULL);
    }else {
	xv_set(clipButton, PANEL_LABEL_IMAGE, clip, NULL);
    }
}
