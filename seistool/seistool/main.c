#ifndef lint
static char id[] = "$Id: main.c,v 1.9 2013/02/28 21:24:59 lombard Exp $";
#endif

/*
 * SeisTool 
 *   
 * Copyright (c) 1997 The Regents of the University of California.
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for educational, research and non-profit purposes,
 * without fee, and without a written agreement is hereby granted,
 * provided that the above copyright notice, this paragraph and the
 * following three paragraphs appear in all copies.
 * 
 * Permission to incorporate this software into commercial products may
 * be obtained from the Office of Technology Licensing, 2150 Shattuck
 * Avenue, Suite 510, Berkeley, CA  94704.
 * 
 * IN NO EVENT SHALL THE UNIVERSITY OF CALIFORNIA BE LIABLE TO ANY PARTY
 * FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES,
 * INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND
 * ITS DOCUMENTATION, EVEN IF THE UNIVERSITY OF CALIFORNIA HAS BEEN
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * THE UNIVERSITY OF CALIFORNIA SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE
 * PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND THE UNIVERSITY OF
 * CALIFORNIA HAS NO OBLIGATIONS TO PROVIDE MAINTENANCE, SUPPORT,
 * UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 */

#include <stdlib.h>
#include <limits.h>
#include <strings.h>
#include <xview/xview.h>
#include <xview/canvas.h>
#include <xview/panel.h>
#include <xview/xv_xrect.h>
#include <xview/notice.h>

#include "proto.h"
#include "xv_proto.h"


/**********************************************************************
 *   Global Variables                                                 *
 **********************************************************************/
Frame	tracesFrame;
Canvas  tracesScvs;
int	FrmHeight= 600;
int	FrmWidth= 800;

extern int subsample_rate;
extern int Mode_timescale;
extern float fq_fl_value;
extern float fq_fh_value;
extern int Mode_Auto_redraw;
extern int Mode_abs_vscale;
extern int TotalTracks;
extern int NumTracks;
extern int highTrkIndex;
extern char default_network[NETSIZE];
extern char EventID[256];

static int	FrmX=-1, FrmY= -1;

char *DEFAULT_LEFT_FOOTER="No file.";

/**********************************************************************
 *   Tracking changes:                                                *
 *     tracking changes that needs to be written back to disk.        *
 **********************************************************************/
int ChangesMade= 0;

void SetChangesFlag()
{
    char *footer, *newfooter;
    
    if (!ChangesMade) {
	ChangesMade= 1;
	footer=(char *)xv_get(tracesFrame, FRAME_LEFT_FOOTER);
	newfooter= (char *)Malloc(strlen(footer)+3);
	if (newfooter==NULL) return;	/* not enuff mem */
	strcpy(newfooter,footer);
	strcat(newfooter," *");
	xv_set(tracesFrame, FRAME_LEFT_FOOTER, newfooter, NULL);
	ZWHdr_ShowChange();
    }
}

void ResetChangesFlag()
{
    char *footer, *newfooter;

    if (ChangesMade) {
	ChangesMade= 0;
	footer=(char *)xv_get(tracesFrame, FRAME_LEFT_FOOTER);
	newfooter= (char *)Malloc(strlen(footer)+3);
	if (newfooter==NULL) return; /* not enuff mem */
	strcpy(newfooter,footer);
	newfooter[strlen(footer)-2]='\0';
	xv_set(tracesFrame, FRAME_LEFT_FOOTER, newfooter, NULL);
	ZWHdr_DismissChange();
    }
}

#define NOTICE_CANCEL	101
int CheckChangesFlag()
{
    Xv_notice notice;
    int notice_stat;
    
    if (!ChangesMade) return 1;

    notice= xv_create(tracesFrame, NOTICE,
	NOTICE_MESSAGE_STRINGS, "Changes made since last save.",
			 "Do you want to save?", NULL,
	NOTICE_BUTTON, "Yes", NOTICE_YES,
	NOTICE_BUTTON, "No", NOTICE_NO,
	NOTICE_BUTTON, "Cancel", NOTICE_CANCEL,
	NOTICE_STATUS, &notice_stat,
	XV_SHOW, TRUE,
	NULL);
    switch(notice_stat){
	case NOTICE_YES:
	    WriteOutPickFile();
	    return 1;	    /* written, can proceed */
	    break;
	case NOTICE_NO:
	    ChangesMade=0;  /* forget it */
	    return 1;	    /* OK, that's what you say */
	    break;
    }
    return 0;	/* cancels */
}

