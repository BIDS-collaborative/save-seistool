/*	$Id: clientdata.h,v 1.2 2013/02/28 21:25:00 lombard Exp $	*/

/*
 * clientdata.h--
 *    ClientData is used to store (key,datum) pairs in a Trace.
 *    It is good for storing just about anything specific to a particular
 *    trace.
 *
 * Copyright (c) 1992-3 Andrew K. Yu, Lawrence Berkeley Laboratory
 *      and University of California, Berkeley.
 * All rights reserved.
 */
#ifndef CLIENTDATA_H
#define CLIENTDATA_H

typedef struct ClientData_ {
    int	key;
    int datum;

    struct ClientData_ *next;
} ClientData;


#endif  CLIENTDATA_H
