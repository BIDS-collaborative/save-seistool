#ifndef lint
static char id[] = "$Id: cli.c,v 1.2 2013/02/28 21:25:00 lombard Exp $";
#endif

/*
 * cli.c--
 *    the Command Line Interface (an undocumented feature!)
 *
 * Copyright (c) 1992-3 Andrew K. Yu, Lawrence Berkeley Laboratory
 *      and University of California, Berkeley.
 * All rights reserved.
 */
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include "proto.h"
#include "xv_proto.h"

/* internal function prototypes */
static void my_cat(char *buf);
static void my_catp(char *buf);
static void my_sele(char *buf);
static void my_list(char *buf);
static void printTrace(int itrc);

static void printHelp()
{
    printf("\
ls       cat <itrc>     sele <itrc>     dumpq     prev          next \n\
read resp       amp             coin      exit/quit     help/?\n\
save     hash\n");
}

extern Trace **traces;
extern int LastTrack;

void cli_eval_loop()
{
    char buf[200];

    while(1) {
	printf("(SeisTool) ");
	if(!gets(buf)) return;
	if (!strcmp(buf,"exit") || !strcmp(buf,"quit")) {
	    break;
	}else if(!strncmp(buf,"ls",2)) {
	    my_list(buf);
	}else if(!strcmp(buf,"cat")) {
	    my_cat(buf);
	}else if(!strncmp(buf,"catp",4)) {
	    my_catp(buf);
	}else if(!strncmp(buf,"sele",4)) {
	    my_sele(buf);
	}else if(!strncmp(buf,"dumpq",5)) {
	    print_eqEvtQueue();
	}else if(!strncmp(buf,"prev",4)) {
	    goto_prev_event();
	}else if(!strncmp(buf,"next",4)) {
	    goto_next_event();
	}else if(!strncmp(buf,"read resp",7)) {
	    GetResponses(NULL);
	}else if(!strncmp(buf,"amp",3)) {
	    int which= atoi(buf+4);
	    printf("Amp type: %d\n",which);
	    CalcAmpli(which);
	}else if(!strncmp(buf,"coin",4)) {
	    int which= atoi(buf+4);
	    printf("Convert type: %d\n",which);
	    ConvertResponses(which);
	}else if(!strcmp(buf, "save")) {
	    DumpBIS();
	}else if(!strcmp(buf, "hash")) {
	    PreviewEvent();
	    handle_hashStationNames();
	}else if(!strcmp(buf, "help") || *buf=='?'){
	    printHelp();
	}else {
	    printf("?SYNTAX ERROR.\n");
	}
    }
    exit(1);
}

static void my_cat(char *buf)
{
    char arg[80];
    int itrc;

    sscanf(buf+3,"%s",arg);
    itrc=atoi(arg);
    printf("Trace %d>>>\n",itrc);
    printTrace(itrc);
}

static void my_catp(char *buf)
{
    char arg[80];
    int itrc;
    Pick *p;

    sscanf(buf+4,"%s",arg);
    itrc=atoi(arg);
    printf("Trace %d>>>\n",itrc);
    p = traces[itrc]->picks;
    while(p) {
	printf("%f %.8s",
	       p->secOffset, p->phaseName);
	p = p->next;
    }
}

static void my_sele(char *buf)
{
    char arg[80];
    int itrc;

    sscanf(buf+4,"%s",arg);
    itrc=atoi(arg);
    if(itrc<=LastTrack) {
	printf("trace %d selected.\n",itrc);
	traces[itrc]->selected= !traces[itrc]->selected;    /* toggle */
    }
}

static void my_list(char *buf)
{
    int i;
    BIS3_HEADER *bh;
    printf("*** %d traces\n",LastTrack+1);
    /* list all traces */
    for(i=0;i<=LastTrack;i++) {
	bh= &traces[i]->wave->info;
	printf("%c[%d] %s %s %s %s %d %f %s\n", (traces[i]->selected?'*':' '),
	       i, bh->station, bh->network, 
	       bh->channel, (strlen(bh->location) > 0) ? bh->location : "--",
	       bh->n_values, bh->sample_rate, traces[i]->filename);
    }

}

static void printTrace(int itrc)
{
    if(itrc<=LastTrack)
	printhead(&traces[itrc]->wave->info, 1);
    else
	printf("No such trace.\n");
}

