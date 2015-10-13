#ifndef lint
static char id[] = "$Id: amplitude.c,v 1.4 2013/02/28 21:25:01 lombard Exp $";
#endif

/*
 * amplitude.c--
 *    calculation of amplitude information
 *
 * Copyright (c) 1992-3 Andrew K. Yu, Lawrence Berkeley Laboratory
 *      and University of California, Berkeley.
 * All rights reserved.
 */ 
#include <math.h>
#include <stdio.h>
#include <strings.h>
#include "instr.h"
#include "mathlib.h"
#include "proto.h"
#include "xv_proto.h"

extern Trace **traces;
extern Track **tracks;
extern int LastTrack;
extern int saveWaifPicks;
extern int NumWaifTraces;
extern Trace **waifTraces;
extern char EventID[256];
extern char default_network[NETSIZE];

static STI_TIME getEarliestAmp(int includeWaifs);
static char *type2str(int which );
static char *ftype2str(int which );
static void AdjustTrace(int itrc);
static void PrintAmpLine(FILE *fp, BIS3_HEADER *bh, int which, float gdMax,
			 float perMax, int idx);
static void PrintAmpLine3(FILE *fp, STI_TIME ref_time, BIS3_HEADER *bh,
			  int which, float gdMax, float uncert, float perMax,
			  int idx);
static void splitStatChan(char *statchan, char *stat, char *chan);
static STI_TIME getEarliestAmp(int includeWaifs);


int CheckResponse(int itrc)
{    
    BIS3_HEADER *bh= &traces[itrc]->wave->info;
    if (bh->response < 0)
	return 0;
    return 1;
}

static char *type2str(int which)
{
    switch(which) {
    case 0: return "Do not deconvolve";
    case 1: return "Benioff (100kg)";
    case 2: return "Butterworth filter";
    case 3: return "Wood Anderson";
    case 4: return "ULP instrument";
    case 10: return "ASRO LP response";
    case 11: return "STS2 LP response";
    case 12: return "Highpass Filter";
    case 20: return "Ground Displacement";
    case 21: return "Ground Velocity";
    case 22: return "Ground Acceleration";
    default: return "unknown";
    }
}

static char *ftype2str(int which )
{
    switch(which) {
    case 0: return "Do not filter";
    case 1: return "Bandpass Filter Causal";
    case 2: return "Bandpass Filter Non-causal";
    case 3: return "Lowpass Filter Causal";
    case 4: return "Lowpass Filter Non-Causal";
    case 5: return "Highpass Filter Causal";
    case 6: return "Highpass Filter Non-Causal";
    default: return "unknown";
    }
}

Amp *AddAmps(Amp *pl)
{
    Amp *new= (Amp *)Malloc(sizeof(Amp));
    if (new==NULL) return pl;	/* not enuff mem */
    new->type= RAW_A;
    new->idx = 0;
    new->gdMax = new->perMax = new->gdUncert = 0.0;
    new->next = pl;
    return new;
}


Amp *FindAmpType(Amp *pl, int which)
{
    Amp *t = pl;
    while (t != NULL) {
	if (t->type == which) break;
	t = t->next;
    }
    return t;
}


/*
 * Calculate Amplitudes
 */
