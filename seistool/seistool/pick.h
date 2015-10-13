/*	$Id: pick.h,v 1.3 2013/02/28 21:24:58 lombard Exp $	*/

/*
 * pick.h--
 *    contains everything you need for storing and manipulating
 *    picks.
 *
 * Copyright (c) 1992-3 Andrew K. Yu, Lawrence Berkeley Laboratory
 *      and University of California, Berkeley.
 * All rights reserved.
 */

#ifndef PICK_H
#define PICK_H

typedef enum {
    UNKNOWN_Q, IMPULSIVE, EMERGENT, 
} Quality_t;

typedef enum {
    PICK, LMARK, RMARK,
} Mark_t;

typedef enum { 
    UNKNOWN_M, NONE, UP, DOWN,
} Motion_t;

#define PHSIZE 8

typedef struct _pick {
    Mark_t  type;	    /* what type of mark is this */
    double secOffset;       /* pick time offset from trace start time */
    Motion_t firstMotion;   /* direction of first motion */
    Quality_t quality;	    /* quality of the pick */
    float uncertainty;	    /* accuracy of the pick */
    struct _pick *next;     /* next pick Node */
    char phaseName[PHSIZE+1]; /* phase name */
    char label[PHSIZE+2];   /* shown alongside the pick line */
} Pick;


#endif
