#ifndef lint
static char id[] = "$Id: regexpr.c,v 1.2 2013/02/28 21:24:58 lombard Exp $";
#endif

/*
 * regexpr.c--
 *    regular expressions used in "select trace"
 *
 * Copyright (c) 1992-3 Andrew K. Yu, Lawrence Berkeley Laboratory
 *      and University of California, Berkeley.
 * All rights reserved.
 */
#include <ctype.h>
#include <stdio.h>

#include "proto.h"
#include "regexpr.h"

#define INIT	    register char *sp= instring;
#define GETC()	    (*sp++)
#define PEEKC()	    (*sp)
#define UNGETC(c)   (--sp)
#define RETURN(c)   return;
#define ERROR(c)    regerr()
#include <regexp.h>


static int regerr()
{
    fprintf(stderr,"Regexpr: can't parse expression.\n");
}

static void edit_reg(char *orig, char *result)
{
    char c, prev;
    char *in=orig, *out=result;
    /* tag ^ at beginning */
    while(isspace(*orig))orig++;
    if(*orig!='^') *result++= '^';
    prev='\0';
    while((c=*orig++)!='\0') {
	if(c=='*' && prev!='\\' && prev!='.') {
	    *result++='.';
	    *result++='*';
	}else {
	    *result++=c;
	}
	prev=c;
    }
    if(prev!='$') {
	*result++= '$';
    }
    *result= '\0';
}

void RE_compile(char *expr, char *exprbuf)
{    
    char buf[ESIZE];
    edit_reg(expr,buf);
    compile(buf, exprbuf, &exprbuf[ESIZE], '\0');
}

int RE_match(char *string, char *exprbuf)
{    
    return (step(string,exprbuf));
}
    