void CalcAmpli()
{
    Amp *pl;
    int which;
    int i, count=0;
    int idx;
    float rawMax, gdMax, uncert, perMax;
    int do_all;

    GetResponses(NULL);
    do_all= (firstSelectedTrc()==-1);   /* do all if none selected */
    for(i=0; i <= LastTrack; i++) {
	BIS3_HEADER *bh= &traces[i]->wave->info;
	if(do_all || traces[i]->selected) {
	    count++;

	    if(!CheckResponse(i)) continue;
	    which = (int)cdata_find(traces[i],10);

	    if ( (pl = FindAmpType(traces[i]->amps, which)) == NULL)
		pl = AddAmps(traces[i]->amps);

	    InitInstrResponse(bh->response, bh->digital_sens,
			      bh->gain_factor, bh->n_poles, bh->n_zeros, 
			      bh->poles_zeros);
	    ComputeMaxAmpPeriod(bh->n_values, bh->sample_rate, which,
				traces[i]->wave->data, &rawMax, &gdMax,
				&perMax, &idx);
	    /* How do we determine the uncertainty? Leave it -1 (null) for now */
	    uncert = -1.0;  

	    printf("[%d] %s %s %s %s %s -\n", i, bh->station, bh->network, 
		   bh->channel, 
		   (strlen(bh->location) > 0) ? bh->location : "--" , 
		   type2str(which));
	    if (which==3) { /* wood anderson */
		printf("   rawMax=%f  Record max=%f mm  period=%f at [%d]\n",
		       rawMax,gdMax,perMax,idx-1);
	    }else {
		printf("   rawMax=%f  gdMax=%f microns  period=%f at [%d]\n",
		       rawMax,gdMax,perMax,idx-1);
	    }
	    printf("   log(gdMax/period)= %f\n",(float)log10(gdMax/perMax));
	    pl->idx = idx-1; /* subtract 1 since fortran index starts aat 1 */
	    pl->type = which;
	    pl->gdMax = gdMax;
	    pl->perMax = perMax;
	    pl->gdUncert = uncert;
	    traces[i]->amps = pl;
	}
    }
    printf("*** Calc Ampli: %d traces selected\n",count);
}

static void AdjustTrace(int itrc)
{    
    Trace *trc= traces[itrc];
    float max, min, *data;
    int j;

    data= trc->wave->data;
    max= min= data[0];
    for(j=1;j<trc->wave->info.n_values; j++) {
	if (data[j]>max)
	    max= data[j];
	else if (data[j]<min)
	    min= data[j];
    }

    trc->axis.ymax= max;
    trc->axis.ymin= min;
    trc->axis.y1= max;
    trc->axis.y2= min;

    trc->zaxis.ymax= max;
    trc->zaxis.ymin= min;
    trc->zaxis.y1= max;
    trc->zaxis.y2= min;
    trc->zaxis.vs=0;
    ScaleYAxis(&trc->axis, tracks[0]->height);
    ScaleYAxis(&trc->zaxis, ZTrkHeight);
}

void ConvRespTrace(int which, int itrc)
{
    Trace *trc= traces[itrc];
    BIS3_HEADER *bh= &trc->wave->info;

    if(!CheckResponse(itrc)) return;
    InitInstrResponse(bh->response, bh->digital_sens,
		      bh->gain_factor, bh->n_poles, bh->n_zeros, bh->poles_zeros);
    printf("  converting instr ... ");
    fflush(stdout);
    ConvertInstr(bh->n_values, bh->sample_rate, which,
		 trc->wave->data);
    printf("  %s\n",type2str(which));
    AdjustTrace(itrc);
}

/*
 * Instrument deconvolution and stuff
 */
void ConvertResponses(int which)
{
    int i, count=0;
    char buf[80];
    int do_all;

    GetResponses(NULL);
    do_all= (firstSelectedTrc()==-1);   /* do all if none selected */
    for(i=0; i <= LastTrack; i++) {
	if(do_all || traces[i]->selected) {
	    ConvRespTrace(which, i);
	    cdata_insert(traces[i], 10, which);
	    count++;	
	}
    }
    printf("*** %d traces selected\n",count);
    RedrawScreen();
    if(ZoomWindowMapped)
	RedrawZoomWindow("ConvertResponses");
}


void filterTrc(int which, int decon, int itrc, int fl, float fh)
{
    Trace *trc= traces[itrc];
    BIS3_HEADER *bh= &trc->wave->info;
    float max, min, *data;
    int j;

    if(decon !=0 && !CheckResponse(itrc)) return;

    InitInstrResponse(bh->response, bh->digital_sens,
		      bh->gain_factor, bh->n_poles, bh->n_zeros, bh->poles_zeros);
    printf("  filtering ... ");
    fflush(stdout);
    FilterTrace(bh->n_values, bh->sample_rate, which, decon,
		trc->wave->data, fl, fh);
    if (decon !=0) {printf("  %s  ",type2str(decon));}
    printf("  %s\n",ftype2str(which));
    AdjustTrace(itrc);
}

