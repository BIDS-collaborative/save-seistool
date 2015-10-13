/*
 * seistool function prototypes which use xview types
 */

#ifndef XV_PROTO_H
#define XV_PROTO_H

#include <xview/xview.h>
#include <xview/canvas.h>
#include <xview/cms.h>
#include <xview/frame.h>
#include <xview/panel.h>
#include <xview/rect.h>
#include <xview/scrollbar.h>
#include <xview/window.h>

#include "dial.h"
#include "eqevtmgr.h"
#include "pick.h"
#include "select_reg.h"

#include "setup.h"
#include "trace.h"
#include "track.h"
#include "ztrack.h"


/* auto_rotate.c */
void autor_events (Menu menu,Menu_item menu_item);

/* colours.c */
void handle_colours(Window spc_win);
int update_glob_line_at(int which);
void reset_glob_line_at(void);

/* ctrl.c */
void show_control_panel();
void tag_changeScale ();
void save_config_var(FILE *fp);

/* dial.c */
Dial *newDial(int x, int y, int w, int h, int type);
void InitDial(Frame frm, Dial *dial);
void SetDialLabel(Dial *dial, char *minL, char *midL, char *maxL);
void SetDialVal(Dial *dial, float value);
void SetDialChangeFunc(Dial *dial, int id, void (*func)(float));
void refresh_dial(Dial *dial);
float getTheta(Dial *dial, int x, int y);
void disable_dial(Dial *dial);
void enable_dial(Dial *dial);

/* draw.c */
void drawBracket(Window win, GC gc, int x, int y1, int y2, int leftRight);
void drawThickBracket(Window win, GC gc, int x, int y1, int y2, int leftRight);
void drawXUpTriangle(Window theWindow, GC theGC, int x, int y, int height,
		     int halfbase);
void drawXDnTriangle(Window theWindow, GC theGC, int x, int y, int height,
		     int halfbase);
void drawYTriangle(Window theWindow, GC theGC, int x, int y);
void drawBrokenLine(Window win, GC gc, int x, int height);
void drawOval(Window theWindow, GC theGC, int x, int y, int width, int height);
void fillOval(Window theWindow, GC theGC, int x, int y, int width, int height);

/* em_panel.c */
void handle_eqevtmgr_event();

/* eqevtmgr.c */
void AddSingleFileEvent(char *fname, FileFormat format, int isnewevt);
EqEvent *ParseElfile(FILE *fp, FileFormat format);
void ReadEventList(char *elfname, FileFormat format);
int ReadEventList2 (char *elfname, FileFormat format);
EqEvent *getCurrentEvt();
EqEvent *getEventHead();
EqEvent *getEventTail();
void fixPrevNextButtons();
EqEvent *getPrevEvt();
EqEvent *getNextEvt();
void LoadEvent(EvtFile *ls);
void eqevt_parseArg(int argc, char **argv);
void goto_cur_event();
void goto_prev_event();
void goto_next_event();
void goto_specified_event(EqEvent *eq);
void print_eqEvtQueue();
void CleanCurEvent();
void Write_event_list_file();

/* event.c */
void TxtCvs_event_proc(Xv_Window window, Event *event);
void Cvs_event_proc(Xv_Window window, Event *event);

/* font.c */
void LoadFont(Frame frame);

/* fq_panel.c */
void open_freq_panel();

/* freqplot.c */
void handle_freqplot();

/* fsel_xv.c */
void SelectFile(char *title, FileFormat format, void(*func)(), int ret);

/* info.c */
void open_info_win();
void close_info_win();
void DisplayTraceInfo(int itrc);

/* input.c */
void GetString(char *label, char *str, int numch);

/* location_info.c */
void get_location_info();
void Exit_lloc();
void InitLoadLocPanel(Frame frame);

/* main.c */
void SetChangesFlag();
void ResetChangesFlag();
int CheckChangesFlag();
void quit();
void printHelp();

/* mode.c */
void setmode_autoscroll_on();
void setmode_autoscroll_off();
void setmode_ztrack();

/* notice.c */
void ReportFileNotFound(char *fname, int noPop);
void ReportError(char *template, char *fname);

/* panel.c */
void initPanel(Frame frame);
void activate_sort_event(int tf);

/* pick.c */
Pick *AddPicks(Pick *pl);
void change_subsampling(int num);
int pick_index(Pick *pick, Trace *trace, int subrate);
void FindNearRecent(int itrc, Pick **near_ptr, Pick **recent_ptr,
			       int cur_coord);
