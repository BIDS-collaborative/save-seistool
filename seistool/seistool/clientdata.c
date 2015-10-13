#ifndef lint
static char id[] = "$Id: clientdata.c,v 1.2 2013/02/28 21:25:00 lombard Exp $";
#endif

/*
 * clientdata.c--
 *    implementation of the abstract data type ClientData
 *
 * Copyright (c) 1992-3 Andrew K. Yu, Lawrence Berkeley Laboratory
 *      and University of California, Berkeley.
 * All rights reserved.
 */

#include <stdlib.h>
#include "clientdata.h"
#include "proto.h"

void cdata_insert(Trace *trc, int key, int datum)
{
    ClientData *cdata= (ClientData *)Malloc(sizeof(ClientData));
    cdata->key= key;
    cdata->datum= datum;
    cdata->next= trc->client_data;
    trc->client_data= cdata;
}

int cdata_find(Trace *trc, int key)
{
    ClientData *cdata= trc->client_data;
    while(cdata!=NULL) {
	if(cdata->key==key)
	    return cdata->datum;
	cdata= cdata->next;    
    }
    return -1;
}

void cdata_remove(Trace *trc, int key)
{
    ClientData *cdata= trc->client_data;
    ClientData *prev;

    while(cdata!=NULL) {
	if(cdata->key==key)
	    break;
	prev= cdata;
	cdata= cdata->next;
    }
    if(cdata) {
	if(cdata==trc->client_data) {
	    trc->client_data= cdata->next;
	}else {
	    prev->next= cdata->next;
	}
	free(cdata);
    }
    
    return;
}
