#ifndef lint
static char id[] = "$Id: zevent.c,v 1.2 2013/02/28 21:24:55 lombard Exp $";
#endif

/*
 * zevent.c--
 *    handles X events in zoom window
 *
 * Copyright (c) 1992-3 Andrew K. Yu, Lawrence Berkeley Laboratory
 *      and University of California, Berkeley.
 * All rights reserved.
 */
#include <stdio.h>
#include <strings.h>
#include "proto.h"
#include "xv_proto.h"


extern Trace **traces;
extern Track **tracks;
extern int LastTrack;
extern int lowTrkIndex;
extern int highTrkIndex;
extern char StatusString[500];
extern int Mode_align;

int subsample_rate = 1;

/* prototypes */
static void dragPick(int x);
static void zwin_place_pick(int itrz, Trace *trc, int x);
static void erase_time_line();
static void draw_time_line(int x);
static void handle_MS_MIDDLE(Xv_Window window, Event *event);
static void handle_event_ctrl_right(Event *event, int itrz, Trace *trc);
static void handle_event_ctrl_left_drag(Event *event, int itrz, Trace *trc);
static void handle_event_ctrl_left(Event *event, int itrz, Trace *trc);



static int was_cntrl=0;     /* was the cntrl key down */
static int is_drag= 0;      /* are we dragging ??? */
static int is_LeftDown= 0;  /* this is to solve a LOC_DRAG with no
			       MS_LEFT */
static int curItrz;
static Pick *curPick=NULL;

static int Drawn=0;
static char tme[30];
static int lastx;	/* prevent drag */

static int firstMark=-1;
static int oldMark_x=-1;


/*
 * setStatusString: displays the trace name, time and amplitude of
 *   the point under the cursor.
 *   Currently this is only the raw trace value; should be modified
 *   so it displays rotated trace; see rotate.c:getValue()
 */
void setStatusString(Trace *trc, int x)
{
    int idx;
    STE_TIME tm;
    BIS3_HEADER *bh= &trc->wave->info;

    idx = coordToIndex(&trc->zaxis, x, 1);
    if (trc->wave) {
	tm = sti_to_ste(indexToTime(trc, idx, 1));
	if (idx >= 0 && idx <= trc->wave->info.n_values) {
	    sprintf(StatusString,
		    "%s %s %s %s %d %3d %d:%d:%f (%d %d) Amp: %g",
		    bh->station, bh->network, bh->channel, 
		    (strlen(bh->location) > 0) ? bh->location : "--", 
		    tm.year,tm.doy,tm.hour,tm.minute,
		    (float)tm.second + (float)tm.usec/USECS_PER_SEC, 
		    idx, x, trc->wave->data[idx]);
	} else {
	    sprintf(StatusString,
		    "%s %s %s %s %d %3d %d:%d:%f (%d %d) Amp: ----",
		    bh->station, bh->network, bh->channel, 
		    (strlen(bh->location) > 0) ? bh->location : "--", 
		    tm.year,tm.doy,tm.hour,tm.minute,
		    (float)tm.second + (float)tm.usec/USECS_PER_SEC, idx,x );
	}
    }
}

/**********************************************************************
 *   Handles mouse events                                             *
 **********************************************************************/

static void dragPick(int x)
{
    int idx;
    Trace *trc;
    double cur_off;
    STI_TIME it;
		
    trc = traces[curItrz+lowZTrkIndex];

    idx = coordToIndex(&trc->zaxis, x, subsample_rate);
    it = indexToTime(trc, idx, subsample_rate);
    cur_off = st_tdiff(it,trc->wave->info.start_it);

    if (idx<0 || idx>trc->wave->info.n_values*subsample_rate)
	return;
    /* erase old line */
    if (curPick) {
	if (*curPick->label!='\0')
	    DrawZPickLabel(curPick,curItrz+lowZTrkIndex);
	DrawZPickLine(curPick, curItrz+lowZTrkIndex);
	if (InTrkRange(curItrz+lowZTrkIndex)) {
	    DrawPickLine(curPick, curItrz+lowZTrkIndex);
	}
    }
    /* move to new place */
    curPick->secOffset=cur_off;
    if (*curPick->label!='\0')
	DrawZPickLabel(curPick,curItrz+lowZTrkIndex);
    DrawZPickLine(curPick, curItrz+lowZTrkIndex);
    if (InTrkRange(curItrz+lowZTrkIndex)) {
	DrawPickLine(curPick, curItrz+lowZTrkIndex);
    }
}

