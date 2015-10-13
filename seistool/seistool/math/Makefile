#	$Id: Makefile,v 1.5 2013/02/28 21:24:55 lombard Exp $
#
# SeisTool Ver 2.1 Makefile for Math Libraries
#
#

#LIBTAU  = /m/scr2/u/andrew/libtau/libtau.a
LIBTAU  = /usr/contrib/lib/libtau.a

ARCHIVE = libMath.a
LIBS = amaxper.o coinst.o \
	amaxper_c.o coinst_c.o \
	bpcos.o ben100.o wood.o response.o dresponse.o \
	lpbptf_b.o bpbwth.o lobwth.o hibwth.o \
	asro.o ben14.o ulp.o wwlp.o wwsp.o \
	sts1.o sts2.o hglp.o \
	demean.o detrend.o ipow.o taper.o \
	ttimes.o ttimes_c.o \
	spectrum.o spectrum_c.o \
	azimuth.o azimuth_c.o \
	xcor.o xcor_c.o
#	rltr.o fft.o  fftl.o \

INCLUDE = numerical.inc
FFLAGS = -g
CFLAGS = -g

all: 	libMath.a libtau.a

libMath.a: ${LIBS} ${INCLUDE}
	rm -f libMath.a
	ar q libMath.a ${LIBS}
	ranlib libMath.a

coinst.o: coinst.f
	f77 -c -I/usr/local/include ${FFLAGS} coinst.f

spectrum.o: spectrum.f
	f77 -c -I/usr/local/include ${FFLAGS} spectrum.f

xcor.o: xcor.f
	f77 -c -I/usr/local/include ${FFLAGS} xcor.f

clean:
	/bin/rm -f $(LIBS) $(ARCHIVE) *.o

libtau.a: $(LIBTAU)
	-ln -s $(LIBTAU) .

depend:
	makedepend ${CFLAGS} *.c

dependclean: clean
	makedepend
	-rm -r Makefile.bak

veryclean: dependclean
	-rm TAGS *~ libtau.a

# DO NOT DELETE THIS LINE -- make depend depends on it.
