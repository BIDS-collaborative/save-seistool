#ifndef lint
static char id[] = "$Id: auto_rotate.c,v 1.2 2013/02/28 21:25:01 lombard Exp $";
#endif

/* 
 *    auto_rotate.c - calculate the angle for minimum amplitude
 *                     on horizontal traces
 *
 *     Steven Fulton
 *       97/03/11
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "mathlib.h"
#include "proto.h"
#include "xv_proto.h"

int do_z_win_only = 0;
extern int lowZTrkIndex;
extern int highZTrkIndex;
extern int ZoomWindowMapped;
extern int Mode_rotate;
extern int Mode_triplet;
extern Trace **traces;
extern int LastTrack;

static double dabs(double x) {
    return fabs(x);
}

static double dsqr(double x) {
    return x*x;
}

double (*use_function)(double) = dabs;


static double rotate_origin( Trace *cur_trace )
{
    BIS3_HEADER *bh = &cur_trace->wave->info;
    float rlat,rlon,del,azi,bazi;
    float lat,lon;
    
    if (cur_trace->wave->info.event_latitude == F4FLAG) return 0.0;
    lat=cur_trace->wave->info.event_latitude;
    lon=cur_trace->wave->info.event_longitude;
    rlat=cur_trace->wave->info.latitude;
    rlon=cur_trace->wave->info.longitude;
    if (rlat!=0.0 && rlon !=0.0) {
	azimuth(lat,lon,rlat,rlon,&del,&azi,&bazi);
    }

    cur_trace->trip->rot_theta = 180.0 + bazi;
    while (cur_trace->trip->rot_theta > 360.0)
	cur_trace->trip->rot_theta -= 360.0;
    cur_trace->trip->rot_unc = DEF_UNCERTAINTY;
    cur_trace->trip->rotated = 1;
    return cur_trace->trip->rot_theta;
}

static double Autorotate_search(Trace *x, Trace *y, int left_index,
				int right_index, int y_offset, 
				double (*fun) (double), char mini) {
    int i,j;
    double min_resid = 99e99,resid;
    double min_ang = -1;
    double  pi = atan(1)*4;

    for (i = 0; i < 180; i += 1) {
	double cax = cos(i*pi/180);
	double sax = sin(i*pi/180);
	double residY = 0,residX = 0;

	for (j = left_index; j<right_index; j++) {
	    /* Radial */
	    residY += fun(-sax*x->wave->data[j]-cax*y->wave->data[j+y_offset]);
	    /* Transverse  */
	    residX += fun(cax*x->wave->data[j]-sax*y->wave->data[j+y_offset]);
	}

	switch (mini) {
	case 'x':
	    resid = sqrt(residX); /* trans */
	    break;
	case 'y':
	    resid = sqrt(residY); /* radial */
	    break;
	}

	if (fabs(resid)<min_resid) {
	    /*      printf("%lf %lf %d :: ",min_resid,resid,i); */
	    min_resid = fabs(resid);
	    min_ang = i;
	}
      
    }

/*  printf (" from search min angle is %lf \n",min_ang); */
    return min_ang;
}

static double Autorotate_pavg(Trace *x, Trace *y, int left_index, 
			      int right_index, int y_off, 
			      double (*fun)(double), char mini) {
    int i;
    int num_data = right_index-left_index;
    double weigh,avg_ang = 0,t_ang,t_weigh = 0;
    double weigh_y = 0,weigh_x = 0;
    double  pi = atan(1)*4;

    for (i = left_index;i<right_index;i++) {
	weigh = sqrt(fun(x->wave->data[i])+fun(y->wave->data[i+y_off]));
	t_ang = atan2(x->wave->data[i],y->wave->data[i+y_off]);
	if (t_ang<0) t_ang += pi;
	avg_ang+=t_ang*weigh;
	t_weigh+=weigh;
    }
    avg_ang = (avg_ang/t_weigh)*(180/pi);
    weigh_y = weigh_y/(right_index-left_index+1);
    weigh_x = weigh_x/(right_index-left_index+1);
  
    printf("from point avg min anle is %lf y %lf x %lf\n",avg_ang,weigh_y,weigh_x); 
    return avg_ang;
}

