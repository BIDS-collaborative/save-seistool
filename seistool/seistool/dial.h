/*	$Id: dial.h,v 1.2 2013/02/28 21:25:00 lombard Exp $	*/

/*
 * dial.h--
 *    Dial structure contains information of
 *    each dial (implemented by dial.c since XView doesn't
 *    have dials.)
 *
 * Copyright (c) 1992-3 Andrew K. Yu, Lawrence Berkeley Laboratory
 *      and University of California, Berkeley.
 * All rights reserved.
 */
#ifndef DIAL_H
#define DIAL_H

#include <xview/xview.h>
#include <xview/frame.h>
#include <xview/canvas.h>

typedef struct {
    int x, y;
    int width, height;
    int type;

    Canvas cvs;
    Window win;
    GC	gc, rvgc, xorgc;

    char *maxLabel, *minLabel, *midLabel;

    float value;
    float scale;

    float marked_value;
    int show_marked;

    void (*valChange)(float);
    int dial_id;

    int inactive;
} Dial;

#define DIAL_KNOB_TYPE	0
#define DIAL_ROT_TYPE	1

#endif
