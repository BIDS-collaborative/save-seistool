#ifndef lint
static char id[] = "$Id: zscroll.c,v 1.2 2013/02/28 21:24:55 lombard Exp $";
#endif

/*
 * zscroll.c--
 *    managing the scollbar in the zoom window
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
#include "xv_proto.h"

extern int LastTrack;

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

/* internal function prototypes */
static int sbarWidth();
static Notify_value monitor_zscroll(Notify_client client, Event *event, 
				    Scrollbar sbar, Notify_event_type type);



static int sbarWidth()
{
    return (int)xv_get(scrollbar, XV_WIDTH);
}

/*
 * adjust the height and internal settings of the scrollbar. Should
 * be called when number of tracks changed.
 */
void AdjustZSbar(int height)
{
    int ppu= (int)(height/NumZTracks);

    if (scrollbar) {
	xv_set(scrollbar,
	   SCROLLBAR_PIXELS_PER_UNIT, ppu,
	   SCROLLBAR_OBJECT_LENGTH, LastTrack+1,
	   SCROLLBAR_PAGE_LENGTH, NumZTracks,
	   SCROLLBAR_VIEW_LENGTH, NumZTracks,
	   NULL);
    }
}

/*
 * SetSbarPosition make sure the internals are synchronized when scrolling
 * is done via the buttons panel rather than with the scroll bar directly.
 */
void SetZSbarPosition()
{
    int cur;
    if (scrollbar) {
	cur= (int)xv_get(scrollbar, SCROLLBAR_VIEW_START);
	if (cur!=lowZTrkIndex) {
	    ignore_next_sbar_notify=1;
	    xv_set(scrollbar, SCROLLBAR_VIEW_START, lowZTrkIndex, NULL);
/* NB: tricky part.
 *	doing an "ignore_next_sbar_notify=0" here won't work. An event has
 *      been enqueued but not executed yet. Resetting the ignore here is
 *      too early.
 */
	}
	XFlush(theDisp);
    }
}


void initZScrollbar(Canvas canvas)
{
    scrollbar = xv_create(canvas, SCROLLBAR,
	    SCROLLBAR_DIRECTION, SCROLLBAR_VERTICAL,
	    NULL);
    notify_interpose_event_func(
	xv_get(scrollbar,SCROLLBAR_NOTIFY_CLIENT),
	monitor_zscroll, NOTIFY_SAFE);
}
	    

static Notify_value monitor_zscroll(Notify_client client, Event *event, 
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
	    ztrk_scroll_var(total, is_neg?SCRL_UP: SCRL_DOWN);
	}
    }
    return NOTIFY_DONE;
}
