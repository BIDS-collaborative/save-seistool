#ifndef lint
static char id[] = "$Id: plotwave.c,v 1.3 2013/02/28 21:24:58 lombard Exp $";
#endif

/*
 * plotwave.c--
 *    making hardcopies
 *
 * Copyright (c) 1992-3 Andrew K. Yu, Lawrence Berkeley Laboratory
 *      and University of California, Berkeley.
 * All rights reserved.
 */
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <time.h>

#include "proto.h"
#include "xv_proto.h"

extern Trace **traces;

static FILE *plt_fp;
static int plt_mode;
static float trcheight;

void InitPlot(char *fname, int ntrcs, int mode)
{
    plt_mode= mode;
    if(mode==0) {   /* to a file */
	if ((plt_fp=fopen(fname,"w"))==NULL) {
	    ReportFileNotFound(fname, 0);
	    exit(1);
	}
    }else { /* to a pipe */
	if ((plt_fp=popen(fname,"w"))==NULL) {
	    fprintf(stderr,"InitPlot: cannot start pipe.\n");
	    exit(1);
	}
    }
    fprintf(plt_fp, "%%!\n%%%%Creator: SeisTool (TM)\n");
    fprintf(plt_fp, "/Times-Roman findfont 8 scalefont setfont\n");
    fprintf(plt_fp, "/tempctm matrix def\n");
    fprintf(plt_fp, "/L { lineto } def\n");
    fprintf(plt_fp, "/sethdr { 5 5 moveto 580 0 rlineto 0 65 rlineto\n");
    fprintf(plt_fp, " -580 0 rlineto closepath\n");
    fprintf(plt_fp, " gsave .8 setgray fill grestore 2 setlinewidth\n");
    fprintf(plt_fp, " stroke 50 50 moveto\n");
    fprintf(plt_fp, " /Times-Roman findfont 18 scalefont setfont\n");
    fprintf(plt_fp, " (SeisTool) show 50 25 moveto\n");
    fprintf(plt_fp, " /Helvetica findfont 12 scalefont setfont\n");
    fprintf(plt_fp, "} def\n");

    /* calculate how many traces and how to lay them out */
    trcheight= (float)700/ntrcs;

    fprintf(plt_fp, "gsave 10 %f translate\n", 700-trcheight);
}

void ShowPage(char *title)
{
    time_t tp;
    struct tm *cur_time;
    char date[41];

    /* get date */
    time(&tp);
    cur_time= localtime(&tp);
    strftime(date, 40, "%a %b %d %H:%M:%S %Z %Y", cur_time);
    
    /* print title ??*/
    fprintf(plt_fp, "grestore 10 700 translate sethdr\n");
    fprintf(plt_fp, "(%s) show\n", title);
    fprintf(plt_fp, "350 50 moveto\n");
    fprintf(plt_fp, "(%s) show\n", date);
    fprintf(plt_fp, "showpage\n");
    fprintf(plt_fp, "/Times-Roman findfont 8 scalefont setfont\n");
    fprintf(plt_fp, "gsave 10 %f translate\n", 700-trcheight);
}

void EndPlot(char *title)
{
    time_t tp;
    struct tm *cur_time;
    char date[41];

    /* get date */
    time(&tp);
    cur_time= localtime(&tp);
    strftime(date, 40, "%a %b %d %H:%M:%S %Z %Y", cur_time);
    
    /* print title ??*/
    fprintf(plt_fp, "grestore 10 700 translate sethdr\n");
    fprintf(plt_fp, "(%s) show\n", title);
    fprintf(plt_fp, "350 50 moveto\n");
    fprintf(plt_fp, "(%s) show\n", date);
    fprintf(plt_fp, "showpage\n");
    if(plt_mode==0) { /* file */
	fclose(plt_fp);
    }else { /* pipe */
	pclose(plt_fp);
    }
}


