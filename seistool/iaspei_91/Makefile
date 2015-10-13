# This Makefile will help automate the installation of the binaries that
# are needed for contributed software.
# This is a demo Makefile for a single program.

# Set DESTDIR to the directory in which the executable will be installed.
# Set BINDIR to the directory in which public libraries will be installed.

OBJTAU = libtau.o libsun.o

DESTDIR=/usr/contrib/bin
LIBDIR=/usr/contrib/lib

MANDIR= /usr/contrib/man
MANEXT = 1

CFLAGS = -O
FFLAGS = -g -C
ARFLAGS= rcv

INCL = -I.

#all: 		libtau.a remodl setbrn ttimes install install_man clean
all: 		libtau.a remodl setbrn ttimes 

libtau.a:	${OBJTAU}
		ar ${ARFLAGS} libtau.a ${OBJTAU}
		ranlib libtau.a

remodl:	remodl.o emiasp91.o
		${FC} ${FFLAGS} -o remodl remodl.o emiasp91.o libtau.a

remodl_prem:	remodl.o emprem.o
		${FC} ${FFLAGS} -o remodl_prem remodl.o emprem.o libtau.a

remodl_ca:	remodl.o emiasp91.ca.o
		${FC} ${FFLAGS} -o remodl_ca remodl.o emiasp91.ca.o libtau.a

setbrn:	setbrn.o
		${FC} ${FFLAGS} -o setbrn setbrn.o libtau.a

ttimes:	ttimes.o
		${FC} ${FFLAGS} -o ttimes ttimes.o libtau.a

install: 
		install -c -s -m 755 remodl ${DESTDIR}/remodl
		install -c -s -m 755 setbrn ${DESTDIR}/setbrn
		install -c -s -m 755 ttimes ${DESTDIR}/ttimes
		install -c -s libtau.a ${LIBDIR}/libtau.a

install_man:
	install -m 644 iasp91.man ${MANDIR}/man${MANEXT}/iasp91.${MANEXT}
	install -m 644 gtt91.man ${MANDIR}/man${MANEXT}/gtt91.${MANEXT}
	install -m 644 ttimes91.man ${MANDIR}/man${MANEXT}/ttimes91.${MANEXT}
	install -m 644 zstp91.man ${MANDIR}/man${MANEXT}/zstp91.${MANEXT}

clean:
	rm -f *.o remodl setbrn ttimes

# Additional source dependencies
