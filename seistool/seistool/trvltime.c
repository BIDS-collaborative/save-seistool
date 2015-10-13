#ifndef lint
static char id[] = "$Id: trvltime.c,v 1.2 2013/02/28 21:24:56 lombard Exp $";
#endif

/*
 * trvltime.c--
 *    travel time curves
 *
 * Copyright (c) 1992-3 Andrew K. Yu, Lawrence Berkeley Laboratory
 *      and University of California, Berkeley.
 * All rights reserved.
 */
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <xview/xview.h>
#include <xview/xv_xrect.h>
#include <xview/panel.h>
#include "mathlib.h"
#include "proto.h"
#include "xv_proto.h"

static float evt_lat =0.0, evt_lon =0.0;
static STI_TIME evt_ot;
static int old_x= -1;
static int cur_itrc=-1;
static TtInfo *curTtInfo= NULL;
static int cur_iphs= -1;
static int isdrag= 0;
static float sec_per_pixel;

int Mode_trvlTime = 0;

extern Trace **traces;
extern int LastTrack;
extern int Mode_triplet;
extern int tt_button_picking;

#define VICINITY    5



/* internal function prototypes */
static TtInfo *NewTtInfo();
static void GetTtInfo(TtInfo *ttinfo);
static void PlotTrvlTime(TtInfo *ttinfo);
static int NearPhase(TtInfo *ttinfo, int x);
static void SetOriginTime(TtInfo *curTtInfo);
static void resetParameters(TtInfo *curTtInfo);
static void SetPhaseList(TtInfo *curTtInfo);
static int strcmp_f(char *s1, char *s2);
static void LogPhase(int x);



static TtInfo *NewTtInfo()
{
    TtInfo *new;
    new= (TtInfo *)Malloc(sizeof(TtInfo));
    if(new) bzero(new, sizeof(TtInfo));
    return new;
}
    
static void GetTtInfo(TtInfo *ttinfo)
{
    float *t;
    char *phcd;
    int n, num;
    Phase *phs;
    float g_phase;
    int g_phase_assigned= 0;
    int s_phase_seen=0;

    /* special for rick (use with caution. good for oceanic G only) */
    g_phase= ttinfo->delta/0.040377;
    GetTrvlTimes(ttinfo->depth, ttinfo->delta, ttinfo->which, &n, &t, &phcd);
    ttinfo->phs= phs= (Phase *)Malloc(sizeof(Phase)*(n+1));
    num=0;
    for( ; n > 0; n--) {
	int i;
	if(!g_phase_assigned && *t>g_phase) {
	    /* insert G phase before the later phase; also make sure
	       it comes after the S phase */
	    if(s_phase_seen) {
		strcpy(phs->name,"G");
		phs->tt= g_phase;
		num++;
		phs++;
	    }
	    /* if we haven't seen an S, forget about G */
	    g_phase_assigned= 1;
	}
	strncpy(phs->name, phcd, 8);
	for(i=7;i>=0 && phs->name[i]==' ';i--)
	    phs->name[i]='\0';
	if(phcd[0]=='s'||phcd[0]=='S')
	    s_phase_seen=1;
	phcd+=8;
	phs->tt= *t++;
	num++;
	phs++;
    }
    ttinfo->numPhs= num;
    return;
}


static void PlotTrvlTime(TtInfo *ttinfo)
{
    int i;
    Phase *phs= ttinfo->phs;
    for(i=0; i< ttinfo->numPhs; i++) {
	int x= phs[i].x;
	if(phs[i].active && x!=-1) {
	    XDrawLine(theDisp, ZCvsWin, ZCvsXorGC, x, 0, x, ZCvsHeight);
	    XDrawString(theDisp, ZCvsWin, ZCvsXorGC, x+2, 10,
			phs[i].name, strlen(phs[i].name));
	}
    }
    XFlush(theDisp);
}

/* return index of phase near vicinity; -1 if failed */
static int NearPhase(TtInfo *ttinfo, int x)
{
    int idx, i;
    Phase *p= ttinfo->phs;

    for(i=0; i< ttinfo->numPhs; i++) {
	/* check each of them */
	if (p[i].x-VICINITY<= x && x<= p[i].x+VICINITY) {
	    /* found one */
	    return i;
	}
    }
    return -1;
}


static void SetOriginTime(TtInfo *curTtInfo)
{
    Phase *phs= curTtInfo->phs;

    if(curTtInfo->numPhs>0) {
	STI_TIME ot_it;
	Trace *trc= traces[cur_itrc];
	double sdiff;
	old_x= phs[0].x;
	sdiff = (double)(old_x*sec_per_pixel + trc->zaxis.ix1/
	    trc->wave->info.sample_rate);
	sdiff -= (double)phs[0].tt;
	ot_it = st_add_dtime(trc->wave->info.start_it, sdiff * USECS_PER_SEC);

	tt_updateOTitem(ot_it);
	curTtInfo->ot_it= ot_it;
    }
}

