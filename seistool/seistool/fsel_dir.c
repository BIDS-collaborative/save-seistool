#ifndef lint
static char id[] = "$Id: fsel_dir.c,v 1.2 2013/02/28 21:24:59 lombard Exp $";
#endif

/*
 * fsel_dir.c--
 *    file selection box auxillary routines (see fsel_xv.c for X stuff)
 *
 * Copyright (c) 1992-3 Andrew K. Yu, Lawrence Berkeley Laboratory
 *      and University of California, Berkeley.
 * All rights reserved.
 */
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/param.h>
#include <unistd.h>

#include "proto.h"

/* handles listing the directories */

#define MAX_ENTRIES 100

char *environ_home= NULL;

static int f_maxEnt, d_maxEnt;
char **f_names=NULL;
char **d_names=NULL;
int f_i=0, d_i=0;

extern char curpath[MAXPATHLEN];

static char *wholeName(char *prefix, char *dname);
static DIR *fsel_opendir(char *prefix, char *dname);
static void ListDirectory(DIR *dfd);


void fsel_cwd(char *cwd_path)
{
    int len;
    getwd(cwd_path);
    len=strlen(cwd_path);
    cwd_path[len]='/';
    cwd_path[len+1]='\0';
}

static char *wholeName(char *prefix, char *dname)
{
    int len; 
    char *name;
    if (prefix==NULL || *prefix=='\0') {
	name= (char *)Malloc(strlen(dname)+1);
	if (name==NULL) return NULL;	/* not enuff mem */
	strcpy(name,dname);
    }else {
	len = strlen(prefix)+strlen(dname)+1+1;
	name= (char *)Malloc(len);
	if (name==NULL) return NULL;	/* not enuff mem */
	strcpy(name, prefix);
	name[strlen(prefix)]='/';
	strcpy(name+strlen(prefix)+1, dname);
	name[len]='\0';
    }
    return name;
}

void fsel_chdir(char *prefix, char *path)
{
    DIR *dfd;
    char *s;
    int i;

    dfd=fsel_opendir(prefix, path);
    if (strcmp(path,".")!=0) {
	s= wholeName(curpath, path);
	strcpy(curpath, s);
	free(s);
    }
    f_i= d_i= 0;
    if (dfd!=NULL) {
	if (f_names!=NULL) {
	    for(i=0; i < f_maxEnt; i++) {
		if (f_names[i]!=NULL) free(f_names[i]);
	    }
	    free(f_names);
	}
	if (d_names!=NULL) {
	    for(i=0; i < d_maxEnt; i++) {
		if (d_names[i]!=NULL) free(d_names[i]);
	    }
	    free(d_names);
	}
	f_maxEnt=d_maxEnt= MAX_ENTRIES;
	f_names=(char **)Malloc(sizeof(char *)*f_maxEnt);
	if (f_names==NULL) return;
	bzero(f_names, sizeof(char *)*f_maxEnt);
	d_names=(char **)Malloc(sizeof(char *)*d_maxEnt);
	if (d_names==NULL) return;
	bzero(d_names, sizeof(char *)*d_maxEnt);
	ListDirectory(dfd);
    }
}


void substTilde(char *path)
{
    char sub[200];

    if (environ_home==NULL||*environ_home=='\0')
	return;	/* can't substitute */
    if (*path=='~') {
	/* needs substitution */
	if (path[1]=='/' || path[1]=='\0') {
	    strcpy(sub,path);
	    strcpy(path,environ_home);
	    strcat(path, sub+1);
	}else { /* other user */
	    char *p;
	    strcpy(sub, environ_home);
	    if (p=rindex(sub,'/'))
		*(p+1)='\0';
	    strcat(sub, path+1);
	    strcpy(path,sub);
	}
    }
    return;
}

static DIR *fsel_opendir(char *prefix, char *dname)
{
    DIR *dfd;
    struct stat s_buf;

    if (prefix!=NULL && *prefix!='\0') {
	char *name=wholeName(prefix, dname);
	if (stat(name,&s_buf)==-1||!(s_buf.st_mode&S_IREAD)) {
	    return NULL;	/* if can't read, forget it */
	}
	dfd= opendir(name);
	free(name);
    }else {
	if (stat(dname,&s_buf)==-1||!(s_buf.st_mode&S_IREAD)) {
	    return NULL;	/* if can't read, forget it */
	}
	dfd= opendir(dname);
    }
    return dfd;
}

int StrCmp(const void *si, const void *sj)
{
    const char **i = (const char **)si;
    const char **j = (const char **)sj;
    
    return (strcmp(*i, *j));
}

static void ListDirectory(DIR *dfd)
{
    struct dirent *dp;
    struct stat s_buf;
    int numf, numd;
    char *name;
    
    numf=numd=0;
    while ((dp= readdir(dfd))!=NULL) {
	name=wholeName(curpath,dp->d_name);
	if (stat(name,&s_buf)!=-1) {
	    if (S_ISDIR(s_buf.st_mode)) {
		/* a directory */
		d_names[numd]= (char *)Malloc(strlen(dp->d_name)+1);
		if (d_names[numd]!=NULL) {
		    strcpy(d_names[numd], dp->d_name);
		    numd++;
		    if (numd==d_maxEnt) {
			/* bad, need more memory */
			d_maxEnt*=2;
			d_names=(char **)Realloc(d_names,sizeof(char*)*d_maxEnt);
			if(d_names==NULL) return;
		    }
		}
	    }else if(S_ISREG(s_buf.st_mode)) {
		/* a regular file */
		f_names[numf]=(char *)Malloc(strlen(dp->d_name)+1);
		if (f_names[numf]!=NULL) {
		    strcpy(f_names[numf],dp->d_name);
		    numf++;
		    if (numf==f_maxEnt) {
			/* bad, need more memory */
			f_maxEnt*=2;
			f_names=(char **)Realloc(f_names,sizeof(char*)*f_maxEnt);
			if (f_names==NULL) return;
		    }
		}
	    }
	    /* ignore everything else */
	}
	free(name);
    }
    closedir(dfd);
    f_i= numf;
    d_i= numd;
    /* sort in alphabetical order */
    if (f_i > 1) {
	qsort(f_names, f_i, sizeof(char *), StrCmp);
    }
    /* don't sort ``.''  and ``..'' */
    if (d_i>3) { 
	qsort(d_names+2, d_i-2, sizeof(char *), StrCmp);
    }
    return;
}

