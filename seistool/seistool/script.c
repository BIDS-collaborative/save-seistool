#ifndef lint
static char id[] = "$Id: script.c,v 1.2 2013/02/28 21:24:58 lombard Exp $";
#endif

/*
 * script.c--
 *    handle execute scripts
 *
 * Copyright (c) 1992-3 Andrew K. Yu, Lawrence Berkeley Laboratory
 *      and University of California, Berkeley.
 * All rights reserved.
 */

#include <stdio.h>
#include <strings.h>
#include "proto.h"

char *ExecScriptFname= NULL;

static void parseScriptCmd(char *line, int itrc)
{
    char cmd[1000];

    sscanf(line,"%s",cmd);
    if(*cmd=='#') return; /* comments, ignore */
    /*
     *	MATCH resp
     */
    if(!strcmp(cmd,"match")) {
	sscanf(line,"%*s %s",cmd);
	if(!strncmp(cmd,"resp",4)) {
	    printf(": match response (trc #%d)\n",itrc);
	    GetResponses(NULL);
	}
    /*
     *	DECONV 
     */
    }else if(!strncmp(cmd,"deconv",6)) {
	printf(": deconvolve (trc #%d)\n",itrc);
	ConvRespTrace(0,itrc);
    /*
     *	FILTER butter lo_freq hi_freq
     */
    }else if(!strcmp(cmd,"filter")) {
	char name[100];
	float flo, fhi;
	int which;
	sscanf(line, "%*s %s %f %f", name, &flo, &fhi);
	printf(": filter %s %f - %f Hz (trc #%d)\n",
	       name, flo, fhi, itrc);
	if(!strncmp(name,"band",4)) {
	    which=1;
	}else if(!strncmp(name,"low",3)) {
	    which=3;
	}else if(!strncmp(name,"hi",2)) {
	    which=5;
	}
	filterTrc(which,0,itrc,flo,fhi);       
    }else {
	printf("Unrecognize line: %s\n",line);
    }
}

void exec_script(char *fname, int itrc)
{
    FILE *fp;
    char buf[1000];

    if((fp=fopen(fname,"r"))==NULL) {
	fprintf(stderr, "Cannot open script %s.\n", fname);
	return;
    }
    printf("*** TRACE %d\n", itrc);
    while(fgets(buf,1000,fp)!=NULL)
	parseScriptCmd(buf, itrc);

    fclose(fp);
}
