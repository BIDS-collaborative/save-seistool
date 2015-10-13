#ifndef lint
static char id[] = "$Id: select_reg.c,v 1.2 2013/02/28 21:24:57 lombard Exp $";
#endif

/*                                                                   */
/*   select_reg.c - routines to allow graphical selection of regions */  
/*                  for pmotion , spectra and other routines         */
/*                                                                   */
/*                    Steven Fulton                                  */
/*                     97/03/16                                      */
/*                                                                   */
/*               	                                             */

#include <math.h>
#include <stdlib.h>
#include <xview/xview.h>
#include <xview/frame.h>
#include <xview/canvas.h>
#include <xview/panel.h>
#include <xview/win_input.h>
#include <xview/win_event.h>
#include "proto.h"
#include "xv_proto.h"

#define PANEL_SEL_REG 145

/*********************/
/* global variables  */
/*********************/

/* window stuff */

static int gsrwin_h =  500;        /* window detault height */
static int gsrwin_w =  500;        /* window default width */
static int gsr_boarh = 10;         /* horizontal boarder window */
static int gsr_boarv = 10;         /* vertical boarder window   */
static int gsr_sub_boardh = 20;    /* horizontal boarder for traces */
static int gsr_sub_boardv = 10;    /* vertical boarder for traces */
static int gsr_trace_xlen = 0;     /* width of the traces  */
static int gsr_trace_ylen = 0;     /* hieght of the traces */
static int gsr_num_win = 0;        /* the number of graphical selection wins*/
static Select_Info **gsr_win_Info = NULL; /* the array of gsr windows */

extern Frame tracesFrame;
extern GC glob_colour_gc[9];
extern Display *theDisp;
extern Trace **traces;
extern int LastTrack;
extern int num_glob_gc;

/* global variables */
Trace **sel_traces = NULL;          /* the selection regions in a trace structure */
int num_sel_traces = 0;


static void init_selection_values(Select_Info *i_reg);
static Select_Info *initialize_Graph_Select_Reg(void);
static void gsr_select_all(Panel_item item);
static void gsr_deselect_all(Panel_item item);
static void gsr_default(Panel_item item);
static int find_gsr_num_win(Xv_Window window);
static void set_new_reg(Select_Info *reg,int one);
static void unset_reg(Select_Info *reg, int one);
static void update_sel_disp(Select_Info *reg);
static void gsr_event_handle(Xv_Window	window, Event *event);
static void fil_sel_reg(Select_Info *a);
static void draw_ind_trace(Select_Info *plot_win, int x, int y,
			   double hor_scale, double ver_scale,int which);
static void ind_win_max_min(float *max, float *min, Wave *wve, int start,
			    int end);
static Select_Info *find_sel_win(Canvas find_win);
static void resize_gsrwin(Canvas what_can,int width,int height);
static void update_gsr_wind_reg_sel(void);
static void Destroy_gsr_win(Frame fram);
static Canvas gsr_find_canvas(Frame frame);
static void close_gsr_win(Panel_item item);


/***********************/
/* implementation code */
/***********************/

static void init_selection_values(Select_Info *i_reg) {
    i_reg->can = NULL;
    i_reg->board = NULL;
    i_reg->wind = NULL;
    i_reg->num_wind = 0;
    i_reg->num_row = 0;
    i_reg->num_col = 0;
    i_reg->num_sel = 0;
    i_reg->regs = NULL;
    i_reg->sel_reg = NULL;
}

