/*      $Id: segy_head.h,v 1.2 2013/02/28 21:24:57 lombard Exp $        */

/******  SEG-Y header (from John Peterson, barstow@geo.lbl.gov) ******/
#ifndef SEGY_HEAD_H
#define SEGY_HEAD_H

struct reel {                    
    long   kjob,  kline, kreel;
    short  kntr,   kaux,     sr, kfldsr, knsamp, kfsamp,  kform;
    short  kmfold,  ksort,  kvsum, kswfrs, kswfre,  kmssw, kswtyp;
    short  kswtr, kmsbst, kmsest, ksttyp, kcorcd, kbgrcd, kartyp;
    short  kunits,   kpol,  kvpol;
    short  nyear,   nday,  nhour,   nmin,   nsec,  nmsec;
    char   dum[328];
};

struct trace {                    
    long   kline,  kreel, kfldfn, kfldtn,  ksptc,   kcdp, ktrace;
    short  ktrtyp,  kvsum,  khsum,  kduse;
    float  range;
    long   kgpel,  kspel,  kspdp, kgpdat, kspdat, kspwdp, kgpwdp;
    short  kelsca, kcosca;
    float  shotx,  shoty,  rcvrx,  rcvry;
    short  kunits, kwtvel, kswvel,  sptuh,  gptuh,   spst,   gpst;
    short  statc, kmslga, kmslgb, kmsfds,  fmute,  tmute, knsamp;
    short  sr, kgntyp, kinsgn,   kbgn, kcorcd, kswfrs;
    float  cdpx,   cdpy;
    long   k3dste;
    short  kalhz, kaldbo,  knthz, kntdbo,  klohz,  khihz, klodbo;
    short  khidbo,  kyear,   kday,  khour, kminit, ksecnd, ktmtyp;
#if 0
    short  ktrwgt, kgprs1, kgpbtr, kgpetr, kgpgap, klntpr, dum181;
    char   stn[6];
#endif
    short  ktrwgt, kgprs1, kgpbtr, kgpetr, kgpgap, klntpr;
    char   stn[8];
    char   dum[52];
};

#endif
