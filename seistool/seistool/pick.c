#ifndef lint
static char id[] = "$Id: pick.c,v 1.5 2013/02/28 21:24:58 lombard Exp $";
#endif

/*
 * pick.c--
 *    handles reading/writing/representation of picks
 *
 * Copyright (c) 1992-3 Andrew K. Yu, Lawrence Berkeley Laboratory
 *      and University of California, Berkeley.
 * All rights reserved.
 */

#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <strings.h>

#include "proto.h"
#include "xv_proto.h"

extern Trace **traces;
extern Track **tracks;
extern int lowTrkIndex;
extern int LastTrack;
extern char default_network[NETSIZE];

extern int ChangesMade;
extern int subsample_rate;  /* subsamples per sample, defined in zevent.c */

static STI_TIME Find_firstpick_time(Trace *trc);
static STI_TIME getEarliestPick(int includeWaifs);
static STI_TIME Find_firstpick_trip(Triplet *trp);

/* waifTraces are used to store picks that don't have traces for them */
Trace **waifTraces = NULL;
int TotalWaifTraces = 0;   /* Number of waif traces allocated */
int NumWaifTraces = 0;     /* Number of waif trace structures used */
int saveWaifPicks = 0;
char EventID[256];

/* There are three versions of pickfiles (.pf). The old one is used at LBL
 * while the newer ones are used at the seismo station. Version 2 has the 
 * channel names and station names split and contain a line "Pickfile V.2"
 * Version 3 has the station, network, channel and location names (SNCL),
 * eliminates the sample index from pick lines, and adds the channel dip 
 * and azimuth. This file includes a line "Pickfile V.3.00"
 */

/**********************************************************************
 *   Creating a pick node                                             *
 **********************************************************************/

Pick *AddPicks( pl )
     Pick *pl;
{
    Pick *new= (Pick *)Malloc(sizeof(Pick));
    if (new==NULL) return pl;	/* not enuff mem */
    bzero(new->label, sizeof(new->label));
    new->type= PICK;
    new->secOffset = 0.0;
    bzero(new->phaseName, PHSIZE+1);
    new->firstMotion= UNKNOWN_M;
    new->quality= UNKNOWN_Q;

    new->next= pl;
    return new;
}

void change_subsampling(num) 
     int num;
{
    if (num < 1) num = 1;
    subsample_rate=num;
}

int pick_index(pick, trace, subrate)
     Pick *pick;
     Trace *trace;
     int subrate;
{
    return round(pick->secOffset * trace->wave->info.sample_rate * subrate);
}

     

/**********************************************************************
 *   Associate Attributes to a pick                                   *
 **********************************************************************/

/*
 * if near found, will return immediately.
 */
void FindNearRecent(int itrc, Pick **near_ptr, Pick **recent_ptr,
			       int cur_coord)
{
    int cur_idx, samp_idx;
    Pick *recent, *near, *plist;
    
    cur_idx = coordToIndex(&traces[itrc]->zaxis, cur_coord, 1);
    plist= traces[itrc]->picks;
    recent = near = NULL;
    while(plist!=NULL) {
	if (plist->type==PICK) {
	    if (recent == NULL) recent = plist;
	    samp_idx = pick_index(plist, traces[itrc], 1);
	    if (near == NULL && cur_idx - 10 <= samp_idx
		&& samp_idx <= cur_idx + 10) {
		near = plist;
		break;	/* near pick has priority */
	    }
	}
	plist= plist->next;
    }
    *near_ptr= near;
    *recent_ptr= recent;

    return;
}

