#ifndef lint
static char id[] = "$Id: eqevtmgr.c,v 1.3 2013/02/28 21:25:00 lombard Exp $";
#endif

/*
 * eqevtmgr.c--
 *    the Earthquake Event Manager
 *
 * Copyright (c) 1992-3 Andrew K. Yu, Lawrence Berkeley Laboratory
 *      and University of California, Berkeley.
 * All rights reserved.
 */
#include <stdio.h>

#include "proto.h"
#include "xv_proto.h"

/**********************************************************************
 *   Global Variables, prototypes                                     *
 **********************************************************************/

static EqEvent *eqEventHead= NULL;
static EqEvent *eqEventTail= NULL;

static EqEvent *curEvent= NULL;

#ifdef SITE_BDSN
FileFormat  defaultFormat= SDR_Format;
#else
FileFormat  defaultFormat= SEGY_Format;
#endif

extern Trace **traces;
extern int LastTrack;
extern int ZoomWindowMapped;
extern Panel_item PrevEvtButton;
extern Panel_item NextEvtButton;
extern char EventID[256];

/**********************************************************************
 *   Adding in filenames                                              *
 **********************************************************************/

void AddSingleFileEvent(char *fname, FileFormat format, int isnewevt)
{
    EqEvent *newevt; 
    EvtFile *newls;

    newls=  (EvtFile *)Malloc(sizeof(EvtFile));
    if (newls==NULL) return;	/* not enuff mem */
    strncpy(newls->name, fname, MAXFILENAME);
    newls->format= format;
    newls->next= NULL;

    if (isnewevt || curEvent==NULL) {
	newevt= (EqEvent *)Malloc(sizeof(EqEvent));
	if (newevt==NULL) 
	    return;	    /* not enuff mem */
	newevt->evt= newls;
	if (eqEventHead==NULL) {	/* first time around */
	    eqEventHead= eqEventTail= newevt;
	    newevt->prev= newevt->next= NULL;
	}else {
	    EqEvent *ev;
	    eqEventTail->next= newevt;
	    newevt->prev= eqEventTail;
	    newevt->next= NULL;
	    eqEventTail= newevt;
	}
	curEvent= newevt;
    }else {
	EvtFile *ls;
	ls= curEvent->evt;
	/* append to current event list: */
	if (ls==NULL) {
	    curEvent->evt= newls;
	}else {
	    while(ls->next!=NULL)
		ls= ls->next;
	    ls->next= newls;
	}
    }
}

static void ParseFlfile(FILE *fp, FileFormat format)
{
    char buf[200], temp[200];
    FileFormat form;

    while(fgets(buf, 200, fp)!=NULL) {
	int has_format=1;

	*temp='\0';
	sscanf(buf, "%s", temp);
	if (*temp=='#') continue;    /* ignore comment */
	form= atofilefmt(temp);
	if (form==-1) {
	    form= format;
	    has_format=0;
	}
	if (has_format) {
	    sscanf(buf+strlen(temp), "%s", temp);
	}
	AddSingleFileEvent(temp, form, 1 /* newevt */);
    }
}

EqEvent *ParseElfile(FILE *fp, FileFormat format)
{
    EqEvent *newevt, *evts; 
    EvtFile *ls, *prev, *newls;
    char buf[200], temp[200];

    evts= newevt= (EqEvent *)Malloc(sizeof(EqEvent));
    if (evts==NULL) return NULL; /* not enuff mem */
    newevt->next= NULL;
    newevt->prev= NULL;
    ls= prev= NULL;
    while(fgets(buf, 200, fp)!=NULL) {
	int has_format;

	*temp='\0';
	sscanf(buf, "%s", temp);
	if (*temp=='#') continue;    /* ignore comment */
	if (!strncmp(temp, "::", 2)) {
	    if (ls!=NULL) {
		/* new event */
		newevt->evt= ls;
		newevt->next= (EqEvent *)Malloc(sizeof(EqEvent));
		if (newevt->next==NULL) return NULL;	/* not enuff mem */
		newevt->next->prev= newevt;
		newevt= newevt->next;
		newevt->next= NULL;
		ls= NULL;
		if (prev!=NULL) prev->next=NULL;
	    }
	    continue;
	}
	newls=  (EvtFile *)Malloc(sizeof(EvtFile));	/* check if know format */
	if (newls==NULL) return NULL; /* not enuff mem */
	has_format=1;
	newls->format= atofilefmt(temp);
	if(newls->format==-1) {
	    newls->format= format;
	    has_format=0;
	}
	if (has_format) {
	    sscanf(buf+strlen(temp), "%s", temp);
	}
	strncpy(newls->name, temp, MAXFILENAME);
	if (ls==NULL) {
	    ls=prev=newls;
	}else {
	    prev->next=newls;
	    prev=newls;
	}
    }
    newevt->evt= ls;
    if (prev!=NULL) prev->next=NULL;
    return evts;
}

static void ReadFileList(char *flfname, FileFormat format)
{
    FILE *fp;

    if ((fp=fopen(flfname,"r"))==NULL) {
	ReportError("Cannot open event list file %s.\n",
		    flfname);
	return;
    }
    ParseFlfile(fp, format);
}