static void zwin_place_pick(int itrz, Trace *trc, int x)
{
    STI_TIME it;
    int idx;
    
    curPick=trc->picks= AddPicks(trc->picks);
    curItrz=itrz;
    idx = coordToIndex(&trc->zaxis, x, subsample_rate);

    it = indexToTime(trc, idx, subsample_rate);
    trc->picks->secOffset = st_tdiff(it,trc->wave->info.start_it);

    trc->picks->uncertainty = trc->zaxis.hs / trc->wave->info.sample_rate;
    DrawZPickLine(trc->picks, itrz+lowZTrkIndex);
    if (InTrkRange(itrz+lowZTrkIndex)) {
	DrawPickLine(trc->picks, itrz+lowZTrkIndex);
    }
    SetChangesFlag();
}

void ZCvs_event_proc(Xv_Window window, Event *event)
{
    int x, y, itrz;
    Window win= (Window)xv_get(window, XV_XID);
    Trace *trc;

    x=event_x(event), y=event_y(event);
    itrz= (float)y/ZTrkHeight;

    if (itrz+lowZTrkIndex > LastTrack) return; /* no such track exists */

    trc= traces[itrz+lowZTrkIndex];
    if (event_is_ascii(event)) {
	if (event_is_up(event)) {
	    if(event_action(event)==' ') {
		if(event_middle_is_down(event))
		    zwin_place_pick(itrz,trc,x);
	    }else {
		setPickAttrb(trc->picks, itrz+lowZTrkIndex,event, x);
	    }
	}
    }else
	switch (event_action(event)) {
	case ACTION_ERASE_CHAR_BACKWARD: {
	    Pick *near, *recent, *plist;

	    if (event_is_up(event)) {
		FindNearRecent(itrz+lowZTrkIndex, &near, &recent, x);
		plist= (near!=NULL)? near :
		    ((recent!=NULL)? recent : NULL);
		if (plist) {
		    /* erase the last one */
		    if (*plist->phaseName!='\0') {
			/* erase the old one */
			DrawZPickLabel(plist,itrz+lowZTrkIndex);
			if (plist->phaseName[7]=='\0') {
			    plist->phaseName[strlen(plist->phaseName)-1]='\0';
			}else {
			    plist->phaseName[7]='\0';
			}
			/* draw again */
			SetPickLabel(plist);
			DrawZPickLabel(plist,itrz+lowZTrkIndex);
		    }
		}
	    }
	    break;
	}
	case LOC_MOVE: {
	    setStatusString(trc, x);
	    UpdateZStatus();
	    break;
	}
	case LOC_DRAG:
	    if (event_left_is_down(event)) {
		erase_time_line();
		if (event_ctrl_is_down(event) || was_cntrl) {
		    handle_event_ctrl_left_drag(event, curItrz+lowZTrkIndex, traces[curItrz+lowZTrkIndex]);
		} else {
		    dragPick(x);
		}
	    }else if (event_middle_is_down(event)) {
		erase_time_line();
		draw_time_line(x);
		setStatusString(trc, x);
		UpdateZStatus();
	    }
	    break;
	case LOC_WINENTER:
	    win_set_kbd_focus(window, xv_get(window, XV_XID));
	    UpdateZStatus();
	    break;
	case LOC_WINEXIT:
#if 0
	    bzero(StatusString, 500);
	    CleanZStatus();
#endif
	    break;
	case ACTION_SELECT:
	case MS_LEFT: {
	    if(event_is_down(event)) {
		if(event_ctrl_is_down(event) ||was_cntrl) {
		    if (!is_drag) curItrz=itrz;
		    handle_event_ctrl_left(event, curItrz+lowZTrkIndex,traces[curItrz+lowZTrkIndex] );
		}else {

		    Pick *near, *recent;
		
		    FindNearRecent(itrz+lowZTrkIndex,
				   &near, &recent, x);
		    if(near!=NULL) {
			curPick= near;
			curItrz= itrz;
			dragPick(x);
		    }else {
			zwin_place_pick(itrz,trc,x);
		    }
		}
	    }else {
		if(event_ctrl_is_down(event) ||was_cntrl) {
		    handle_event_ctrl_left(event, curItrz+lowZTrkIndex, traces[curItrz+lowZTrkIndex]);
		}else {
		    curPick=NULL;
		}
	    }
	    break;
	}
	case ACTION_ADJUST:
	case MS_MIDDLE:
	    if(event_ctrl_is_down(event)) {
		if(event_is_up(event)) open_sp_panel();
	    }else {
		handle_MS_MIDDLE(window,event);
	    }
	    break;
	case ACTION_MENU:
	case MS_RIGHT: {		/* cancel pick */
	    if (event_is_up(event)) {
		if(event_ctrl_is_down(event) || was_cntrl) {
		    curItrz=itrz;
		    handle_event_ctrl_right(event,curItrz+lowZTrkIndex , trc);
		}else {
		    int samp_x;
		    Pick *p, *recent, *near, *rprev, *nprev, *prev;

		    p= trc->picks;
		    rprev=nprev=prev=recent=near=NULL;
		    while (p!=NULL) {
			if (p->type==PICK) {
			    if (recent==NULL) {
				recent= p; rprev=prev;
			    }
			    samp_x= timeToCoord(&trc->zaxis, trc, p->secOffset);
			    if (near==NULL && x-10<= samp_x
				&& samp_x <= x+10) {
				near= p; nprev=prev;
				break;
			    }
			}
			prev=p;
			p=p->next;		/* skip over L, R marks */
		    }
		    p=NULL;
		    if (near!=NULL) {
			p= near; prev=nprev;
		    }else if (recent!=NULL) {
			p= recent; prev=rprev;
		    }
		    if (p!=NULL) {
			SetChangesFlag();
			DrawZPickLine(p, itrz+lowZTrkIndex);
			if (InTrkRange(itrz+lowZTrkIndex)) {
			    DrawPickLine(p, itrz+lowZTrkIndex);
			}
			if (*p->label!='\0') DrawZPickLabel(p, itrz+lowZTrkIndex);
			if (p==trc->picks) {
			    trc->picks= trc->picks->next;
			    free(p);
			}else {
			    prev->next=p->next;
			    free(p);
			}
		    }
		}
		break;
	    }
	}
	default:
	    return;
	}
}