void setPickAttrb(Pick *plist, int itrc, Event *ev, int x)
{
    Pick *recent, *near;
    int redraw= 0;
    char ch=event_action(ev);

    FindNearRecent(itrc, &near, &recent, x);
    if (near!=NULL) {
	plist= near; /* for latter use */
    }else if (recent!=NULL) {
	plist= recent; /* for latter use */
    }else {
	return;	/* no pick to set */
    }
    /* note that the SetChanges has to be there even though we have
       that for adding a pick. If we make changes to existing picks just
       loaded in, this will be necesssary. */
    switch(ch) {
    case '\n': case '\r': case '\t':
	break;
	/* assign polarity: up or down or none */
    case '+': case '=':
    case '-':
    case '\\': {
	Motion_t fmotion= UNKNOWN_M;
	    
	switch(ch) {
	case '+': case '=':
	    fmotion= UP; break;
	case '-': 
	    fmotion= DOWN; break;
	case '\\':
	    fmotion= UNKNOWN_M; break;
	}
	DrawZPickLine(plist, itrc); /* erase the old one */
	redraw=1;
	plist->firstMotion= fmotion;
	SetChangesFlag();	    /* register changes */
	break;
    }
    /* assign quality: impulsive or emergent */
    case 'E': case 'e':
    case 'I': case 'i': {
	Quality_t qual= UNKNOWN_Q;

	/* i, e specifies first motion direction when there is no
	   phase Name yet-- hm... can't do that. otherwise, can't
	   switch from i to e if named already: use pick panel maybe? */
#if 0
	if(plist->phaseName[0]=='\0') {
#endif
	switch(ch) {
	case 'E': case 'e':
	    qual= EMERGENT; break;
	case 'I': case 'i':
	    qual= IMPULSIVE; break;
	}
	
	plist->quality= qual;
#if 0
	}else {
	    if (plist->phaseName[PHSIZE-1]=='\0')
		*(index(plist->phaseName,'\0'))= ch;
		}
#endif	    
	SetChangesFlag();	    /* register changes */
	break;
    }

    /* make the capital Y the equivalent of a i */
    case 'Y': 
	ch='i';
	/* assign phase name: eg. P or S */
    default: {
	    if (plist->phaseName[PHSIZE-1]=='\0')
		*(index(plist->phaseName,'\0'))= ch;
	    SetChangesFlag();	    /* register changes */
	    break;
	}
    }
    if (plist) {
	if (*plist->label!='\0') {
	    DrawZPickLabel(plist,itrc);	/* erase the old one */
	}
	SetPickLabel(plist);
	if (redraw) DrawZPickLine(plist,itrc);
	DrawZPickLabel(plist,itrc);
    }
    return;
}

/* This emulates the UW styles of picking-- well, not quite but
   that's not what Rick wants! */
void setPickAttrb2(Pick *plist, int itrc, Event *ev, char ch, int x)
{
    Pick *recent, *near;
    int redraw= 0;

    FindNearRecent(itrc,&near,&recent,x);
    if (near!=NULL) {
	plist= near; /* for latter use */
    }else if (recent!=NULL) {
	plist= recent; /* for latter use */
    }
    switch(ch) {
	/* assign common phase: P or S */
    case 'P': case 'p':
    case 'S': case 's': {
	plist->phaseName[0]=toupper(ch);
	SetChangesFlag();	    /* register changes */
	break;
    }
    /* assign polarity: up or down or none */
    case 'D': case 'd':
    case 'U': case 'u':
    case 'N': case 'n': {
	Motion_t fmotion= UNKNOWN_M;
	    
	switch(ch) {
	case 'U': case 'u':
	    fmotion= UP; break;
	case 'D': case 'd':
	    fmotion= DOWN; break;
	case 'N': case 'n':
	    fmotion= NONE; break;
	}
	DrawZPickLine(plist, itrc); /* erase the old one */
	redraw=1;
	plist->firstMotion= fmotion;
	SetChangesFlag();	    /* register changes */
	break;
    }
    /* assign quality: impulsive or emergent */
    case 'E': case 'e':
    case 'I': case 'i': {
	Quality_t qual= UNKNOWN_Q;
	    
	/* if near an unmarked pick, use that */
	switch(ch) {
	case 'E': case 'e':
	    qual= EMERGENT; break;
	case 'I': case 'i':
	    qual= IMPULSIVE; break;
	}
	plist->quality= qual;
	SetChangesFlag();	    /* register changes */
	break;
    }
    default:
	return; 	    /* ignore */
    }
    if (plist) {
	if (*plist->label!='\0') {
	    DrawZPickLabel(plist,itrc);	/* erase the old one */
	}
	SetPickLabel(plist);
	if (redraw) DrawZPickLine(plist,itrc);
	DrawZPickLabel(plist,itrc);
    }
    return;
}

void SetPickLabel(Pick *plist)
{
    char *buf, buffer[PHSIZE+1];

    /* set the label like this 'iP' for impulsive, P phase */
    buf= buffer;
    if (plist->quality!=UNKNOWN_Q) {
	switch(plist->quality) {
	case IMPULSIVE:
	    *buf='i'; buf++; break;
	case EMERGENT:
	    *buf='e'; buf++; break;
	}
    }
    *buf='\0';	/* safe for strcat */
    strcat(buf, plist->phaseName);
    if (buf[PHSIZE]!='\0') buf[PHSIZE]='\0';
    strcpy(plist->label, buffer);
}

/**********************************************************************
 *   Drawing out the pick                                             *
 **********************************************************************/

