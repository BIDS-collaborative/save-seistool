#ifndef lint
static char id[] = "$Id: scroll.c,v 1.2 2013/02/28 21:24:57 lombard Exp $";
#endif

/*
 * scroll.c--
 *    management of scroll bar in the main window
 *
 * Copyright (c) 1992-3 Andrew K. Yu, Lawrence Berkeley Laboratory
 *      and University of California, Berkeley.
 * All rights reserved.
 */
#include <stdio.h>
#include <xview/xview.h>
#include <xview/canvas.h>
#include <xview/scrollbar.h>
#include <xview/xv_xrect.h>
#include "proto.h"
#include "xv_proto.h"

extern int NumTracks;
extern int LastTrack;
extern int lowTrkIndex;

/*  this module closely resembles the zscoll.c one  */

static Scrollbar scrollbar;
/* the following provide a handshake between ``SetSbarPosition'' and
 * ``monitor_scroll''. When the scroll bar position is set via xv_set,
 * monitor_scroll will be notified and the variable disables the actual
 * scrolling of data. The variable is reseted within monitor_scroll.
 * (this is a quick'n dirty solution to xv_setting scrollbar position
 * and by-pass monitor_scroll due to the xv_set. If better facility is
 * available, this should absolutely be removed. -- AY)
 */
static int  ignore_next_sbar_notify= 0;

static Notify_value monitor_scroll(Notify_client client, Event *event, 
				   Scrollbar sbar, Notify_event_type type);



/*
 * adjust the height and internal settings of the scrollbar. Should
 * be called when number of tracks changed.
 */
void AdjustSbar()
{
    int ppu= (int)(FrmHeight/NumTracks);

    if (scrollbar) {
	xv_set(scrollbar,
	   SCROLLBAR_PIXELS_PER_UNIT, ppu,
	   SCROLLBAR_OBJECT_LENGTH, LastTrack+1,
	   SCROLLBAR_PAGE_LENGTH, NumTracks,
	   SCROLLBAR_VIEW_LENGTH, NumTracks,
	   NULL);
    }
}

/*
 * SetSbarPosition make sure the internals are synchronized when scrolling
 * is done via the buttons panel rather than with the scroll bar directly.
 */
void SetSbarPosition()
{
    int cur;
    if (scrollbar) {
	cur= (int)xv_get(scrollbar, SCROLLBAR_VIEW_START);
	if (cur!=lowTrkIndex) {
	    ignore_next_sbar_notify=1;
	    xv_set(scrollbar, SCROLLBAR_VIEW_START, lowTrkIndex, NULL);
	}
	XFlush(theDisp);
    }
}


void initScrollbar(Canvas canvas)
{
    scrollbar = xv_create(canvas, SCROLLBAR,
	    SCROLLBAR_DIRECTION, SCROLLBAR_VERTICAL,
	    NULL);
    notify_interpose_event_func(
	xv_get(scrollbar,SCROLLBAR_NOTIFY_CLIENT),
	monitor_scroll, NOTIFY_SAFE);
}
	    

static Notify_value monitor_scroll(Notify_client client, Event *event, 
				   Scrollbar sbar, Notify_event_type type)
{
    int view_start, last_view_start, pixels_per, is_neg=0, total;

    if (event_id(event) == SCROLLBAR_REQUEST) {
	view_start= (int)xv_get(sbar, SCROLLBAR_VIEW_START);
	last_view_start= (int)xv_get(sbar, SCROLLBAR_LAST_VIEW_START);
	pixels_per= (int)xv_get(sbar, SCROLLBAR_PIXELS_PER_UNIT);
	if (ignore_next_sbar_notify) {
	    ignore_next_sbar_notify=0;	/* reset */
	}else {
	    if ((total=view_start-last_view_start) < 0)
		total = -total, is_neg=1;
	    if (total==NumTracks) {	/* 1 page */
		if (is_neg) 
		    trk_scroll_pgup();
		else
		    trk_scroll_pgdown();
	    }else if (total==1) {
		if (is_neg)
		    trk_scroll_up();
		else
		    trk_scroll_down();
	    }else {  /* scroll more than that! */
		trk_scroll_var(total, is_neg?SCRL_UP:SCRL_DOWN);
	    }
	}
    }
    return NOTIFY_DONE;
}
