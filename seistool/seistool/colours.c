#ifndef lint
static char id[] = "$Id: colours.c,v 1.2 2013/02/28 21:25:00 lombard Exp $";
#endif

/* 
   /*  colour.c - defines and setups the colours for
       /*             the selection regions and analysis
		      /*
		       */
#include <xview/xview.h>
#include <xview/cms.h>
#include <xview/window.h>

#include "xv_proto.h"

Cms glob_colours;
int num_glob_gc=8;
GC  glob_colour_gc[9];

void handle_colours(Window spc_win)
{
    int i;
    char dash_list[2];
    unsigned long *pix_tab;
    static int done_col=0;

    if (!done_col) {
	glob_colours=xv_create(NULL,CMS,CMS_SIZE, num_glob_gc+1,
			       CMS_NAMED_COLORS,"black", "red","blue","green",
			       "orchid","dark sea green","dark orange","HotPink",
			       "white",NULL,
			       NULL);
	pix_tab=(unsigned long *)xv_get(glob_colours, CMS_INDEX_TABLE);
  
	for(i=0;i<num_glob_gc+1;i++) {
	    createGC(spc_win,&glob_colour_gc[i]);
	    XSetForeground(theDisp,glob_colour_gc[i],pix_tab[i]);
	    XSetBackground(theDisp,glob_colour_gc[i],pix_tab[num_glob_gc]);
	    XSetLineAttributes(theDisp, glob_colour_gc[i], 1, LineSolid,
			       CapRound, JoinRound);  
	}
	done_col=1;
    }
}

int update_glob_line_at(int which)
{
    int j,i=which;
    char  dash[2]={8,4};
    char  dots[2]={4,4};

    j=i;
    if (j>num_glob_gc-1) {
	int type;
	j=i%num_glob_gc;
	type=(i/num_glob_gc)%3;
	switch (type) {
	case 0:
	    XSetLineAttributes(theDisp, glob_colour_gc[j], 1, LineSolid,
			       CapRound, JoinRound);
	    break;
	case 1:
	    XSetLineAttributes(theDisp, glob_colour_gc[j], 1, LineOnOffDash,
			       CapRound, JoinRound);
	    XSetDashes(theDisp, glob_colour_gc[j],0,dash,2);
	    break;
	case 2:
	    XSetLineAttributes(theDisp, glob_colour_gc[j], 1, LineOnOffDash,
			       CapRound, JoinRound);
	    XSetDashes(theDisp, glob_colour_gc[j],0,dots,2);
	    break;
	}
    }
    return j;
}

void reset_glob_line_at(void)
{
    int i;
    for (i=0;i<num_glob_gc;i++) {
	XSetLineAttributes(theDisp, glob_colour_gc[i], 1, LineSolid,
			   CapRound, JoinRound);
    }	
}