/*  draw a pick line in the main traces window  */
void DrawPickLine(Pick *picks, int itrc)
{
    int x;
    Trace *trc= traces[itrc];
    Track *trk= tracks[itrc-lowTrkIndex];
    
    x = timeToCoord(&trc->axis, trc, picks->secOffset);
    XDrawLine(theDisp, trk->xwin, trk->ovrGC, x-2, 8, x+2, 8);
    XDrawLine(theDisp, trk->xwin, trk->ovrGC, x, 8, x, trk->height-8);
    XDrawLine(theDisp, trk->xwin, trk->ovrGC, x-2, trk->height-8,
	      x+2, trk->height-8);
    XFlush(theDisp);
}

/*  draw every pick in a trace in the main traces window  */
void DrawPicks(Pick *plist, int itrc)
{
    while(plist!=NULL) {
	switch(plist->type) {
	case PICK:
	    DrawPickLine(plist, itrc);
	    break;
	case LMARK: case RMARK: 
	    /* ignore */
	default:
	    /*ignore*/
	    break;
	}
	plist=plist->next;
    }
}

/*  draw a pick line in the zoom traces window  */
void DrawZPickLine(Pick *picks, int itrc)
{
    int iztrk, yoffset, x;
    Trace *trc= traces[itrc];

    iztrk= itrc-lowZTrkIndex;
    yoffset= iztrk * ZTrkHeight;
    x= timeToCoord(&trc->zaxis,trc, picks->secOffset);

    switch(picks->firstMotion) {
#define YOFF	6
#define TRI_B	6
#define TRI_H	10
    case UP:
	drawXUpTriangle(ZCvsWin, ZCvsXorGC, x, yoffset+YOFF,TRI_H,TRI_B);
	drawXUpTriangle(ZCvsWin, ZCvsXorGC, x,
			yoffset+ZTrkHeight-TRI_H-YOFF,TRI_H,TRI_B);
	XDrawLine(theDisp, ZCvsWin, ZCvsXorGC, x, yoffset+TRI_H+YOFF,
		  x, yoffset+ZTrkHeight-TRI_H-YOFF);
	break;
    case DOWN:
	drawXDnTriangle(ZCvsWin, ZCvsXorGC, x, yoffset+TRI_H+YOFF,TRI_H,TRI_B);
	drawXDnTriangle(ZCvsWin, ZCvsXorGC, x,
			yoffset+ZTrkHeight-YOFF,TRI_H,TRI_B);
	XDrawLine(theDisp, ZCvsWin, ZCvsXorGC, x, yoffset+TRI_H+YOFF,
		  x, yoffset+ZTrkHeight-TRI_H-YOFF);
	break;
    default:
	XDrawLine(theDisp, ZCvsWin, ZCvsXorGC, x, yoffset+YOFF,
		  x, yoffset+ZTrkHeight-YOFF);
	XDrawLine(theDisp, ZCvsWin, ZCvsXorGC, x-TRI_B, yoffset+YOFF,
		  x+TRI_B, yoffset+YOFF);
	XDrawLine(theDisp, ZCvsWin, ZCvsXorGC, x-TRI_B,
		  yoffset+ZTrkHeight-YOFF, x+TRI_B, yoffset+ZTrkHeight-YOFF);
	break;
    }
    XFlush(theDisp);
}

/*  label a pick in the zoom traces window too  */
void DrawZPickLabel(Pick *pick, int itrc)
{
    int iztrk, yoffset, x;
    Trace *trc= traces[itrc];

#define LBL_OFF	    TRI_B
    iztrk= itrc-lowZTrkIndex,
	yoffset= iztrk * ZTrkHeight,
	x= timeToCoord(&trc->zaxis, trc, pick->secOffset);
    XDrawString(theDisp, ZCvsWin, ZCvsXorGC, x+LBL_OFF, yoffset+YOFF,
		pick->label, strlen(pick->label));
    XFlush(theDisp);
}

/*  draw all the picks in the zoom traces window  */
void DrawZPicks(Pick *plist, int iztrk)
{
    while(plist!=NULL) {
	switch(plist->type) {
	case PICK:
	    DrawZPickLine(plist, iztrk+lowZTrkIndex);
	    DrawZPickLabel(plist, iztrk+lowZTrkIndex);
	    break;
	case LMARK: case RMARK: {
	    Trace *trc= traces[iztrk+lowZTrkIndex];
	    int x= indexToCoord(&trc->zaxis, 
				pick_index(plist, trc, subsample_rate),
				subsample_rate);
	    int yoffset= iztrk * ZTrkHeight;
	    drawThickBracket(ZCvsWin, ZCvsXorGC, x, yoffset+10,
			     yoffset+ZTrkHeight-10,(plist->type==LMARK)?1:0);
	    break;
	}
	default:
	    /*ignore*/
	    break;
	}
	plist=plist->next;
    }
}