/**********************************************************************
 *   Main Program                                                     *
 **********************************************************************/

/* prototypes */
static void parseArgBefore(int argc, char **argv);
static void parseInitFile(void);
static void parseArgAfter(int argc, char **argv);

extern char *environ_home;
extern char *defaultPrinter;

int Mode_CLI= 0;

#ifdef SITE_BDSN
#define RELEASE_NOTES_FILE  "/data/cvs01/lombard/bdsn/new-seistool/release.motd"
#else
#define RELEASE_NOTES_FILE  "/m/home/u/andrew/SEISTOOL/src/release.motd"
#endif

int main(int argc, char **argv)
{
    char dir[PATH_MAX+1], title[PATH_MAX+20], *pc;

    fprintf(stderr, "Welcome to SeisTool Version 3.2.4 (2005.042)\n");
    fprintf(stderr, "Copyright (c) 2004 The Regents of the University of California.\n");

    {
	FILE *fp;
	char buf[500];
	if (fp=fopen(RELEASE_NOTES_FILE,"r")) {
	    while(fgets(buf,500,fp)!=NULL)
		fprintf(stderr,"%s",buf);
	    fclose(fp);
	}
    }

    /* print help screen and quit if "-h" flag passed */
    if (argc>1 && !strncmp(argv[1],"-h",2)) {
	printHelp();
	exit(0);
    }

    fprintf(stderr, "\nInitializing");
    bzero(EventID, 256);
    
    environ_home=(char *)getenv("HOME");
    defaultPrinter=(char *)getenv("PRINTER");
    
    if (argc>1 && !strcmp(argv[1],"-cli")) {
	Mode_CLI= 1;

	parseInitFile();
	parseArgBefore(argc, argv);
	InitTrackMgr();
	initTracks(NULL);
	parseArgAfter(argc, argv);
	
	fprintf(stderr,".\n");
	/* use the command line interface */
	cli_eval_loop();
	exit(0);    
    }

#ifdef SITE_LBL    
    check_NCD();    /* NCD have weird fonts set up. (in special.c) */
#endif
#ifdef SITE_BDSN
    strcpy(default_network, "BK"); /* the default default network */
#endif
    
    xv_init(XV_INIT_ARGC_PTR_ARGV, &argc, argv, NULL);
    fprintf(stderr, "."); /* xv_init OK */

    parseInitFile();
    parseArgBefore(argc, argv);   /* before initialization... */

    getcwd(dir, PATH_MAX);
    if ((pc = strrchr(argv[0], (int)'/')) == NULL) 
	pc = argv[0];
    else
	pc++;
    sprintf(title, "%s: %s", pc, dir);
    
    tracesFrame= (Frame)xv_create(NULL, FRAME, 
				  /*FRAME_LABEL, "SeisTool", */
				  FRAME_LABEL, title,
				  /*XV_WIDTH, FrmWidth, XV_HEIGHT, FrmWidth,*/
				  FRAME_SHOW_FOOTER, TRUE,
				  FRAME_LEFT_FOOTER, DEFAULT_LEFT_FOOTER,
				  NULL);
    xv_set(tracesFrame,
	WIN_EVENT_PROC, ResizeScreen, 
	WIN_CONSUME_EVENTS, WIN_RESIZE, WIN_REPAINT, NULL,
	NULL);
    if(FrmX!=-1 || FrmY!=-1) {
	xv_set(tracesFrame, XV_X, FrmX, XV_Y, FrmY, NULL);
    }
    tracesScvs= (Canvas)xv_create(tracesFrame, CANVAS,
	XV_Y, 40,			  
	XV_WIDTH, XVSBARWIDTH, XV_HEIGHT, FrmHeight-40,
	OPENWIN_SHOW_BORDERS, FALSE,			  
	NULL);
    initScrollbar(tracesScvs);

    /* initialize the various setups */
    setup(tracesFrame);
    LoadFont(tracesFrame);
    fprintf(stderr, "."); /* load font OK */

    initPanel(tracesFrame);
    InitTrackMgr();
    initTracks(tracesFrame);
    window_fit(tracesFrame);
    InitZoomWindow(tracesFrame);
    InitLoadLocPanel(tracesFrame);
    InitRTPanel(tracesFrame);
    fprintf(stderr, "."); /* init Xv_objects OK */

    parseArgAfter(argc, argv);   /* after initialization... */
    fprintf(stderr, " done.\n");

    xv_main_loop(tracesFrame);
    exit(0);
}