/* this emulates the UW style of picking */
void ZCvs_event_proc2(Xv_Window window, Event *event)
{
    int x, y, itrz;
    Window win= (Window)xv_get(window, XV_XID);
    Trace *trc;
    STI_TIME ptime;
  
    x=event_x(event), y=event_y(event);
    itrz= (float)y/ZTrkHeight;

    if (itrz+lowZTrkIndex > LastTrack) return; /* no such track exists */
    trc= traces[itrz+lowZTrkIndex];

    if (event_is_ascii(event)) {
	if (event_is_up(event)) {
	    /* recent or near inside setPickAttrb */
	    /* following appears incorrect - wrong arguments: 
	     * setPickAttrb2(trc->picks, itrz+lowZTrkIndex, event_action(event),x);
	     */
	    setPickAttrb2(trc->picks, itrz+lowZTrkIndex, event, 
			  event_action(event), x);

	}
    } else {
	switch (event_action(event)) {
	case LOC_MOVE: {
	    setStatusString(trc, x);
	    UpdateZStatus();
	    break;
	}
	case LOC_DRAG:
	    if (event_left_is_down(event)) {
		erase_time_line();
		if (event_ctrl_is_down(event) || was_cntrl) {
		    handle_event_ctrl_left_drag(event, curItrz+lowZTrkIndex,
						traces[curItrz+lowZTrkIndex] );
		}
	    }else if (event_middle_is_down(event)) {
		erase_time_line();
		draw_time_line(event_x(event));
		setStatusString(trc, event_x(event));
		UpdateZStatus();
	    }
	    break;
	case LOC_WINENTER:
	    UpdateZStatus();
	    break;
	case LOC_WINEXIT:
	    /*	    bzero(StatusString, 500);
		    CleanZStatus();*/
	    break;
	case ACTION_SELECT:
	case MS_LEFT: {	/* add pick */
	    int idx;
	    if(event_is_down(event)) {
		if(event_ctrl_is_down(event) ||was_cntrl) {
		    if (!is_drag) curItrz=itrz;
		    handle_event_ctrl_left(event, curItrz+lowZTrkIndex, 
					   traces[curItrz+lowZTrkIndex]);
		}
	    }else if(event_is_up(event)) {
		if(event_ctrl_is_down(event) ||was_cntrl) {
		    handle_event_ctrl_left(event, curItrz+lowZTrkIndex, 
					   traces[curItrz+lowZTrkIndex]);
		}else {
		    float srate;
	  
		    /* check if recent pick is unnamed and move it
		       if so. */
		    srate= trc->wave->info.sample_rate;   
		    idx = coordToIndex(&trc->zaxis, x, subsample_rate);
		    if (trc->picks && trc->picks->type==PICK &&
			trc->picks->phaseName[0]=='\0'){
			/* erase it */
			DrawZPickLine(trc->picks, itrz+lowZTrkIndex);
			if (InTrkRange(itrz+lowZTrkIndex)) {
			    DrawPickLine(trc->picks, itrz+lowZTrkIndex);
			}
		    }else {
			/* add new pick */
			trc->picks= AddPicks(trc->picks);
			SetChangesFlag();
		    }
		    ptime = indexToTime(trc, idx, subsample_rate);
		    trc->picks->secOffset = st_tdiff(ptime, trc->wave->info.start_it);
		    trc->picks->uncertainty= (trc->zaxis.hs < 1)?
			(float)1/srate : trc->zaxis.hs / srate;
		    DrawZPickLine(trc->picks, itrz+lowZTrkIndex);
		    if (InTrkRange(itrz+lowZTrkIndex)) {
			DrawPickLine(trc->picks, itrz+lowZTrkIndex);
		    }
		}
		break;
	    }
	}
	case ACTION_ADJUST:
	case MS_MIDDLE:	    /* no function */
	    if(event_ctrl_is_down(event)) {
		if(event_is_up(event)) open_sp_panel();
	    }else {
		handle_MS_MIDDLE(window,event);
	    }
	    break;
	case ACTION_MENU:
	case MS_RIGHT: {	/* cancel pick */
	    if (event_is_up(event)) {
		if(event_ctrl_is_down(event) || was_cntrl) {
		    curItrz=itrz;
		    handle_event_ctrl_right(event,curItrz+lowZTrkIndex, trc);
		}else {
		    if (event_is_up(event)) {
			int samp_coord;
			Pick *p, *recent, *near, *rprev, *nprev, *prev;
	    
			p= trc->picks;
			recent=near=NULL;
			while (p!=NULL) {
			    if (p->type==PICK) {
				samp_coord = timeToCoord(&trc->zaxis, trc, 
							 p->secOffset);
				if (near==NULL && x-5 <= samp_coord
				    && samp_coord <= x+5) {
				    near= p; nprev=prev;
				    break;
				}
				if (recent==NULL) {
				    recent= p; rprev=prev;
				}
			    }
			    prev=p;
			    p=p->next;	/* skip over L, R marks */
			}
	    
			p=NULL;
			if (near!=NULL) {
			    p= near; prev=nprev;
			}else if (recent!=NULL) {
			    p= recent; prev=rprev;
			}
			if (p!=NULL) {
			    SetChangesFlag();
			    DrawZPickLine(p, itrz+lowZTrkIndex);
			    if (InTrkRange(itrz+lowZTrkIndex)) {
				DrawPickLine(p, itrz+lowZTrkIndex);
			    }
			    if (*p->label!='\0') 
				DrawZPickLabel(p, itrz+lowZTrkIndex);
			    if (p==trc->picks) {
				trc->picks= trc->picks->next;
				free(p);
			    }else {
				prev->next=p->next;
				free(p);
			    }
			}
		    }
		    break;
		}
	    }
	default:
	    return;
	}
	}
    }
}

