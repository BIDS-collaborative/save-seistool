/*	$Id: hook.h,v 1.1 2001/12/21 18:39:03 lombard Exp $	*/

/*
 * hook.h--
 *    Hook contains certain procedures to be called when necessary.
 *
 * Copyright (c) 1992-3 Andrew K. Yu, Lawrence Berkeley Laboratory
 *      and University of California, Berkeley.
 * All rights reserved.
 */
#ifndef HOOK_H
#define HOOK_H

typedef struct {
    void (*plot_ztrack)();	/* when UpdatePlotZTrack called */
    void (*plot_track)();	/* when UpdatePlotTrack called */
    void (*cleanup_trace)();	/* when CleanTrace called */
} Hook;

#endif  HOOK_H
