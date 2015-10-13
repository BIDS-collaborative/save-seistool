#ifndef lint
static char id[] = "$Id: setup.c,v 1.2 2013/02/28 21:24:57 lombard Exp $";
#endif

/*
 * setup.c--
 *    general set-up, mostly X stuff
 *
 * Copyright (c) 1992-3 Andrew K. Yu, Lawrence Berkeley Laboratory
 *      and University of California, Berkeley.
 * All rights reserved.
 */
#include <xview/xview.h>
#include <xview/font.h>
#include "xv_proto.h"

Display *theDisp;
static int theScrn;
static unsigned long theBlackPixel;
static unsigned long theWhitePixel;
int use_colour;

void setup(Frame frame)
{
    XSetWindowAttributes setwinattr;
    unsigned long winattrMask;
    static int fSetBS = 0;

    theDisp = (Display *)xv_get(frame, XV_DISPLAY);
    theScrn = DefaultScreen(theDisp);
    theBlackPixel= BlackPixel(theDisp, theScrn);
    theWhitePixel= WhitePixel(theDisp, theScrn);
    use_colour= (DefaultDepth(theDisp,DefaultScreen(theDisp))>=2);

    if (fSetBS == 0)
    {
	/* Try setting backing store on the traces display */
	if (DoesBackingStore(ScreenOfDisplay(theDisp,theScrn)))
	{
	    setwinattr.backing_store = Always;
	    winattrMask = CWBackingStore;
	    XChangeWindowAttributes(theDisp, xv_get(frame, XV_XID), 
				    winattrMask, &setwinattr);
	}
	fSetBS = 1;
    }
}

int createGC(Window theNewWindow, GC *theNewGC)
{
    XGCValues	theGCValues;
#ifdef SITE_LBL
    extern int Mode_NCD;
#endif

    *theNewGC= XCreateGC( theDisp, theNewWindow,
			  (unsigned long) 0,
			  &theGCValues );

    if (*theNewGC == 0) {
	return 0;
    }else {
	extern Xv_Font text_font;

	XSetForeground(theDisp, *theNewGC, theBlackPixel);
	XSetBackground(theDisp, *theNewGC, theWhitePixel);
	XSetFont(theDisp, *theNewGC, (Font)xv_get(text_font,XV_XID));
	return (1);
    }
}
