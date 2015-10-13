#ifndef lint
static char id[] = "$Id: font.c,v 1.2 2013/02/28 21:25:00 lombard Exp $";
#endif

/*
 * font.c--
 *    setting up of desired fonts
 *
 * Copyright (c) 1992-3 Andrew K. Yu, Lawrence Berkeley Laboratory
 *      and University of California, Berkeley.
 * All rights reserved.
 */
#include <xview/xview.h>
#include <xview/font.h>
#include <xview/cms.h>

#include "xv_proto.h"

/* fonts */
Xv_Font helb_font, hel_font, text_font;
GC helb_gc, hel_gc, hel_invgc;

static void Create_font_GC(Frame frame)
{
    XGCValues gcvalues;
    Cms	cms;
    
    theDisp= (Display *)xv_get(frame, XV_DISPLAY);
    gcvalues.font= (Font)xv_get(helb_font, XV_XID);

    cms= (Cms)xv_get(frame, WIN_CMS);
    gcvalues.foreground= (unsigned long)xv_get(cms, CMS_FOREGROUND_PIXEL);
    gcvalues.background= (unsigned long)xv_get(cms, CMS_BACKGROUND_PIXEL);
    gcvalues.graphics_exposures = False;
    helb_gc= XCreateGC(theDisp, RootWindow(theDisp, DefaultScreen(theDisp)),
		       GCForeground|GCBackground|GCFont|GCGraphicsExposures,
		       &gcvalues);
    gcvalues.font= (Font)xv_get(hel_font, XV_XID);
    gcvalues.graphics_exposures = False;
    hel_gc= XCreateGC(theDisp, RootWindow(theDisp, DefaultScreen(theDisp)),
		      GCForeground|GCBackground|GCFont|GCGraphicsExposures,
		      &gcvalues);
    gcvalues.foreground= (unsigned long)xv_get(cms, CMS_BACKGROUND_PIXEL);
    gcvalues.background= (unsigned long)xv_get(cms, CMS_FOREGROUND_PIXEL);
    hel_invgc= XCreateGC(theDisp, RootWindow(theDisp, DefaultScreen(theDisp)),
			 GCForeground|GCBackground|GCFont|GCGraphicsExposures,
			 &gcvalues);
}

void LoadFont(Frame frame)
{
    extern int Mode_NCD;

    /* these are originally helvetica fonts, hence their name */
    if(!Mode_NCD) {
	helb_font= (Xv_Font)xv_find(frame, FONT,
				    FONT_FAMILY, FONT_FAMILY_LUCIDA,
				    FONT_STYLE, FONT_STYLE_BOLD,
				    FONT_SIZE,  12,
				    NULL);
	hel_font= (Xv_Font)xv_find(frame, FONT,
				   FONT_FAMILY, FONT_FAMILY_LUCIDA,
				   FONT_STYLE, FONT_STYLE_NORMAL,
				   FONT_SIZE,  12,
				   NULL);
	text_font= (Xv_Font)xv_find(frame, FONT,
				    FONT_NAME, "-adobe-courier-medium-r-normal--12-120-75-75-m-70-iso8859-1",
				    NULL);
    }else {
	/* need to be more specific for NCD's */
	helb_font= (Xv_Font)xv_find(frame, FONT,
				    FONT_NAME, "lucidasans-bold-8", /* 14 */
				    NULL);
	hel_font= (Xv_Font)xv_find(frame, FONT,
				   FONT_NAME, "lucidasans-8", /* bold-12",*/
				   NULL);
	text_font= (Xv_Font)xv_find(frame, FONT,
				    FONT_NAME, "-adobe-courier-medium-r-normal--12-120-75-75-m-70-iso8859-1",
				    NULL);
    }
    Create_font_GC(frame);
}


