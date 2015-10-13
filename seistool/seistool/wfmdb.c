#ifndef lint
static char id[] = "$Id: wfmdb.c,v 1.2 2013/02/28 21:24:56 lombard Exp $";
#endif

/*
 * wfmdb.c--
 *    the "waveform database" (unfinished)
 *
 * Copyright (c) 1992-3 Andrew K. Yu, Lawrence Berkeley Laboratory
 *      and University of California, Berkeley.
 * All rights reserved.
 */
#include <stdio.h>
#include <xview/xview.h>
#include <xview/frame.h>
#include <xview/canvas.h>
#include <xview/panel.h>
#include <xview/notice.h>
#include "proto.h"
#include "xv_proto.h"

/**********************************************************************
 *   Globals                                                          *
 **********************************************************************/
EqEvent *Wdb_eqEvent= NULL;
Btree *StationNameIndex= NULL;

extern int NumTracks;
extern int LastTrack;

/* internal function prototypes */
static void Wdb_file_notify(Panel_item item, Event *event);
static char **read_stn_name(char *fname, int *pnum);
static void stn_load_wfm(BtreeNode **stn_idx, char *stn_name);
static void stn_file_notify(Panel_item item, Event *event);
static void Wdb_LoadFile();
static void InitWdbPanel(Frame frame);
static void update_wdb_stn_list();


/**********************************************************************
 *   WdbPanel                                                         *
 **********************************************************************/

static Frame Wdb_frame= (Frame)NULL;
static Panel_item Wdb_list_i, Wdb_file_txt, stn_file_txt;

static void Wdb_file_notify(Panel_item item, Event *event)
{
    FILE *fp;
    char *fname= (char *)xv_get(Wdb_file_txt,PANEL_VALUE);

    if(*fname=='\0') return;
    
    if ((fp=fopen(fname,"r"))==NULL) {
	ReportError("Cannot open event list file %s.\n",
		fname);
	return;
    }
    Wdb_eqEvent= (EqEvent *)ParseElfile(fp,defaultFormat);
    xv_set(Wdb_frame,FRAME_BUSY, TRUE, NULL);
    PreviewEvent();
    handle_hashStationNames();
    update_wdb_stn_list();
    xv_set(Wdb_frame,FRAME_BUSY, FALSE, NULL);
}

static char **read_stn_name(char *fname, int *pnum)
{
    char buf[500];
    char **arr;
    int max, i;
    FILE *fp;
    if ((fp=fopen(fname,"r"))==NULL) {
	ReportError("Cannot open event list file %s.\n",
		fname);
	return NULL;
    }
    max= 25;
    arr= (char **)Malloc(sizeof(char *)*max);
    if(!arr) return NULL;
    i=0;
    while(fgets(buf,500,fp)!=NULL) {
	arr[i]= strncpy(Malloc(strlen(buf)),buf,strlen(buf)-1);   /* take the \n out */
	arr[i++][strlen(buf)-1]='\0';
	if(i==max) {
	    max*=2;
	    arr= (char **)Realloc(arr, sizeof(char *)*max);
	    if(!arr) return NULL;
	}
    }
    fclose(fp);
    *pnum=i;
    return arr;
}

static void stn_load_wfm(BtreeNode **stn_idx, char *stn_name)
{
    int i, num= StationNameIndex->numElt;
    wfmTuple **wfm;
    int found=0;

    for(i=0;i<num;i++) {
	int j,numwfm;
	numwfm= stn_idx[i]->numVal;
	wfm= (wfmTuple **)stn_idx[i]->val;
	if (!strcmp(wfm[0]->trcName, stn_name)) {
	    found= 1;
	    for(j=0; j < numwfm; j++) {
		char *fname= wfm[j]->evt->name;
		FileFormat format= wfm[j]->evt->format;
		if(AssociateWfmTrack(wfm[j], 1)) { /* plot mode */
		    CompleteLoad(fname,format);
		    /* add in to curEvt list */
		    AddSingleFileEvent(fname,format,0);
		}
	    }
	    break;
	}
    }
    if(!found)
	fprintf(stderr,"Preview: cannot find station \"%s\".\n",stn_name);
}

static void stn_file_notify(Panel_item item, Event *event)
{
    char *fname= (char *)xv_get(stn_file_txt,PANEL_VALUE);
    char **stn_names;
    int num, max, i;
    BtreeNode **stn_idx;

    if(*fname=='\0') return;
    
    if(!StationNameIndex) return;
    stn_idx= (BtreeNode **)btree_linearize(StationNameIndex);

    stn_names= read_stn_name(fname, &num);
    if(!stn_names)return;
    for(i=0; i< num; i++) {
	stn_load_wfm(stn_idx,stn_names[i]);
	free(stn_names[i]);
    }
    free(stn_names);
    if (Mode_align) RedrawScreen();
    
    return;
}