void quit()
{
    if (!CheckChangesFlag())
	return;
    xv_destroy_safe(tracesFrame);
    exit(0);	/* hm... shouldn't have to do this! */
}


/**********************************************************************
 *   Parsing Command Line Arguments                                   *
 **********************************************************************/

static void parseArgAfter(int argc, char **argv)
{
    EqEvent *cur;
    EvtFile *ls;
    
    eqevt_parseArg(argc, argv);
    cur= getCurrentEvt();
    if (cur!=NULL) {
	ls= cur->evt;
	LoadEvent(ls);
    }
}

static void parseArgBefore(int argc, char **argv)
{
    argc--; argv++;
    while(argc>0) {
	if (**argv=='-') {
	    if (!strncmp(*argv, "-noborder", 9)) {
		Mode_noBorder= 1;
	    }else if (!strncmp(*argv, "-format", 7)) {
		FileFormat fmt;
		argc--; *argv++=NULL;
		
		/* should check whether have enuff args */
		fmt=atofilefmt(*argv);
		if(fmt!=-1)defaultFormat=fmt;
	    }else if (!strncmp(*argv, "-numtr", 6)) {
		int newnumtracks;
		argc--;*argv++=NULL;
		/* should check enuff arg */
		newnumtracks=atoi(*argv);
		if (newnumtracks<=TotalTracks) {
		    NumTracks= newnumtracks;
		    highTrkIndex= newnumtracks-1;
		}
	    }else if (!strncmp(*argv, "-numztr", 7)) {
		int newnumtracks;
		argc--;*argv++=NULL;
		/* should check enuff arg */
		newnumtracks=atoi(*argv);
		if (newnumtracks<=TotalTracks) {
		    NumZTracks= newnumtracks;
		    highZTrkIndex= newnumtracks-1;
		    /* this is a quick fix: */
		    ZTrkHeight=ZCvsHeight/newnumtracks;
		}
	    }else if (!strncmp(*argv, "-trcini", 7)) {
		argc--;*argv++=NULL;
		/* should check enuff arg */
		parseTRCiniFile(*argv);
	    }else if (!strncmp(*argv, "-align", 6)) {	      
	      Mode_align= 1;  /* enable time alignment */
	      Mode_ZDisplayTScale=1;
	    }else if (!strncmp(*argv, "-noalign", 8)) {
	      Mode_ZDisplayTScale=0;
	      Mode_align= 0;  /* disable time alignment */
	    }else if (!strncmp(*argv,"-talign",7)) {
	      int t_line=2;
	      argc--;*argv++=NULL;
	      t_line=atoi(*argv);
	      Mode_align= t_line;  /* enable time alignment */
	      Mode_ZDisplayTScale=1;
	    }else if (!strncmp(*argv, "-exec", 5)) {
		extern char *ExecScriptFname;
		argc--;*argv++=NULL;
		ExecScriptFname= *argv;
	    }else if (!strncmp(*argv, "-sele", 5)) {
		argc--;*argv++=NULL;
		ParseSelectFile(*argv);
		Mode_autoFilterNames= 1;
	    } else if (!strncmp(*argv, "-lofreq",7)) {
	      argc--;*argv++=NULL;
	      fq_fl_value=atof(*argv);
	    } else if (!strncmp(*argv, "-hifreq",7)) {	
	      argc--;*argv++=NULL;
	      fq_fh_value=atof(*argv);
	    }else if (!strcmp(*argv, "-subsample")) {
		int sub_sample_num;
		argc--;*argv++=NULL;
		/* should check enuff arg */
		sub_sample_num=atoi(*argv);
		if (sub_sample_num>0 && sub_sample_num<1000) {
		  subsample_rate=sub_sample_num;
		}
	    }else if (!strcmp(*argv, "-net")) {
		argc--;*argv++=NULL;
		strncpy(default_network, *argv, NETSIZE);
		default_network[NETSIZE-1] = '\0';
	    }else if (!strcmp(*argv, "-event")) {
		argc--;*argv++=NULL;
		strncpy(EventID, *argv, 255);
		EventID[255] = '\0';
	    }else if (!strncmp(*argv, "-cli", 4)) {
		;
	    }
	    
	    /* unknown switch will be ignored and nullified
	       (exception: leave -el for parseafter)	    */
	    if (strncmp(*argv,"-el",3)!=0 &&
		strncmp(*argv,"-fl",3)!=0 ) {
		*argv=NULL; /* done with this switch */
	    }
	}
	argc--; argv++;
    }
}


