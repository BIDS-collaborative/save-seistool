#	$Id: Makefile,v 1.1 2002/04/05 00:41:46 lombard Exp $
# Makefile for libresplib.a

INCDIR	= /usr/local/include
LIBDIR	= /usr/local/lib
MANDIR	= /usr/local/man
MANEXT	= 3

TARGET1	= libresplib.a
TARGET2 = libresplib.debug.a
FTEST	= test_instr
DIR	= resplib

RANLIB	= ranlib

# -Dfortran_suffix is required for fortran Compilers that use the BSD
# convention of appending a "_" suffix to the subroutine or function name

CFLAGS	= -Dfortran_suffix
FFLAGS	= -g
COPT	= -O

$(TARGET1) := CFLAGS += $(COPT)
$(TARGET2) := CFLAGS += -g

SRCS	= resplib.c
OBJS	= $(SRCS:%.c=%.o)

HDRS	= resplib.h resplib.inc

FSRCS	= test_instr.f fetch_response.f sdr_read.f
FOBJS	= $(FSRCS:%.f=%.o)
FLIBS	= -lfio -lqlib2.debug 

ALL	= $(TARGET1) $(TARGET2) $(FTEST)

all:	$(ALL)

$(TARGET1): $(TARGET1)($(OBJS))
	$(RANLIB) $@

$(TARGET2): $(TARGET2)($(OBJS))
	$(RANLIB) $@

install:	$(TARGET1) $(TARGET2) $(HDRS)
	cp -p $(TARGET1) $(LIBDIR)/$(TARGET1)
	cp -p $(TARGET2) $(LIBDIR)/$(TARGET2)
	cp -pf $(HDRS) $(INCDIR)

install_man:	$(DIR).man
	cp $(DIR).man $(MANDIR)/man$(MANEXT)/$(DIR).$(MANEXT)

tar:
	(cd ..; tar cf - $(DIR)/*.c $(DIR)/*.h $(DIR)/*.f $(DIR)/*.inc \
		$(DIR)/Makefile $(DIR)/*.man \
		$(DIR)/instr.* \
		$(DIR)/$(TARGET1)) \$(DIR)/$(TARGET2)) \
		| gzip > /tmp/$(DIR).tar.gz

$(FTEST): $(FOBJS) $(TARGET2) resplib.inc station_hdr.inc
	$(FC) -g -o $@ $(FOBJS) $(TARGET2) $(FLIBS)

clean:
	-rm *.o

veryclean:	clean
	-rm $(ALL)

depend:

