/*	$Id: setup.h,v 1.2 2013/02/28 21:24:57 lombard Exp $	*/

/*
 *  setup.h--
 *     most of the global setup
 *
 * Copyright (c) 1992-3 Andrew K. Yu, Lawrence Berkeley Laboratory
 *      and University of California, Berkeley.
 * All rights reserved. 
 */

#ifndef SETUP_H
#define SETUP_H

#include <xview/xview.h>
#include "types.h"


extern Display *theDisp;
extern Frame tracesFrame;	/* top-level frame     */
extern int FrmHeight;		/* height of the frame */
extern int FrmWidth;		/* width of the frame  */
extern int use_colour;
extern int top_panel_width;

/*
 * width of a scrollbar. Shouldn't have to do this. This is a kludge to
 * set up scrollbars without attaching it to the canvas to be scrolled.
 */
#define	    XVSBARWIDTH	    22

/* Modes (rf "mode.c") */

extern int Mode_align;
extern int Mode_autoLoadPicks;
extern int Mode_autoScroll;
extern int Mode_noBorder;
extern int Mode_UWPickStyle;
extern int Mode_clipPlot;
extern int Mode_decimPlot;

extern int Mode_trvlTime;   /* in trvltime.c */
extern int Mode_rotate;	    /* in rotate.c */

extern int Mode_autoRegroup;	/* in group.c */
extern int Mode_autoGroupComp;	/* in group.c */

extern int Mode_CLI;	/* in main */

extern int Zoom_Mode_sameVertScale;	/* in ztrack.c */

extern int Mode_triplet;	/* in triplet.c */

extern int Mode_autoFilterNames;    /* in select.c */

extern int Mode_labDetails; /* in track.c */

extern int Mode_ZDisplayTScale;	/* in zscale.c */

extern int Mode_pfUseTime;  /* in pick.c */

#endif
