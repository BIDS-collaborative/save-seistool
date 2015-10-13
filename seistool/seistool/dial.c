#ifndef lint
static char id[] = "$Id: dial.c,v 1.4 2013/02/28 21:25:00 lombard Exp $";
#endif

/*
 * dial.c--
 *    implements Dials
 *
 * Copyright (c) 1992-3 Andrew K. Yu, Lawrence Berkeley Laboratory
 *      and University of California, Berkeley.
 * All rights reserved.
 */
#include <xview/xview.h>
#include <xview/canvas.h>
#include <xview/frame.h>
#include <xview/panel.h>
#include <xview/xv_xrect.h>
#include <math.h>
#include <strings.h>

#include "proto.h"
#include "xv_proto.h"

static void repaint_cvs(Canvas canvas, Xv_Window paint_window, Display *dpy,
			Window xwin, Xv_xrectlist *area);
static void cvs_event_proc(Xv_Window window, Event *event);
static void RedrawKnob(Dial *dial);
static void RedrawDial(Dial *dial);
static void Redraw_event_az (Dial *dial);


Dial *newDial(int x, int y, int w, int h, int type)
{
    Dial *new;

    new= (Dial *)Malloc(sizeof(Dial));
    bzero(new,sizeof(Dial));
    new->x= x;
    new->y= y;
    new->width= w;
    new->height= h;
    new->scale=1;
    new->type= type;
    new->show_marked=0;
    new->marked_value=0;

    return new;
}

void InitDial(Frame frm, Dial *dial)
{
    int theScrn;
    
    dial->cvs= (Canvas)xv_create(frm, CANVAS,
				 XV_X, dial->x, XV_Y, dial->y,
				 XV_WIDTH, dial->width, XV_HEIGHT, dial->height,
				 CANVAS_REPAINT_PROC, repaint_cvs,
				 OPENWIN_SHOW_BORDERS, FALSE,
				 NULL);
    xv_set(canvas_paint_window(dial->cvs),
	   XV_KEY_DATA, 33, (int)dial,
	   WIN_EVENT_PROC, cvs_event_proc,
	   WIN_CONSUME_EVENTS, WIN_MOUSE_BUTTONS, LOC_DRAG,
	   NULL,
	   NULL);
	   
    dial->win= (Window)xv_get(canvas_paint_window(dial->cvs),XV_XID);
    createGC(dial->win, &dial->gc);
    createGC(dial->win, &dial->rvgc);
    XSetForeground(theDisp, dial->rvgc,
		   WhitePixel(theDisp,DefaultScreen(theDisp)));
    XSetBackground(theDisp, dial->rvgc,
		   BlackPixel(theDisp,DefaultScreen(theDisp)));
    createGC(dial->win, &dial->xorgc);
    XSetFunction(theDisp, dial->xorgc, GXxor);
    theScrn = DefaultScreen(theDisp);
    XSetForeground(theDisp, dial->xorgc,
		   WhitePixel(theDisp, theScrn)^BlackPixel(theDisp, theScrn));
}

void SetDialLabel(Dial *dial, char *minL, char *midL, char *maxL)
{
    dial->maxLabel= maxL;
    dial->midLabel= midL;
    dial->minLabel= minL;
}

void SetDialVal(Dial *dial, float value)
{
    RedrawKnob(dial);
    dial->value= value/dial->scale;
    RedrawDial(dial);
    RedrawKnob(dial); 
    Redraw_event_az(dial);
}

void SetDialChangeFunc(Dial *dial, int id, void (*func)(float))
{
    dial->dial_id= id;
    dial->valChange= func;
}

static void repaint_cvs(Canvas canvas, Xv_Window paint_window, Display *dpy,
			Window xwin, Xv_xrectlist *area)
{
    Dial *dial=(Dial *)xv_get(paint_window, XV_KEY_DATA, 33);
    refresh_dial( dial );
}

void refresh_dial(Dial *dial)
{
    RedrawDial(dial);
    RedrawKnob(dial);
    Redraw_event_az(dial);
}