static void parseInitFile()
{
    FILE *fp;
    char buf[200];

    /* we try the current working directory first and then
       look under the home directory */
    if ((fp=fopen(".seistool-init","r"))==NULL) {
	if (environ_home) {
	    strcpy(buf,environ_home);
	    strcat(buf,"/.seistool-init");
	    if((fp=fopen(buf,"r"))==NULL)
		return; /* not found, give up */
	}
    }
    
    while(fgets(buf, 200, fp)!=NULL) {
	if (!strncmp(buf, "set", 3)) {
	    char key[200], val[200];

	    sscanf(buf,"%*s %s %s",key,val);
	    /*  set border [on | off]  */
	    if (!strncmp(key, "bor", 3)) {  
		if(!strncmp(val,"off",3))
		    Mode_noBorder= 1;
		else if(!strncmp(val,"on",2))
		    Mode_noBorder= 0;
	    /*  set time alignment  */
	    }else if (!strncmp(key, "align", 5)) {
		if(!strncmp(val,"off",3))
		    Mode_align= 0;
		else if(!strncmp(val,"on",2))
		    Mode_align= 1;
	    }else if (!strncmp(key, "talign", 6)) {
	      Mode_align=atoi(val);
	      /* FullScale(); */
	    /*  set auto-regroup mode */
	    }else if (!strcmp(key, "regroup")) {
		if(!strncmp(val,"off",3))
		    Mode_autoRegroup= 0;
		else if(!strncmp(val,"on",2))
		    Mode_autoRegroup= 1;
	    }else if (!strcmp(key, "groupcomp")) {
		if(!strncmp(val,"off",3))
		    Mode_autoGroupComp= 0;
		else if(!strncmp(val,"on",2))
		    Mode_autoGroupComp= 1;
	    /* autodemeaning */
	    }else if (!strcmp(key, "autodemean")) {
		extern int auto_demean;
		if(!strncmp(val,"off",3))
		    auto_demean= 0;
		else if(!strncmp(val,"on",2))
		    auto_demean= 1;
	    /* saveing picks with no traces */
	    }else if (!strcmp(key, "saveWaifPicks")) {
		extern int saveWaifPicks;
		if(!strncmp(val,"off",3))
		    saveWaifPicks= 0;
		else if(!strncmp(val,"on",2))
		    saveWaifPicks= 1;
	    /* decimate plot */
	    }else if (!strncmp(key, "decim",5)) {
		extern int Mode_decimPlot;
		if(!strncmp(val,"off",3))
		    Mode_decimPlot= 0;
		else if(!strncmp(val,"on",2))
		    Mode_decimPlot= 1;
	    /*  set uw pick style  */
	    }else if (!strncmp(key, "uwpick", 6)) {
		if(!strncmp(val,"off",3))
		    Mode_UWPickStyle= 0;
		else if(!strncmp(val,"on",2))
		    Mode_UWPickStyle= 1;
	    /*  set low frequency filter */
	    } else if (!strncmp(key,"lofreq",6)) {
	      fq_fl_value=atof(val);
            /*  set high frequency filter */
	    } else if (!strncmp(key,"hifreq",6)) {
	      fq_fh_value=atof(val);
	    /*  set time scale in zoom window */
	    }else if (!strncmp(key, "timescale", 9)) {
	      if(!strncmp(val,"off",3))
		Mode_ZDisplayTScale = 0;
	      else if(!strncmp(val,"on",2))
		Mode_ZDisplayTScale = 1;
	    /*  set auto loading of pick */
	    } else if (!strncmp(key, "pickload", 8)) {
	      if (!strncmp(val,"off",3)) {
		Mode_autoLoadPicks=0;
	      } else if (!strncmp(val,"on",2)) {
		Mode_autoLoadPicks=1;
	      }
            /* set auto scroll mode */
	    } else if (!strncmp(key, "ascroll", 7)) {
	      if (!strncmp(val,"off",3)) {
		Mode_autoScroll=0;
	      } else if (!strncmp(val,"on",2)) {
		Mode_autoScroll=1;
	      }
            /* set auto redraw mode */
	    } else if (!strncmp(key, "aredraw", 7)) {
	      if (!strncmp(val,"off",3)) {
		Mode_Auto_redraw=0;
	      } else if (!strncmp(val,"on",2)) {
		Mode_Auto_redraw=1;
	      }
            /* set subsample pick size */
	    } else if (!strncmp(key, "subsample", 9)) {
	      subsample_rate=atoi(val);
            /* set scale mode */
	    } else if (!strncmp(key, "vscale", 9)) {
	      switch (*val) {
	      case 'N': /* Normal mode */
	      case 'n':
		Mode_abs_vscale=0;
		Zoom_Mode_sameVertScale=0;
		break;
	      case 'Z':  /* zoom abs mode */
	      case 'z':
		Mode_abs_vscale=0;
		Zoom_Mode_sameVertScale=1;
		break;
	      case 'A':  /* all abs mode */
	      case 'a':
		Mode_abs_vscale=1;
		Zoom_Mode_sameVertScale=0;
		break;
	      }
	    /*  set format [BIS | SAC | SGY | TRC | etc. ]  */
	    }else if(!strncmp(key, "form", 4)) {
		FileFormat fmt;
		/* should check whether have enuff args */
		if((fmt=atofilefmt(val))!=-1) {
		    defaultFormat= fmt;
		}
#if 0	/* don't need set total now */
	    /*  set total value  */
	    }else if(!strncmp(key, "total", 5)) {
		int numtr;
		/* should check whether have enuff args */
		numtr=atoi(val);
		TotalTracks= numtr;
#endif
	    /*  set numtr value  */
	    }else if (!strncmp(key, "numtr", 5)) {
		int newnumtracks;
		newnumtracks=atoi(val);
		if (newnumtracks<=TotalTracks) {
		    NumTracks= newnumtracks;
		    highTrkIndex= newnumtracks-1;
		}
	    /*  set numztr value  */
	    }else if (!strncmp(key, "numztr", 6)) {
		int newnumtracks;
		newnumtracks=atoi(val);
		if (newnumtracks<=TotalTracks) {
		    NumZTracks= newnumtracks;
		    highZTrkIndex= newnumtracks-1;
		    /* this is a quick fix: */
		    ZTrkHeight=ZCvsHeight/newnumtracks;
		}
	    /* set instr resp file */
	    }else if (!strncmp(key, "instr", 4)) {
		extern char *default_instr_file;
		default_instr_file= 
		    (char *)strcpy(Malloc(strlen(val)+1),val);
	    /* set alternate instr resp file */
	    }else if (!strcmp(key, "instr-alt")) {
		extern char *alt_instr_file;
		alt_instr_file= 
		    (char *)strcpy(Malloc(strlen(val)+1),val);
	    /*  set trcinit filename  */
	    }else if (!strncmp(key, "trci", 4)) {
		parseTRCiniFile(val);
	    }
	}else if (!strncmp(buf, "pos", 3)) { /* position */
	    char obj[200];
	    int x, y;
	    sscanf(buf,"%*s %s %d %d",obj,&x,&y);

	    if (!strncmp(obj, "main", 4)) {  
		FrmX= x; FrmY= y;
	    }else if (!strncmp(obj, "zwin", 4)) {
		extern int ZFrmX, ZFrmY;
		ZFrmX=x; ZFrmY=y;
	    }else if (!strncmp(obj, "zpan", 4)) {
		extern int ZPFrmX, ZPFrmY;
		ZPFrmX=x; ZPFrmY=y;
	    }
	}else if (!strncmp(buf, "size", 3)) { /* position */
	    char obj[200];
	    int w, h;

	    sscanf(buf,"%*s %s %d %d",obj,&w,&h);
	    if (!strncmp(obj, "main", 4)) {  
		FrmWidth= w;
		FrmHeight= h;
	    }else if (!strncmp(obj, "zwin", 4)) {
		ZCvsWidth= w-ZTrkLabWidth;
		ZCvsHeight= h-ZSCvsHeight;
	    }
	} else if (!strcmp(buf, "network")) {
	    strncpy(default_network, buf, NETSIZE);
	    default_network[NETSIZE-1] = '\0';
	}
    }
    fclose(fp);
}


void printHelp()
{
    printf("\n\
{ <file1> <file2> ... }  load group of files as events\n\
<file> ...               load files specified as events\n\
-el <file>  use event list file (multiple files delimited by ::)\n\
-fl <file>  use file list file  (single file as events)\n\
\n\
-format [ BIS | TRC | MSEED | SEGY | SAC ]   select format\n\
-noborder  set no border             -trcini    set TRC init file\n\
-align     set time align mode       -noalign   set no time align mode\n\
-numtr <num>  set number of traces   -numztr <num> set number of zoomed traces\n\
-exec <file>  set execute script     -sele <file>  set select traces script\n");
}