static Select_Info *initialize_Graph_Select_Reg(void){
    Panel panel;
    Frame	gsr_frame =  NULL;
    Canvas gsr_cvs =  NULL;
    Window gsr_win;
    Xv_Window window;
    Select_Info *ret_val = (Select_Info*)Malloc(sizeof(Select_Info));

    gsr_frame = xv_create(tracesFrame, FRAME,
			XV_HEIGHT, gsrwin_h+40, XV_WIDTH, gsrwin_w,
			NULL);		  
    gsr_cvs =  xv_create(gsr_frame, CANVAS,
		       XV_X, 0, XV_Y, 40,		  
		       CANVAS_REPAINT_PROC, redraw_gsrwin,
		       CANVAS_RESIZE_PROC, resize_gsrwin,
		       NULL);		  
    gsr_win =  xv_get(canvas_paint_window(gsr_cvs), XV_XID);

    handle_colours(gsr_win);

    window = (Xv_Window)xv_get(gsr_cvs,CANVAS_NTH_PAINT_WINDOW,0);
    xv_set(window,WIN_EVENT_PROC,gsr_event_handle,
	   WIN_CONSUME_EVENTS,WIN_MOUSE_BUTTONS,NULL,
	   NULL);
    panel =  (Panel)xv_create(gsr_frame, PANEL,
			    XV_X, 0, XV_Y, 0, XV_HEIGHT, 40,
			    NULL);
    (void)xv_create(panel, PANEL_BUTTON,
		    PANEL_LABEL_STRING, "Quit",
		    PANEL_NOTIFY_PROC, close_gsr_win,
		    PANEL_CLIENT_DATA,gsr_frame,
		    NULL);
    (void)xv_create(panel, PANEL_BUTTON,
		    PANEL_LABEL_STRING, "Select All",
		    PANEL_NOTIFY_PROC, gsr_select_all,
		    XV_KEY_DATA, PANEL_SEL_REG,ret_val,
		    NULL);
    (void)xv_create(panel, PANEL_BUTTON,
		    PANEL_LABEL_STRING, "Deselect All",
		    PANEL_NOTIFY_PROC, gsr_deselect_all,
		    XV_KEY_DATA, PANEL_SEL_REG,ret_val,
		    NULL);
    (void)xv_create(panel, PANEL_BUTTON,
		    PANEL_LABEL_STRING, "Default Selection",
		    PANEL_NOTIFY_PROC, gsr_default,
		    XV_KEY_DATA, PANEL_SEL_REG,ret_val,
		    NULL);

    init_selection_values(ret_val);
    ret_val->can = gsr_cvs;
    ret_val->board = gsr_frame;
    ret_val->wind = gsr_win;
    createGC(gsr_win, &ret_val->txtgc);
    fil_sel_reg(ret_val);
    return ret_val;
}

void gsr_select_all(Panel_item item)
{
    Select_Info *reg_work = (Select_Info*)xv_get(item,XV_KEY_DATA, PANEL_SEL_REG);
    int *new_sel;
    int num_iteams = 1;
    int i;

    switch(reg_work->sel_type) {
    case 0:
	reg_work->num_sel = 1;
	break;
    case 1:
	reg_work->num_sel = reg_work->num_wind;
	break;
    case 2:
	reg_work->num_sel = 2;
	break;
    }

    new_sel = (int*)Malloc(sizeof(int)*(reg_work->num_sel+1));
  
    for (i = 0;i<reg_work->num_sel;i++) {
	new_sel[i] = i;
    }

    free(reg_work->sel_reg);
    reg_work->sel_reg = new_sel;
    redraw_gsrwin(reg_work->can);

    switch(reg_work->sel_type) {
    case 1:
	event_hand_spectrum(reg_work);
	break;
    case 2:
	if (reg_work->num_wind == 1) {
	    reg_work->sel_reg[1] = 0;
	}
	redrawXcorr();
	break;
    }

    return;
}
static void gsr_deselect_all(Panel_item item)
{
    Select_Info *reg_work = (Select_Info*)xv_get(item,XV_KEY_DATA, PANEL_SEL_REG);
    int *new_sel,num_iteams = 1;
    int i;

    switch(reg_work->sel_type) {
    case 0:
	reg_work->num_sel = 1;
	break;
    case 1:
	reg_work->num_sel = 0;
	break;
    case 2:
	reg_work->num_sel = 2;
	break;
    }

    new_sel = (int*)Malloc(sizeof(int)*(reg_work->num_sel+1));
  
    for (i = 0;i<reg_work->num_sel;i++) {
	new_sel[i] = i;
    }

    free(reg_work->sel_reg);
    reg_work->sel_reg = new_sel;
    redraw_gsrwin(reg_work->can);

    switch(reg_work->sel_type) {
    case 1:
	event_hand_spectrum(reg_work);
	break;
    case 2:
	if (reg_work->num_wind == 1) {
	    reg_work->sel_reg[1] = 0;
	}
	redrawXcorr();
	break;
    }

    return;
}

