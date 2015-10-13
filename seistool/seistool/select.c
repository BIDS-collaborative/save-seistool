#ifndef lint
static char id[] = "$Id: select.c,v 1.3 2013/02/28 21:24:57 lombard Exp $";
#endif

/*
 * select.c--
 *    handles "select traces"
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


int Mode_autoFilterNames= 0;
extern Trace **traces;
extern int LastTrack;


static SeleRE *sele_strings= NULL;

static void AddSeleString(char *expr)
{
    SeleRE *re= (SeleRE *)Malloc(sizeof(SeleRE));
    re->expr= strcpy(Malloc(strlen(expr)+1),expr);
    RE_compile(expr, re->exprbuf);
    re->next= sele_strings;
    sele_strings= re;
    return;
}

void CleanSeleStrings()
{
    SeleRE *re=sele_strings, *tmp;
    while(re) {
	tmp=re;
	re=re->next;
	if(tmp->expr)free(tmp->expr);
	free(tmp);
    }
    sele_strings=NULL;
}

void ParseSelectFile(char *fname)
{
    FILE *fp;
    char buf[500];
    CleanSeleStrings();
    if((fp=fopen(fname,"r"))==NULL) {
	fprintf(stderr,"Cannot open file %s.\n",fname);
	return;
    }
    while(fgets(buf,500,fp)!=NULL) {
	buf[strlen(buf)-1]='\0';    /* take \n out */
	AddSeleString(buf);
    }
    return;
}

static int matchSeleStrings(char *str)
{
    SeleRE *re= sele_strings;
    if(!re)return 1;	/* always matches if not set */
    while(re) {
	if (RE_match(str,re->exprbuf))
	    return 1;
	re=re->next;
    }
    return 0;
}

int matchSeleSNCL(BIS3_HEADER *bh)
{
    char sncl[20];
    SeleRE *re= sele_strings;
    sprintf(sncl, "%s.%s.%s.%s", bh->station, bh->network, bh->channel,
	    bh->location);
    if(!re)return 1;	/* always matches if not set */
    while(re) {
	if (RE_match(sncl,re->exprbuf))
	    return 1;
	re=re->next;
    }
    return 0;
}

static void handle_s_t(char *fname)
{
    if(*fname!='\0')
	ParseSelectFile(fname);
}

void handle_select_trace_script()
{
    SelectFile("select trace", 0, handle_s_t, 1);
}

void handle_select_trace_enter()
{
    char buf[1000];
    GetString("Select Name as STA.NET.CHAN.LOC: ", buf, 1000);
    if(*buf=='\0')return;
    AddSeleString(buf);
}

void handle_select_trace_select()
{
    int i;
    for(i=0; i <= LastTrack; i++) {
	if(matchSeleSNCL(&traces[i]->wave->info)) {
	    traces[i]->selected= 1;
	}else {
	    traces[i]->selected= 0;
	}
    }
    RedrawScreen();
}

void handle_select_trace_selerest()
{
    int i;
    for(i=0; i <= LastTrack; i++) {
	if(!matchSeleSNCL(&traces[i]->wave->info)) {
	    traces[i]->selected= 1;
	}else {
	    traces[i]->selected= 0;
	}
    }
    RedrawScreen();
}

void handle_select_trace_keep()
{
    int i;
    for(i=0; i <= LastTrack; i++) {
	if(matchSeleSNCL(&traces[i]->wave->info)) {
	    traces[i]->selected= 0;
	}else {
	    traces[i]->selected= 1; /* discard others */
	}
    }
    Discard_Selected();
}

void handle_select_trace_keeprest()
{
    int i;
    for(i=0; i <= LastTrack; i++) {
	if(!matchSeleSNCL(&traces[i]->wave->info)) {
	    traces[i]->selected= 0;
	}else {
	    traces[i]->selected= 1; /* discard matched */
	}
    }
    Discard_Selected();
}

void handle_select_trace_filter()
{
    int i;
    for(i=0; i <= LastTrack; i++) {
	if(matchSeleSNCL(&traces[i]->wave->info)) {
	    traces[i]->selected= 0;
	}else {
	    traces[i]->selected= 1; /* discard others */
	}
    }
    Discard_Selected();
    Mode_autoFilterNames= 1;
}

void handle_select_trace_rearrange()
{
    int i;
    for(i=0; i <= LastTrack; i++) {
	if(matchSeleSNCL(&traces[i]->wave->info)) {
	    traces[i]->selected= 1; /* rearrange */
	}else {
	    traces[i]->selected= 0; 
	}
    }
    Rearrange_Selected_top();
}
