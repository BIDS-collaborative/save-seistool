#ifndef lint
static char id[] = "$Id: stipple.c,v 1.2 2013/02/28 21:24:57 lombard Exp $";
#endif

/*
 * stipple.c--
 *    implements stippling on monochromes
 *
 * Copyright (c) 1992-3 Andrew K. Yu, Lawrence Berkeley Laboratory
 *      and University of California, Berkeley.
 * All rights reserved.
 */
#include <xview/xview.h>
#include <xview/canvas.h>
#include <xview/xv_xrect.h>
#include <xview/cms.h>
#include "xv_proto.h"

#include "bitmap/fill12.bmp"
#include "bitmap/fill25.bmp"
#include "bitmap/fill37.bmp"
#include "bitmap/fill50.bmp"
#include "bitmap/fill63.bmp"
#include "bitmap/fill75.bmp"
#include "bitmap/fill88.bmp"

GC shade_gc[8];
static Cms cms;
static Pixmap stip12=0, stip25, stip37, stip50, stip63, stip75, stip88;
static unsigned long *pixel_table;

void InitStipples()
{
    Window rw= RootWindow(theDisp, DefaultScreen(theDisp));
    GC	s12_gc, s25_gc, s37_gc, s50_gc,	s63_gc, s75_gc, s88_gc, s100_gc;
    
    if(stip12)return;	/* initialized already */
    if (use_colour) {
	cms= (Cms) xv_create(NULL, CMS,
	     CMS_SIZE, 9,
	     CMS_NAMED_COLORS,
#if 0
		"white", "purple","blue","pale green",
		"green","yellow","orange","red", /* "black",*/
		"IndianRed4",

		"blue1","red1","green1","blue2","red2","green2",
		"blue3","red3","green3",
#endif
		"yellow", "SlateBlue4","SlateBlue3","SlateBlue2",
		"SlateBlue1",
		"RosyBrown1","IndianRed2","IndianRed3",
		"IndianRed4",
		NULL,

	     NULL);
	pixel_table= (unsigned long *)xv_get(cms, CMS_INDEX_TABLE);
	createGC(rw, &s12_gc);
	XSetForeground(theDisp,s12_gc,pixel_table[1]);
	createGC(rw, &s25_gc);
	XSetForeground(theDisp,s25_gc,pixel_table[2]);
	createGC(rw, &s37_gc);
	XSetForeground(theDisp,s37_gc,pixel_table[3]);
	createGC(rw, &s50_gc);
	XSetForeground(theDisp,s50_gc,pixel_table[4]);
	createGC(rw, &s63_gc);
	XSetForeground(theDisp,s63_gc,pixel_table[5]);
	createGC(rw, &s75_gc);
	XSetForeground(theDisp,s75_gc,pixel_table[6]);
	createGC(rw, &s88_gc);
	XSetForeground(theDisp,s88_gc,pixel_table[7]);
	createGC(rw, &s100_gc);
	XSetForeground(theDisp,s100_gc,pixel_table[8]);
    }else {
	stip12= XCreateBitmapFromData(theDisp, rw,
	      fill12_bits, fill12_width, fill12_height);
	stip25= XCreateBitmapFromData(theDisp, rw,
	      fill25_bits, fill25_width, fill25_height);
	stip37= XCreateBitmapFromData(theDisp, rw,
	      fill37_bits, fill37_width, fill37_height);
	stip50= XCreateBitmapFromData(theDisp, rw,
	      fill50_bits, fill50_width, fill50_height);
	stip63= XCreateBitmapFromData(theDisp, rw,
	      fill63_bits, fill63_width, fill63_height);
	stip75= XCreateBitmapFromData(theDisp, rw,
	      fill75_bits, fill75_width, fill75_height);
	stip88= XCreateBitmapFromData(theDisp, rw,
	      fill88_bits, fill88_width, fill88_height);
	createGC(rw, &s12_gc);
	XSetFillStyle(theDisp,s12_gc,FillStippled);
	XSetStipple(theDisp,s12_gc,stip12);
	createGC(rw, &s25_gc);
	XSetFillStyle(theDisp,s25_gc,FillStippled);
	XSetStipple(theDisp,s25_gc,stip25);
	createGC(rw, &s37_gc);
	XSetFillStyle(theDisp,s37_gc,FillStippled);
	XSetStipple(theDisp,s37_gc,stip37);
	createGC(rw, &s50_gc);
	XSetFillStyle(theDisp,s50_gc,FillStippled);
	XSetStipple(theDisp,s50_gc,stip50);
	createGC(rw, &s63_gc);
	XSetFillStyle(theDisp,s63_gc,FillStippled);
	XSetStipple(theDisp,s63_gc,stip63);
	createGC(rw, &s75_gc);
	XSetFillStyle(theDisp,s75_gc,FillStippled);
	XSetStipple(theDisp,s75_gc,stip75);
	createGC(rw, &s88_gc);
	XSetFillStyle(theDisp,s88_gc,FillStippled);
	XSetStipple(theDisp,s88_gc,stip88);
	createGC(rw, &s100_gc);
    }

    shade_gc[0]= s12_gc;
    shade_gc[1]= s25_gc;
    shade_gc[2]= s37_gc;
    shade_gc[3]= s50_gc;
    shade_gc[4]= s63_gc;
    shade_gc[5]= s75_gc;
    shade_gc[6]= s88_gc;
    shade_gc[7]= s100_gc;
}



			    
	
    
