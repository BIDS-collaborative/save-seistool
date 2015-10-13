/*	$Id: time.h,v 1.2 2013/02/28 21:24:57 lombard Exp $	*/

/*
 *  time.h--
 *    header for time manipulation module
 *       Allows use of qlib2 time routines with different names
 *       The problem is that qlib2.h defines a FRAME structure
 *       which conflicts with a FRAME structure in xview.
 *
 *  Copyright (c) 1992-3 Andrew K. Yu, Lawrence Berkeley Laboratory
 *       and University of California, Berkeley.
 *  All rights reserved.
 */

#ifndef TIME_H
#define TIME_H

#ifndef USECS_PER_SEC
#define USECS_PER_SEC   1000000
#define TICKS_PER_SEC   10000
#define USECS_PER_MSEC  (USECS_PER_SEC/1000)
#define USECS_PER_TICK  (USECS_PER_SEC/TICKS_PER_SEC)
#endif

typedef struct {
    int yr;
    int doy;
    int hr;
    int min;
    int sec;
    float fract;
} TIME;   /* Old definition, to be deleted: PNL */

/*
 * These are identical to qlib2's EXT_TIME and INT_TIME
 */
typedef struct _ste_time {
    int         year;           /* Year.                        */
    int         doy;            /* Day of year (1-366)          */
    int         month;          /* Month (1-12)                 */
    int         day;            /* Day of month (1-31)          */
    int         hour;           /* Hour (0-23)                  */
    int         minute;         /* Minute (0-59)                */
    int         second;         /* Second (0-60 (leap))         */
    int         usec;           /* Microseconds (0-999999)      */
} STE_TIME;

typedef struct  _sti_time {
    int         year;           /* Year.                        */
    int         second;         /* Seconds in year (0-...)      */
    int         usec;           /* Microseconds (0-999999)      */
} STI_TIME;

typedef struct _date {
    int   year;            /* year	(possibly in range (0 - 99) */
    int   day;	           /* day of year (1 - 366)                 */
    int   hour;            /* hour        (0 - 23)                  */
    int   minute;          /* minute      (0 - 59)                  */
    int   second;          /* second      (0 - 60 (leapsecond))     */
    int   ticks;           /* 1/10 millisecond (0 - 9999)           */
} DATE;



/*  export prototypes  */
extern STI_TIME ste_to_sti( STE_TIME );
extern STE_TIME sti_to_ste( STI_TIME );
extern STI_TIME date_to_sti( DATE );
extern DATE sti_to_date( STI_TIME );

extern STI_TIME st_add_dtime( STI_TIME, double );
extern double st_tdiff( STI_TIME, STI_TIME );   /* st_tdiff=tm1-tm2 */
extern int TimeCmp( STI_TIME, STI_TIME );
    /* >0 if tm1 more recent than tm2, =0 if equals, <0 otherwise */

extern void printTime( STI_TIME );
extern void sprintTime( char *str, STI_TIME );


#endif