/*
 * 1- bandpass
 * 2- bandpass 0 phase
 * 3- lowpass
 * 4- lowpass 0 phase
 * 5- highpass
 * 6- highpass 0 phase
 *
 * all- 0: only selected; 1: all
 */
void ApplyFilter(int which, int decon, int all, float fl, float fh)
{
    extern int Mode_abs_vscale;	/* in track.c */

    int i, count=0;
    char buf[80];

    GetResponses(NULL);
    for(i=0; i <= LastTrack; i++) {
	if(all || traces[i]->selected) {
	    count++;
	    filterTrc(which,decon,i,fl,fh);
	    cdata_insert(traces[i], 10, decon);
	}
    }
    printf("*** %d traces selected\n",count);
    if (Mode_abs_vscale) {
	/*    printf("recalcing vert scale\n"); */
	newAbsVertScale;
    }
    RedrawScreen();
    if(ZoomWindowMapped)
	RedrawZoomWindow("ApplyFilter");
}

/* Reading and Writing Amp Files */

/* PrintAmpLine--  write out to old-format file */
static void PrintAmpLine(FILE *fp, BIS3_HEADER *bh, int which, float gdMax,
			 float perMax, int idx)
{
    char *s, inst[10];
    char *sta = bh->station;
    char *comp= bh->channel;

    switch(which) {
    case 20: s="D"; break;
    case 21: s="V"; break;
    case 22: s="A"; break;    
    case 1: s="P"; break;
    case 2: s="L"; break;
    case 3: s="WAS"; break;
    case 0: default: s="RAW"; break;
    }
    if(which==1 || which==2) {
        sprintf(inst,"%s%c",s, comp[strlen(comp)-1]);
    }else {
        strcpy(inst,s);
    }
    /* (stn_name) (inst) (ampli) (period) (index) (mag) */
    fprintf(fp," %s%s %s %.3f %.3f %d %.3f\n",
            sta, comp, inst, gdMax, perMax, idx, (float)log10(gdMax/perMax));
}


/* PrintAmpLine3--  write out to new-format file */
static void PrintAmpLine3(FILE *fp, STI_TIME ref_time, BIS3_HEADER *bh,
			  int which, float gdMax, float uncert, float perMax,
			  int idx)
{
    STI_TIME ptime;
    double offset;
    char *s, inst[10], tmestr[50];
    char *comp= bh->channel;
    char unc[20], per[20];

    switch(which) {
    case 20: s="PGD"; break;
    case 21: s="PGV"; break;
    case 22: s="PGA"; break;	  
    case 1: s="P"; break;
    case 2: s="L"; break;
    case 3: s="WAS"; break;
    case 0: default: s="RAW"; break;
    }
    if(which==1 || which==2) {
	sprintf(inst,"%s%c",s, comp[strlen(comp)-1]);
    }else {
	strcpy(inst,s);
    }
    /* reference offset to amp_file reference time instead of bh->start_it */
    offset = st_tdiff(bh->start_it, ref_time); 
    offset += (double)(idx / bh->sample_rate);
    ptime = st_add_dtime( ref_time, offset * USECS_PER_SEC);
    sprintTime(tmestr, ptime);

    if (uncert < 0.0) 
	strcpy(unc, "?    ");
    else 
	sprintf(unc, "%-10.4f", uncert);
    if (perMax < 0.0)
	strcpy(per, "?    ");
    else
	sprintf(per, "%-10.4f", perMax);
    
    fprintf(fp," %-5s %-2s %-3s %-2s %-8s %-10.4f %-10s %-10s %-s\n",
	    bh->station, bh->network, 
	    bh->channel, 
	    (strlen(bh->location) > 0) ? bh->location : "--", inst, 
	    gdMax, unc, per, tmestr);
    
}

void cvtToAFname(char *afname, char *fname, FileFormat format)
{
    char *pf;
    
    strcpy(afname, fname);
    switch(format) {
    case BIS_Format:
    case TRC_Format:
    case SDR_Format:
    case SEGY_Format:
    case SAC_Format:
    default:
	if ( (pf = strstr(afname, ".pf")) != NULL) *pf = '\0';
	strcat(afname,".af");
	break;
    }
}