static void gsr_default(Panel_item item)
{
    Select_Info *reg_work = (Select_Info*)xv_get(item,XV_KEY_DATA, PANEL_SEL_REG);
    int *new_sel,num_iteams = 1;
    int i;

    switch(reg_work->sel_type) {
    case 0:
	reg_work->num_sel = 1;
	break;
    case 1:
	reg_work->num_sel = 1;
	break;
    case 2:
	reg_work->num_sel = 2;
	break;
    }

    new_sel = (int*)Malloc(sizeof(int)*(reg_work->num_sel+1));
  
    for (i = 0;i<reg_work->num_sel;i++) {
	new_sel[i] = i;
    }

    free(reg_work->sel_reg);
    reg_work->sel_reg = new_sel;
    redraw_gsrwin(reg_work->can);

    switch(reg_work->sel_type) {
    case 1:
	event_hand_spectrum(reg_work);
	break;
    case 2:
	if (reg_work->num_wind == 1) {
	    reg_work->sel_reg[1] = 0;
	}
	redrawXcorr();
	break;
    }

    return;
}

static int find_gsr_num_win(Xv_Window window)
{
    Window win = (Window)xv_get(window,XV_XID);
    int i;

    for (i = 0;i<gsr_num_win;i++) {
	if (win == gsr_win_Info[i]->wind) {
	    return i;
	}
    }
    return -1;
}

static void set_new_reg(Select_Info *reg,int one)
{
    int i;
    /* add new one to end of list */
    if (reg->sel_type == 1) {
	/* if already selected do not multiply select */
	for (i = 0;i<reg->num_sel;i++) {
	    if (reg->sel_reg[i]==one) {
		return;
	    }
	}
    }

    reg->sel_reg = (int*)realloc(reg->sel_reg,sizeof(int)*(reg->num_sel+2));
    reg->num_sel+=1;
    reg->sel_reg[reg->num_sel-1] = one;
}

static void unset_reg(Select_Info *reg, int one)
{
    /* find the region to remove */
    /* rebuild list */
    int *temp_reg = (int*)Malloc(sizeof(int)*(reg->num_sel));
    int i,j = 0,found=-1,first = 0;

    if (one==-1) {
	found = 1;
	first = 1;
    }

    for (i = first;i<reg->num_sel;i++) {
	if (reg->sel_reg[i]!=one) {
	    temp_reg[j++] = reg->sel_reg[i];
	} else {
	    found = 1;
	}
    }

    if (reg->sel_reg!=NULL && found!=-1) {
	free(reg->sel_reg);
	reg->sel_reg = temp_reg;
	reg->num_sel-=1;
    }
}

static void update_sel_disp(Select_Info *reg)
{
    int i,j,x,y;

    /* clear all the selection marks */
    for (i = 0;i<reg->num_col;i++) {
	x = i*reg->hor_size+gsr_boarh;
	for (j = 0;j<reg->num_row && (j+i*reg->num_row)<reg->num_wind;j++) {
	    /* find upper right corner of rectangle */
	    /* draw blanking rectangle there */
	    y = j*reg->ver_size+gsr_boarv;
	    XDrawRectangle(theDisp,reg->wind,glob_colour_gc[num_glob_gc],x+1,y+1,reg->hor_size-1,reg->ver_size-1);
	}
    }

    /* draw the selection marks */
    for (i = 0;i<reg->num_sel;i++) {
	int s;
	/* find upper right corner of rectangle */
	/* draw selection  rectangle there */
	s = update_glob_line_at(i);
	x = (int)floor(reg->sel_reg[i]/reg->num_row);
	x = x*reg->hor_size+gsr_boarh;
	y = (int)reg->sel_reg[i]%reg->num_row;
	y = y*reg->ver_size+gsr_boarv;
	XDrawRectangle(theDisp,reg->wind,glob_colour_gc[s],x+1,y+1,reg->hor_size-1,reg->ver_size-1);
    }
    if (reg->num_sel>num_glob_gc) {reset_glob_line_at();}
    XFlush(theDisp);
}