/**********************************************************************
 *   Time line (middle button)                                        *
 **********************************************************************/

static void erase_time_line()
{
    if (Drawn) {
	/* erase */
	XDrawLine(theDisp, ZCvsWin, ZCvsXorGC, lastx, 0, lastx, ZCvsHeight);
	XDrawString(theDisp, ZCvsWin, ZCvsXorGC, lastx+5, 10,
		    tme, strlen(tme));
	XDrawString(theDisp, ZCvsWin, ZCvsXorGC, lastx+5, ZCvsHeight-4,
		    tme, strlen(tme));
	XFlush(theDisp);
	Drawn=0;
    }
}

static void draw_time_line(int x)
{
    int ok,i,idx;
    STI_TIME common;
    Trace *trc;

    trc= traces[lowZTrkIndex];
    ok=0;

    if (Mode_align==2) {
	double c_off_sec;

	bzero(&common, sizeof(STI_TIME));
	ok=1;
	idx= coordToIndex(&trc->zaxis, x, 1);
	c_off_sec = (double)idx/trc->wave->info.sample_rate;
	common = st_add_dtime(common,c_off_sec * USECS_PER_SEC);

	/* check to make sure all have the same sec_off */
	for(i=lowZTrkIndex+1; i<=highZTrkIndex; i++) {
	    double off_sec,dt,sdiff;

	    dt = (double)1.0/trc->wave->info.sample_rate;
	    idx = coordToIndex(&trc->zaxis, x, 1);
	    off_sec = (double)idx/trc->wave->info.sample_rate;
	    sdiff=off_sec - c_off_sec;

	    if (sdiff > dt || sdiff < -dt) { /* off by at least a sample */
		ok=0;
		break; /* forget it! */
	    }
	}
    } else {
	if (trc && trc->wave) {
	    idx = coordToIndex(&trc->zaxis, x, 1);
	    common = indexToTime(trc, idx, 1);
	    ok=1;
	}
#ifdef DEBUG    
	printf("common time: ");printTime(common);printf("\n");
#endif    
	/* check if time is the same for all traces */
	for(i=lowZTrkIndex+1; i<=highZTrkIndex; i++) {
	    STI_TIME t;
	    double sdiff, dt;
	    trc= traces[i];
	    if (trc && trc->wave) {
		dt = (double)1.0/trc->wave->info.sample_rate;
		idx= coordToIndex(&trc->zaxis, x, 1);
		t= indexToTime(trc, idx, 1);
#ifdef DEBUG
		printf("%d time: ",i);printTime(t);printf("\n");
#endif
		sdiff= st_tdiff(t, common);
		if (sdiff > dt || sdiff < -dt) { /* off by at least a sample */
		    ok=0;
		    break; /* forget it! */
		}
	    }
	}
    }
    if (ok) {	/* common time at a coord */
	STE_TIME et = sti_to_ste(common);
	XDrawLine(theDisp, ZCvsWin, ZCvsXorGC, x, 0, x, ZCvsHeight);
	sprintf(tme, "%d:%d:%.3f", et.hour, et.minute,
		(float)et.second + (float)et.usec / USECS_PER_SEC);
	XDrawString(theDisp, ZCvsWin, ZCvsXorGC, x+5, 10,
		    tme, strlen(tme));
	XDrawString(theDisp, ZCvsWin, ZCvsXorGC, x+5, ZCvsHeight-4,
		    tme, strlen(tme));
	XFlush(theDisp);
	Drawn=1;
	lastx=x;
    }
}

