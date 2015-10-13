#ifndef lint
static char id[] = "$Id: info.c,v 1.2 2013/02/28 21:24:59 lombard Exp $";
#endif

/*
 * info.c--
 *    the infomation window
 *
 * Copyright (c) 1992-3 Andrew K. Yu, Lawrence Berkeley Laboratory
 *      and University of California, Berkeley.
 * All rights reserved.
 */
#include <xview/xview.h>
#include <xview/frame.h>
#include <xview/textsw.h>
#include <xview/panel.h>
#include "proto.h"
#include "xv_proto.h"

extern Trace **traces;
extern int LastTrack;

static Frame  info_frame= NULL;
static Textsw info_textsw= NULL;
static int infoWinMapped= 0;


static void InitInfoText(Frame frame)
{
    int x, y;
    Panel panel;
    x= (int)xv_get(tracesFrame, XV_X);
    y= (int)xv_get(tracesFrame, XV_Y);
    info_frame= xv_create(frame, FRAME,
	XV_X, x+FrmWidth/2-150, XV_Y, y+FrmHeight/2-200,
	XV_HEIGHT, 300, XV_WIDTH, 400,
	NULL);		  
    panel= (Panel)xv_create(info_frame, PANEL,
	XV_X, 0, XV_Y, 0, XV_HEIGHT, 40,
	NULL);
    info_textsw= xv_create(info_frame, TEXTSW,
	XV_Y, 40, TEXTSW_MEMORY_MAXIMUM,2000000,NULL);
    (void)xv_create(panel, PANEL_BUTTON,
	PANEL_LABEL_STRING, "Quit",
	PANEL_NOTIFY_PROC, close_info_win,
	NULL);
}

void open_info_win()
{
    if (!info_textsw) {
	InitInfoText(tracesFrame);
    }
    /* clean screen */
    textsw_erase(info_textsw, 0, TEXTSW_INFINITY);
    if (!infoWinMapped)
	xv_set(info_frame, XV_SHOW, TRUE, NULL);
    infoWinMapped=1;
}

void close_info_win()
{
    xv_set(info_frame, XV_SHOW, FALSE, NULL);
    infoWinMapped=0;
}

int textsw_printf(char *str)
{
    return textsw_insert(info_textsw, str, strlen(str));
}

void DisplayTraceInfo(int itrc)
{
    char buf[30];
    BIS3_HEADER *bh= &traces[itrc]->wave->info;

    if (itrc<=LastTrack) {
	sprintf(buf,"(%d) %s %s %s %s",itrc,bh->station, bh->network, 
		bh->channel, (strlen(bh->location) > 0) ? bh->location : "--");
	xv_set(info_frame, FRAME_LABEL, buf, NULL);
    }else {
	xv_set(info_frame, FRAME_LABEL, "No Trace.", NULL);
    }
    Printf("TRACE %d\n",itrc);
    Printf("(from file: %s)\n",traces[itrc]->filename);
    printhead(&traces[itrc]->wave->info, 0);
}
