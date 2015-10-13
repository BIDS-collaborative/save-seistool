#ifndef lint
static char id[] = "$Id: ctrl.c,v 1.3 2013/02/28 21:25:00 lombard Exp $";
#endif

/*
 * ctrl.c--
 *    the controls panel
 *
 * Copyright (c) 1992-3 Andrew K. Yu, Lawrence Berkeley Laboratory
 *      and University of California, Berkeley.
 * All rights reserved.
 */
#include <stdio.h>

#include "proto.h"
#include "xv_proto.h"

static Frame ctl_frame= 0;
static Panel panel;
static Panel_item check_box;
static Scrollbar bar;
static Panel_item slider,zslider,subslider;
static Panel_item scale_choice;
static Panel_item align_choice;
static int old_scale_cho;

extern Panel_item tscaleButton;
extern int Mode_abs_vscale;
extern int Zoom_Mode_sameVertScale;
extern int subsample_rate;
extern int Mode_Auto_redraw;
extern int auto_demean;
extern float fq_fl_value;
extern float fq_fh_value;
extern Trace **traces;
extern int TotalTracks;
extern int NumTracks;
extern int LastTrack;
extern int saveWaifPicks;

static void ctl_activate(), ctl_cancel(), ctl_reset();

static void InitCtrlPanel(Frame frame, int x, int y)
{   
    Rect *rect;
    Panel_item *t_panel;

    ctl_frame = (Frame)xv_create(frame, FRAME_CMD,
				 FRAME_LABEL, "Controls Panel",
				 XV_X, x, XV_Y, y,
				 XV_HEIGHT, 20, XV_WIDTH, 20,
				 NULL);
    panel = (Panel)xv_get(ctl_frame, FRAME_CMD_PANEL);
    xv_set(panel,PANEL_ITEM_X_GAP, 20,
	   PANEL_ITEM_Y_GAP, 20,
	   /*	   PANEL_LAYOUT, PANEL_VERTICAL,	     */
	   NULL);
    check_box= (Panel_item)xv_create(panel, PANEL_CHECK_BOX,
				     /*	XV_X, 30, XV_Y, 20,    */
				     PANEL_LAYOUT, PANEL_VERTICAL,	     
				     PANEL_LABEL_STRING, "Modes:",
				     PANEL_CHOICE_STRINGS,
				     "auto-load picks", "decimate plotting",
				     "auto-scroll", "show borders",
				     "UW pick style","Auto Refresh", 
				     "regroup", "group comp", 
				     "demean", "save picks with no traces",
				     NULL,
				     PANEL_CHOICE_NCOLS, 2, 
				     NULL);
    scale_choice=(Panel_item)xv_create(panel,PANEL_CHOICE,
				       PANEL_LABEL_STRING,"Scaling:",
				       /*		  XV_X, 20, XV_Y, 190,*/
				       PANEL_LAYOUT,PANEL_HORIZONTAL,
				       PANEL_CHOICE_STRINGS,"relative","zoom","all",NULL,
				       PANEL_VALUE,0,
				       NULL);
    align_choice=(Panel_item)xv_create(panel,PANEL_CHOICE,
				       PANEL_LABEL_STRING,"Time Alignment:",
				       /* 		  XV_X, 20, XV_Y, 160, */
				       PANEL_LAYOUT,PANEL_HORIZONTAL,
				       PANEL_CHOICE_STRINGS,"none","Time Align","Time Scale",NULL,
				       PANEL_VALUE,0,
				       NULL);
    slider= xv_create(panel, PANEL_SLIDER,
		      /*	XV_X, 20, XV_Y, 220,	 */
		      PANEL_LAYOUT, PANEL_VERTICAL,
		      PANEL_LABEL_STRING, "Main Traces:     ",
		      PANEL_MIN_VALUE, 1,
		      PANEL_MAX_VALUE, TotalTracks,
		      PANEL_SHOW_RANGE, TRUE,
		      PANEL_VALUE, NumTracks,
		      NULL);
    zslider= xv_create(panel, PANEL_SLIDER,
		       /*	XV_X, 20, XV_Y, 250,  */
		       PANEL_LAYOUT, PANEL_VERTICAL,
		       PANEL_LABEL_STRING, "Zoomed Traces:   ",
		       PANEL_MIN_VALUE, 1,
		       PANEL_MAX_VALUE, TotalTracks,
		       PANEL_SHOW_RANGE, TRUE,
		       PANEL_VALUE, NumZTracks,
		       NULL);
    subslider= xv_create(panel, PANEL_SLIDER,
			 /*	XV_X, 20, XV_Y, 280,  */
			 PANEL_LAYOUT, PANEL_VERTICAL,
			 PANEL_LABEL_STRING, "Number of Subsamples: ",
			 PANEL_MIN_VALUE, 1,
			 PANEL_MAX_VALUE, 1000,
			 PANEL_SHOW_RANGE, TRUE,
			 PANEL_VALUE,subsample_rate ,
			 NULL);

    t_panel=(Panel_item *)xv_create(panel,
				    PANEL_BUTTON,
				    /*	XV_X, 70, XV_Y, 320,	*/
				    PANEL_LABEL_STRING, "activate",
				    PANEL_NOTIFY_PROC, ctl_activate, NULL);
    rect=(Rect *)xv_get((Xv_opaque)t_panel,XV_RECT);
    t_panel=(Panel_item *) xv_create(panel, PANEL_BUTTON,
				     XV_X,rect_right(rect)+20,
				     XV_Y,rect->r_top,
				     PANEL_LABEL_STRING, "reset",
				     PANEL_NOTIFY_PROC, ctl_reset, NULL);
    rect=(Rect *)xv_get((Xv_opaque)t_panel,XV_RECT);
    t_panel=(Panel_item *) xv_create(panel, PANEL_BUTTON,
				     XV_X,rect_right(rect)+20,
				     XV_Y,rect->r_top,
				     PANEL_LABEL_STRING, "cancel",
				     PANEL_NOTIFY_PROC, ctl_cancel, NULL);
    window_fit(panel);
    window_fit(ctl_frame);
}

