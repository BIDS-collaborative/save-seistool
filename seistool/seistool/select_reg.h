/*	$Id: select_reg.h,v 1.2 2013/02/28 21:24:57 lombard Exp $	*/

#ifndef SELECT_REG_H
#define SELECT_REG_H

#include <xview/xview.h>
#include <xview/canvas.h>
#include <xview/frame.h>
#include <xview/window.h>
#include "wave.h"

typedef struct {
    Canvas can;      /* who owns this */
    Frame  board;    /* the frame */
    Window wind;     /* the window */
    int num_wind;    /* number of windows */
    int num_col;     /* number of windows per row */
    int num_row;     /* number of windows per column */
    int num_sel;     /* number of selection */
    int hor_size;    /* the horizontal sub window size */
    int ver_size;    /* the vertical subwindow size */
    Wave *regs;      /* an array of selection regions */
    int *sel_reg;    /* array for actual selection */ 
    int sel_type;    /* 1 - all, 0- only trip 2- fixed num of reg*/
    int trace_hbox;  /* the horizontal size of the trace box */ 
    int trace_vbox;  /* the vertical size of the trace box */ 
    GC  txtgc;        /* text graphics condition */
} Select_Info;

#endif
