#ifndef lint
static char id[] = "$Id: util.c,v 1.2 2013/02/28 21:24:56 lombard Exp $";
#endif

/*
 * util.c--
 *    more graceful memory allocation
 *
 * Copyright (c) 1992-3 Andrew K. Yu, Lawrence Berkeley Laboratory
 *      and University of California, Berkeley.
 * All rights reserved.
 */
#include <stdio.h>
#include <stdlib.h>
#include "proto.h"

/* a generic Malloc to handle memory allocation.
 * it is possible that we do something (like garbage collection)
 * to squeeze more memory. This is left as an exercise to the
 * programer.
 */
void *Malloc(int size)
{
    void *chunk;
    chunk = malloc(size);
    if (chunk == NULL)
	fprintf(stderr, "Red Alert: Insufficient Memory!\n");
    return chunk;
}

/* A generic Realloc to handle memory allocation.
 * It is possible that we do something (like garbage collection)
 * to squeeze more memory. This is left as an exercise to the
 * programer.
 */
void *Realloc(void *p, int size)
{
    void *chunk;
    chunk = realloc(p, size);
    if (chunk==NULL)
	fprintf(stderr, "Red Alert: Insufficient Memory!\n");
    return chunk;
}