static void gsr_event_handle(Xv_Window	window, Event *event)
{
    /* 1. which gsr */
    int gsr_int = find_gsr_num_win(window);
    int which_wave = -1,x_loc,y_loc;
    int x_mouse = event->ie_locx,y_mouse = event->ie_locy;

    if (gsr_int ==-1) {
	fprintf (stderr,"no such window found for event %d\n",window);
	return;
    }

    /* 1a determine the select region to work on */
    x_loc = floor(x_mouse/gsr_win_Info[gsr_int]->hor_size);
    y_loc = floor(y_mouse/gsr_win_Info[gsr_int]->ver_size);
    which_wave = x_loc*gsr_win_Info[gsr_int]->num_row+y_loc;
    if (which_wave >= gsr_win_Info[gsr_int]->num_wind) {return;}

    /* 2. what action to do :*/
    /*     set / unset / call new window */
    if (!event_is_button(event)) {return;}

    switch(gsr_win_Info[gsr_int]->sel_type) {
    case 1: /* any number of selections */
	if (event_left_is_down(event)) {
	    /* select new reg */
	    set_new_reg(gsr_win_Info[gsr_int],which_wave);
	} else if (event_right_is_down(event)) {
	    /* delete region */
	    unset_reg(gsr_win_Info[gsr_int],which_wave);
	}
	event_hand_spectrum(gsr_win_Info[gsr_int]);
	break;
    case 0: /* triplet only (1) selection */
    case 2: /* fixed number (1 or 2) */
	if (event_is_down(event)) {
	    unset_reg(gsr_win_Info[gsr_int],-1);
	    set_new_reg(gsr_win_Info[gsr_int],which_wave);
	}
	redrawXcorr();
	break;
    }
    /* 2a display the new selection states*/
    update_sel_disp(gsr_win_Info[gsr_int]);

    /* 3. inform the calling process of the change */
}

void fil_sel_reg(Select_Info *a)
{
    int i,j,count = 0,num_count = 0;
    Trace *pnt_trc;
    Reg_select *cur_reg;
    double sec_off;

    for (i = 0;i<=LastTrack;i++) {
	pnt_trc = traces[i];
	cur_reg = pnt_trc->sel_reg;
	while(cur_reg!=NULL) {
	    num_count+=1;
	    cur_reg = cur_reg->next;
	}
    }

    a->regs = (Wave*)Malloc(sizeof(Wave)*(num_count+1));
    for (i = 0;i<=LastTrack;i++) {
	pnt_trc = traces[i];
	cur_reg = pnt_trc->sel_reg;
	while(cur_reg!=NULL) {

	    /* fill in all wave info for the region*/
	    a->regs[count]=*(traces[i]->wave);
	    a->regs[count].info.n_values = cur_reg->right_index - 
		cur_reg->left_index;
	    if (a->regs[count].info.n_values < 0) 
		a->regs[count].info.n_values *= -1;
	    a->regs[count].data+=cur_reg->left_index;

	    /* fix the date */
	    sec_off = (double)cur_reg->right_index / 
		a->regs[count].info.sample_rate;
	    a->regs[count].info.start_it = 
		st_add_dtime(pnt_trc->wave->info.start_it, 
			     sec_off * USECS_PER_SEC);
	    count += 1;
	    cur_reg = cur_reg->next;
	}
    }
    a->num_wind = count;
    a->num_row = ceil(sqrt(count));
    a->num_col = sqrt(count);

    if (count>a->num_row*a->num_col) a->num_col+=1;

}

/* figure out what is really needed to be done */
void graphical_selection_routine(Trace sel_reg_list[])
{
}

