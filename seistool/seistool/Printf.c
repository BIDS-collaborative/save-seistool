#ifndef lint
static char id[] = "$Id: Printf.c,v 1.2 2013/02/28 21:25:01 lombard Exp $";
#endif

/*
 * Printf.c--
 *    handles printing to info window or controlling terminal
 *
 * Copyright (c) 1992-3 Andrew K. Yu, Lawrence Berkeley Laboratory
 *      and University of California, Berkeley.
 * All rights reserved. 
 */
#include <stdio.h>
#include "proto.h"

extern int Mode_CLI;

void Printf(char *s, ... )
{
    va_list ap;
    va_start(ap, s);

    if(Mode_CLI) {
	vprintf(s,ap);
    }else {
	char buf[1000];
	vsprintf(buf, s, ap);
	textsw_printf(buf);
    }
    va_end(ap);
}