#if 0	/* good for 200x200 */
#define C_OFF  30
#define C2_OFF 35
#define A_OFF  25
#endif

#define C_OFF  20
#define C2_OFF 25
#define A_OFF  15

static void RedrawKnob(Dial *dial)
{
    int r= dial->width/2 - C2_OFF-5;
    float theta;
#define DEG2RAD(d)  ((d)*3.14159265/180)
    int x, y;
    char buf[20];

    if(dial->type!=DIAL_ROT_TYPE) {
	theta= 60.0-dial->value;
	x= C2_OFF+r*(1-cos(DEG2RAD(theta)));
	y= C2_OFF+r*(1+sin(DEG2RAD(theta)));
	fillOval( dial->win, dial->xorgc, x, y,10, 10);
	sprintf(buf,"%-3.2f",dial->value*dial->scale);
	XDrawString(theDisp, dial->win, dial->xorgc,
		    r+A_OFF, r+C2_OFF+5, buf, strlen(buf));
    }else {
	theta= 90.0-dial->value;
	x= C2_OFF+5+(1+cos(DEG2RAD(theta)))*r;
	y= C2_OFF+5+(1-sin(DEG2RAD(theta)))*r;
	XDrawLine(theDisp, dial->win, dial->xorgc,
		  dial->width/2, dial->height/2, x, y);
	XDrawString(theDisp, dial->win, dial->xorgc,
		    x, y, "N", 1);
	x= C2_OFF+5+r*(1+cos(DEG2RAD(theta-90)));
	y= C2_OFF+5+r*(1-sin(DEG2RAD(theta-90)));
	XDrawLine(theDisp, dial->win, dial->xorgc,
		  dial->width/2, dial->height/2, x, y);
	XDrawString(theDisp, dial->win, dial->xorgc,
		    x, y, "E", 1);
    }
    XFlush(theDisp);
}


static void RedrawDial(Dial *dial)
{
    int r= dial->width/2-A_OFF;
    int x1,y1,x2,y2;
    
    XClearWindow(theDisp,dial->win);
    fillOval( dial->win, dial->gc, C_OFF, C_OFF,
	      dial->width-C_OFF*2, dial->height-C_OFF*2);
    fillOval( dial->win, dial->rvgc, C2_OFF, C2_OFF,
	      dial->width-C2_OFF*2, dial->height-C2_OFF*2);
    XDrawArc(theDisp, dial->win, dial->gc, A_OFF, A_OFF,
	     dial->width-A_OFF*2, dial->height-A_OFF*2,
	     -60*64, 300*64);

    x1= A_OFF-2+r*0.5;   /* r-rcosT+24 */
    y1= A_OFF-2+r*1.87;  /* r+rsinT+24 */
    x2= x1-5*1.5;
    y2= A_OFF-2+(r+5)*1.87;
    XDrawLine(theDisp, dial->win, dial->gc,
	      x1, y1, x2, y2);      
    if(dial->minLabel) {
	XDrawString(theDisp, dial->win, dial->gc,
		    x2-15, y2+5, dial->minLabel, strlen(dial->minLabel));
    }
    x1= A_OFF+r*1.5;   /* r-rcosT+24 */
    x2= A_OFF+(r+5)*1.5;
    XDrawLine(theDisp, dial->win, dial->gc,
	      x1, y1, x2, y2);      
    if(dial->maxLabel) {
	XDrawString(theDisp, dial->win, dial->gc,
		    x2+5, y2+5, dial->maxLabel, strlen(dial->maxLabel));
    }
    XDrawLine(theDisp, dial->win, dial->gc,
	      A_OFF-1+r, A_OFF-1, A_OFF-1+r, A_OFF-4);      
    if(dial->midLabel) {
	XDrawString(theDisp, dial->win, dial->gc,
		    A_OFF-1+r-5, A_OFF-6, dial->midLabel, strlen(dial->midLabel));
    }
    XDrawLine(theDisp, dial->win, dial->gc,
	      A_OFF-1, A_OFF-1+r, A_OFF-6, A_OFF-1+r);      
    XDrawLine(theDisp, dial->win, dial->gc,
	      A_OFF-1+2*r, A_OFF-1+r, A_OFF-1+2*r+5, A_OFF-1+r);      

    XFlush(theDisp);
}

