#ifndef lint
static char id[] = "$Id: event.c,v 1.2 2013/02/28 21:25:00 lombard Exp $";
#endif

/*
 * event.c--
 *    handles X events in main window
 *
 * Copyright (c) 1992-3 Andrew K. Yu, Lawrence Berkeley Laboratory
 *      and University of California, Berkeley.
 * All rights reserved.
 */
#include <stdio.h>
#include "proto.h"
#include "xv_proto.h"

extern Trace **traces;
extern Track **tracks;
extern int lowTrkIndex;
extern int NumTracks;
extern int highTrkIndex;
extern int LastTrack;

void TxtCvs_event_proc(Xv_Window window, Event *event)
{
    int itrc, i;
    Window win= (Window)xv_get(window, XV_XID);
    Trace *trc;

    itrc=-1;
    for(i=0; i < NumTracks; i++) {
	if (win==tracks[i]->txtwin) {	/* match track */
	    itrc= i + lowTrkIndex;
	    break;
	}
    }
    if (itrc==-1 || traces[itrc]->wave==NULL)
	return;	/* ignore */
    trc= traces[itrc];
    switch (event_action(event)) {
    case ACTION_SELECT:
    case MS_LEFT: {
	int idx, len, i;

	if(event_is_up(event)) {
	    /* toggle */
	    if (trc->selected) {	/* deselect */
		trc->selected=0;
	    }else {	/* select */
		trc->selected=1;
	    }
	    UpdateLabTrack(trc, tracks[itrc-lowTrkIndex]);
	}
	break;
    }
    case ACTION_MENU:
    case MS_RIGHT: {
	if(event_is_up(event)) {
	    if(Mode_labDetails) {
		Mode_labDetails= 0;
		UpdateLabTrack(trc, tracks[itrc-lowTrkIndex]);
	    }else {
		open_info_win();
		DisplayTraceInfo(itrc);
	    }
	}else {
	    if(event_ctrl_is_down(event)) {
		Mode_labDetails= 1;
		UpdateLabTrack(trc, tracks[itrc-lowTrkIndex]);
	    }
	}
	break;
    }
    default:
	break;
    }
}

static void handle_event_left(Event *event, int itrc, Trace *trc);
static void handle_event_ctrl_right(Event *event, int itrc, Trace *trc);
static void handle_event_ctrl_left_drag(Event *event, int itrc, Trace *trc);
static void handle_event_ctrl_left(Event *event, int itrc, Trace *trc);
static void handle_Z(Event *event, int itrc, Trace *trc);
static int was_cntrl=0;     /* was the cntrl key down */
static int is_drag= 0;      /* are we dragging ??? */
static int is_LeftDown= 0;  /* this is to solve a LOC_DRAG with no
			       MS_LEFT */

void Cvs_event_proc(Xv_Window window, Event *event)
{
    int x, y, itrc, i;
    Window win= (Window)xv_get(window, XV_XID);
    Trace *trc;

    x=event_x(event), y=event_y(event);
    itrc=-1;
    for(i=0; i < NumTracks; i++) {
	if (win==tracks[i]->xwin) {	/* match track */
	    itrc= i + lowTrkIndex;
	    break;
	}
    }
    if (itrc==-1 || traces[itrc]->wave==NULL)
	return;	/* ignore */

    trc= traces[itrc];
    if (event_is_ascii(event)) {
	if (event_is_up(event)) {
	    switch (event_action(event)) {
	    case 'f': case 'F':   /* full scale */
		FullScale();
		break;
	    case 'l': case 'L':   /* clip portion to the left */
		if(HasBoundary()) {
		    ClipTracksLeft(x);
		    ClearBound();
		}
		break;
	    case 'r': case 'R':   /* clip portion to the right */
		if(HasBoundary()) {
		    ClipTracksRight(x);
		    ClearBound();
		}
		break;
	    case ' ':    /* clip to left/right-- position dependent */
		if(HasBoundary()) {
		    ClipTracks(x);
		    ClearBound();
		}
		break;
	    case 'z': case'Z':
		handle_Z(event,itrc, trc);
		break;
	    default:
		break;
	    }
	}
    }else 
	switch (event_action(event)) {
	case LOC_WINENTER:
	    win_set_kbd_focus(window, xv_get(window,XV_XID));
	    break;
	case LOC_DRAG:
	    if (event_left_is_down(event)) {
		if (event_ctrl_is_down(event) ||was_cntrl)
		    handle_event_ctrl_left_drag(event, itrc, trc);
	    }
	    if (event_middle_is_down(event))
		BoundTracks_drag( x, event );	    
	    break;
	case ACTION_SELECT:
	case MS_LEFT: {
	    if(event_ctrl_is_down(event)||was_cntrl) {
		handle_event_ctrl_left(event, itrc, trc);
	    }else if(event_middle_is_down(event)) {
		if(HasBoundary()) {
		    ClipTracksLeft(x);
		    ClearBound();
		}
	    }else {
		handle_event_left(event, itrc, trc);
	    }
	    break;
	}
	case ACTION_ADJUST:
	case MS_MIDDLE:
	    if(event_ctrl_is_down(event)) {
		if(event_is_up(event)) open_sp_panel();
	    }else {
		BoundTracks(x,event);
	    }
	    break;
	case ACTION_MENU:
	case MS_RIGHT: {
	    if (event_ctrl_is_down(event) && event_is_up(event)) {
		/* cancel interval */
		handle_event_ctrl_right(event,itrc,trc);
	    }else if(event_middle_is_down(event)) {
		if(HasBoundary()) {
		    ClipTracksRight(x);
		    ClearBound();
		}
	    }
	    break;
	}
	default:
	    return;
	}
}