static void resetParameters(TtInfo *curTtInfo)
{
    Phase *phs, *old_phs;
    int old_numPhs;
    int x, i, j, last;

    old_numPhs= curTtInfo->numPhs;
    old_phs= curTtInfo->phs;
    
    GetTtInfo(curTtInfo);
    phs= curTtInfo->phs;

    x= (old_x==-1)?ZCvsWidth/2:old_x;
    sec_per_pixel= traces[cur_itrc]->zaxis.hs /
	traces[cur_itrc]->wave->info.sample_rate;
    /* assume ot half way */
    x -= iround(phs[0].tt/sec_per_pixel);
    for(i=0; i < curTtInfo->numPhs; i++) {
	phs[i].x= x + iround(phs[i].tt/sec_per_pixel);
	phs[i].active= 0;
    }

/*
 * we only want the P, S and previous active phases now. (2/12/93 AY)
 * the two loops here are kind of inefficient, but forget it for now.
 */
    for(i=0; i< curTtInfo->numPhs; i++) {
	if (!strcmp(phs[i].name,"P") || !strcmp(phs[i].name,"S"))
		phs[i].active=1;
    }
    for(i=0, last=j=0; i<old_numPhs; i++) {
	if(old_phs[i].active) {
	    while (j< curTtInfo->numPhs) {
		if(!strcmp(old_phs[i].name,phs[j].name)) {
                    last= j;		
		    phs[j++].active=1;
		    break;
		}
		j++;
	    }
	    j= last;	
	    if (j==curTtInfo->numPhs) break;
	}
    }
    free(old_phs);
    
    SetOriginTime(curTtInfo);
    SetPhaseList(curTtInfo);
}

void ShowPhase(int iphs)
{
    Phase *phs=curTtInfo->phs;
    int x= phs[iphs].x;
    if (!phs[iphs].active && x!=-1) {
	XDrawLine(theDisp, ZCvsWin, ZCvsXorGC, x, 0, x, ZCvsHeight);
	XDrawString(theDisp, ZCvsWin, ZCvsXorGC, x+2, 10,
		phs[iphs].name, strlen(phs[iphs].name));
    }
    phs[iphs].active=1;
}

void UnshowPhase(int iphs)
{
    Phase *phs=curTtInfo->phs;
    int x= phs[iphs].x;
    if (phs[iphs].active && x!=-1) {
	XDrawLine(theDisp, ZCvsWin, ZCvsXorGC, x, 0, x, ZCvsHeight);
	XDrawString(theDisp, ZCvsWin, ZCvsXorGC, x+2, 10,
		phs[iphs].name, strlen(phs[iphs].name));
    }
    phs[iphs].active=0;
}

static void SetPhaseList(TtInfo *curTtInfo)
{
    Phase *phs=curTtInfo->phs;
    int x, i, prev;
    extern Panel_item tt_phs_list;

    prev= (int)xv_get(tt_phs_list, XV_KEY_DATA, 33);
    if(prev>0) {
	xv_set(tt_phs_list, PANEL_LIST_DELETE_ROWS, 0, prev,
	       NULL);
    }

    for(i=0; i < curTtInfo->numPhs; i++) {
	int p= curTtInfo->numPhs-i-1;
	xv_set(tt_phs_list, PANEL_LIST_INSERT, 0,
	       PANEL_LIST_STRING, 0, phs[p].name,
	       PANEL_LIST_CLIENT_DATA, 0, p,
	       PANEL_LIST_SELECT, 0, (phs[p].active)?TRUE:FALSE,
	       NULL);
    }
    xv_set(tt_phs_list, XV_KEY_DATA, 33, curTtInfo->numPhs, NULL);
}

void Trvl_Rescale()
{
    STI_TIME ot_it;
    STI_TIME zfst, pht;
    Trace *trc= traces[cur_itrc];
    int i;
    Phase *phs= curTtInfo->phs;

    ot_it= curTtInfo->ot_it;
    /* new sec_per_pixel */
    sec_per_pixel= trc->zaxis.hs/trc->wave->info.sample_rate;
    zfst = st_add_dtime(trc->wave->info.start_it, 
			(double)trc->zaxis.ix1 * USECS_PER_SEC /
			trc->wave->info.sample_rate);
    for(i=0; i<curTtInfo->numPhs; i++) {
	phs[i].x= -1;
    }
    for(i=0; i<curTtInfo->numPhs; i++) {
	double sdiff;
	pht= st_add_dtime(ot_it, (double)phs[i].tt * USECS_PER_SEC);
	sdiff= st_tdiff(pht,zfst);
	if(sdiff>0.0) {
	    int x = iround(sdiff/sec_per_pixel);
	    if(x>ZCvsWidth) break;
	    phs[i].x = x;
	}
    }
		  
}