static void Wdb_LoadFile()
{
    int i, num;
    wfmTuple **wfm;
    BtreeNode **stn_idx;

    if(Mode_align) {
	Xv_notice notice;
	int notice_stat;
	notice= xv_create(tracesFrame, NOTICE,
		NOTICE_MESSAGE_STRINGS, "Time align mode on. Do you",
			 "want to turn it off?", NULL,
		NOTICE_BUTTON, "Yes", NOTICE_YES,
		NOTICE_BUTTON, "No", NOTICE_NO,
		NOTICE_STATUS, &notice_stat,
		XV_SHOW, TRUE,
		NULL);
	switch(notice_stat){
	    case NOTICE_YES:
		Mode_align= 0;
		ChangeNumTracks(NumTracks); /* copy from ctrl.c */
		if(LastTrack>=0) {
		    FullScale();
		}
		break;
	    case NOTICE_NO:
		break;
	}
    }
    if(!StationNameIndex)return;
    num= StationNameIndex->numElt;
    stn_idx= (BtreeNode **)btree_linearize(StationNameIndex);
    for(i=0;i<num;i++) {
	int j,numwfm;
	if (xv_get(Wdb_list_i, PANEL_LIST_SELECTED, i)==TRUE) {
	    numwfm= stn_idx[i]->numVal;
	    wfm= (wfmTuple **)stn_idx[i]->val;
	    for(j=0; j < numwfm; j++) {
		char *fname= wfm[j]->evt->name;
		FileFormat format= wfm[j]->evt->format;
		if(AssociateWfmTrack(wfm[j], 1)) { /* plot mode */
		    CompleteLoad(fname,format);
		    /* add in to curEvt list */
		    AddSingleFileEvent(fname,format,0);
		}
	    }
	    if (Mode_align) RedrawScreen();
	}
    }
}

static void InitWdbPanel(Frame frame)
{
    Panel panel;
    Wdb_frame= (Frame)xv_create(frame, FRAME,
	FRAME_LABEL, "Waveform Database",
	XV_HEIGHT, 520, XV_WIDTH,350,
	NULL);
    panel = (Panel)xv_create(Wdb_frame,PANEL, NULL);
    xv_set(Wdb_frame, XV_SHOW, TRUE, NULL);
    Wdb_file_txt=(Panel_item)xv_create(panel, PANEL_TEXT,
	XV_X, 20, XV_Y, 25,
	PANEL_VALUE_DISPLAY_WIDTH, 165,
	PANEL_LABEL_STRING, "Event file:",
	PANEL_NOTIFY_PROC, Wdb_file_notify,
	NULL);
    stn_file_txt=(Panel_item)xv_create(panel, PANEL_TEXT,
	XV_X, 20, XV_Y, 50,
	PANEL_VALUE_DISPLAY_WIDTH, 160,
	PANEL_LABEL_STRING, "Station list:",
	PANEL_NOTIFY_PROC, stn_file_notify,
	NULL);
    (void)xv_create(panel, PANEL_BUTTON,
	XV_X, 280, XV_Y, 20,	    
	PANEL_LABEL_STRING,	"Read",
	PANEL_NOTIFY_PROC,	Wdb_file_notify,
	NULL);
    (void)xv_create(panel, PANEL_BUTTON,
	XV_X, 280, XV_Y, 50,	    
	PANEL_LABEL_STRING,	"Match",
	PANEL_NOTIFY_PROC,	stn_file_notify,
	NULL);
    Wdb_list_i= (Panel_item)xv_create(panel, PANEL_LIST,
	XV_X, 20, XV_Y, 80,	    
	PANEL_LIST_TITLE, "Stations",			  
	PANEL_LIST_DISPLAY_ROWS, 20,
	PANEL_LIST_WIDTH, 300,
	PANEL_CHOOSE_ONE, FALSE,	    
	NULL);
    (void)xv_create(panel, PANEL_BUTTON,
	XV_X, 20, XV_Y, 490,
	PANEL_LABEL_STRING,	"                             Load Traces",
	PANEL_LABEL_WIDTH, 300,	    
	PANEL_NOTIFY_PROC,	Wdb_LoadFile,
	NULL);

}
    
static void update_wdb_stn_list()
{
    Btree *bt= StationNameIndex;
    int i, num= bt->numElt;
    BtreeNode **stn_idx= (BtreeNode **)btree_linearize(bt);
    char *s;
    for(i=0;i<num;i++) {
	s= ((wfmTuple *)stn_idx[i]->val[0])->trcName;
	xv_set(Wdb_list_i, PANEL_LIST_INSERT, i,
	       PANEL_LIST_STRING, i, s,
	       PANEL_LIST_SELECT, i, FALSE,
	       NULL);
    }
}

/**********************************************************************
 *                                                                    *
 **********************************************************************/

int wfm_cmp(wfmTuple *wfm1, wfmTuple *wfm2)
{
    return strcmp(wfm1->trcName, wfm2->trcName);
}

static void bt_prn(int n, wfmTuple **wfm)
{
    printf("%d %s\n",n,wfm[0]->trcName);
}

void handle_hashStationNames()
{
    EqEvent *evt= Wdb_eqEvent;
    Btree *bt= empty_btree(wfm_cmp);

    StationNameIndex= bt;   /* what about the old one? */
    while (evt) {
	EvtFile *efl= evt->evt;
	while(efl) {
	    wfmTuple *wfm= efl->wfmTuples;
	    while(wfm) {
		btree_insert(bt, wfm);
		wfm=wfm->next;
	    }
	    efl=efl->next;
	}
	evt=evt->next;
    }
/*    btree_print(bt, bt_prn);*/
}

void PreviewEvent()
{
    EqEvent *evt= Wdb_eqEvent;

    while (evt) {
	PreviewOneEvent(evt->evt);
	evt=evt->next;
    }
}


void handle_preview_event()
{
    InitWdbPanel(tracesFrame);
}

