/*
 * seistool function prototypes
 */

#ifndef PROTO_H
#define PROTO_H

#include <stdarg.h>
#include <rpc/types.h>
#include <rpc/xdr.h>

#include "amplitude.h"
#include "axis.h"
#include "bis_header.h"
#include "bis3_header.h"
#include "btree.h"
#include "eqevtmgr.h"
#include "pick.h"
#include "reg_select.h"
#include "regexpr.h"
#include "time.h"
#include "trace.h"
#include "types.h"
#include "wave.h"
#include "wfmdb.h"


/* Printf.c */
void Printf(char *s, ... );

/* action.c */
void handle_load(FileFormat format);
void handle_newload(FileFormat format);
void handle_load_event(FileFormat format);
void load_pickfile();
void set_execscript();

/* align.c */
STI_TIME getEarliestSample(int includeWaifs);
void UnifyTimeScale();
void Fill_glob_Times(double earl_sec,double late_sec);
void timescale_noalign();

/* amplitude.c */
int CheckResponse(int itrc);
Amp *AddAmps(Amp *pl);
Amp *FindAmpType(Amp *pl, int which);
void CalcAmpli();
void ConvRespTrace(int which, int itrc);
void ConvertResponses(int which);
void filterTrc(int which, int decon, int itrc, int fl, float fh);
void ApplyFilter(int which, int decon, int all, float fl, float fh);
void cvtToAFname(char *afname, char *fname, FileFormat format);
void WriteAmps(char *fname);
void ReadAmps(char *fname);

/* axis.c */
void InitAxis(BIS3_HEADER *bh, Axis *ax);
void ScaleYAxis( Axis *ax, int height);
void UniformScaleYAxis(Axis *ax, int height);
void ScaleTAxis(Axis *ax, float sps, int width);
int fixIxBounds(int *ix1p, int *ix2p, int nval);

/* bcat.c */
void printhead(BIS3_HEADER *bp, int level);

/* bis_wave.c */
int strib(char *string);
int LoadBISWave(char *fname, Wave ***waves_ptr);
int PreviewBIS(char *fname,  wfmTuple **wfmtup, void **fsi, EvtFile *evt);

/* btree.c */
Btree *empty_btree(int(*)());
void btree_insert(Btree *, void *);
void btree_print(Btree *, void(*)());
BtreeNode **btree_linearize(Btree *bt);
void btree_destroy(Btree *bt);

/* cli.c */
void cli_eval_loop();

/* clientdata.c */
void cdata_insert(Trace *trc, int key, int datum);
int cdata_find(Trace *trc, int key);
void cdata_remove(Trace *trc, int key);

/* fsel_dir.c */
void fsel_cwd(char *cwd_path);
void fsel_chdir(char *prefix, char *path);
void substTilde(char *path);

/* group.c */
void match_and_regroup();
void regroup_3comp();
void Group_ZNE();

/* info.c */
int textsw_printf(char *str);

/* instr.c */
int reset_Resp(void);
void GetResponses(char* fname);

/* lineaxes.c */
void scale_line_axis (double min_val,double max_val,double *inter,
		      double *out_min,double *in_min);
/* mark.c */
int iround(double x);
int timeToIndex(Trace *trc, STI_TIME time);
STI_TIME indexToTime(Trace *trc, int index, int subrate);
int timeToCoord(Axis *axis, Trace *trc, double off_set);
int coordToIndex(Axis *axis, int x, int subrate);
int indexToCoord(Axis *axis, int idx, int subrate);

/* mseed_wave.c */
int LoadSDRWave(char *fname, Wave ***waves_ptr);
int PreviewSDR(char *fname, wfmTuple **wfmtup, void **fsi, EvtFile *evt);
int LoadSDRWfm(wfmTuple *wfm, Wave ***waveptr);

/* plotwave.c */
void ShowPage(char *title);
void EndPlot(char *title);
void PS_PlotWave(int itrc);
void InitPlot(char *fname, int ntrcs, int mode);

