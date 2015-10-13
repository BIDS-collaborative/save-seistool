#ifndef lint
static char id[] = "$Id: notice.c,v 1.3 2013/02/28 21:24:58 lombard Exp $";
#endif

/*
 * notice.c--
 *    implements notices and error messages
 *
 * Copyright (c) 1992-3 Andrew K. Yu, Lawrence Berkeley Laboratory
 *      and University of California, Berkeley.
 * All rights reserved.
 */
#include <xview/xview.h>
#include <xview/notice.h>
#include "xv_proto.h"

void ReportFileNotFound(char *fname, int noPop)
{
    Xv_notice notice;
    char msg[200];

    sprintf(msg, "File %s nonexistent.", fname);
    if (tracesFrame && ! noPop) {
	notice=xv_create(tracesFrame, NOTICE,
	     NOTICE_MESSAGE_STRING, msg,
	     XV_SHOW, TRUE,
	     NULL);
    }else {
	fprintf(stderr, "%s\n", msg);
    }
    return;
}

void ReportError(char *template, char *fname)
{
    Xv_notice notice;
    char msg[200];

    sprintf(msg, template, fname);
    if (tracesFrame) {
	notice=xv_create(tracesFrame, NOTICE,
	     NOTICE_MESSAGE_STRING, msg,
	     XV_SHOW, TRUE,
	     NULL);
    }else {
	fprintf(stderr, "%s\n", msg);
    }
    return;
}