static double Autorotate_analytic(Trace *x, Trace *y, int left_index,
				  int right_index, int y_off,
				  double (*fun) (double),char mini) 
{
    int i;
    double total_amp[2],answer;
    double  pi = atan(1)*4;
    double resid1,resid2;

    total_amp[0] = 0;
    total_amp[1] = 0;
  
    for(i = left_index;i<right_index;i++) {
	total_amp[0] += fun(x->wave->data[i]);
	total_amp[1] += fun(y->wave->data[i+y_off]);
    }

    if (fun == dsqr) {
	total_amp[0] = sqrt(total_amp[0]);
	total_amp[1] = sqrt(total_amp[1]);
    }
		      
    /* minimize the rotation of the traces */
    /* this needs station orientation to be added */
    switch (mini) {
    case 'y':
	answer = (atan2(total_amp[1],total_amp[0]));
	/* Transverse??? */
	resid1 = cos(answer)*total_amp[0]-sin(answer)*total_amp[1];
	/* Transverse??? */
	resid2 = -sin(answer)*total_amp[0]-cos(answer)*total_amp[1];

	break;
    case 'x':
	answer = (atan2(total_amp[0],-total_amp[1]));
	/* Radial??? */
	resid1 = -sin(answer)*total_amp[0]-cos(answer)*total_amp[1];
	/* Radial??? */
	resid2 = -cos(answer)*total_amp[0]+sin(answer)*total_amp[1];

	break;
    }

    if (fabs(resid1)>fabs(resid2)) {
	answer+=pi/2;
	printf("resid2\n");
    }
    answer = (answer/pi)*180;

    printf("from analytic min angle is %lf res1 %lf res2 %lf\n",answer,resid1,resid2);
    return answer;
}

/* the actual working routine */
static double Autorotate_Area(Triplet *trp,STI_TIME left, STI_TIME right, 
			      char mini, double (*fun) (double)) {
    double total_amp[] = {0,0};
    double answer = 0;
    double  pi = atan(1)*4;
    int i,y_off,left_index,right_index;
    STI_TIME fTime,x_ftime,x_etime;
    STI_TIME y_ftime,y_etime;

    Trace *x_trc = trp->trc[TRC_X];
    Trace *y_trc = trp->trc[TRC_Y];

    /* find if both have the to two end points */
    /* if not complain and adjust them         */
    /* and figure out the y_offset             */
    /* first see if either trace is not in the Time window */

    x_ftime = x_trc->wave->info.start_it;
    x_etime = indexToTime(x_trc, x_trc->wave->info.n_values, 1);
    y_ftime = y_trc->wave->info.start_it;
    y_etime = indexToTime(y_trc,y_trc->wave->info.n_values, 1);

    if (TimeCmp(right,x_ftime)<0 ||
	TimeCmp(right,y_ftime)<0 ||
	TimeCmp(left,x_etime)>0 ||
	TimeCmp(left,y_etime)>0){
	fprintf(stderr,"none overlapping Time window 1\n");
	return -361;
    }

    /* now the left end                      */
    if ( TimeCmp(left,x_ftime)<0) left = x_ftime;
    if ( TimeCmp(left,y_ftime)<0) left = y_ftime;
    /* and the right with last sample Times */
    if (TimeCmp(right,x_etime)>0) right = x_etime;
    if (TimeCmp(right,y_etime)>0) right = y_etime;

    /* now see if either trace is not in the Time window */
    if (TimeCmp(right,x_ftime)<0 ||
	TimeCmp(right,y_ftime)<0 ||
	TimeCmp(left,x_etime)>0 ||
	TimeCmp(left,y_etime)>0)    {
	fprintf(stderr,"none overlapping Time window 2 \n");
	return -361;
    }

    left_index = timeToIndex(x_trc,left);
    right_index = timeToIndex(x_trc,right);
    y_off = timeToIndex(y_trc,left)-left_index;

/*  printf("left_in %d reight_in %d y_offset %d\n",left_index, right_index, 
    y_off); */

#if 0
    answer = Autorotate_analytic(x_trc, y_trc, left_index, right_index, y_off, 
				 fun, mini);
    answer = Autorotate_pavg(x_trc, y_trc, left_index, right_index, y_off, fun,
			     mini);
#endif
    answer = Autorotate_search(x_trc, y_trc, left_index, right_index, y_off, 
			       fun, mini);

    trp->rot_theta = answer;
    trp->rot_unc = DEF_UNCERTAINTY; /* until we know something better... */
    trp->rotated = 1;   /* mark this triplet as `rotated' */
    return answer;
}


static void Autorotate_trace(Trace *cur_trace) {
    /* find the rotation passing the x-axis Time window */
    /* then store the rotation if the Time window exists.... */

    BIS3_HEADER *bh =  &cur_trace->wave->info;
    double rotation;
    Trace *y_trace;
    char tr_letter;

    if ( (y_trace = cur_trace->trip->trc[TRC_Y]) == NULL || 
	 cur_trace->trip->trc[TRC_X] == NULL )
	return;
    tr_letter = (cur_trace==y_trace)?'y':'x';
    rotation = Autorotate_Area(cur_trace->trip, bh->start_it,
			       indexToTime(cur_trace, bh->n_values, 1),
			       tr_letter, use_function);

    if (rotation != -361) {
	printf("%s %s %s %s rotation %lf\n", bh->station, bh->network, 
	       bh->channel, (strlen(bh->location) > 0) ? bh->location : "--", 
	       rotation);
    }
    return;
}