void WriteAmps(char *fname)
{
    FILE *af_fp= NULL;
    STI_TIME earl;
    STE_TIME earl_et;
    Trace *trc;
    BIS3_HEADER *bh;
    Amp *pl;
    char tmestr[50];
    int i;
    
    if ((af_fp= fopen(fname, "w")) == NULL ) {
	/* try stripping off the path (which can be someone
	   else's directory) and retry */
	fname=getfilename(fname);
	if ((af_fp=fopen(fname,"w"))==NULL) {
	    /* give up */
	    fprintf(stderr,"Cannot write to file %s\n",fname);
	    return;
	}
    }
    NotifyWriting(fname);
    earl = getEarliestAmp(saveWaifPicks);
    earl_et = sti_to_ste(earl);
    earl_et.second = 0;
    earl_et.usec = 0;
    earl = ste_to_sti(earl_et);
    sprintTime(tmestr, earl);

#ifdef SITE_BDSN
    /* Start new-format amp file */
    fprintf(af_fp, "Ampfile V.3.00\n");
    fprintf(af_fp,"T %s\n",tmestr);
    if (strlen(EventID) > 0)
	fprintf(af_fp, "E %s\n", EventID);
    fprintf(af_fp, "H STN  NT CHN LC TYPE     AMP        UNCERT     PER        DATE\n");
#else
    /* Start old-format amp file */
    fprintf(af_fp, "# %s\n", curEvt->evt->name);
    fprintf(af_fp, "# STN_NAME TYPE AMPLI PERIOD INDEX MAG\n");
#endif

    for(i=0; i <= LastTrack; i++) {
	trc= traces[i];
	pl= trc->amps;
	bh = &trc->wave->info;
	while(pl!=NULL) {   /* loop thru all amps */
	    
#ifdef SITE_BDSN
	    PrintAmpLine3(af_fp,earl,bh,pl->type, pl->gdMax, pl->gdUncert, 
			  pl->perMax, pl->idx);
#else
	    PrintAmpLine(af_fp,bh,pl->type, pl->gdMax,pl->perMax,pl->idx);
#endif
	    pl = pl->next;
	}
    }
    if (saveWaifPicks) {
	for(i=0; i < NumWaifTraces; i++) {
	    trc= waifTraces[i];
	    pl= trc->amps;
	    bh = &trc->wave->info;
	    while(pl!=NULL) {   /* loop thru all amps */
		
#ifdef SITE_BDSN
		PrintAmpLine3(af_fp,earl,bh,pl->type, pl->gdMax, pl->gdUncert, 
			      pl->perMax, pl->idx);
#else
		PrintAmpLine(af_fp,bh,pl->type, pl->gdMax,pl->perMax,pl->idx);
#endif
		pl = pl->next;
	    }
	}
    }
    fclose(af_fp);
}

static void splitStatChan(char *statchan, char *stat, char *chan)
{
    int i;
    
    i = strlen(statchan) - 3;
    strncpy(chan, statchan+i, CHSIZE);
    statchan[i] = '\0';
    strncpy(stat, statchan, STATSIZE);
}


/*
 * Note that when the amp file is read in, the amps are matched with
 * the existing track names. They are assumed to be in the same order.
 * Any mismatch causes the next track to be matched.
 */