/* draw the waves */
/* trc is the trace to draw */
/* x and y are the lower left corner */
static void draw_ind_trace(Select_Info *plot_win, int x, int y,
			   double hor_scale, double ver_scale,int which)
{
    int x_loc,y_loc;
    int n,i;
    char m_label[100];
    char s_label[100];
    XPoint *points;
    Wave wve = plot_win->regs[which];
    double fxval,fyval,f_max,f_min;
    int txval,t_prev=-1,plot_pos = 0,n_plot = 0;
    STE_TIME et;

    /* 1. figure where the box should be */
    /* then draw it */
    XDrawRectangle(theDisp,plot_win->wind,glob_colour_gc[1],x,y,plot_win->trace_hbox,plot_win->trace_vbox);
    /* draw time axis */
    /* draw amplitude axis */
    /* draw trace */
    points = (XPoint *)Malloc(sizeof(XPoint)*(wve.info.n_values));
    for (i = 0;i<wve.info.n_values;i++) {
	txval = x+i/hor_scale;
	fyval = y+(wve.dcOffset-wve.data[i])/ver_scale;

	if (t_prev != txval) {
	    /* plot old point */
	    if (n_plot != 0) {
		points[plot_pos].x = t_prev;
		points[plot_pos++].y = f_min;
		if (n_plot>1) {
		    points[plot_pos].x = t_prev;
		    points[plot_pos++].y = f_max;
		}
	    }

	    /* new_point */
	    t_prev = txval;
	    f_min = f_max = fyval;
	    n_plot = 1;
	} else {
	    n_plot += 1;
	    if (fyval >= f_max) {
		f_max = fyval;
	    } else if (fyval < f_min) {
		f_min = fyval;
	    }
	}

    }

    XDrawLines(theDisp, plot_win->wind,glob_colour_gc[0], points,plot_pos-1,CoordModeOrigin);
    XFlush(theDisp);

    /* draw region label */
    sprintf(m_label,"%s.%s.%s.%s",wve.info.station,wve.info.network,
	    wve.info.channel, (wve.info.location[0] == '\0' ? "--" :
			       wve.info.location));
    et = sti_to_ste(wve.info.start_it);
    sprintf(s_label,"%4d.%03d %02d:%02d:%02d", et.year, et.doy, et.hour, 
	    et.minute, et.second);
    y_loc = y-.15*plot_win->ver_size;
    XDrawString(theDisp, plot_win->wind,plot_win->txtgc,x,y_loc,m_label,strlen(m_label)); 
    y_loc = y-.05*plot_win->ver_size;
    XDrawString(theDisp, plot_win->wind,plot_win->txtgc,x,y_loc,s_label,strlen(s_label));

    XFlush(theDisp);

    free(points); 
}


/* this routine finds the max and min values */
/* for a data set defined by the indexes     */
static void ind_win_max_min(float *max, float *min, Wave *wve, int start,
			    int end)
{
    float t_min = 99e29,t_max = -99e29,point;
    int i;
     
    for (i = start;i <= end;i++) {
	point = wve->data[i];
       
	if (point>t_max) t_max = point;
	if (point<t_min) t_min = point;
    }
    *max = t_max;
    *min = t_min;
}


static Select_Info *find_sel_win(Canvas find_win)
{
    /* search the array of these windows */
    int i;
  
    for (i = 0;i<gsr_num_win;i++) {
	if (find_win==gsr_win_Info[i]->can) {
	    return gsr_win_Info[i];
	}
    }
    return NULL;
}

static void resize_gsrwin(Canvas what_can,int width,int height)
{
    int redraw = 0;
    Select_Info *sel_win = NULL;
    sel_win = find_sel_win(what_can);
    if (sel_win == NULL) {
	fprintf (stderr,"what is up with redraw no window found\n");
	return;
    }
 
    if (sel_win->ver_size >= height && width <= sel_win->hor_size)
	redraw = 1;
    sel_win->ver_size =  height;
    sel_win->hor_size = width;

    if (redraw)
	redraw_gsrwin(what_can);
}

