#ifndef lint
static char id[] = "$Id: reg_select.c,v 1.2 2013/02/28 21:24:58 lombard Exp $";
#endif

/*                                                  */
/* reg_select.c                                     */
/*                                                  */
/* implementaion of the "new" region selection code */
/*                                                  */
/*                                                  */

#include <stdlib.h>
#include <stdio.h>
#include "proto.h"


Reg_select *Create_SRegion ()
{
    Reg_select *new=(Reg_select *)Malloc(sizeof(Reg_select));

    if (new==NULL) return NULL; /* no memory left */
    new->next= NULL;
    new->right_index=-1;
    new->left_index=-1;

    return new;
}

void RemoveSRegion(Reg_select *dest,Trace *this_trace)
{
    Reg_select *curr,*prev;
    int right=dest->right_index,left=dest->left_index;

    curr=this_trace->sel_reg;

    if (curr==dest) {
	this_trace->sel_reg=dest->next;
    } else {

	/* find dest in the list of selected regions */
	while (!(curr->right_index==right && curr->left_index ==left && curr==dest)) {
	    prev=curr;
	    curr=prev->next;
	}

	/* rehang the old pointer */
	prev->next=curr->next;
    }

    free(dest);
}


Reg_select *Find_Nearest_SRegion(Trace *this_trace, int x)
{
    Reg_select *curr=NULL,*near=NULL;
    int near_dist=10000000;
    int r_loc,l_loc,dist=-1;
    int found=0;

    curr=this_trace->sel_reg;

    while(curr!=NULL && found==0) {
	r_loc=indexToCoord(&this_trace->axis,curr->right_index,1);
	l_loc=indexToCoord(&this_trace->axis,curr->left_index,1);


	if (l_loc<x) {
	    if (r_loc >x) {
		/* event in region */
		found=1;
		near=curr;
	    } else {
		/* left must be closest */
		dist=x-r_loc;
	    }
	} else {
	    /* right must be closest */
	    dist=l_loc-x;
	}

	if (dist!=-1 && dist<near_dist) {
	    near_dist=dist;
	    near=curr;
	}

	curr=curr->next;
    }

    return near;
}


void Insert_SRegion (Reg_select *reg,Trace *this_trace)
{
    Reg_select *curr,*prev=NULL;
    int right;

    curr=this_trace->sel_reg;
  
    if (curr==NULL || reg->right_index< curr->right_index) {
	reg->next=this_trace->sel_reg;
	this_trace->sel_reg=reg;
    } else {
	while (curr !=NULL && reg->right_index> curr->right_index) {
	    prev=curr;
	    curr=prev->next;
	}

	reg->next=curr;
	if (prev == NULL) {
	    this_trace->sel_reg=reg;
	} else {
	    prev->next=reg;
	}
    }
}

Trace *SReg_in_trace(Reg_select *find_reg,Trace *trc1, Trace *trc2)
{
    Reg_select *curr_reg=trc1->sel_reg;
    int found=0;

    while(curr_reg!=NULL) {
	if (curr_reg==find_reg) return trc1;
	curr_reg=curr_reg->next;
    }

    curr_reg=trc2->sel_reg;
    while(curr_reg!=NULL) {
	if (curr_reg==find_reg) return trc2;
	curr_reg=curr_reg->next;
    }

    return NULL;
}