/**********************************************************************
 *   Sorting the picks                                                *
 **********************************************************************/

/*  Sort the picks according to the offset. Since all these picks are *
 *  for the same trace, the offsets all refer to the same start time. */
void sortPicks(Trace *trc)
{
    Pick *pl, cur, prev, temp, insert;

    return; 
#if 0 /* don't do it now */ 
    pl=trc->picks;
    /* break list into sorted (pl) and unsorted (cur) */
    cur=pl->next;
    pl->next=NULL;
    /* sort while we still have unsorted elements */
    while(cur!=NULL) {
	temp= cur;
	cur= cur->next;
	/* insert temp into sorted list */
	prev= pl; insert= pl;
	while(insert!=NULL) {
	    if (temp->secOffset <= insert->secOffset) {
		if (insert==pl) {
		    temp->next= pl;
		    trc->picks= temp;
		}else {
		    prev->next= temp;
		    temp->next= insert;
		}
		break;	/* inserted */
	    }else {
		prev= insert;
		insert= insert->next;
		if (insert==NULL) {
		    /* no more, insert at the end */
		    prev->next= temp;
		    temp->next= NULL;
		}
	    }
	}
    }
#endif    
}
    
/**********************************************************************
 *   Input/Output of picks                                            *
 **********************************************************************/
void cvtToPFname(char *pfname, char *fname, FileFormat format)
{
    strcpy(pfname, fname);
    switch(format) {
    case BIS_Format:
    case TRC_Format:
    case SDR_Format:
    case SEGY_Format:
    case SAC_Format:
    default:
	/* no cure, just add ``.pf'' */
	strcat(pfname,".pf");
	break;
    }
}

char *getfilename(char *path)
{
    char *p;
    if ((p=rindex(path, '/'))!=NULL)
	p++;
    else
	p = path;
    return p;   
}

void NotifyWriting(char *fname)
{
    fprintf(stderr,"Writing to %s.\n",fname);
}