/* Redraw the window                   */
/* use this to redraw the whole window */
void redraw_gsrwin(Canvas what_win)
{
    int num_select = 0,select_num = 0;
    int i,j,k;
    int x,y;
    int full_width,full_height;
    int sub_width,sub_height;
    int num_hor,len_hor;
    int num_ver,len_ver;
    float y_max,y_min;

    Select_Info *sel_win = NULL;

    /* find which window we are working on */
    sel_win = find_sel_win(what_win);
    if (sel_win==NULL) {
	fprintf (stderr,"what is up with redraw no window found\n");
	return;
    }
  
    if (sel_win->num_wind == 0) {
	/* close up shop */
	fprintf(stderr,"no regions selected\n");
	return;
    }

    XClearWindow(theDisp,sel_win->wind);
    /* figure out the size of the main window */
    /* and each of the selection windows      */

    num_hor = sel_win->num_col;
    num_ver = sel_win->num_row;
    if (sel_win->num_wind>(num_hor*num_ver)) {num_hor+=1;}

    full_width = (int)xv_get(sel_win->can,CANVAS_WIDTH);
    full_height = (int)xv_get(sel_win->can,CANVAS_HEIGHT);
    sel_win->hor_size = (full_width-gsr_boarh*2)/num_hor;
    sel_win->ver_size = (full_height-gsr_boarv*2)/num_ver;
    sub_width = 2*sel_win->hor_size/3;
    sub_height = sel_win->ver_size/2;
    sel_win->trace_hbox = sub_width;
    sel_win->trace_vbox = sub_height;

    /* draw the individual traces */
    for (j = 0;j<sel_win->num_wind;j++) {
	float min,max;
	double h_scale, v_scale;

	/* figure location of window */    
	x = floor(j/num_ver);
	y = j-x*num_ver;
	x = gsr_boarh+(x+.25)*sel_win->hor_size;
	y = gsr_boarv+(y+.25)*sel_win->ver_size;

	/* figure the scale of the window */
	ind_win_max_min(&max,&min,&sel_win->regs[j],0,sel_win->regs[j].info.n_values);
	h_scale = (double)(sel_win->regs[j].info.n_values-1)/sub_width;
	v_scale = (double)(max-min)/sub_height;
	sel_win->regs[j].dcOffset = max;

	draw_ind_trace(sel_win,x,y,h_scale,v_scale,j);
    }
    /* now indicate selection locations */
    update_sel_disp (sel_win);
}

/* update regions */
/* these routines update the windows when the number of */
/* selection regions change */
static void update_gsr_wind_reg_sel(void)
{
    int i,num_reg = 0;
    int num_col = 0;
    int num_row = 0;
    int count = 0;
    Trace *pnt_trc;
    Reg_select *cur_reg;
    Wave *temp_wave;
    double sec_off;


    /* first are there any gsrs in use? */
    if (gsr_win_Info==NULL) {return;}

    /* loop through all traces to see how many selection */
    /* regions there are */

    for (i = 0;i<=LastTrack;i++) {
	pnt_trc = traces[i];
	cur_reg = pnt_trc->sel_reg;
	while(cur_reg!=NULL) {
	    num_reg+=1;
	    cur_reg = cur_reg->next;
	}
    }

    if (num_reg==0) {
	/* what should happen??? */
	/* need to close off everthing related to these regions */
    }
    num_row = ceil(sqrt(num_reg));
    num_col = sqrt(num_reg);
    if (num_col*num_row<num_reg) {num_col+=1;}

    /* fill a temp array with all the regions */
    temp_wave = (Wave*)Malloc((sizeof(Wave)+1)*num_reg);
    for (i = 0;i<=LastTrack;i++) {
	pnt_trc = traces[i];
	cur_reg = pnt_trc->sel_reg;
	while(cur_reg!=NULL) {

	    /* fill in all wave info for the region*/
	    temp_wave[count]=*(traces[i]->wave);
	    temp_wave[count].info.n_values = cur_reg->right_index-cur_reg->left_index;
	    if (temp_wave[count].info.n_values<0) temp_wave[count].info.n_values*=-1;
	    temp_wave[count].data+=cur_reg->left_index;

	    /* fix the date */

	    sec_off = (double)cur_reg->right_index /
		temp_wave[count].info.sample_rate;
	    temp_wave[count].info.start_it = 
		st_add_dtime(pnt_trc->wave->info.start_it,
			     sec_off * USECS_PER_SEC);
	    count+=1;
	    cur_reg = cur_reg->next;

	}
    }

    /* go through all the windows and do what??? */
    for (i = 0;i<gsr_num_win;i++) {
	if (gsr_win_Info[i]->sel_type) {
	    int remove_sel = 0;
	    int j;
	    gsr_win_Info[i]->num_row = num_row;
	    gsr_win_Info[i]->num_col = num_col;
	    gsr_win_Info[i]->num_wind = num_reg;
	    for (j = 0;j<num_reg;j++) {
		gsr_win_Info[i]->regs[j] = temp_wave[j];
	    }

	    /* fix selection */
	    /* note - I do not know which was removed so the       */
	    /*        used selections will get messed up           */
	    /*        this only fixes the obvious off the end case */
	    for (j = 0;j<gsr_win_Info[i]->num_sel;j++) {
		if (gsr_win_Info[i]->sel_reg[j]>num_reg) {
		    if (gsr_win_Info[i]->sel_type==2) {
			gsr_win_Info[i]->sel_reg[j] = 0;
		    } else {
			gsr_win_Info[i]->sel_reg[j]=-1;
			remove_sel+=1;
		    }
		}
	    }
	    if (remove_sel!=0) {
		int *t_sel = (int*)Malloc(sizeof(int)*(gsr_win_Info[i]->num_sel-remove_sel)+1);
		int t,c = -1;
		for (t = 0;t<gsr_win_Info[i]->num_sel &&c<=gsr_win_Info[i]->num_sel-remove_sel;t++) {
		    if (gsr_win_Info[i]->sel_reg[j]!=-1) {
			t_sel[++c] = gsr_win_Info[i]->sel_reg[j];
		    }
		}
		free(gsr_win_Info[i]->sel_reg);
		gsr_win_Info[i]->sel_reg = t_sel;
	    }

	    redraw_gsrwin(gsr_win_Info[j]->can);

	} else {
	    /* what about the specific trip windows */
	    /* I will not worry about this case     */
	}

    }
}
     

