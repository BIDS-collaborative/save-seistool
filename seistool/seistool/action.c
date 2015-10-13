#ifndef lint
static char id[] = "$Id: action.c,v 1.3 2013/02/28 21:25:01 lombard Exp $";
#endif

/*
 * action.c--
 *    handles several (files) menu item actions
 *
 * Copyright (c) 1992-3 Andrew K. Yu, Lawrence Berkeley Laboratory
 *      and University of California, Berkeley.
 * All rights reserved.
 */
#include <xview/xview.h>
#include <xview/frame.h>
#include <xview/panel.h>
#include "proto.h"
#include "xv_proto.h"

/**********************************************************************
 *   Handle Load File Action                                          *
 **********************************************************************/

static void handle_load_callback();
static void handle_load_event_callback();
static void load_pickfile_callback();
static void set_execscript_callback();

static int closeBeforeLoad= 0;

void handle_load(FileFormat format)
{
    char title[100];
    extern char *formatName();

    closeBeforeLoad= 0;
    sprintf(title, "Load %s file", formatName(format));
    SelectFile(title,format,handle_load_callback,0);	/* don't return */
}

void handle_newload(FileFormat format )
{
    char title[100];
    extern char *formatName();

    closeBeforeLoad= 1;
    sprintf(title, "Load New %s file", formatName(format));
    SelectFile(title,format,handle_load_callback,0);	/* don't return */
}

static void handle_load_callback(char *fname, FileFormat format )
{
    if (*fname!='\0') {
	/* load file */
	if(closeBeforeLoad) {
	    CloseAllTracks();
	    CleanCurEvent();
	}
	
	/* AssociateTrack loads waves from fname */
	if (AssociateTrack(fname, format, 0))  {  /* plot mode */
	    EqEvent *cur;
	    
	    CompleteLoad(fname, format);
	    /* add in to curEvt list */
	    AddSingleFileEvent( fname, format, 0);
	}
	if (Mode_align)
	    RedrawScreen();	/* time scale may have changed */
    }
}

void handle_load_event(FileFormat format )
{
    if (!CheckChangesFlag())
	return;
    /* ask for the event file */
    SelectFile("Load Event", format,
	       handle_load_event_callback, 1);	    /* should return */
}

static void handle_load_event_callback(char *fname, FileFormat format)
{
    EqEvent *cur;
    
    if (*fname!='\0') {
	/* load the eventlist file */
	if (!ReadEventList2(fname, format))
	    return;
	CloseAllTracks();
	cur= getCurrentEvt();
	LoadEvent(cur->evt);
	fixPrevNextButtons();
	RedrawScreen();
    }
}

void load_pickfile()
{
    SelectFile("Load Pick File", 0 /* doesn't matter */,
	       load_pickfile_callback, 1);	    /* should return */
}

static void load_pickfile_callback(char *fname, FileFormat format )
{
    char afname[131];
    
    if (*fname!='\0') {
	/* load file */
	ReadPicks(fname);
	cvtToAFname(afname, fname, format);
	ReadAmps(afname);
    }
    RedrawScreen();
}

void set_execscript()
{
    /* ask for the script file */
    SelectFile("Set Exec File", 0 /* doesn't matter */,
	       set_execscript_callback, 1);	    /* should return */
}

static void set_execscript_callback(char *fname, FileFormat format)
{
    extern char *ExecScriptFname;
    
    if (*fname!='\0') {
	ExecScriptFname=strcpy(Malloc(strlen(fname)+1),fname);
    }
}