void Trvl_ZoomContentChanged()
{
    Trace *trc= traces[lowZTrkIndex];
    TtInfo *ttinfo;
    float lat,lon;
    char *inp_v;
    extern Panel_item tt_del_txt,tt_dep_txt;
    
    cur_itrc= lowZTrkIndex;
    curTtInfo= ttinfo= trc->trip->ttinfo;

    if(!ttinfo || ttinfo->delta != trc->wave->info.event_delta ) {
	curTtInfo= ttinfo= NewTtInfo();
	lat=trc->wave->info.event_latitude;
	lon=trc->wave->info.event_longitude;

	if (lat==F4FLAG || lon==F4FLAG ||trc->wave->info.event_delta >180 || trc->wave->info.event_delta <0) {
	  inp_v=(char *)xv_get(tt_del_txt,PANEL_VALUE);
	  if (strlen(inp_v)>0 && atof(inp_v)>0 && atof(inp_v)<180) {
	    ttinfo->delta=atof(inp_v);
	   } else {
	     ttinfo->delta= 70.0;	/* arbitrary */
	   }
	} else {
	  ttinfo->delta= trc->wave->info.event_delta;
	}

	if (trc->wave->info.event_depth>=0 && trc->wave->info.event_depth<2000) {
	  ttinfo->depth=trc->wave->info.event_depth;
	} else {
	  inp_v=(char *)xv_get(tt_dep_txt,PANEL_VALUE);
	  if (strlen(inp_v)>0 && atof(inp_v)<2000 && atof(inp_v)>=0) {
	    ttinfo->depth=atof(inp_v);
	   } else {
	     ttinfo->depth= 33.0;	/* initial depth */ 
	   }
	}

	trc->trip->ttinfo= ttinfo;
	ttinfo->which= "basic";

	resetParameters(ttinfo);
    }else {
	SetPhaseList(ttinfo);
	Trvl_Rescale();
    }

    tt_SetStartDepth(ttinfo->depth);
    tt_SetStartDelta(ttinfo->delta);

    if(evt_lat!=0. || evt_lon!=0.) {
	curTtInfo->ot_it = evt_ot;
	tt_ChangeEvtLoc(evt_lat, evt_lon);
    }
}


void EnterTTMode()
{
    if (Mode_triplet==0)
	StartTripletMode();	
    if(!traces[lowZTrkIndex]->wave) {
/*	EndTripletMode(); */
	return;	    /* at least one trace to start this up */
    }
    Mode_trvlTime= 1;	/* start triplet mode before doing this! */
    if(tt_button_picking)tt_togglePicking();
    open_tt_panel();
    Trvl_ZoomContentChanged();
    PlotTrvlTime(curTtInfo);
    SwitchTTZevtProc();
}

void ExitTTMode()
{
    PlotTrvlTime(curTtInfo);
    Mode_trvlTime= 0;
    RestoreZevtProc();
/*    EndTripletMode();	    /* do Mode_trvlTime=0 first */
    close_tt_panel();
}

/* 
 * LEFT BUT: translate
 * MIDDLE BUT: 
 * RIGHT BUT: remove phase.
 */
static int strcmp_f(char *s1, char *s2)
{
    char s[9];
    int i;
    bzero(s,9);
    strncpy(s,s1,8);
    for(i=7;i>=0 && s[i]==' '; i--)
	s[i]='\0';
    return (strcmp(s,s2));
}

static void LogPhase(int x)
{
    Phase *phs;
    int diff,i;

    phs= curTtInfo->phs;
    diff=x-phs[cur_iphs].x;
    for(i=0; i< curTtInfo->numPhs; i++) {
	if(phs[i].x!=-1) phs[i].x += diff;
    }
    
}

void ChangeDelta(float diff)
{
    int x,i;
    Phase *phs;
    x= curTtInfo->phs[0].x - curTtInfo->phs[0].tt/sec_per_pixel;
    curTtInfo->delta+= diff;
    GetTtInfo(curTtInfo);
    phs= curTtInfo->phs;
    for(i=0; i < curTtInfo->numPhs; i++) {
	phs[i].x= x+ phs[i].tt/sec_per_pixel;
    }
}