static void handle_MS_MIDDLE(Xv_Window window, Event *event)
{
    if (event_is_down(event)) {
	draw_time_line( event_x(event) );
    }else { /* event is up */
	erase_time_line();
    }
}


/* these next events deal with the creation and destruction            *
 * of selection regions                                                *
 * 1. to destroy a region the cntrl-right-mouse-button is pressed      *
 *       and the code finds the nearest selection in the trace and     *
 *       removes it from the list                                      *
 * 2. to create and modify a region  the cntrl-left-mouse-button       *
 *       is a. pressed and released or b. pressed and held and release *
 * on pressing the initial location of the region is defined           *
 * on releasing the region is then created and stored in the list      *
 * on holding the size of the region is manipulated                    */



/* remove selected region */
static void handle_event_ctrl_right(Event *event, int itrz, Trace *trc)
{
    int x=event_x(event);
    Reg_select *near=Find_Nearest_SRegion(trc,x);

    /* erase all */
    UpdateInterval(itrz);
    Zoom_UpdateInterval(itrz);

    if (near!=NULL) {
	RemoveSRegion(near,trc);
    }

    /* redraw without */
    UpdateInterval(itrz);
    Zoom_UpdateInterval(itrz);
    was_cntrl=0;
}


/* expand or contract select region */
static void handle_event_ctrl_left_drag(Event *event, int itrz, Trace *trc)
{
    int x, y;
    int idx, len, i;
    Track *trk=tracks[itrz-lowTrkIndex];
    int unz_x,unz_y;

    was_cntrl=1;

    x=event_x(event), y=event_y(event);
    if (event_is_down(event)) { 
	if(is_drag) {
	    unz_x=indexToCoord(&trc->axis, 
			       coordToIndex(&trc->zaxis, oldMark_x, 1), 1);
	    unz_y=indexToCoord(&trc->axis, 
			       coordToIndex(&trc->zaxis, firstMark, 1), 1);
	    Zoom_DrawInterval(itrz,oldMark_x,firstMark,trk->ovrGC); 
	    DrawInterval(itrz,unz_x,unz_y,trk->ovrGC); 
	}else {
	    /* this piece of code is here to cure LOC_DRAG
	       events with no preceding MS_LEFT. */
	    if(!is_LeftDown) {
		firstMark=x;
	    }
	}
	/* final mark is x */
	unz_y =indexToCoord(&trc->axis,
			    coordToIndex(&trc->zaxis,x,1),1);
	unz_x = indexToCoord(&trc->axis,
			     coordToIndex(&trc->zaxis,firstMark,1),1);
	Zoom_DrawInterval(itrz,firstMark,x,trk->ovrGC);
	DrawInterval(itrz,unz_x,unz_y,trk->ovrGC); 
	oldMark_x=x;
	is_drag= 1;
    }
}