static void Redraw_event_az (Dial *dial) 
{
    /* draw in the marker outside the arc */
    float ang,r;
    int xc,yc,xp,yp,xp1,yp1;
    int dr,dt;

    ang=DEG2RAD(90-dial->marked_value);

    /* circle center */
    r=10+(dial->height)/2- C2_OFF;
    xc=(dial->height)/2;
    yc=(dial->height)/2;

    /* apex for the triangle */
    xp=xc+r*(cos(ang));
    yp=yc+r*(-sin(ang));

    /* arrow shaft */
    dr=15;
    xp1=xc+(r+dr)*(cos(ang));
    yp1=yc+(r+dr)*(-sin(ang));

    if (dial->show_marked==1) {

	XDrawLine(theDisp, dial->win, dial->gc,
		  xp,yp,xp1,yp1);  
      
	/* arrow head */
	dr=5;
	dt=5;
	ang=DEG2RAD(90-dial->marked_value+dt);
	xp1=xc+(r+dr)*(cos(ang));
	yp1=yc+(r+dr)*(-sin(ang));
	XDrawLine(theDisp, dial->win, dial->gc,
		  xp,yp,xp1,yp1);  
	ang=DEG2RAD(90-dial->marked_value-dt);
	xp1=xc+(r+dr)*(cos(ang));
	yp1=yc+(r+dr)*(-sin(ang));
	XDrawLine(theDisp, dial->win, dial->gc,
		  xp,yp,xp1,yp1);  
    }


    XFlush(theDisp);
}

static float cur_theta;
static int is_drag=0;
static char number_buf[20];

float getTheta(Dial *dial, int x, int y)
{
#define RAD2DEG(r)  ((r)/3.14159265*180);
    float theta;

    if(x==dial->width/2) {
	theta=(y<dial->width/2)? 90. : -90.0;
    }else {
	theta=RAD2DEG(atan2( (double)(dial->width/2 - y),
			     (double)(x-dial->width/2)));
    }
    if(dial->type!=DIAL_ROT_TYPE) {
	if (theta >= -60) {
	    theta = theta - 150;
	}else if (theta >= -120) {
	    theta = -1;
	}else {
	    theta += 210;
	}
    }
    return theta;
}

static void cvs_event_proc(Xv_Window window, Event *event)
{
    Dial *dial=(Dial *)xv_get(window, XV_KEY_DATA, 33);
    int x, y;
    float theta;
    x=event_x(event), y=event_y(event);

    if (dial->inactive) return;
    if (!event_is_ascii(event)) {
	switch(event_action(event)) {
	case ACTION_SELECT:
	    theta=90.0-getTheta(dial,x, y);
	    if (theta!= 91.0) {
		if (event_is_down(event)) {
		    RedrawKnob(dial);
		    cur_theta=theta;
		    /*	Redraw_event_az(dial); */
		}else {
		    if(!is_drag) {
			dial->value= cur_theta;
			RedrawKnob(dial);
			/*   Redraw_event_az(dial);*/
		    }
		}
	    }
	    if(event_is_up(event)) {
		is_drag=0;
		if(dial->valChange)
		    dial->valChange(dial->value*dial->scale);
	    }
	    break;
	case LOC_DRAG:
	    if(event_left_is_down(event)) {
		theta=90.0-getTheta(dial,x, y);
		if (is_drag && theta!=-1) {
		    RedrawKnob(dial);
		    /*	Redraw_event_az(dial);*/
		}
		is_drag= event_is_down(event);
		if (theta == 91.0) return;
		cur_theta=theta;
		dial->value= theta;
		RedrawKnob(dial);
		/*    Redraw_event_az(dial);*/
	    }
	    break;
	}
    }

}
		    
void disable_dial(Dial *dial)
{
    dial->inactive=1;
}

void enable_dial(Dial *dial)
{
    dial->inactive=0;
}