void replotTrvlTime()
{
    PlotTrvlTime(curTtInfo);
}

void TTMode_Zevent_proc(Xv_Window window, Event *event)
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
	    TtInfo oldttinfo;
	    /* no key board actions yet */
	    switch(event_action(event)) {
	    case '+':
		oldttinfo= *curTtInfo;
		ChangeDelta(5.0);
		PlotTrvlTime(&oldttinfo);   /* erase */
		PlotTrvlTime(curTtInfo);
		break;
	    case '-':
		oldttinfo= *curTtInfo;
		ChangeDelta(-5.0);
		PlotTrvlTime(&oldttinfo);   /* erase */
		PlotTrvlTime(curTtInfo);
		break;
	    }
	}
    }else
	switch (event_action(event)) {
	case LOC_MOVE: {
	    setStatusString(trc, x);	/* from zevent.c */
	    UpdateZStatus();
	    break;
	}
	case LOC_DRAG:
	    if (event_left_is_down(event)) {
		if(isdrag) {
		    PlotTrvlTime(curTtInfo);    /* erase */
		}
		LogPhase(x);
		PlotTrvlTime(curTtInfo);
		isdrag=1;
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
	    if(event_is_down(event)) {
		int iphs= NearPhase(curTtInfo,x);
		if(iphs!=-1) {
		    PlotTrvlTime(curTtInfo);    /* erase */
		    cur_iphs= iphs;
		    isdrag=0;	/* prepare for drag */
		}
	    }else {
		if(isdrag) {
		    PlotTrvlTime(curTtInfo);    /* erase */
		}
		LogPhase(x);
		SetOriginTime(curTtInfo);
		PlotTrvlTime(curTtInfo);
	    }
	    break;
	default:
	    return;
	}
}



void tt_ChangeOT(char *otStr)
{
    int hr, min;
    float sec;
    STI_TIME ot_it;
    STE_TIME ot_et;
    
    ot_it= curTtInfo->ot_it;
    ot_et = sti_to_ste(ot_it);
    
    sscanf(otStr,"%d:%d:%f",&hr,&min,&sec);
    /* checks */
    if( !(hr<0 || hr>23 || min<0 || min>59 || sec<0 || sec>60) ) {
	ot_et.hour= hr;
	ot_et.minute = min;
	ot_et.second = (int)sec;
	ot_et.usec = ((int)sec-sec) * USECS_PER_SEC;
	ot_it = ste_to_sti(ot_et);
	curTtInfo->ot_it= ot_it;
    }
    tt_updateOTitem(ot_it);
    Trvl_Rescale();
}

void tt_ChangeDelta(float del)
{
    PlotTrvlTime(curTtInfo);	/* erase */
    
    if (del<0) del=0;
    if (del >180) del=180;

    curTtInfo->delta= del;
    resetParameters(curTtInfo);
    PlotTrvlTime(curTtInfo);

}

void tt_ChangeDepth(float dep)
{
  PlotTrvlTime(curTtInfo);	/* erase */

  if(dep<0) dep=0;
  if (dep>=2000) dep=2000;

  curTtInfo->depth= dep;
  resetParameters(curTtInfo);
  PlotTrvlTime(curTtInfo);
}


void tt_ChangeEvtLoc(float slat, float slon)
{
    Trace *trc = traces[cur_itrc];
    float rlat, rlon;
    float del,azi,bazi;
    STI_TIME ot_it;

    evt_lat = slat;
    evt_lon = slon;
    evt_ot = curTtInfo->ot_it;  /* maybe shouldn't be here... */
    rlat = trc->wave->info.latitude;
    rlon = trc->wave->info.longitude;
    if(rlat==0.0 && rlon==0.0) {
	fprintf(stderr,"Did you do \"read coord\" yet?\n");
	return;
    }
    azimuth(slat,slon,rlat,rlon,&del,&azi,&bazi);
/*    printf("%f %f delta=%f azi=%f bazi=%f\n",slat,slon,del,azi,bazi); */
    ot_it= curTtInfo->ot_it;
    tt_ChangeDelta(del);
    curTtInfo->ot_it= ot_it;
    tt_updateDelTxtitem(del);
    tt_updateOTitem(ot_it);
    Trvl_Rescale();
}

void AllPhases()
{
    PlotTrvlTime(curTtInfo);	/* erase */
    curTtInfo->which="ALL";
    resetParameters(curTtInfo);
    PlotTrvlTime(curTtInfo);
}

void BasicPhases()
{
    PlotTrvlTime(curTtInfo);	/* erase */
    curTtInfo->which="basic";
    resetParameters(curTtInfo);
    PlotTrvlTime(curTtInfo);
}

