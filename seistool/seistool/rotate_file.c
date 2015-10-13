#ifndef lint
static char id[] = "$Id: rotate_file.c,v 1.3 2013/02/28 21:24:58 lombard Exp $";
#endif

/* This code deals with the reading and writing of the */
/* station rotations to and from files                 */

#include <stdio.h>
#include <strings.h>
#include "proto.h"
#include "xv_proto.h"

extern Trace **traces;
extern int LastTrack;


static char *getfilename(char *path)
{
    char *p;

    if ((p=rindex(path, '/'))!=NULL)
	p++;
    else
	p = path;
    return p;   
}

void cvtToRFname(char *ffname, char *fname)
{
    strcpy(ffname, fname);
    /* just add ``.trf'' */
    strcat(ffname,".trf");
  }


void WriteRotation(char *fname)
{
    int i;
    int rotated = 0;
    FILE *fp;
    STI_TIME earl;
    Trace *trc;
    Triplet *t;
    BIS3_HEADER *bh;
    char stream[4];
    char sta_pre[20];
    char str_pre[4];
    char tmestr[50];
    
    /* Is there anything to write? */
    for (i = 0; i <= LastTrack; i++) {
	if (traces[i]->trip && traces[i]->trip->rotated > 0) {
	    rotated = 1;
	    break;
	}
    }
    if (! rotated ) return;

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

    earl = getEarliestSample(0);
    sprintTime(tmestr, earl);

    fprintf(fp,"Rotationfile V.3.00\n");
    fprintf(fp,"T %s\n",tmestr);
    fprintf(fp, "H STN NT CH LC AZI         UNCERT\n");

    sta_pre[0] = '\0';
    for(i=0; i <= LastTrack; i++) {
	trc=traces[i];
	t=trc->trip;
	bh = &trc->wave->info;
	strncpy(stream,(bh->channel[0]!='\0') ? bh->channel : "???", 2);
	stream[2]='\0';
	if ((strcmp(sta_pre, bh->station) || strncmp (str_pre,stream,2)) && 
	    t!=NULL && t->rotated==1) {
	  fprintf(fp,"%4s %2s %3s %2s %010.6f %f\n",bh->station,bh->network,
		  stream, (bh->location[0]== '\0' ? "--" : bh->location),
		  t->rot_theta, t->rot_unc);
	  strcpy(sta_pre, bh->station);
	  strcpy(str_pre, stream);
	}
      }

    fclose(fp);
    return;
}

