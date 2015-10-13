#ifndef lint
static char id[] = "$Id: special.c,v 1.3 2013/02/28 21:24:57 lombard Exp $";
#endif


#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include "proto.h"
#include "xv_proto.h"


int Mode_NCD= 0;

static void print_dimensions(FILE *fp);
/* This might be an LBNL thing; not found at UCB */
void defaults_load_db(char *);

/**********************************************************************
 *   Load in Windows                                                  *
 **********************************************************************/

extern Trace **traces;
extern int TotalTracks;
extern int lowTrkIndex;
extern int highTrkIndex;
extern int NumTracks;



/* historically: load in amplitude windows created by
 *     "geo.lbl.gov:~andrew/ampli/ampli" for UW files
 * format:-
station_name	lowIndex    highIndex
eg. 1v 581 641
*/
void LoadWindows(char *fname)
{
    FILE *fp;
    char buf[100];
    Pick *pl;
    char stat[20];
    int ix1, ix2, n;
    
    if ((fp=fopen(fname,"r"))==NULL) {
	/* if no such file exists, forget it */
	return;
    }
    while(fgets(buf,100,fp)!=NULL) {
	sscanf(buf, "%s %d %d", stat, &ix1, &ix2);
	if (ix1>ix2) {
	    int temp=ix1; ix1=ix2; ix2=temp;
	}
	for(n=0; n < TotalTracks; n++) {
	    if (traces[n] && traces[n]->wave) {
		if (!strncmp(stat, traces[n]->wave->info.station,
			strlen(stat))) {

		    pl= AddPicks(traces[n]->picks);
		    /* Let's assume that these indexes are sample counts */
		    pl->secOffset = st_tdiff(indexToTime(traces[n], ix2, 1),
					     traces[n]->wave->info.start_it);
		    pl->type=RMARK;
		    pl= AddPicks(pl);
		    pl->secOffset = st_tdiff(indexToTime(traces[n], ix1, 1),
					     traces[n]->wave->info.start_it);
		    pl->type=LMARK;
		    traces[n]->picks= pl;

		    break;    /* found */
		}
	    }
	}
    }
    if (ZoomWindowMapped)
	RedrawZoomWindow("LoadWindows");
}

/**********************************************************************
 *   Write out Pick File                                              *
 **********************************************************************/

void WriteOutPickFile()
{
    extern int ChangesMade;
    char pfname[MAXFILENAME+3];
    char rfname[MAXFILENAME+3];
    char afname[MAXFILENAME+3];
    EqEvent *curEvt;

    curEvt= getCurrentEvt();
    if (curEvt!=NULL) {
	/* if more than one trace file is open, the first file will
	   be used as the pick file name */
	cvtToPFname(pfname, curEvt->evt->name,
		curEvt->evt->format);
	cvtToRFname(rfname, curEvt->evt->name);
	cvtToAFname(afname, curEvt->evt->name, curEvt->evt->format);
	WritePicks(pfname);
	WriteRotation(rfname);
	WriteAmps(afname);
	ResetChangesFlag();  /* reset changes flag */
    }
}


/**********************************************************************
 *   Grouping Traces                                                  *
 **********************************************************************/

void restore_orig_order()
{
    int i;
    RestoreLoadOrder();
    lowTrkIndex= 0;
    highTrkIndex= NumTracks-1;
    lowZTrkIndex= 0;
    highZTrkIndex= NumZTracks-1;
    for(i=0; i < TotalTracks; i++) {
	if (traces[i]) {
	    traces[i]->zaxis.hs=0;
	}
    }
    /* rescale zoomed traces, if necessary */
    if (ZoomWindowMapped) {
	for(i=lowZTrkIndex; i<=highZTrkIndex; i++) {
	    ScaleZTrack(traces[i], traces[lowZTrkIndex]);
	}
	RedrawZoomWindow("restore_orig_order");
    }
    RedrawScreen();
}

/**********************************************************************
 *   Dump picks                                                       *
 **********************************************************************/

void dump_all_picks()
{
    char buf[1000];
    GetString("Pick file: ", buf, 1000);
    if (*buf!='\0') WritePicks(buf);
}


/**********************************************************************
 *   NCD PROBLEM                                                      *
 **********************************************************************/

void check_NCD()
{
    char *disp= (char *)getenv("DISPLAY");
    if(disp &&
       /* this hosts are NCD's */
       (!strncmp(disp,"edk", 3) ||
	!strncmp(disp,"quinn",5) ||
	!strncmp(disp,"xt0",3) ||
	!strncmp(disp,"xt1",3) ||
	!strncmp(disp,"td0",3) ||
	!strncmp(disp,"td1",3) ||
	!strncmp(disp,"td2",3) ||
	!strncmp(disp,"td3",3))) {

	Mode_NCD= 1;
	/* This might be an LBNL thing; not found at UCB */
	defaults_load_db("/m/home/u/andrew/SEISTOOL/src/.Xdefaults-ncd");

    }
}



static void print_dimensions(FILE *fp)
{
    fprintf(fp,"position main %d %d\n",xv_get(tracesFrame,XV_X),
	   xv_get(tracesFrame,XV_Y));
    printZoomFrameDim(fp);
    fprintf(fp,"size main %d %d\n", FrmWidth, FrmHeight);
    fprintf(fp,"size zwin %d %d\n", ZCvsWidth+ZTrkLabWidth,
	   ZCvsHeight+ZSCvsHeight);
}

void save_config()
{
    FILE *fp;
    char fname[256], oname[256];
    extern char *environ_home;
    int rc;
    
    sprintf(fname, "%s/.seistool-init", environ_home);
    sprintf(oname, "%s.old", fname);
    if ( (rc = rename(fname, oname)) < 0) {
	if (errno != ENOENT) {
	    fprintf(stderr, "Error renaming %s: %s\n", fname, strerror(errno));
	    return;
	}
    }
    

    if ((fp=fopen(fname, "a"))==NULL) {
	fprintf(stderr, "Cannot write to %s: %s\n",fname, strerror(errno));
	return;
    }
    if (rc < 0) 
	fprintf(stderr,"Creating %s\n",fname);
    else
	fprintf(stderr,"Writing new settings to %s;\n\tsaving old settings in %s\n", 
		fname, oname);
    fprintf(fp,"# SeisTool configuration (SeisTool v3.0)\n");
    print_dimensions(fp);
    save_config_var(fp);
    fclose(fp);
}
