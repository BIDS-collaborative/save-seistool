#	$Id: Makefile,v 1.5 2013/02/28 21:25:01 lombard Exp $
#
#  Makefile for SeisTool
#
#  SeisTool	Version 2.1
#  (c) Andrew K. Yu, L.B.L. & U.C.B., 1992-3.
#
#  Copyright (c) 1992-3 Andrew K. Yu, Lawrence Berkeley Laboratory
#       and University of California, Berkeley.
#  All rights reserved.
#
#  [ Sat Jun  5 16:01:17 PDT 1993 ]
#

# select the appropriate site (note that you have to go in and change the
# path variable modnam in math/ttimes.f)
#SITE_LBL=
#SITE_FLAGS= -DSITE_LBL

SITE_BDSN=
SITE_FLAGS= -DSITE_BDSN

# fill in appropriate path:
#INSTALL_DIR= /m/exe/u/andrew/
INSTALL_DIR= /usr/contrib/bin

F77=	f77
FFLAGS= -g
CC=	cc
ACC=    cc
INCL= 	-I/usr/openwin/include -I/usr/local/include -Imath
CFLAGS= -g $(INCL) $(SITE_FLAGS) -DOWTOOLKIT_WARNING_DISABLED

LIBS= 	-L/usr/openwin/lib -L/usr/local/lib -L/usr/contrib/lib -lxview -lolgx \
    -lX11 -ltau -lqlib2 -lresplib.debug -lnsl -B static -lfftw3f -B dynamic -lm

MAIN_OBJ= eqevtmgr.o plotwave.o \
	track.o trackmgr.o event.o  \
	panel.o scroll.o

ACC_OBJ=  reg_select.o auto_rotate.o line_axes.o align.o \
          colours.o select_reg.o spctrm.o xcorr.o

WAVE_OBJ= bis_wave.o trc_wave.o mseed_wave.o segy_wave.o sac_wave.o \
      bis_header.o bis3_header.o

ZOOM_OBJ= ztrack.o ztrackmgr.o zevent.o ztrklab.o zpanel.o zscroll.o \
          trvltime.o tt_panel.o fq_panel.o rt_panel.o rotate.o rotate_file.o \
	  triplet.o location_info.o zscale.o

ANY_OBJ=  pmotion.o amplitude.o instr.o\
	sp_panel.o freqplot.o xspctrm.o stipple.o

UTIL_OBJ= setup.o draw.o notice.o font.o input.o util.o \
	pick.o mark.o time.o sort.o \
	special.o action.o mode.o scale.o \
	fsel_xv.o fsel_dir.o ctrl.o print.o \
	info.o bcat.o save.o \
	group.o dial.o \
	btree.o regexpr.o select.o \
	script.o cli.o \
	clientdata.o \
	preview.o wfmdb.o Printf.o \
        axis.o wave.o em_panel.o 

MATHLIB= math/libMath.a

OBJS= $(WAVE_OBJ) $(ZOOM_OBJ) $(MAIN_OBJ) $(ANY_OBJ) $(UTIL_OBJ)

PROG=	main.o

MYLIBS= -Lmath -lMath

ALLOBJS= $(OBJS) $(ACC_OBJ) $(PROG)

# ALL= testst testst_static
ALL= testst

all:	${ALL}

testst: 	$(ALLOBJS)  $(MATHLIB)
	$(F77) $(FFLAGS) $(ALLOBJS) $(MYLIBS) $(LIBS) -o testst

testst_static: 	$(ALLOBJS)
	$(F77) $(FFLAGS) $(ALLOBJS) -Bstatic $(MYLIBS) $(LIBS) -o testst_static

$(MATHLIB):
	cd math; make

$(ACC_OBJ) : 
	$(ACC) $(CFLAGS) -c $<

# clean up the object files
clean:
	-/bin/rm -f *.o 
	cd math; make clean

dependclean: clean
	makedepend
	-rm -f Makefile.bak
	cd math; make dependclean

veryclean: dependclean
	-rm -f $(ALL) TAGS *~
	(cd math; make veryclean)

# installations:
install:  testst
	- mv $(INSTALL_DIR)/seistool $(INSTALL_DIR)/seistool.old
	install -c testst $(INSTALL_DIR)/seistool
	- /bin/rm $(INSTALL_DIR)/seistool.old

depend:
	makedepend ${CFLAGS} *.c
	(cd math; make depend $(ARGS))

# DO NOT DELETE THIS LINE -- make depend depends on it.
