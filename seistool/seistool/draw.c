#ifndef lint
static char id[] = "$Id: draw.c,v 1.3 2013/02/28 21:25:00 lombard Exp $";
#endif

/*
 * draw.c--
 *    miscellaneous shapes drawing routines.
 *
 * Copyright (c) 1992-3 Andrew K. Yu, Lawrence Berkeley Laboratory
 *      and University of California, Berkeley.
 * All rights reserved.
 */

#include "proto.h"
#include "xv_proto.h"

void drawBracket(Window win, GC gc, int x, int y1, int y2, int leftRight)
{
    XPoint bracket[4];

    bracket[0].x= bracket[3].x= (leftRight? x+8: x-8);
    bracket[1].x= bracket[2].x= x;
    bracket[0].y= bracket[1].y= y1;
    bracket[2].y= bracket[3].y= y2;
    XDrawLines(theDisp, win, gc, bracket, 4, CoordModeOrigin);
}

void drawThickBracket(Window win, GC gc, int x, int y1, int y2, int leftRight)
{
    XPoint bracket[4];
    int bx;

    bracket[0].x= bracket[3].x= (leftRight? x+8: x-8);
    bracket[1].x= bracket[2].x= x;
    bracket[0].y= bracket[1].y= y1;
    bracket[2].y= bracket[3].y= y2;
    XDrawLines(theDisp, win, gc, bracket, 4, CoordModeOrigin);
    bx= (leftRight? x+1: x-1);
    XDrawLine(theDisp, win, gc, bx, y1, bx, y2);
}

/* draw a triangle
    /\  apex: (x, y)
   /--\
*/
void drawXUpTriangle(Window theWindow, GC theGC, int x, int y, int height,
		     int halfbase)
{
    XPoint vertex[3];
    vertex[0].x= x; vertex[0].y= y;
    vertex[1].x= x-halfbase; vertex[1].y= y+height;
    vertex[2].x= x+halfbase; vertex[2].y= y+height;
    XFillPolygon(theDisp, theWindow, theGC,
		 vertex, 3, Convex, CoordModeOrigin);
}

/* draw a triangle
   \--/
    \/  apex: (x, y)
*/
void drawXDnTriangle(Window theWindow, GC theGC, int x, int y, int height,
		     int halfbase)
{
    XPoint vertex[3];
    vertex[0].x= x; vertex[0].y= y;
    vertex[1].x= x-halfbase; vertex[1].y= y-height;
    vertex[2].x= x+halfbase; vertex[2].y= y-height;
    XFillPolygon(theDisp, theWindow, theGC,
		 vertex, 3, Convex, CoordModeOrigin);
}

/* draw a triangle
    |\   apex: (x, y)
    |/
*/
void drawYTriangle(Window theWindow, GC theGC, int x, int y)
{
    XPoint vertex[3];
    vertex[0].x= x; vertex[0].y= y;
    vertex[1].x= x-5; vertex[1].y= y-4;
    vertex[2].x= x-5; vertex[2].y= y+4;
    XFillPolygon(theDisp, theWindow, theGC,
		 vertex, 3, Convex, CoordModeOrigin);
}

void drawBrokenLine(Window win, GC gc, int x, int height)
{
    XPoint *line;
    int n, nn, i, y;

    n= ((float)height/6+0.5);
    nn= n * 2;
    line= (XPoint *)Malloc(sizeof(XPoint)*nn);
    if (line==NULL) return;	/* not enuff mem */
    y=0;
    for(i=0; i < nn; i+=2)  {
	line[i].x= line[i+1].x= x;
	line[i].y= y;
	line[i+1].y= y+2;
	y+=6;
    }
    XDrawSegments(theDisp, win, gc, (XSegment*)line, n);
    free(line);
}


#define		FULL_CIRCLE	(360*64)
#define		START_CIRCLE	0

void drawOval(Window theWindow, GC theGC, int x, int y, int width, int height)
{
    XDrawArc(theDisp, theWindow, theGC, 
	     x, y, width, height,
	     START_CIRCLE,
	     FULL_CIRCLE);
}

void fillOval(Window theWindow, GC theGC, int x, int y, int width, int height)
{
    XFillArc(theDisp, theWindow, theGC, 
	     x, y, width, height,
	     START_CIRCLE,
	     FULL_CIRCLE);
}