#define MODE_AUTOLOAD_PIK    0x1
#define MODE_DECIMPLOT       0x2  
#define MODE_AUTO_SCROLL     0x4  
#define MODE_SHOW_BORDERS    0x8  
#define MODE_UW_PICK_STYLE   0x10 
#define MODE_AUTO_REFRESH    0x20 
#define MODE_AUTO_REGROUP    0x40 
#define MODE_AUTO_GROUP_COMP 0x80 
#define MODE_AUTO_DEMEAN     0x100
#define MODE_SAVE_PICKS      0x200

static int s_choice_value ()
{
    if (Mode_abs_vscale==1) {
	Zoom_Mode_sameVertScale=0;
	return 2;
    } else if (Zoom_Mode_sameVertScale==1) {
	return 1;
    }
    return 0;
}

static void setup_panel_vals()
{
    unsigned int mode=0;
    xv_set(slider, PANEL_VALUE, NumTracks, NULL);
    xv_set(zslider, PANEL_VALUE, NumZTracks, NULL);
    mode=0;
    if(Mode_autoLoadPicks) mode|=MODE_AUTOLOAD_PIK;
    if(Mode_decimPlot)mode|=MODE_DECIMPLOT;
    if(Mode_autoScroll) mode|=MODE_AUTO_SCROLL;
    if(!Mode_noBorder) mode|=MODE_SHOW_BORDERS;
    if(Mode_UWPickStyle) mode|=MODE_UW_PICK_STYLE;
    if(Mode_Auto_redraw) mode|=MODE_AUTO_REFRESH;
    if(Mode_autoRegroup) mode|=MODE_AUTO_REGROUP;
    if(Mode_autoGroupComp) mode|=MODE_AUTO_GROUP_COMP;
    if(auto_demean) mode|=MODE_AUTO_DEMEAN;
    if (saveWaifPicks) mode|=MODE_SAVE_PICKS;
    xv_set(check_box, PANEL_VALUE, mode, NULL);
    xv_set(scale_choice,PANEL_VALUE,s_choice_value(),NULL);
    xv_set(align_choice,PANEL_VALUE,Mode_align,NULL);
}

