/*	$Id: reg_select.h,v 1.2 2013/02/28 21:24:58 lombard Exp $	*/

/*                                                  */
/* reg_select.h                                     */
/*                                                  */
/* defines the structure for the "new" region sel   */
/*                                                  */
/*                                                  */

#ifndef REG_SELECT_H
#define REG_SELECT_H

typedef struct _reg_select {
  int right_index;               /* the beginning of the region */      
  int left_index;                /* the end of the region       */
  struct _reg_select *next;      /* next region */
} Reg_select;

#endif