static void handle_event_left(Event *event, int itrc, Trace *trc)
{
    int x, y;
    int idx, len, i;
    int oldl=lowZTrkIndex;

    x=event_x(event), y=event_y(event);
    if(event_is_up(event)) {
	if (!ZoomWindowMapped) return; /* ignore */

	for(i=lowZTrkIndex; i<= highZTrkIndex; i++) {
	    UpdateMarks(i); /* clean marks */
	}
	if(Mode_triplet)
	    itrc= trc->trip->first_trc->itrc;
	lowZTrkIndex= itrc;
	highZTrkIndex= itrc+NumZTracks-1;
	if (highZTrkIndex>LastTrack) {
	    itrc= lowZTrkIndex= LastTrack-NumZTracks+1;
	    highZTrkIndex= LastTrack;
	}
	if (lowZTrkIndex<0) {
	    itrc= lowZTrkIndex= 0;
	    highZTrkIndex= NumZTracks-1;
	}
	/* move to new location */
	idx= coordToIndex(&traces[itrc]->axis, x, 1);

	if (Mode_triplet)
	    Trip_ZoomContentChanged(lowZTrkIndex);

	for (i=0; i < NumZTracks; i++)  {
	    trc= traces[itrc];
	    if (trc->wave) {
#if RESTORE_VS
		float restore_vs; /* quick fix ! */
		restore_vs= trc->zaxis.vs;
#endif
		if (i!=0) { 
		    trc->zaxis.hs=0;    /*reset*/
		    ScaleZTrack(trc, traces[itrc-1]);
		    if(Mode_triplet)
			checkTripBound(trc);
		}else {
		    int nval;
		    nval= trc->wave->info.n_values;

		    ScaleZTrack(trc, traces[oldl]);
		    /* using the scale, move to desired posn */
		    len= trc->zaxis.ix2-trc->zaxis.ix1;

		    trc->zaxis.ix2= idx + len/2;
		    trc->zaxis.ix1= trc->zaxis.ix2-len;
		    fixZtrkBounds(trc, len);
		    if(Mode_triplet)
			checkTripBound(trc);
		}
#if RESTORE_VS
		if (restore_vs!=0) {    /* quick fix-- bad!!! */
		    trc->zaxis.vs= restore_vs;
		}
#endif
		if (Zoom_Mode_sameVertScale)
		    trc->zaxis_needScale= 1;
		UpdateMarks(itrc);
		itrc++; /* next trace */
	    }
	}
	if (Zoom_Mode_sameVertScale) newVertScale();
	if (Mode_trvlTime) {
	    if(oldl!=lowZTrkIndex) {
		Trvl_ZoomContentChanged();
	    }else {
		Trvl_Rescale();
	    }
	}
	if (Mode_rotate && oldl!=lowZTrkIndex) {
	    Rot_ZoomContentChanged();
	}
	RedrawZoomWindow("handle_event_left");
	if (Mode_autoScroll) {
	    AutoScroll((oldl>lowZTrkIndex)?SCRL_UP:SCRL_DOWN); /* can be up/down with compGp mode */
	    SetSbarPosition();
	}
    }
}

/* these next events deal with the creation and destruction            */
/* of selection regions                                                */
/* 1. to destroy a region the cntrl-right-mouse-button is pressed      */
/*       and the code finds the nearest selection in the trace and     */
/*       removes it from the list                                      */
/* 2. to create and modify a region  the cntrl-leftt-mouse-button      */
/*       is a. pressed and released or b. pressed and held and release */
/* on pressing the initial location of the region is defined           */
/* on releasing the region is then created and stored in the list      */
/* on holding the size of the region is manipulated                    */