void setPickAttrb(Pick *plist, int itrc, Event *ev, int x);
void setPickAttrb2(Pick *plist, int itrc, Event *ev, char ch, int x);
void SetPickLabel(Pick *plist);
void DrawPickLine(Pick *picks, int itrc);
void DrawPicks(Pick *plist, int itrc);
void DrawZPickLine(Pick *picks, int itrc);
void DrawZPickLabel(Pick *pick, int itrc);
void DrawZPicks(Pick *plist, int iztrk);
void sortPicks(Trace *trc);
void cvtToPFname(char *pfname, char *fname, FileFormat format);
char *getfilename(char *path);
void NotifyWriting(char *fname);
void WritePicks(char *fname);
int match_sncl(BIS3_HEADER *bh, char *s, char *n, char *c, char *l);
double pick_trace_diff(BIS3_HEADER *bh, STI_TIME ptime);
Trace* newWaifTrace(char *stname, char *net, char *chan, char *loc, 
		    STI_TIME ptime_it);
void ReadPicks(char *fname);
int pick_cmp_trace(Trace *trc1, Trace *trc2);
int pick_cmp_trip(Triplet *trp1,Triplet *trp2);
void sprintDipAzString(Trace *trc, char *da_str, int use_rot);

/* pmotion.c */
void handle_particle_motion();

/* preview.c */
void PreviewOneEvent(EvtFile *ls);

/* print.c */
void show_print_panel();


/* rotate.c */
void rt_setAngle(float ang);
void rt_setflag (int val);
void rt_PlotWave(Trace *trc, Axis *axis, int width, int height, Window xwin,
		 GC gc, int toClip, int yoffset, int itrc);
void Rot_ZoomContentChanged();

/* rt_panel.c */
void set_evt_azi(float az);
void unset_evt_azi(float az);
void open_rotate_panel();
void close_rotate_panel();
void rt_setAzimuth();
void rt_setNewTheta(float theta);
void InitRTPanel(Frame frame);
void rt_activatePanel();
void rt_inactivatePanel();

/* scale.c */
void CreateTimeScale(Frame frame, int y, int height);
void DestroyTimeScale();
void RedrawTScale();
void ResizeTScale(int y, int width, int height);

/* scroll.c */
void AdjustSbar();
void SetSbarPosition();
void initScrollbar(Canvas canvas);

/* select_reg.c */
void redraw_gsrwin (Canvas what_win);
Select_Info *creating_gsr_win(int type);

/* setup.c */
void setup(Frame frame);
int createGC(Window theNewWindow, GC *theNewGC);

/* sp_panel.c */
int get_window_length(int *nsamp, float *sec);
void open_sp_panel();

/* spctrm.c */
void close_spctrm_win(void);
void DrawVertString(Window win, GC gc, int x,int y, char *str);
void handle_spectrum ();
void event_hand_spectrum(Select_Info *new_event);
void PlotWavePortion(int itrc, int ix1, int ix2, int box_x, int box_y,
		     int box_h, int box_w);

/* stipple.c */
void InitStipples();

/* track.c */
Trace *newTrace();
void cleanTrace(Trace *trc);
void initTracks(Frame frame);
void RedrawScreen();
void ResizeScreen(Xv_Window window, Event *event);
void UpdatePlotTrack(int itrc, Track *trk);
void UpdateLabTrack(Trace *trc, Track *trk);
void UpdateZTrkMark(int itrc, Trace *trc, Track *trk);
void ChangeNumTracks(int total);
void ClipTracksLeft(int x);
void ClipTracksRight(int x);
void ClipTracks(int x);
void FullScale();
void PermanentClip();
void newAbsVertScale();

/* trackmgr.c */
void InitTrackMgr();
int AssociateTrack(char *fname, FileFormat format, int mode);
int AssociateWfmTrack(wfmTuple *wfm, int mode);
void CloseAllTracks();
void trk_scroll_up();
void trk_scroll_down();
void trk_scroll_pgup();
void trk_scroll_pgdown();
void trk_scroll_var(int amount, int dir);
void ClearBound();
int HasBoundary();
void BoundTracks(int x, Event *event);
void BoundTracks_drag(int x, Event *event);
void UpdateInterval(int itrc);
void DrawInterval(int itrc, int x1, int x2, GC gc_val);
int firstSelectedTrc();
void Select_all();
void Deselect_all();
void Discard_Selected();
void Rearrange_Selected_top();