/* Creation of window */
Select_Info *creating_gsr_win(int type)
{
    Select_Info *new_win = initialize_Graph_Select_Reg();
    int new_loc;

    switch (type) {
    case 0:			/* spectrum */ 
    case 1:			/* time-spectrum */
	new_win->sel_type = 1;
	new_win->num_sel = 1;
	new_win->sel_reg = (int*)Malloc(sizeof(int)*(2));
	new_win->sel_reg[0] = 0;
	break;
    case 2:			/* x-spectrum */
                                /* x-correlation */
	new_win->sel_type = 2;
	new_win->num_sel = 2;
	new_win->sel_reg = (int*)Malloc(sizeof(int)*(3));
	new_win->sel_reg[0] = 0;
	if (new_win->num_wind==1) {
	    new_win->sel_reg[1] = 0;
	} else {
	    new_win->sel_reg[1] = 1;
	}
	break;
    case 3:			/* particle motion */
	new_win->num_sel = 1;
	new_win->sel_type = 0;
	break;
    }
     
    gsr_num_win+=1;

    if (gsr_num_win<2) {
	gsr_win_Info = (Select_Info **)Malloc(sizeof(Select_Info*)*(gsr_num_win+1));
    } else {
	gsr_win_Info = (Select_Info **)realloc(gsr_win_Info,sizeof(Select_Info*)*(gsr_num_win+1));
    }

    new_loc = gsr_num_win-1;
    gsr_win_Info[new_loc] = new_win;

    redraw_gsrwin(new_win->can);
    xv_set(gsr_win_Info[new_loc]->board,XV_SHOW,TRUE,NULL);
    return new_win;
}

static void Destroy_gsr_win(Frame fram)
{
    Select_Info **temp_stuff = (Select_Info**)Malloc(sizeof(Select_Info*)*gsr_num_win-1);
    int i,j = 0,mark=-1;
 
    /* copy data to new array */
    for(i = 0;i<gsr_num_win;i++) {
	if (fram!=gsr_win_Info[i]->board) {
	    temp_stuff[j++] = gsr_win_Info[i];
	} else {
	    mark = i;
	}
    }
  
    if (mark==-1) {
	fprintf(stderr,"Not sure what is up Destroy_gsr_win finds no match\n");
    }

    /* free old memory */
    free(gsr_win_Info);

    /* relocation of new memory */
    gsr_win_Info = temp_stuff;
    gsr_num_win -= 1;
}

static Canvas gsr_find_canvas(Frame frame)
{
    /* search the array of these windows */
    int i;
  
    for (i = 0;i<gsr_num_win;i++) {
	if (frame==gsr_win_Info[i]->board) {
	    return gsr_win_Info[i]->can;
	}
    }
    return NULL;
}

static void close_gsr_win(Panel_item item)
{
    Frame t_frame = (Frame)xv_get(item,PANEL_CLIENT_DATA);

    /* clear the region from memory */
    Destroy_gsr_win(t_frame);
    xv_destroy_safe(t_frame);
    return ;
}