void WritePicks(char *fname)
{
    FILE *fp;
    STI_TIME ptime, earl;
    STE_TIME earl_et;
    BIS3_HEADER *bh;
    Trace *trc;
    Pick *pl;
    int i;
    double offset;
    char tmestr[50];
    char name[PHSIZE];
    char uncert[20];
    
#ifndef SITE_BDSN
    /* Old version 1, still used at LBL */
    char lbl_name[10];
#endif
    if ((fp=fopen(fname,"w"))==NULL) {
	/* try stripping off the path (which can be someone
	   else's directory) and retry */
	fname=getfilename(fname);
	if ((fp=fopen(fname,"w"))==NULL) {
	    /* give up */
	    fprintf(stderr,"Cannot write to file %s\n",fname);
	    return;
	}
    }
    NotifyWriting(fname);
    earl= getEarliestPick(saveWaifPicks);
    earl_et = sti_to_ste(earl);
    earl_et.second = 0;
    earl_et.usec = 0;
    earl = ste_to_sti(earl_et);
    sprintTime(tmestr, earl);
#ifdef SITE_BDSN
    fprintf(fp,"Pickfile V.3.00\n");
#endif    
    fprintf(fp,"T %s\n",tmestr);
#ifdef SITE_BDSN
    if (strlen(EventID) > 0)
	fprintf(fp, "E %s\n", EventID);
    fprintf(fp, "H STN NT CHN LC  Q PHASE    M   OFFSET    UNCERT    DATE\n");
#endif    
    for(i=0; i <= LastTrack; i++) {
	trc= traces[i];
	pl= trc->picks;
	bh = &trc->wave->info;
	sortPicks(trc);
	while(pl!=NULL) {   /* loop thro' all picks */
	    if (pl->type==PICK) {
		char qual, fm;
		switch(pl->firstMotion) {
		case UP:    
		    fm='U';
		    break;
		case DOWN:  
		    fm='D';
		    break;
		default:    
		    fm='?';
		    break;
		}
		switch(pl->quality) {
		case IMPULSIVE:  
		    qual='i';
		    break;
		case EMERGENT:  
		    qual='e';
		    break;
		default:	    
		    qual='?';
		    break;
		}
		strcpy(name,pl->phaseName);
		if (*name=='\0') {
		    name[0]='?',
			name[1]='\0';
		}
		
		ptime = st_add_dtime( bh->start_it, 
				      pl->secOffset * USECS_PER_SEC);
		sprintTime(tmestr, ptime);
    /* reference offset to pick_file reference time instead of bh->start_it */
		offset = st_tdiff(bh->start_it, earl);
		offset += pl->secOffset;

#ifdef SITE_BDSN
		/* newer format */
		if (pl->uncertainty < 0.0) 
		    strcpy(uncert, "?        ");
		else
		    sprintf(uncert, "%9.6f", pl->uncertainty);
		
		fprintf(fp," %-4s %-2s %-3s %-2s  %c %-*s %c %11.6f %s %s\n",
			bh->station, bh->network,
			(bh->channel[0]!='\0') ? bh->channel:"---",
			(bh->location[0]!='\0') ? bh->location:"--",
			qual, PHSIZE, name, fm, offset,
			uncert, tmestr);
			
#else
		/* "old" format-- still used at LBL */
		sprintf(lbl_name, "%s%s",bh->station, 
			bh->channel);
		fprintf(fp," %-7s %4d %c %-4s %.4f %c %.4f %s\n",
			lbl_name, pl->sampIdx, qual, name,
			offset, fm, pl->uncertainty, tmestr);
#endif
	    }
	    pl=pl->next;
	}
    }
    if (saveWaifPicks) {
	for(i=0; i < NumWaifTraces; i++) {
	    trc= waifTraces[i];
	    pl= trc->picks;
	    bh = &trc->wave->info;
	    sortPicks(trc);
	    while(pl!=NULL) {   /* loop thro' all picks */
		if (pl->type==PICK) {
		    char qual, fm;
		    switch(pl->firstMotion) {
		    case UP:    
			fm='U';
			break;
		    case DOWN:  
			fm='D';
			break;
		    default:    
			fm='?';
			break;
		    }
		    switch(pl->quality) {
		    case IMPULSIVE:  
			qual='i';
			break;
		    case EMERGENT:  
			qual='e';
			break;
		    default:	    
			qual='?';
			break;
		    }
		    strcpy(name,pl->phaseName);
		    if (*name=='\0') {
			name[0]='?',
			    name[1]='\0';
		    }
		    
		    ptime = st_add_dtime( bh->start_it, 
					  pl->secOffset * USECS_PER_SEC);
		    sprintTime(tmestr, ptime);
		    /* reference offset to pick_file reference time instead of bh->start_it */
		    offset = st_tdiff(bh->start_it, earl);
		    offset += pl->secOffset;
		    
#ifdef SITE_BDSN
		    /* newer format */
		    if (pl->uncertainty < 0.0) 
			strcpy(uncert, "?        ");
		    else
			sprintf(uncert, "%9.6f", pl->uncertainty);
		    
		    fprintf(fp," %-4s %-2s %-3s %-2s  %c %-*s %c %11.6f %s %s\n",
			    bh->station, bh->network,
			    (bh->channel[0]!='\0') ? bh->channel:"---",
			    (bh->location[0]!='\0') ? bh->location:"--",
			    qual, PHSIZE, name, fm, offset,
			    uncert, tmestr);
		    
#else
		    /* "old" format-- still used at LBL */
		    sprintf(lbl_name, "%s%s",bh->station, 
			    bh->channel);
		    fprintf(fp," %-7s %4d %c %-4s %.4f %c %.4f %s\n",
			    lbl_name, pl->sampIdx, qual, name,
			    offset, fm, pl->uncertainty, tmestr);
#endif
		}
		pl=pl->next;
	    }
	}
    }
    fclose(fp);
}

int match_sncl(BIS3_HEADER *bh, char *s, char *n, char *c, char *l)
{
    if (strncmp(bh->station, s, STATSIZE) == 0 &&
	strncmp(bh->network, n, NETSIZE) == 0 &&
	strncmp(bh->channel, c, CHSIZE) == 0 &&
	strncmp(bh->location, l, LOCSIZE) == 0)
	return 1;
    return 0;
}

/*
 * pick_trace_diff: compares pick and trace times. If pick is between
 *    trace start and end, returns 0. If pick is before trace start, 
 *    returns picktime - start time; if pick is after trace end,
 *    returns end-time - picktime. All returned times are in usecs.
 */
double pick_trace_diff(BIS3_HEADER *bh, STI_TIME ptime)
{
    double sdiff, ediff;

    /* Can't use pick->secOffset because the pick structure isn't filled yet */
    sdiff = st_tdiff(bh->start_it, ptime);
    ediff = st_tdiff(ptime, 
		     st_add_dtime(bh->start_it, 
				  (double)bh->n_values * USECS_PER_SEC / 
				  bh->sample_rate));
    if (sdiff < 0.0 && ediff < 0.0 )
	return 0.0;
    else if (sdiff > 0)
	return -sdiff;
    else
	return ediff;
}

    
		  
