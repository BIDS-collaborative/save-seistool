#ifndef lint
static char id[] = "$Id: line_axes.c,v 1.2 2013/02/28 21:24:59 lombard Exp $";
#endif

/*
/* line_axes.c - routines to plot nice linear axis 
/*               
/*   Steven Fulton 97/03/05
/*
/* 
*/

#include <math.h>
#include "proto.h"

void scale_line_axis (double min_val,double max_val,double *inter,
		      double *out_min,double *out_max)
{
  double divs[]={0.1,0.2,0.25,0.4,0.5,0.75,1.0};
  double plus,units,len,work;
  double bias,n_len;
  int index,intervals=3,i,temp,count=0;

  if (min_val>max_val) {
    double temp=max_val;
    max_val=min_val;
    min_val=temp;
  } else if (min_val == max_val) {
    max_val=max_val + fabs(max_val) + 1.0;
  }

  len=max_val-min_val;

  n_len=2*len;
  while(n_len>len*1.5 && count<6) {
    count+=1;
    intervals+=1;
    work=(len/intervals)/pow(10,ceil(log10(len/intervals)));

    for (i=0;i<7;i++) {
      if (divs[i]>=work) break;
    }

    *inter=divs[i]*pow(10,ceil(log10(len/intervals)));

    *out_min=*inter*floor(min_val/(*inter));
    *out_max=*inter*ceil(max_val/(*inter));

    if (fabs(*out_min/units) < .01) *out_min=0.0;
    if (fabs(*out_max/units) < .01) *out_max=0.0;
    n_len=*out_max-*out_min;
  }
  return;
}