/* reg_select.c */
Reg_select *Create_SRegion ();
void RemoveSRegion(Reg_select *dest,Trace *this_trace);
Reg_select *Find_Nearest_SRegion(Trace *this_trace, int x);
void Insert_SRegion (Reg_select *reg,Trace *this_trace);
Trace *SReg_in_trace(Reg_select *find_reg,Trace *trc1, Trace *trc2);

/* regexpr.c */
void RE_compile(char *expr, char *exprbuf);
int RE_match(char *string, char *exprbuf);

/* rotate_file.c */
void cvtToRFname(char *ffname, char *fname);
void WriteRotation(char *fname);

/* sac_wave.c */
int LoadSACWave(char *fname, Wave ***waves_ptr);
void WriteSAC(char *fname, int itrc);

/* save.c */
void DumpWave(XDR *xdrs, int itrc);
void DumpBIS();
void DumpSAC();
void DumpSEGY();

/* script.c */
void exec_script(char *fname, int itrc);

/* segy_wave.c */
int LoadSEGYWave(char *fname, Wave ***waves_ptr);
int PreviewSEGY(char *fname, wfmTuple **wfmtup, void **fsi, EvtFile *evt);
int LoadSEGYWfm(wfmTuple  *wfm, Wave ***waveptr);
int SaveSEGYWave(char *fname, Wave **waves, int numWaves);

/* select.c */
void CleanSeleStrings();
void ParseSelectFile(char *fname);
int matchSeleSNCL(BIS3_HEADER *bh);
void handle_select_trace_script();
void handle_select_trace_enter();
void handle_select_trace_select();
void handle_select_trace_selerest();
void handle_select_trace_keep();
void handle_select_trace_keeprest();
void handle_select_trace_filter();
void handle_select_trace_rearrange();

/* sort.c */
int RestoreLoadOrder();
void sort_traces();

/* special.c */
void LoadWindows(char *fname);
void WriteOutPickFile();
void restore_orig_order();
void dump_all_picks();
void check_NCD();
void save_config();

/* trc_wave.c */
int LoadTRCWave(char *fname, Wave ***waves_ptr);
int Demand_TRC_setup();
void parseTRCiniFile(char *fname);
int PreviewTRC(char *fname, wfmTuple **wfmtup, void **fsi, EvtFile *evt);

/* triplet.c */
Triplet *makeTriplet();
void destroyTriplet(Trace *trc);
int trc_cmp(Trace *trc1, Trace *trc2);
int associate_triplet(Triplet *trip, Trace *trc);
void reorg_trcs(int num_trips, Triplet **trips);
void Trip_ZoomContentChanged(int ref_itrc);
void StartTripletMode();
void EndTripletMode();
void checkTripBound(Trace *trc);
void getTripBound(Trace *trc, int *pS_ix, int *pE_ix);
void reorg_trcs(int num_trips, Triplet **trips);

/* util.c */
void *Malloc(int size);
void *Realloc(void *p, int size);

/* ztrackmgr.c */
void ztrk_scroll_up();
void ztrk_scroll_down();
void ztrk_scroll_pgup();
void ztrk_scroll_pgdown();
void ztrk_scroll_right();
void ztrk_scroll_halfright();
void ztrk_scroll_left();
void ztrk_scroll_halfleft();
void AutoScroll(int direction);
void ztrk_stretch();
void ztrk_unstretch();
void CompressZTrk(int iztrk);
void ztrk_RaiseDC(int itrc);
void ztrk_LowerDC(int itrc);
void ztrk_CenterDC(int itrc);
void ztrk_ZeroDC(int itrc);
void ztrk_compress();
void ExpandZTrk(int iztrk);
void ztrk_expand();
void Zoom_UpdateInterval(int itrc);
void fixZtrkBounds(Trace *trc, int len);

#endif