Trace* newWaifTrace(char *stname, char *net, char *chan, char *loc, 
		    STI_TIME ptime_it)
{
    int i, newTraces;
    Trace *trh;
    BIS3_HEADER *bh;
    
    if (NumWaifTraces >= TotalWaifTraces) {
	newTraces = TotalWaifTraces + TotalInitTracks;
	waifTraces = (Trace **)Realloc(waifTraces, sizeof(Trace *)*newTraces);
	if(waifTraces==NULL) return 0; /* not enough mem */
	for (i = TotalWaifTraces; i < newTraces; i++){
	    waifTraces[i] = (Trace *)newTrace();
	    if (waifTraces[i] == NULL) return 0;
	}
	TotalWaifTraces = newTraces;
    }
    trh = waifTraces[NumWaifTraces++];
    trh->wave = (Wave *)Malloc(sizeof(Wave));
    if (trh->wave == NULL) return 0;
    bzero(trh->wave, sizeof(Wave));
    bh = &trh->wave->info;
    strncpy(bh->station, stname, STATSIZE);
    bh->station[STATSIZE-1] = '\0';
    strncpy(bh->network, net, NETSIZE);
    strncpy(bh->channel, chan, CHSIZE);
    strncpy(bh->location, loc, LOCSIZE);
    bh->dip= 0;
    bh->azimuth= -1.0;
    bh->latitude= 0;
    bh->longitude= 0;
    bh->elevation= 0;
    bh->depth= 0;
    bh->start_it = ptime_it;
    bh->sample_rate = 1.0;   /* better than zero! */
    
    return trh;
}



/*
 * Note that when the pick file is read in, the picks are matched with
 * the existing track names. They are assumed to be in the same order.
 * Any mismatch causes the next track to be matched.
 */