void ReadEventList(char *elfname, FileFormat format)
{
    EqEvent *newevt;
    FILE *fp;

    if ((fp=fopen(elfname,"r"))==NULL) {
	ReportError("Cannot open event list file %s.\n",
		    elfname);
	return;
    }
    newevt= ParseElfile(fp, format);
    if (eqEventHead==NULL) {	/* first time around */
	curEvent= eqEventHead= eqEventTail= newevt;
	while(newevt->next!=NULL)
	    newevt=newevt->next;
	eqEventTail= newevt;
    }else {
	eqEventTail->next= newevt;
	newevt->prev= eqEventTail;
	eqEventTail= newevt;
    }
}

int ReadEventList2 (char *elfname, FileFormat format)
{
    EqEvent *newevt;
    FILE *fp;
    char buf[200], temp[200];

    if ((fp=fopen(elfname,"r"))==NULL) {
	ReportError("Cannot open event list file %s.\n",
		    elfname);
	return 0;
    }
    newevt= ParseElfile(fp, format);
    /* this is somewhat of a hack */
    if (curEvent!=NULL) {
	EqEvent *tmp, *ev;
	tmp= curEvent->next;
	/* replace current event's file list */
	curEvent->evt= newevt->evt;
	if (newevt->next)
	    newevt->next->prev= curEvent;
	curEvent->next= newevt->next;
	free(newevt);
	/* search for the tail */
	ev=curEvent;
	while(ev->next!=NULL)
	    ev= ev->next;
	ev->next= tmp;
	if(tmp)tmp->prev= ev;
    }else {
	if (eqEventHead!=NULL) {
	    /* eqEventHead should be NULL */
	    fprintf(stderr, "problem: eqvtmgr.c\n");
	}
	curEvent= eqEventHead= newevt;
	newevt->prev= NULL;
	while(newevt->next!=NULL)
	    newevt=newevt->next;
	eqEventTail= newevt;
    }
    return 1;
}

/**********************************************************************
 *   Getting Events from the queue                                    *
 **********************************************************************/

EqEvent *getCurrentEvt()
{
    return curEvent;
}

EqEvent *getEventHead()
{
    return eqEventHead;
}

EqEvent *getEventTail()
{
    return eqEventTail;
}

void fixPrevNextButtons()
{
    EqEvent *evt= curEvent;

    if (PrevEvtButton) {
	xv_set(PrevEvtButton, PANEL_INACTIVE,
	       evt->prev==NULL?TRUE:FALSE, NULL);
    }
    if (NextEvtButton) {
	xv_set(NextEvtButton, PANEL_INACTIVE,
	       evt->next==NULL?TRUE:FALSE, NULL);
    }
}

EqEvent *getPrevEvt()
{
    EqEvent *evt;
    evt=curEvent->prev;
    if (evt!=NULL) {
	curEvent= evt;
	fixPrevNextButtons();
    }
    return evt;
}

EqEvent *getNextEvt()
{
    EqEvent *evt;
    evt=curEvent->next;
    if (evt!=NULL) {
	curEvent= evt;
	fixPrevNextButtons();
    }
    return evt;
}

/**********************************************************************
 *   Load in all files in a event list                                *
 **********************************************************************/

void LoadEvent(EvtFile *ls)
{
    char *pfile=ls->name;
    FileFormat pform= ls->format;
    /* bzero(EventID, 256); */
    while(ls!=NULL) {
	char *s;
	s= ls->name;
	if (AssociateTrack(s, ls->format, 0)) /* silent load mode */
	    CompleteSingleLoad(s, ls->format);
	ls=ls->next;
    }
    /* try to load in the pick file using the first file's name */
    CompleteRest(pfile,pform);
    activate_sort_event( 0 );
}
	

/**********************************************************************
 *   Parsing command Line Arguments                                   *
 **********************************************************************/

void eqevt_parseArg(int argc, char **argv)
{
    int isfirstfile=0, grouped=0, is_elf=0, is_flf=0;
    argc--; argv++;
    while(argc>0) {
	if (*argv!=NULL) {
	    if (!strncmp(*argv, "-fl", 3)) {
		is_flf=1;
	    }else if (!strncmp(*argv, "-el", 3)) {    /* event list */
		is_elf=1;
	    }else if (**argv=='{') {	/* note nested {} is not allowed */
		grouped=1;
		isfirstfile=1;
	    }else if (**argv=='}') {
		grouped=0;
		is_elf=0;   /* reset */
	    }else {
		if (is_elf) {
		    if (grouped) {
			ReadEventList(*argv, defaultFormat );
			isfirstfile=0;
		    }else {
			ReadEventList(*argv, defaultFormat );
			is_elf=0;   /* once only */
		    }
		}else if (is_flf) {
		    if (grouped) {
			ReadFileList(*argv, defaultFormat );
			isfirstfile=0;
		    }else {
			ReadFileList(*argv, defaultFormat );
			is_flf=0;   /* once only */
		    }
		    
		}else {
		    if (grouped) {
			AddSingleFileEvent( *argv, defaultFormat,
					    isfirstfile);
			isfirstfile=0;	       
		    }else {
			AddSingleFileEvent( *argv, defaultFormat, 1);
		    }
		}
	    }
	}
	argc--; argv++;
    }
    if(!Mode_CLI) xv_set(PrevEvtButton, PANEL_INACTIVE, TRUE, NULL);
    if (eqEventHead!=NULL) {
	curEvent=eqEventHead;
	if (curEvent->next==NULL && !Mode_CLI) {
	    xv_set(NextEvtButton, PANEL_INACTIVE, TRUE, NULL);
	}
    }else {
	if(!Mode_CLI)
	    xv_set(NextEvtButton, PANEL_INACTIVE, TRUE, NULL);
    }
}