/* trvltime.c * */
void ShowPhase(int iphs);
void UnshowPhase(int iphs);
void Trvl_Rescale();
void Trvl_ZoomContentChanged();
void EnterTTMode();
void ExitTTMode();
void ChangeDelta(float diff);
void replotTrvlTime();
void TTMode_Zevent_proc(Xv_Window window, Event *event);
void tt_ChangeOT(char *otStr);
void tt_ChangeDelta(float del);
void tt_ChangeDepth(float dep);
void tt_ChangeEvtLoc(float slat, float slon);
void AllPhases();
void BasicPhases();

/* tt_panel.c */
void tt_SetStartDepth(float dep);
void tt_SetStartDelta(float del);
void tt_updateOTitem(STI_TIME it);
void tt_updateDelTxtitem(float del);
void tt_togglePicking();
void InitTTPanel(Frame frame);

/* wave.c */
FileFormat atofilefmt(char *str);
char *formatName(FileFormat format);
int LoadWave(char *fname, FileFormat format, Wave ***waves_ptr,
	     Pick ***picks_ptr, Amp ***amps_ptr);
int LoadWfm(wfmTuple *wfm, Wave **waveptr);
void demean_trace(Wave *wave, Axis *axis, int ix1, int ix2);
void init_trace_hook(Wave *wave, Axis *axis);
void CompleteSingleLoad(char *fname, FileFormat format);
void CompleteRest(char *fname, FileFormat format);
void CompleteLoad(char *fname, FileFormat format);
void PlotWave(Trace *trc, Axis *axis, int width, int height, Window xwin,
	      GC gc, int toClip, int yoffset, int itrc);
void PlotWave_decim(Trace *trc, Axis *axis, int width, int height, Window xwin,
		    GC gc, int toClip, int yoffset, int itrc);

/* wfmdb.c */
int wfm_cmp(wfmTuple *wfm1, wfmTuple *wfm2);
void handle_hashStationNames();
void PreviewEvent();
void handle_preview_event();

/* xcorr.c */
void handle_xcorr (void);
void redrawXcorr();

/* xspctrm.c */
void handle_xspectrum();

/* zevent.c */
void ZCvs_event_proc(Xv_Window window, Event *event);
void ZCvs_event_proc2(Xv_Window window, Event *event);
void setStatusString(Trace *trc, int x);

/* zpanel.c */
void InitZPanel(Frame frame);
void toggleClipButton();

/* zscale.c */
int initZTScale(Frame frame);
void RedrawBothZTScale();
void ResizeZTScale();
void ZTimeScale_disp_undisp();
void ToggleZTScale();
void CalcTimeScaleParam(STI_TIME earlTime, double totSecs, int div,
			float *tincp, STI_TIME *nearTime);
void TS_ZoomContentChanged(int itrc);
void DrawTimeScale(Window win, GC gc, int xstart, int xoff, STI_TIME tstart,
		   float tinc, int width, int height, float totSecs, int dir);

/* zscroll.c */
void AdjustZSbar(int height);
void SetZSbarPosition();
void initZScrollbar(Canvas canvas);

/* ztrack.c */
void InitZoomWindow(Frame frame);
void open_zoom_window();
void close_zoom_window();
void ScaleZTrack(Trace *trc, Trace *reftrc);
void redrawZoomCanvas(char *funcname);
void RedrawZoomWindow(char *funcname);
void ResizeZoomWindow();
void UpdatePlotZTrack(Trace *trc, int iztrk);
void CleanZStatus();
void UpdateZStatus();
void internal_ChangeNumZTracks(int total);
void ChangeNumZTracks(int total);
void ZWHdr_ShowChange();
void ZWHdr_DismissChange();
void setmode_UWPickStyle_on();
void setmode_UWPickStyle_off();
void open_tt_panel();
void close_tt_panel();
void SwitchTTZevtProc();
void RestoreZevtProc();
void newVertScale();
void printZoomFrameDim(FILE *fp);

/* ztrackmkr.c */
void ztrk_scroll_var(int amount, int dir);
void Zoom_DrawInterval(int itrc, int x1, int x2, GC gc_val);

/* ztrcklab.c */
void InitZTrkLab(Frame frame);
void DestroyZTrkLab();
void ResizeZlab(int shrink);
void UpdateZlab(int iztrk);
void CleanZLab();

#endif
