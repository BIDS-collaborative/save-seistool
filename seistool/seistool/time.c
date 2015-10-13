#ifndef lint
static char id[] = "$Id: time.c,v 1.3 2013/02/28 21:24:57 lombard Exp $";
#endif

/*
 * time.c--
 *    routines for handling seistool's STE_TIME and STI_TIME
 *      structures. These are identical to qlib2's EXT_TIME
 *      and INT_TIME. But qlib2.h defines a FRAME structure for SEED
 *      which conflicts with a FRAME structure in xview.
 *      Then we provide some time functions that were used
 *      in earlier versions of seistool; now they are wrappers
 *      for similar functions in qlib2.  Pete Lombard, October, 2001
 *
 */
#include <string.h>
#include <qlib2.h>
#include "time.h"

/**********************************************************************
 *   Conversion                                                       *
 **********************************************************************/

STE_TIME sti_to_ste( STI_TIME sti )
{
    EXT_TIME et;
    
    et = int_to_ext(*(INT_TIME*)&sti);
    
    return (*(STE_TIME*)&et);
}

STI_TIME ste_to_sti( STE_TIME ste )
{
    INT_TIME it;
    
    it = ext_to_int(*(EXT_TIME*)&ste);
    
    return (*(STI_TIME*)&it);
}

STI_TIME date_to_sti( date )
     DATE date;
{
    EXT_TIME et;
    INT_TIME it;
    
    et.year = date.year;
    et.doy = date.day;
    et.hour = date.hour;
    et.minute = date.minute;
    et.second = date.second;
    et.usec = date.ticks * USECS_PER_TICK;

    it = ext_to_int(et);
    return (*(STI_TIME*)&it);
}

DATE sti_to_date(STI_TIME it)
{
    DATE dt;
    STE_TIME et = sti_to_ste(it);
    
    dt.year = et.year;
    dt.day = et.doy;
    dt.hour = et.hour;
    dt.minute = et.minute;
    dt.second = et.second;
    dt.ticks = et.usec / USECS_PER_TICK;
    return dt;
}


/**********************************************************************
 *   Addition & Subtraction                                           *
 **********************************************************************/
STI_TIME st_add_dtime( sti, sec )
    STI_TIME sti; double sec;
{
    INT_TIME it;

    it = add_dtime(*(INT_TIME*)&sti, sec);
    
    return (*(STI_TIME*)&it);
}


/* Subtract times, st_tdiff = tm1 - tm2 in SECONDS (not usecs!) */
double st_tdiff( tm1, tm2 )
    STI_TIME tm1, tm2;
{
    return (tdiff(*(INT_TIME*)&tm1, *(INT_TIME*)&tm2)/USECS_PER_SEC);
}


/**********************************************************************
 *   printing out the aggregate TIME                                  *
 **********************************************************************/
void printTime( sti )
    STI_TIME sti;
{
    EXT_TIME et = int_to_ext(*(INT_TIME*)&sti);
    float sec = (float)et.second + (float)et.usec / USECS_PER_SEC;
    
    printf("%d %3d %02d:%02d:%09.6f",et.year,et.doy, et.hour,
	   et.minute, sec);
    return;
}

/*
 * it is the caller's responsibility to ensure that string will hold
 * at least 25 chars including null terminator
 */
void sprintTime(string, sti)
     char *string; STI_TIME sti;
{    

    EXT_TIME et = int_to_ext(*(INT_TIME*)&sti);
    double sec = (double)et.second + (double)et.usec / USECS_PER_SEC;
    
    sprintf(string, "%4d %03d %02d:%02d:%09.6f",et.year,et.doy, et.hour,
	   et.minute, sec);
    return;
}

/**********************************************************************
 *   Comparision                                                      *
 **********************************************************************/

/*
 * int>0  if tm1 > tm2 (more recent)
 * int=0  if tm1== tm2
 * int<0  if tm1 < tm2
 *
 */
int TimeCmp( tm1, tm2 )
     STI_TIME tm1, tm2;
{
    if (tm1.year!=tm2.year) {
	return (tm1.year-tm2.year);
    }
    if (tm1.second!=tm2.second) {
	return (tm1.second-tm2.second);
    }
    if (tm1.usec!=tm2.usec) {
	return (tm1.usec-tm2.usec);
    }
    return 0;	/* equal */
}

