/*      $Id: amplitude.h,v 1.1 2003/02/21 21:15:08 lombard Exp $

/*
 * amplitude.h--
 *    contains everything you need for storing and manipulating
 *    amplitudes.
 *
 */

#ifndef AMPLITUDE_H
#define AMPLITUDE_H

#define ATSIZE 8

/* enum values based on input to math/coinst.f */
typedef enum {
    UNKNOWN_A = -1, RAW_A, BENIOFF_A, BUTTER_A, WOOD_A, 
    PGD_A =  20, PGV_A, PGA_A,
} Amplitude_t;

typedef struct _amp 
{
    Amplitude_t type;
    float gdMax;
    float perMax;
    float gdUncert;
    int idx;
    struct _amp *next;
} Amp;



#endif