/* creates new regions */
static void handle_event_ctrl_left(Event *event, int itrz, Trace *trc)
{
    int x, y;
    int idx, len, i; 
    int store=1;
    int unz_x,unz_y;
    Track *trk=tracks[itrz-lowTrkIndex];

    was_cntrl=1;

    x=event_x(event), y=event_y(event);

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
	pos1=coordToIndex( &trc->zaxis, firstMark,1);


	if(is_drag) {
	    /* clear temporary drawing */
	    unz_x=indexToCoord(&trc->axis,
			       coordToIndex(&trc->zaxis,oldMark_x,1),1);
	    unz_y=indexToCoord(&trc->axis,
			       coordToIndex(&trc->zaxis,firstMark,1),1);
	    Zoom_DrawInterval(itrz-lowTrkIndex,oldMark_x,firstMark,trk->ovrGC);
	    DrawInterval(itrz,unz_x,unz_y,trk->ovrGC); 
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
	    pos2= coordToIndex( &trc->zaxis, x, 1);
	}

	/* which way do we go */
	if (pos1<pos2) {
	    new->right_index=pos2;
	    new->left_index=pos1;
	} else if (pos1>pos2) {
	    new->right_index=pos1;
	    new->left_index=pos1;
	} else {
	    store=0;
	}

	/* this point is in the window */
	if (new->left_index>=trc->wave->info.n_values) new->left_index=trc->wave->info.n_values-1;
	if (new->right_index<0) new->right_index=0;
	if (new->left_index<0)new->left_index=0;
	if (new->right_index>=trc->wave->info.n_values) new->right_index=trc->wave->info.n_values-1;
	/* again reverse indexes if needed */
	if (new->right_index<new->left_index) {
	    int t_pos=new->right_index;
	    new->right_index=new->left_index;
	    new->left_index=t_pos;
	}

	Zoom_UpdateInterval( itrz-lowTrkIndex ); /* erase regions */
	UpdateInterval(itrz);

	if (store==1 && new->left_index<new->right_index) {
	    /*      DrawInterval(itrz,new->left_index,new->right_index,trk->ovrGC);*/
	    Insert_SRegion(new,trc);
	} else {
	    /* if region is of no length do not store it */
	    free(new);
	    new=NULL;
	}

	UpdateInterval( itrz ); /* redraw regions */
	Zoom_UpdateInterval(itrz-lowTrkIndex);

	/* reset flags etc */
	oldMark_x=-1;
	is_drag= 0;			/* reset drag */
	is_LeftDown= 0;
    }
}