void ReadPicks(char *fname)
{
    FILE *fp;
    BIS3_HEADER *bh;
    Trace *trc;
    Pick *newpl;
    STI_TIME ptime_it;
    STE_TIME ptime_et;
    char stname[STATSIZE], net[NETSIZE], chan[CHSIZE], loc[LOCSIZE];
    char phname[PHSIZE], qual, fm;
    char buf[200], unc[20];
    int version=1;
    int i;
    double diff_time, least_diff;
    int sidx;
    double soff, uncert;
    double secs;
	
    if ( (fp = fopen(fname, "r")) == NULL) {
	if (errno == ENOENT)
	    fprintf(stderr, "Warning: %s does not exist\n", fname);
	else
	    fprintf(stderr, "Error opening %s: %s\n", fname, strerror(errno));
	return;
    }
    
    while(fgets(buf,200,fp)!=NULL) {
	
#ifdef SITE_BDSN
	/* read in the stuff */
	if (*buf!=' ') { 
	    /* Check for pickfile version */
	    switch (*buf) {
	    case 'P':
		if (sscanf(buf, "Pickfile V.%d", &version) != 1) {
		    fprintf(stderr, "Can't determine version of pickfile\n");
		    fclose(fp);
		    return;
		}
		break;
	    case 'E':
		if (strlen(EventID) > 0) {
		    char newEV[256];
		    if (sscanf(buf, "%*s %s", newEV) != 1) 
			fprintf(stderr, "Event ID missing from pick file\n");
		    if (strcmp(EventID, newEV) != 0) {
			fprintf(stderr, "%s event ID (%s) doesn't match current event (%s)\n",
				fname, newEV, EventID);
			fclose(fp);
			return;
		    }
		}
		else if (sscanf(buf, "%*s %s", EventID) != 1) {
		    fprintf(stderr, "Event ID missing from pick file\n");
		}
		break;
	    default:   /* nothing special */
		break;
	    }
	    continue;
	}
	bzero(stname, STATSIZE);
	bzero(net,NETSIZE);
	bzero(chan, CHSIZE);
	bzero(loc, LOCSIZE);
	switch(version) {
	case 1:    /* read original pick file format: station name only */
	    sscanf(buf,"%s %d %c %s %lf %c %lf %d %d %d:%d:%lf",
		   stname, &sidx, &qual, phname, &soff, &fm, &uncert,
		   &ptime_et.year,&ptime_et.doy,&ptime_et.hour,
		   &ptime_et.minute,&secs);
	    strcpy(net, default_network);
	    break;
	case 2:   /* version 2: station and channel names */
	    sscanf(buf,"%s %s %d %c %s %lf %c %lf %d %d %d:%d:%lf",
		   stname,chan,&sidx,&qual,phname,&soff,&fm,&uncert,
		   &ptime_et.year,&ptime_et.doy,&ptime_et.hour,
		   &ptime_et.minute,&secs);
	    if (strcmp(chan,"???") == 0)
		chan[0] = '\0';
	    strcpy(net, default_network);
	    break;
	case 3:  /* version 3: full SNCL names */
	    sscanf(buf,"%s %s %s %s %c %s %c %lf %s %d %d %d:%d:%lf",
		   stname,net,chan,loc,&qual,phname,&fm,&soff,unc,
		   &ptime_et.year,&ptime_et.doy,&ptime_et.hour,
		   &ptime_et.minute,&secs);
	    if (strcmp(chan,"???")==0)
		chan[0] = '\0';
	    if (net[0] == '?' || net[0] == '-')
		net[0] = '\0';
	    if (loc[0] == '?' || loc[0] == '-')
		loc[0] = '\0';
	    if (unc[0] == '?')
		uncert = -1.0;
	    else
		uncert = atof(unc);
	    
	    break;
	default:
	    fprintf(stderr, "Unsupported pickfile format: %d\n", version);
	    fclose(fp);
	    return;
	}

#else    
	/* Original pickfile format only */
	
	/* read in the stuff */
	if (*buf!=' ') continue; /* ignore */
	
	sscanf(buf,"%s %d %c %s %lf %c %lf %d %d %d:%d:%lf",
	       stname, &sidx, &qual, phname, &soff, &fm, &uncert,
	       &ptime_et.year,&ptime_et.doy,&ptime_et.hour,
	       &ptime_et.minute,&secs);
#endif    
	/* a fix for the previous erraneous pick files
	   note: for backward compatibility */
	if (stname[3]=='_')
	    stname[3]='\0';
	
	ptime_et.second= (int)secs;
	ptime_et.usec= (int)(0.5 + (secs - ptime_et.second) * USECS_PER_SEC);
	ptime_it = ste_to_sti( ptime_et);
	
	/*
	 * Match pick against the traces. Pick station, network,
	 * channel and location must match. In addition, the pick
	 * must be within the trace time interval, or it must be
	 * in the trace that is closest to the pick time.
	 * This is because some users want to look at several traces
	 * for different times of the same SNCL, thereby stretching
	 * the expectation that seistool will look at one event
	 * at a time.
	 */
	trc= NULL; 
	least_diff = 1.0e30;  /* a large number */
	diff_time = 0.0;
	for(i=0; i <= LastTrack; i++) {
	    bh = &(traces[i]->wave->info);
	    if (match_sncl(bh, stname, net, chan, loc) ) {
		diff_time = pick_trace_diff(bh, ptime_it);
		if (diff_time == 0.0) {
		    if (least_diff > 0.0) {
			trc=traces[i];
		    } 
		    else {
			fprintf(stderr, 
				"pick matches more than one trace\n\t%s\n",
				buf);
		    }
		    least_diff = diff_time;
		    
		}
		else if (fabs(diff_time) < fabs(least_diff)) {
		    least_diff = diff_time;
		    trc = traces[i];
		}
	    }
	}
	if (!trc) {
	    fprintf(stderr, "Pick matches no traces:\n\t%s",
		    buf);
	    for(i=0; i < NumWaifTraces; i++) {
		bh = &(waifTraces[i]->wave->info);
		if (match_sncl(bh, stname, net, chan, loc) ) {
		    trc=waifTraces[i];
		}
	    }
	}
	if (!trc) {
	    trc = newWaifTrace(stname, net, chan, loc, ptime_it);
	    if (!trc) {
		fprintf(stderr, "out of memory\n");
		exit(1);
	    }
	}
	
	
	newpl=AddPicks(trc->picks);
	newpl->secOffset = st_tdiff(ptime_it, trc->wave->info.start_it);
	
	switch(qual) {
	case 'i': 
	    newpl->quality=IMPULSIVE;
	    break;
	case 'e': 
	    newpl->quality=EMERGENT;
	    break;
	default: 
	    newpl->quality=UNKNOWN_Q;
	    break;
	}
	if (strcmp(phname, "?") != 0) {
	    strncpy(newpl->phaseName, phname, PHSIZE);
	    newpl->phaseName[PHSIZE-1] = '\0';
	}
	
	switch(fm) {
	case 'U': 
	    newpl->firstMotion= UP; 
	    break;
	case 'D':
	    newpl->firstMotion= DOWN; 
	    break;
	default:
	    newpl->firstMotion= UNKNOWN_M; 
	    break;
	}
	newpl->uncertainty= uncert;
	SetPickLabel(newpl);
	trc->picks= newpl;
    }
    fclose(fp);
    /* DEBUG*/
    fprintf(stderr, "picks: %d\n", NumWaifTraces);
}