static void Autorotate_region(Trace *cur_trace) {
    Reg_select *next = cur_trace->sel_reg;
    char tr_letter;
    double rotation;
    BIS3_HEADER *bh =  &cur_trace->wave->info;

    if ( cur_trace->trip->trc[TRC_Y] == NULL ||
	 cur_trace->trip->trc[TRC_X] == NULL )
	return;
    tr_letter = (cur_trace==cur_trace->trip->trc[TRC_Y]) ? 'y' : 'x';
    while(next!=NULL) {
	rotation = Autorotate_Area(cur_trace->trip,
				   indexToTime(cur_trace, next->left_index, 1),
				   indexToTime(cur_trace, next->right_index, 1),
				   tr_letter, use_function);
	if (rotation !=-361) {
	    printf("%s %s %s %s  rotation %lf\n", bh->station, bh->network, 
		   bh->channel, (strlen(bh->location) > 0) ? bh->location : "--",
		   rotation);
	}
	next = next->next;
    }
    return;
}

static void Autorotate_Wrapper (int origin_flag)
 {
    /* When the button is pressed then .... */
    /* clear any old stored rotations       */
    /* 1. are there any regions selected?   */
    /*    yes do them  and quit             */
    /* 2. are there any selected traces?    */
    /*    yes do them                       */
    /* 3. do them all                       */
  
    /* now need to store all these rotations */
    /* store this stuff in client data and worry */
    /* about output later                    */
    /* add step through for rotation tool    */

    /* find which of the three modes to do */
    int mode = 2;
    int i;
    int min_tr,max_tr;

    /* make sure triplet mode is going */
    /* otherwise start it              */

    if (Mode_triplet==0) {
	StartTripletMode();
	/*    printf("Starting trip %d\n",Mode_triplet); */
    }
  
    if (do_z_win_only) {
	min_tr = lowZTrkIndex;
	max_tr = highZTrkIndex;
    }else {
	min_tr = 0;
	max_tr = LastTrack;
    }
  
    for (i = min_tr;i<=max_tr && mode!=0;i++) {
	if (traces[i]->trip->rotated < 0) 
	    continue;
	if (traces[i]->selected!=0) 
	    mode = 1;
	if (traces[i]->sel_reg!=NULL) 
	    mode = 0;
    }

    /* loop over each trace and do the autorotations */
    for(i = min_tr;i<=max_tr;i++) {
	if (traces[i]->trip->rotated == -1) 
	    continue;
	switch (mode) {
	    /* if the user selected a region or trace, rotate to minimize that
	     * component */
	case 0: 
	    if ( traces[i]->sel_reg != NULL) {
		if (origin_flag)
		    rotate_origin(traces[i]);
		else
		    Autorotate_region(traces[i]);
	    }
	    break;
	case 1:
	    if (traces[i]->selected!=0) {
		if (origin_flag)
		    rotate_origin(traces[i]);
		else
		    Autorotate_trace(traces[i]);
	    }
	    break;
	case 2:
	    /* None selected' rotate to minimize the X (transverse) component */
	    if (traces[i]->trip->trc[TRC_X] == traces[i]) {
		if (origin_flag)
		    rotate_origin(traces[i]);
		else
		    Autorotate_trace(traces[i]);
	    }
	    break;
	}
    }
    return;
}


void autor_events (Menu menu, Menu_item menu_item) 
{
    char *s =  (char *)xv_get(menu_item, MENU_STRING);
    int event_flag = 0;
    
    if (!strcmp(s,"rotate rms")) {
	use_function = dsqr;
	do_z_win_only = 0;
    } else if(!strcmp(s,"rotate abs")) {
	use_function = dabs;
	do_z_win_only = 0;
    } else if (!strcmp(s,"rotate event")) {
	do_z_win_only = 0;
	event_flag = 1;
    } else if(!strcmp(s,"rotate rms zoom")) {
	use_function = dsqr;
	do_z_win_only = 1;
    } else if(!strcmp(s,"rotate abs zoom")) {
	use_function = dabs;
	do_z_win_only = 1;
    } else if (!strcmp(s,"rotate event zoom")) {
	do_z_win_only = 1;
	event_flag = 1;
    }

    Autorotate_Wrapper( event_flag );
    if (Mode_rotate)
	Rot_ZoomContentChanged();
	     
}