void PS_PlotWave(int itrc)
{
    int k,i,j, yoff;
    int nsamp, decimate;
    int pidx;
    double *x, *y;
    float vs, hs;
    static float hres= (float)(580*300)/72;	/* highest resolution */
    float min,max;
    Wave *wave= traces[itrc]->wave;
    BIS3_HEADER *bh = &wave->info;
    Pick *pl;
    
    nsamp= wave->info.n_values;
    min= traces[itrc]->axis.ymin; 
    max= traces[itrc]->axis.ymax; 
    /* scaling info */
    fprintf(plt_fp, "gsave\n");
    fprintf(plt_fp, "newpath\n");

    vs= ((max-min)==0) ? 1 : trcheight/(max-min);
    /* the horizontal scaling assumes a 300dpi laser writer,
     * 580/72 inches wide. and will decimate anything more than that.
     */
    decimate= nsamp/hres;
    hs= (nsamp>hres)? (float)580/nsamp*decimate : (float)580/nsamp;
    fprintf(plt_fp, "%f 1 scale\n",hs);
#ifdef DEBUG
    printf("max= %f min=%f nsamp= %d vs= %f hs= %f\ndecimate=%d\n",
	   max,min,nsamp,vs,hs,decimate);
#endif

    fprintf(plt_fp, "tempctm currentmatrix\n"); /* save scaling */
    fprintf(plt_fp, "0 %f moveto\n", (wave->data[0]-min)*vs);
    if (decimate<=1) {	/* hopefully the common case-- faster: */
	for(j=1; j < nsamp; j++) {
	    fprintf(plt_fp, "%d %f L\n", j, (wave->data[j]-min)*vs);
	}
    }else {
	int n;
	for(j=1,n=1; j < nsamp; j+=decimate,n++) {
	    fprintf(plt_fp, "%d %f L\n", n, (wave->data[j]-min)*vs);
	}
    }
    fprintf(plt_fp, "stroke\n");

    /* picks */
    pl= traces[itrc]->picks;
    
    while(pl) {
	pidx = pick_index(pl, traces[itrc], 1);
	if(decimate<=1) {
	    fprintf(plt_fp, "%d 0 moveto %d %f L stroke\n",
		    pidx,pidx,trcheight);
	    fprintf(plt_fp, "%d 0 moveto %d 0 L stroke\n",
		    pidx-10,pidx+10);
	    fprintf(plt_fp, "%d %f moveto %d %f L stroke\n",
		    pidx-10,trcheight,pidx+10,trcheight);
	    fprintf(plt_fp, "%d %f moveto (%8.8s) show\n",
		    pidx+12,trcheight,pl->phaseName);
	}else {
	    float s= (float)pidx/decimate;
	    fprintf(plt_fp, "%f 0 moveto %f %f L stroke\n",
		    s, s, trcheight);
	    fprintf(plt_fp, "%f 0 moveto %f 0 L stroke\n",
		    s-10/decimate, s+10/decimate);
	    fprintf(plt_fp, "%f %f moveto %f %f L stroke\n",
		    s-10/decimate, trcheight,s+10/decimate,trcheight);
	    fprintf(plt_fp, "%f %f moveto (%8.8s) show\n",
		    s+12/decimate,trcheight,pl->phaseName); 
	}
	pl=pl->next;
    }
	
    /* new trace */
    fprintf(plt_fp, "grestore\n");
    /* labels... */
    /* stn name */
    fprintf(plt_fp, "10 %f moveto\n",trcheight*0.7);
    fprintf(plt_fp, "(%s %s %s %s) show\n", bh->station, bh->network, 
	    bh->channel, (strlen(bh->location) > 0) ? bh->location : "--");
    /* min & max */
    fprintf(plt_fp, "10 %f moveto\n",trcheight*0.8);
    fprintf(plt_fp, "(%g) show\n", max);
    fprintf(plt_fp, "10 %f moveto\n",trcheight*0.2);
    fprintf(plt_fp, "(%g) show\n", min);
    /* time length */
    fprintf(plt_fp, "%f %f moveto\n",540.0 ,trcheight*0.4);
    fprintf(plt_fp, "(%.4g sec) show\n", nsamp/wave->info.sample_rate);
    fprintf(plt_fp, "0 %f translate\n", -trcheight);
}