static STI_TIME Find_firstpick_time(Trace *trc)
{
  Pick *pk;
  double first_t = 31536000.0; /* one year in seconds */
  STI_TIME it;
  
  pk=trc->picks;

  while(pk!=NULL) {
      if (pk->secOffset < first_t)
	  first_t = pk->secOffset;
      pk=pk->next;
  }
  it = st_add_dtime( trc->wave->info.start_it, first_t * USECS_PER_SEC);

  return it;
}

int pick_cmp_trace(Trace *trc1, Trace *trc2)
{
    return TimeCmp(Find_firstpick_time(trc1), Find_firstpick_time(trc2) );
}

static STI_TIME Find_firstpick_trip(Triplet *trp)
{
  STI_TIME f_p[3],ret_time;
  int i=0,j;

  if (trp->trc[TRC_X]!=NULL) f_p[i++]=Find_firstpick_time(trp->trc[TRC_X]);
  if (trp->trc[TRC_Y]!=NULL) f_p[i++]=Find_firstpick_time(trp->trc[TRC_Y]);
  if (trp->trc[TRC_Z]!=NULL) f_p[i++]=Find_firstpick_time(trp->trc[TRC_Z]);

  ret_time = f_p[0];

  for (j=1; j<i; j++) {
      if (TimeCmp(f_p[j], ret_time) < 0)
	  ret_time = f_p[j];
  }

  return ret_time;
}


int pick_cmp_trip(Triplet *trp1,Triplet *trp2)
{
  return TimeCmp(Find_firstpick_trip(trp1), Find_firstpick_trip(trp2));
}

void sprintDipAzString(Trace *trc, char *da_str, int use_rot)
{
    BIS3_HEADER *bh = &trc->wave->info;

    if (bh->azimuth < 0)
	strcpy(da_str, "- -");
    else {
	if (use_rot > 0 && trc->trip && trc->trip->rotated == 1) {
	    /* triplet is rotated about vertical; use the 
	       synthetic azimuth angles for N and E components */
	    if (trc == trc->trip->trc[TRC_Y])   /* 'N' */
		sprintf(da_str, "%5.1f %5.1f", bh->dip, 
			trc->trip->rot_theta);
	    else if (trc == trc->trip->trc[TRC_X]) {  /* 'E' */
		float theta = trc->trip->rot_theta -
		    trc->trip->sta_rotation + bh->azimuth;
		if (theta > 360.0) 
		    theta -= 360.0;
		else if (theta < 0.0)
		    theta += 360.0;
		sprintf(da_str, "%5.1f %5.1f", bh->dip, theta);
	    }
	    else                              /* 'Z' */
		sprintf(da_str, "%5.1f %5.1f", bh->dip, bh->azimuth);
	} 
	else {  /* not rotated, just use the instrument values */
	    sprintf(da_str, "%5.1f %5.1f", bh->dip, bh->azimuth);
	}
    }
    return;
}

		
    
/*
 * getEarliestPick--
 *    find earliest arrival time amongst all picks
 */
static STI_TIME getEarliestPick(int includeWaifs)
{
    STI_TIME earl, ptime, *fst;
    Pick *pl;
    int i;
    
    earl.year = 3000;
    earl.second = earl.usec = 0;
    
    for(i=0; i<=LastTrack; i++) {
	pl= traces[i]->picks;
	fst = &traces[i]->wave->info.start_it;
	while(pl!=NULL) {   /* loop thro' all picks */
	    if (pl->type==PICK) {
		ptime = st_add_dtime( *fst, pl->secOffset * USECS_PER_SEC);
		if (TimeCmp(earl, ptime) > 0) {
		    earl = ptime;
		}
	    }
	    pl=pl->next;
	}
    }
    if (includeWaifs) {
	for(i=0; i<NumWaifTraces; i++) {
	    pl= waifTraces[i]->picks;
	    fst = &waifTraces[i]->wave->info.start_it;
	    while(pl!=NULL) {   /* loop thro' all picks */
		if (pl->type==PICK) {
		    if (TimeCmp(earl, *fst) > 0) {
			earl = *fst;
		    }
		}
		pl=pl->next;
	    }
	}
    }
    
    return earl;
}