static int firstMark=-1;
static int oldMark_x=-1;

/* remove selected region */
static void handle_event_ctrl_right(Event *event, int itrc, Trace *trc)
{
    int x=event_x(event);
    Reg_select *near=Find_Nearest_SRegion(trc,x);

    /* erase all */
    UpdateInterval(itrc);
    if (ZoomWindowMapped) {Zoom_UpdateInterval(itrc);}

    if (near!=NULL) {
	RemoveSRegion(near,trc);
    }

    /* redraw without */
    UpdateInterval(itrc);
    if (ZoomWindowMapped) {Zoom_UpdateInterval(itrc);}
    was_cntrl=0;
}


/* expand or contract select region */
static void handle_event_ctrl_left_drag(Event *event, int itrc, Trace *trc)
{
    int x, y;
    int idx, len, i;
    Track *trk=tracks[itrc-lowTrkIndex];

    was_cntrl=1;

    x=event_x(event), y=event_y(event);
    if (event_is_down(event)) {

	if(is_drag) {
	    DrawInterval(itrc,oldMark_x,firstMark,trk->ovrGC); 
	    if (ZoomWindowMapped) {
		int zoom_x,zoom_y;
		zoom_x=indexToCoord(&trc->zaxis,
				    coordToIndex(&trc->axis,oldMark_x, 1), 1);
		zoom_y=indexToCoord(&trc->zaxis,
				    coordToIndex(&trc->axis,firstMark, 1), 1);
		Zoom_DrawInterval(itrc,zoom_x,zoom_y,trk->ovrGC); 
	    }
	}else {
	    /* this piece of code is here to cure LOC_DRAG
	       events with no preceding MS_LEFT. */
	    if(!is_LeftDown) {
		firstMark=x;
	    }
	}
	/* final mark is x */
	DrawInterval(itrc,firstMark,x,trk->ovrGC);
	if (ZoomWindowMapped) {
	    int zoom_x,zoom_y;
	    zoom_y=indexToCoord(&trc->zaxis, coordToIndex(&trc->axis,x,1),1);
	    zoom_x=indexToCoord(&trc->zaxis, coordToIndex(&trc->axis,firstMark,1),1);
	    Zoom_DrawInterval(itrc,zoom_x,zoom_y,trk->ovrGC); 
	}

	oldMark_x=x;
	is_drag= 1;
    }
}

/* creates new regions */
static void handle_event_ctrl_left(Event *event, int itrc, Trace *trc)
{
    int x, y;
    int idx, len, i; 
    int store=1;
    Track *trk=tracks[itrc-lowTrkIndex];

    x=event_x(event), y=event_y(event);

    if(was_cntrl==0) {was_cntrl=1;}
    if(event_is_down(event)) {
	/* start creating the region */
	firstMark=x;
	oldMark_x=x;
	is_LeftDown= 1;
    }else {
	/* the release of the button */
	/* create and store the region in the list */

	Reg_select *new=Create_SRegion();
	int pos1,pos2;
	float srate=trc->wave->info.sample_rate;
	was_cntrl=0;

	/* this point is in the window */
	pos1=coordToIndex( &trc->axis, firstMark, 1);


	if(is_drag) {
	    /* clear temporary drawing */
	    DrawInterval(itrc,oldMark_x,firstMark,trk->ovrGC);
	    if (ZoomWindowMapped) {
		int zoom_x,zoom_y;
		zoom_x=indexToCoord(&trc->zaxis,
				    coordToIndex(&trc->axis,oldMark_x,1),1);
		zoom_y=indexToCoord(&trc->zaxis, 
				    coordToIndex(&trc->axis,firstMark,1),1);
		Zoom_DrawInterval(itrc,zoom_x,zoom_y,trk->ovrGC); 
	    }
	}


	if (x==firstMark) {
	    /* use defined window size */
	    int nsamp;
	    float sec;
	    int nsampOrSec= get_window_length(&nsamp,&sec);
	    if(nsampOrSec==0) {	/* nsamp */
		pos2= pos1 + ((nsamp>0)?(nsamp-1):0);
	    }else {			/* sec */
		pos2= pos1 + ((sec>0)? (sec*srate-1):0);
	    }
	}else {
	    pos2= coordToIndex( &trc->axis, x, 1);
	}

	/* which way do we go */
	if (pos1<pos2) {
	    new->right_index=pos2;
	    new->left_index=pos1;
	} else if (pos1>pos2) {
	    new->right_index=pos1;
	    new->left_index=pos2;
	} else {
	    store=0;
	}

	/* this point is in the window */
	if (new->left_index>=trc->wave->info.n_values) new->left_index=trc->wave->info.n_values-1;
	if (new->right_index<0) new->right_index=1;
	if (new->left_index<0)new->left_index=1;
	if (new->right_index>=trc->wave->info.n_values) new->right_index=trc->wave->info.n_values-1;
	/* again reverse indexes if needed */
	if (new->right_index<new->left_index) {
	    int t_pos=new->right_index;
	    new->right_index=new->left_index;
	    new->left_index=t_pos;
	}

	UpdateInterval( itrc ); /* erase regions */
	if (ZoomWindowMapped) {Zoom_UpdateInterval(itrc);}

	if (store==1&& new->left_index<new->right_index) {
	    /*      DrawInterval(itrc,new->left_index,new->right_index,trk->ovrGC);*/
	    Insert_SRegion(new,trc);
	} else {
	    /* if region is of no length do not store it */
	    free(new);
	    new=NULL;
	}

	UpdateInterval( itrc ); /* redraw regions */
	if (ZoomWindowMapped) {Zoom_UpdateInterval(itrc);}

	/* reset flags etc */
	oldMark_x=-1;
	is_drag= 0;			/* reset drag */
	is_LeftDown= 0;
    }
}