void show_control_panel()
{
    if(!ctl_frame) {
	int x, y;
	x= (int)xv_get(tracesFrame, XV_X);
	y= (int)xv_get(tracesFrame, XV_Y);
	InitCtrlPanel(tracesFrame,x+FrmWidth-400,y+30);
    }
    /* save old values */
    setup_panel_vals();
    xv_set(ctl_frame, XV_SHOW, TRUE, NULL);
}

void tag_changeScale ()
{
    int i;

    for(i=0; i<=LastTrack; i++) {
	traces[i]->axis_needScale=1;
	traces[i]->zaxis_needScale=1;
    }
}

static void ctl_activate()
{
    int val, oldalign, oldborder, oldstyle, olddplot;
    int sca_val;
    int Trkchanged=0;

    /* num tracks */
    val=(int)xv_get(slider, PANEL_VALUE);
    if (val!=NumTracks) {
	Mode_noBorder= (xv_get(check_box,PANEL_VALUE)
			&MODE_SHOW_BORDERS)?0:1;	/* inverted! */
	ChangeNumTracks(val);
	Trkchanged=1;
    }
    /* num z tracks */
    val=(int)xv_get(zslider, PANEL_VALUE);
    if (val!=NumZTracks) {
	ChangeNumZTracks(val);
    }
    /* deal with changing the indexing of the picks */
    val=(int)xv_get(subslider, PANEL_VALUE);
    if (val!=subsample_rate) {
	change_subsampling(val);
    }

    /* the various modes */

    val=(int)xv_get(check_box,PANEL_VALUE);

    if (!Trkchanged) {	/* taken care of if numTrk changes */
	oldborder= Mode_noBorder;
	Mode_noBorder= (val&MODE_SHOW_BORDERS)?0:1;	/* inverted! */
	if (Mode_noBorder!=oldborder) {
	    /* this is a hack -- tracks are created again */
	    ChangeNumTracks(NumTracks); 
	}
    }

    oldstyle= Mode_UWPickStyle;
    Mode_UWPickStyle= (val&MODE_UW_PICK_STYLE)?1:0;
    if (Mode_UWPickStyle!=oldstyle) {
	if (Mode_UWPickStyle) {
	    setmode_UWPickStyle_on();
	}else {
	    setmode_UWPickStyle_off();
	}
    }

    olddplot= Mode_decimPlot;
    Mode_decimPlot= (val&MODE_DECIMPLOT)?1:0;
    if(Mode_decimPlot!=olddplot) {
	RedrawScreen();
    }

    olddplot= Mode_autoGroupComp;
    Mode_autoGroupComp= (val&MODE_AUTO_GROUP_COMP)?1:0;
    if( Mode_autoGroupComp!=olddplot && Mode_triplet == 0) {
	if (Mode_autoGroupComp==1) {
	    Group_ZNE();
	} else {
	    regroup_3comp();
	}
    }

    Mode_Auto_redraw= (val&MODE_AUTO_REFRESH)?1:0;
    auto_demean= (val&MODE_AUTO_DEMEAN)?1:0;
    Mode_autoRegroup= (val&MODE_AUTO_REGROUP)?1:0;
    saveWaifPicks = (val & MODE_SAVE_PICKS) ? 1 : 0;

    sca_val=(int)xv_get(scale_choice,PANEL_VALUE);
    if (sca_val!=old_scale_cho) {
	switch (sca_val) {
	case 0:  /* normal mode */
	    Mode_abs_vscale=0;
	    Zoom_Mode_sameVertScale=0;
	    break;
	case 1:  /* zoom abs mode */
	    Mode_abs_vscale=0;
	    Zoom_Mode_sameVertScale=1;
	    break;
	case 2:  /* all abs mode */
	    Mode_abs_vscale=1;
	    Zoom_Mode_sameVertScale=0;
	    newAbsVertScale();
	    break;
	}
	/* if Zoom - zoom contents changed */
	/* also need to redraw main ?      */
	tag_changeScale ();
	RedrawScreen();
	if (ZoomWindowMapped) {
	    RedrawZoomWindow("crt_ch_traces"); /* just in case */
	    if(Zoom_Mode_sameVertScale) newVertScale();
	}
	old_scale_cho=sca_val;
    }

    oldalign=Mode_align;
    Mode_align=(int)xv_get(align_choice,PANEL_VALUE);

    if (Mode_align!=oldalign) {
	int i;
	extern Canvas tscl_canvas;
      
	xv_set(tscaleButton, PANEL_INACTIVE,
	       (Mode_align?FALSE:TRUE), NULL);
	xv_set(tscl_canvas,XV_SHOW,(Mode_align?TRUE:FALSE),NULL);
      
	switch (Mode_align) {
	case 0:  /* no align mode */
	    Mode_ZDisplayTScale=0; 
	    ZTimeScale_disp_undisp (); 
	    break;
	case 1:  /* time align mode */
	    Mode_ZDisplayTScale=1;
	    ZTimeScale_disp_undisp ();
	    break;
	case 2:  /* time scaled mode */
	    Mode_ZDisplayTScale=1;
	    ZTimeScale_disp_undisp ();
	    break;
	}

	FullScale(); 
	ChangeNumZTracks(NumZTracks);	/* yet another hack */
    }


    /* the following requires no immediate action */
    Mode_autoLoadPicks= (val&MODE_AUTOLOAD_PIK)?1:0;
    Mode_autoScroll= (val&MODE_AUTO_SCROLL)?1:0;
    xv_set(ctl_frame, XV_SHOW, FALSE, NULL);	/* done */
}

