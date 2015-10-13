/* /*
 * regexpr.h--
 *    regular expressions used in "select trace"
 *
 * Copyright (c) 1992-3 Andrew K. Yu, Lawrence Berkeley Laboratory
 *      and University of California, Berkeley.
 * All rights reserved.
 */

#ifndef REGEXPR_H
#define REGEXPR_H

#define ESIZE	    500
typedef struct sele_re_ {
    struct sele_re_ *next;
    
    char *expr;
    char exprbuf[ESIZE];
} SeleRE;

#endif