static void handle_Z(Event *event, int itrc, Trace *trc)
{
    int i;
    int just_open=0;
    int mark1, mark2;
    int oldl=lowZTrkIndex;
    int x=event_x(event);
    Reg_select *near=Find_Nearest_SRegion(trc,x);

    if (!ZoomWindowMapped) {
	just_open=1;
	open_zoom_window();
    }

    /* move to new location */
					
    if(near==NULL) 
	return;	    /* no mark, ignore */

    mark1= near->left_index;
    mark2= near->right_index;

    /* remove the mark */
    UpdateInterval(itrc);
    if (ZoomWindowMapped) {Zoom_UpdateInterval(itrc);}

    RemoveSRegion(near,trc);

    UpdateInterval(itrc);
    if (ZoomWindowMapped) {Zoom_UpdateInterval(itrc);}

    for(i=lowZTrkIndex; i<= highZTrkIndex; i++)
	UpdateMarks(i); /* clean marks */
    
    if(Mode_triplet)
	itrc= trc->trip->first_trc->itrc;
    lowZTrkIndex= itrc;
    highZTrkIndex= itrc+NumZTracks-1;
    if (highZTrkIndex>LastTrack) {
	itrc= lowZTrkIndex= LastTrack-NumZTracks+1;
	highZTrkIndex= LastTrack;
    }
    if (lowZTrkIndex<0) {
	itrc= lowZTrkIndex= 0;
	highZTrkIndex= NumZTracks-1;
    }
    if (Mode_triplet)
	Trip_ZoomContentChanged(lowZTrkIndex);
    for (i=0; i < NumZTracks; i++)  {
	trc= traces[itrc];
	if (trc->wave) {
	    float restore_vs; /* quick fix ! */
	    restore_vs= trc->zaxis.vs;
	    if (i!=0) { 
		trc->zaxis.hs=0;    /*reset*/
		ScaleZTrack(trc, traces[itrc-1]);
		if(Mode_triplet)
		    checkTripBound(trc);
	    }else {
		/* use mark1, mark2: we might have changed itrc */
		trc->zaxis.ix2= mark2;
		trc->zaxis.ix1= mark1;
		if(Mode_triplet)
		    checkTripBound(trc);
		ScaleYAxis(&trc->zaxis, ZTrkHeight);
		ScaleTAxis(&trc->zaxis, trc->wave->info.sample_rate,
			   ZCvsWidth);
	    }
	    if (restore_vs!=0) {    /* quick fix-- bad!!! */
		trc->zaxis.vs= restore_vs;
	    }
	    UpdateMarks(itrc);
	    itrc++; /* next trace */
	}
    }
    if (Zoom_Mode_sameVertScale) newVertScale();
    if (Mode_trvlTime) {
	if(oldl!=lowZTrkIndex) {
	    Trvl_ZoomContentChanged();
	}else {
	    Trvl_Rescale();
	}
    }
    if (Mode_rotate && oldl!=lowZTrkIndex) {
	Rot_ZoomContentChanged();
    }
    RedrawZoomWindow("handle_z");
    if (Mode_autoScroll) {
	AutoScroll((oldl>lowZTrkIndex)?SCRL_UP:SCRL_DOWN);
	SetSbarPosition();
    }
}