void ReadAmps(char *fname)
{
    FILE *fp;
    BIS3_HEADER *bh;
    Trace *trc;
    Amp *pl;
    STI_TIME ptime_it;
    STE_TIME ptime_et;
    char stname[STATSIZE], net[NETSIZE], chan[CHSIZE], loc[LOCSIZE];
    char statchan[STATSIZE+CHSIZE];
    char amptype[ATSIZE];
    char AmpEventID[256];
    char buf[200];
    char unc[20], per[20];
    int version=1;
    int i, idx, type;
    double diff_time, least_diff, secs;
    float gdMax, perMax, gdUncert;
    
    bzero(AmpEventID, 256);    
    if ( (fp = fopen(fname, "r")) == NULL) {
	if (errno == ENOENT) 
	    fprintf(stderr, "Warning: %s does not exist\n", fname);
	else
	    fprintf(stderr, "Error opening %s for reading: %s\n", fname,
		   strerror(errno));
	return;
    }
    
    while(fgets(buf,200,fp)!=NULL) {
	
#ifdef SITE_BDSN
	/* read in the stuff */
	if (*buf!=' ') { 
	    /* Check for ampfile version */
	    switch (*buf) {
	    case 'A':
		if (sscanf(buf, "Ampfile V.%d", &version) != 1) {
		    fprintf(stderr, "Can't determine version of ampfile\n");
		    fclose(fp);
		    return;
		}
		break;
	    case 'E':
		if (sscanf(buf, "%*s %s", AmpEventID) != 1) {
		    fprintf(stderr, "Event ID missing from amp file\n");
		}
		if (strcmp(AmpEventID, EventID) != 0) {
		    fprintf(stderr, "Event ID (%s) from %s doesn't match current event %s\n",
			    AmpEventID, fname, EventID);
		    fclose(fp);
		    return;
		}
		/* don't use the event ID; it should already be set by pickfile */
		break;
	    default:   /* nothing special */
		break;
	    }
	    continue;
	}
	bzero(statchan, STATSIZE+CHSIZE);
	bzero(stname, STATSIZE);
	bzero(net,NETSIZE);
	bzero(chan, CHSIZE);
	bzero(loc, LOCSIZE);
	switch(version) {
	case 1:    /* read original amp file format:      *
		    * station and channel jammed together */
	    sscanf(buf,"%s %s %f %f %d", statchan, amptype, &gdMax, &perMax, 
		   &idx);
	    splitStatChan(statchan, stname, chan);
	    strcpy(net, default_network);
	    break;
	case 3:  /* version 3: full SNCL names */
	    sscanf(buf,"%s %s %s %s %s %f %s %s %d %d %d:%d:%lf",
		   stname,net,chan,loc,amptype,&gdMax, unc, per,
		   &ptime_et.year,&ptime_et.doy,&ptime_et.hour,
		   &ptime_et.minute,&secs);
	    if (strcmp(chan,"???")==0)
		chan[0] = '\0';
	    if (net[0] == '?' || net[0] == '-')
		net[0] = '\0';
	    if (loc[0] == '?' || loc[0] == '-')
		loc[0] = '\0';
	    if (unc[0] == '?')
		gdUncert = -1.0;
	    else
		gdUncert = (float) atof(unc);
	    if (per[0] == '?')
		perMax = -1.0;
	    else
		perMax = (float) atof(per);
	    
	    ptime_et.second= (int)secs;
	    ptime_et.usec= (int)(0.5 + (secs - ptime_et.second) * USECS_PER_SEC);
	    ptime_it = ste_to_sti( ptime_et);
	    break;
	default:
	    fprintf(stderr, "Unsupported ampfile format: %d\n", version);
	    fclose(fp);
	    return;
	}

#else    
	/* Original ampfile format only */
	
	/* read in the stuff */
	if (*buf!=' ') continue; /* ignore */
	
	sscanf(buf,"%s %s %f %f %d", statchan, amptype, &gdMax, &perMax, &idx);
	splitStatChan(statchan, stname, chan);
	strcpy(net, default_network);

#endif    
	/* a fix for the previous erraneous amp files
	   note: for backward compatibility */
	if (stname[3]=='_')
	    stname[3]='\0';
	
	switch(amptype[0]) {
	case 'R':
	    type = RAW_A; break;
	case 'P':
	    switch(amptype[1]) {
	    case 'G':
		switch(amptype[2]) {
		case 'D':
		    type = PGD_A; break;
		case 'V':
		    type = PGV_A; break;
		case 'A':
		    type = PGA_A; break;
		default:
		    type = UNKNOWN_A;
		    fprintf(stderr, "Unknown amp type:\n\t%s\n", buf);
		}
		break;
	    case 'E':
	    case 'N':
	    case 'Z':
	    case '1':
	    case '2':
	    case '3':
		type = BENIOFF_A; break;
	    default:
		type = UNKNOWN_A;
		fprintf(stderr, "Unknown amp type:\n\t%s\n", buf);
	    }
	    break;
	case 'L':
	    type = BUTTER_A; break;
	case 'W':
	    type = WOOD_A; break;
	case 'D':
	    type = PGD_A; break;
	case 'V':
	    type = PGV_A; break;
	case 'A':
	    type = PGA_A; break;
	default:
	    type = UNKNOWN_A;
	    fprintf(stderr, "Unknown amp type:\n\t%s\n", buf);
	}
	
	/*
	 * Match amp against the traces. Amp station, network,
	 * channel and location must match. In addition, the amp
	 * must be within the trace time interval, or it must be
	 * in the trace that is closest to the amp time.
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
		if (version > 1) {
		    diff_time = pick_trace_diff(bh, ptime_it);
		    if (diff_time == 0.0 ) {
			if (least_diff > 0.0) {
			    trc=traces[i];
			} 
			else {
			    fprintf(stderr, 
				    "amp matches more than one trace\n\t%s\n",
				    buf);
			}
			least_diff = diff_time;
			
		    }
		    else if (fabs(diff_time) < fabs(least_diff)) {
			least_diff = diff_time;
			trc = traces[i];
		    }
		} 
		else {
		    if (trc == NULL)
			trc =traces[i];
		    else
			fprintf(stderr, "Version 1 amplitude matches more than one trace; amp time is indeterminate\n\t%s\n",
				buf);
		}
	    }
	}
	if (!trc) {
	    fprintf(stderr, "Amp matches no traces:\n\t%s",
		    buf);
	    for(i=0; i < NumWaifTraces; i++) {
		bh = &(waifTraces[i]->wave->info);
		if (match_sncl(bh, stname, net, chan, loc) ) {
		    trc=waifTraces[i];
		}
	    }
	}
	if (!trc) {
	    trc = (Trace *)newWaifTrace(stname, net, chan, loc, ptime_it);
	    if (!trc) {
		fprintf(stderr, "out of memory\n");
		exit(1);
	    }
	}
	
	
	if ( (pl = FindAmpType(trc->amps, type)) == NULL)
	    pl=AddAmps(trc->amps);

	if (version == 1)
	    pl->idx = idx;
	else
	    pl->idx = iround(st_tdiff(ptime_it, trc->wave->info.start_it) *
			       trc->wave->info.sample_rate);
	pl->gdMax = gdMax;
	pl->perMax = perMax;
	pl->gdUncert = gdUncert;
	pl->type = type;
	
	trc->amps= pl;
    }
    fclose(fp);
    /* DEBUG*/
    fprintf(stderr, "amps: %d\n", NumWaifTraces);
}

/*
 * getEarliestAmp--
 *    find earliest time amongst all amplitudes
 */
static STI_TIME getEarliestAmp(int includeWaifs)
{
    STI_TIME earl, ptime, *fst;
    BIS3_HEADER *bh;
    Amp *pl;
    double offset;
    int i;
    
    earl.year = 3000;
    earl.second = earl.usec = 0;
    
    for(i=0; i<=LastTrack; i++) {
	pl= traces[i]->amps;
	bh = &traces[i]->wave->info;
	fst = &bh->start_it;
	while(pl!=NULL) {   /* loop thro' all amps */
	    offset = (double)(pl->idx / bh->sample_rate);
	    ptime = st_add_dtime( *fst, offset * USECS_PER_SEC);
	    if (TimeCmp(earl, ptime) > 0) {
		earl = ptime;
	    }
	    pl=pl->next;
	}
    }
    if (includeWaifs) {
	for(i=0; i<NumWaifTraces; i++) {
	    pl= waifTraces[i]->amps;
	    bh = &waifTraces[i]->wave->info;
	    fst = &bh->start_it;
	    while(pl!=NULL) {   /* loop thro' all picks */
		offset = (double)(pl->idx / bh->sample_rate);
		ptime = st_add_dtime( *fst, offset * USECS_PER_SEC);
		if (TimeCmp(earl, *fst) > 0) {
		    earl = *fst;
		}
		pl=pl->next;
	    }
	}
    }
    
    return earl;
}