/**********************************************************************
 *   Moving among the event queue                                     *
 **********************************************************************/

void goto_cur_event()
{
    EqEvent *eq;
    extern int Mode_CompGp;

    if (!CheckChangesFlag())
	return;	/* forget it! */

    if (Mode_triplet==1) {
	EndTripletMode();  /* turn triplets off */
	Mode_triplet=1;    /* but remember that it was on */  
    }

    eq= getCurrentEvt();
    if (eq!=NULL) {
	CloseAllTracks();
	LoadEvent(eq->evt);
        if (Mode_triplet==1) {
	    Mode_triplet=0;     /* turn off our "remember" flag... */
	    StartTripletMode(); /* so we can turn triplets back on */
        }
	RedrawScreen();
	activate_sort_event( 0 );
	reset_Resp();
	unset_evt_azi(0.0);
	if (ZoomWindowMapped) {
	    RedrawZoomWindow("goto_cur_event");
	}
    }
}

void goto_prev_event()
{
    EqEvent *eq;

    if (!CheckChangesFlag())
	return;	/* forget it! */
    eq= getPrevEvt();
    if (eq!=NULL) {
	CloseAllTracks();
	LoadEvent(eq->evt);
	if (Mode_CLI) return;
	RedrawScreen();
	activate_sort_event( 0 );
	reset_Resp();
	if (ZoomWindowMapped) {
	    if (Mode_triplet) {
		Mode_triplet = 0;
		StartTripletMode();
	    }
	    RedrawZoomWindow("goto_prev_event");
	}
    }
}

void goto_next_event()
{
    EqEvent *eq;

    if (!CheckChangesFlag())
	return;	/* forget it! */
    eq= getNextEvt();
    if (eq!=NULL) {
	CloseAllTracks();
	LoadEvent(eq->evt);
	if (Mode_CLI) return;
	RedrawScreen();
	activate_sort_event( 0 );
	reset_Resp();
	if (ZoomWindowMapped) {
	    if (Mode_triplet) {
		Mode_triplet = 0;
		StartTripletMode();
	    }
	    RedrawZoomWindow("goto_next_event");
	}
    }
}

void goto_specified_event(EqEvent *eq)
{
    if (!CheckChangesFlag())
	return;	/* forget it! */
    if (eq!=NULL) {
	curEvent= eq;
	fixPrevNextButtons();
	CloseAllTracks();
	LoadEvent(eq->evt);
	if (Mode_CLI) return;
	RedrawScreen();
	activate_sort_event( 0 );
	reset_Resp();
	if (ZoomWindowMapped)
	    RedrawZoomWindow("goto_specified_event");
    }
}

void print_eqEvtQueue()
{
    EqEvent *evt= eqEventHead;

    while (evt) {
	if (evt->evt && evt->evt->next!=NULL) {
	    Printf("%s ... ", evt->evt->name);
	}else {
	    Printf("%s ", evt->evt->name);
	}
	if (evt==curEvent) Printf("   <-");
	Printf("\n");

	evt=evt->next;
    }
}

void CleanCurEvent()
{
    EqEvent *eq= curEvent;
    EvtFile *ls, *p;
    if(eq) {
	ls= eq->evt;
	while(ls) {
	    p=ls;
	    ls=ls->next;
	    free(p);
	}
	eq->evt= NULL;
    }
}

void Write_event_list_file()
{
    EqEvent *evt= eqEventHead;
    FILE *fp;
    char fname[1000];

    GetString("Evt list file: ", fname, 1000);
    if(*fname=='\0') return;
    if((fp=fopen(fname,"w"))==NULL) {
	fprintf(stderr,"Cannot write to file \"%s\".\n",fname);
	return;
    }
    fprintf(fp,"#\n# SeisTool V2.0 Event List File\n#\n");
    while (evt) {
	EvtFile *ls= evt->evt;
	fprintf(fp,"::\n");

	/* no good solns yet... */
	if(evt!=curEvent) {
	    while(ls) {
		fprintf(fp,"%s\t%s\n", formatName(ls->format),ls->name);
		ls=ls->next;
	    }
	}else {
	    int i;
	    fprintf(fp,"# only active traces listed--\n");
	    if(LastTrack!=-1) {
		for(i=0; i<= LastTrack; i++) {
		    fprintf(fp,"%s\n", traces[i]->filename);
		}
	    }
	}
	evt=evt->next;
    }
    fclose(fp);
}

    