static void ctl_reset(item, event)
     Panel_item item; Event *event;
{
    setup_panel_vals();
    xv_set(item, PANEL_NOTIFY_STATUS, XV_ERROR, NULL);	/* prevent dismiss */
}

static void ctl_cancel()
{
    xv_set(ctl_frame, XV_SHOW, FALSE, NULL);
}

void save_config_var(FILE *fp)
{

    if(Mode_decimPlot) {
	fprintf(fp,"set decim on\n");
    } else {
	fprintf(fp,"set decim off\n");
    }

    if(!Mode_noBorder) {
	fprintf(fp,"set bor on\n");
    } else {
	fprintf(fp,"set bor off\n");
    }

    if(Mode_UWPickStyle) {
	fprintf(fp,"set uwpick on\n");
    } else {
	fprintf(fp,"set uwpick off\n");
    }

    if(Mode_autoLoadPicks) {
	fprintf (fp,"set pickload on\n");
    } else {
	fprintf(fp,"set pickload off\n");
    }

    if(Mode_autoScroll) {
	fprintf (fp,"set ascroll on\n");
    } else {
	fprintf(fp,"set ascroll off\n");
    }

    if(Mode_Auto_redraw) {
	fprintf (fp,"set aredraw on\n");
    } else {
	fprintf(fp,"set aredraw off\n");
    }

    if (Mode_abs_vscale==1) {
	fprintf (fp,"set vscale all\n");
    } else if (Zoom_Mode_sameVertScale==1) {
	fprintf (fp,"set vscale zoom\n");
    } else {
	fprintf (fp,"set vscale normal\n");
    }

    if (Mode_autoRegroup) {
	fprintf (fp,"set regroup on\n");
    } else {
	fprintf (fp,"set regroup off\n");
    }

    if (Mode_autoGroupComp) {
	fprintf (fp,"set groupcomp on\n");
    } else {
	fprintf (fp,"set groupcomp off\n");
    }

    if (auto_demean) {
	fprintf (fp,"set autodemean on\n");
    } else {
	fprintf (fp,"set autodemean off\n");
    }
    
    if (saveWaifPicks) {
	fprintf (fp,"set saveWaifPicks on\n");
    } else {
	fprintf (fp,"set saveWaifPicks off\n");
    }
    
    fprintf(fp,"set talign %d\n",Mode_align);

    fprintf(fp,"set numtr %d\n",NumTracks);
    fprintf(fp,"set numztr %d\n",NumZTracks);  
    fprintf(fp,"set subsample %d\n",subsample_rate);
    fprintf(fp,"set lofreq %f\n",fq_fl_value);
    fprintf(fp,"set hifreq %f\n",fq_fh_value);
    fprintf(fp,"set form %s\n",formatName(defaultFormat));
}
