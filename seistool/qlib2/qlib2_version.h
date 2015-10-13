/************************************************************************/
/*  Version number for qlib2.						*/
/*									*/
/*	Douglas Neuhauser						*/
/*	Seismological Laboratory					*/
/*	University of California, Berkeley				*/
/*	doug@seismo.berkeley.edu					*/
/*									*/
/************************************************************************/

/*
 * Copyright (c) 1996-2013 The Regents of the University of California.
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for educational, research and non-profit purposes,
 * without fee, and without a written agreement is hereby granted,
 * provided that the above copyright notice, this paragraph and the
 * following three paragraphs appear in all copies.
 * 
 * Permission to incorporate this software into commercial products may
 * be obtained from the Office of Technology Licensing, 2150 Shattuck
 * Avenue, Suite 510, Berkeley, CA  94704.
 * 
 * IN NO EVENT SHALL THE UNIVERSITY OF CALIFORNIA BE LIABLE TO ANY PARTY
 * FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES,
 * INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND
 * ITS DOCUMENTATION, EVEN IF THE UNIVERSITY OF CALIFORNIA HAS BEEN
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * THE UNIVERSITY OF CALIFORNIA SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE
 * PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND THE UNIVERSITY OF
 * CALIFORNIA HAS NO OBLIGATIONS TO PROVIDE MAINTENANCE, SUPPORT,
 * UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 */

/* Explicitly set overall qlib2 version here by hand.	*/
#ifndef lint
char *qlib2_version = "@(#)qlib2 version 1.53 (2013.260)";
#endif

/************************************************************************/
/*
Modifications:
Ver	Date and Action
------------------------------------------------------------------------
1.53	2013.260    DSN
	Updated sccsid string definitions to avoid warnings.
	Updated Makefile
1.52	2013.256    DSN
	Update qtime.c to support ISO 8601 Calendar and Ordinal date-times.
	Added support in qtime.c to check expiration date of leapsecond file.
	Changed default location of leapsecond file.
1.51	2011.222    DSN
	Fix ms_unpack() to return 0 if 0 samples in record rather than error.
	Updated all source files to remove many compiler warnings.
1.50	2011.140    DSN
	Fix it.usec computation in nepoch_to_int and tepoch_to_int.
1.49	2011.046    DSN
	Fixed arg definition to swabf().
1.48	2010.109    DSN
	Added MiniSEED type 'M' records as a valid record type.
1.47	2010.092    DSN
	Fix to force qlib2_version into executable that use qlib2.
1.46	2009.189    DSN
	Use log2() from math.h for linux.
1.45
	Fix bug in pack_text() and ms_pack2_text().
	Add support for ARM complilation.
1.44	2009.002    DSN
 	Change INT32_... limit definitions to INT_... for POSIX compliance.
1.43	2008.298    (MA)
	Added support for SP and DP floating point data formats.
1.42	2008.108
	Added f_nepoch_to_tepoch and f_tepoch_to_nepoch fortran interludes.
1.41	2008.098
	Change stdint.h to limits.h for Solaris 8 compatability.
1.40	2008.098
	qtime.c:
	Augmented leapsecond table to include nominal and true epoch times.
	Added nepoch_to_tepoch and tepoch_to_nepoch conversion functions.
	Corrected problem in epoch time conversion for time times following a
	mid-year positive leapsecond.  Added code to handle negative leapseconds.
1.39	2008.084
	Add ms_pack2 function, add missing prototype declarations.
1.38	2007.163
	Fixed misc type casts, function declarations.
1.37	2007.114
	Added ifdefs for __MACH__ for MacOSX.
1.36	2006.313
	Change MALLOC_INCREMENT in ms_pack.c from 10 to 1000.
1.35	2006.007
	Changed init_leap_second to treat LEAPSECOND point to a directory
	as an error.  Allow only files or char devices (for /dev/null).
1.34	2005.180
	Changed hdr_wordorder and data_wordorder in data_hdr.h from 
	unsigned char to signed char to allow for negative error codes.
	Fixed call to valid_span(span).
1.33	2004.171
	Modify read_ms_record(), read_ms_hdr(), and decode_fixed_data_hdr() 
	to properly handle volume headers.
	Modify ms_unpack() to properly set xm1 in data_hdr for STEIM 
	compressed data.
1.32	2004.145
	Fixed "unsigned char = int" assigment from wordorder_from_time calls.
1.31	2004.098
	Changed extra field in data_hdr structure to rate_spsec to hold the
	blockette 100 samples_per_second value if a blockette 100 is found.
	Changed drm_utils to put drm-specific info in the xm1 and xm2 fields.
1.30	2004.066
	Include malloc.h for SUNOS4 only. ISTI added support for MACOSX.
1.29	2004.041
	Allow seconds in time to end with a decimal point.
1.28	2004.038
	Free malloced memory before returns in various ms_pack functions.
	Thanks to Dave Ketchum.
1.27	2004.035
	Use sample rate in blockette 100 for timespan when packing data.
	Use blockette 100 for computing data_hdr endtime.
1.26	2003.213
	Allow tabs in whitespace between date and time.
	See CHANGES file.
1.25	2003.210
	Changed herrno to qlib2_errno.  Fix bug in decode_hdr_sdr().
1.24	2003.178
	Updated dump_hdr to output data_rate as float if necessary.
	See CHANGES file.
1.23	2003.108
	Added init_qlib2() function to provide mode where qlib2 never
	calls exit().  See CHANGES file.
1.22	2002.326
	data_hdr.h, drm_utils.c, ms_utils.c, qda_utils.c, qdefines.h,
	qlib2_version.c, qtime.c, qtime.h, qutils.c, qutils.h,
	sdr_utils.c, sdr_utils.h
	Added support for Q and R as well as D data_hdr_ind values.
	Added int_to_nepoch, nepoch_to_int.  
	Added nominal and epoch time to parse_date.
1.22b	2002.268
1.21	2002.141
	pack.h, qtime.c ms_pack.c, qtime.h, pack.c
	Fix normalize_ext by allowing for input minutes, hours, and days 
	to be below normalized values.
	Added ms_pack_text and pack_text functions.
	Added parse_date_month function.
1.20	2002.002
	qtime.c,
	Fix normalize_ext by allowing for input minutes, hours, and days 
	to be below normalized values.
1.19	2001.285
	qdefines.h, qutils.c
	Added external attribute to qlib2_version to satisfy gnu ld,
	force load by reference in get_mywordorder().
1.18	2001.042
	sdr_utils.c
	Fix bug in init_sdr_hdr that caused additional space after the
	SDR to be zeroed.
1.17	2001.003
	qtime.c
	Change parse_date() and parse_interval() to allow leading and 
	trailing blanks and to allow blank(s) delimiting date and time.
1.16	2000.309
	sdr_utils.c
	Fix read_blockette() for determing b2000 length for opposite byte order.
1.15	2000.299
	qtime.c, qdefines.h (all files)
	Fix includes for linux, change version number to 1.1.x to 1.x.
	Extend copyright through 2000, change name to BSL.
1.1.14	2000.279
	qtime.c
	Fix normalize_ext() to handle input seconds field that span multiple
	days and include leap second(s).  Speed it up significantly for
	large second values.
1.1.13	2000.112
	qtime.c
	Provide optional NO_LEAPSEONDS compile definition to disable 
	leapsecond inclusion for those sites that don't use leapseconds.
1.1.12	2000.026
	ms_pack.c
	ms_pack sets x0, xn, xm1, xm2 fields in data_hdr supplied by user.
	Used to maintain compressor history across multiple calls to ms_pack.
1.1.11	2000.020
	qtime.c, sdr_utils.c
	Handle blockettes whose lengths are not a multiple of 4.
	Fix int_to_tepoch for dates earlier than 1970.
1.1.10	1999.313
	ms_utils.c, unpack.c
	Fix to always ensure buf pointing to valid buffer in read_ms_hdr().
	Do not return error from ms_upack() if hdr->num_samples == 0.
1.1.9	1999.276
	qutils.c
	Fix from Craig Scrivner to properly determine FSDH wordorder from
	the time field on a little-endian system.
1.1.8	1999.246
	ms_utils.c
	Fix from Craig Scrivner to assign *pbuf in read_ms_hdr() only if
	allocated buffer is returned to caller.
1.1.7	1999.214
	qtime.c, qtime.h, sdr.h
	Added support for true epoch time, Y2K upgrade for no 2 digit years.
	Changed reserved field in b1001 to flags, defined mshear timing flag.
1.1.6	1999.106
	sdr_utils.c
	Updated dump_hdr to dump full SEED stream name in 
	station.network.channel.location format.
1.1.5	1999.098
	Updated typedefs for SEED datatypes to have SEED_ prefix to
	avoid conflicts with other packages.
1.1.4	1998.149
	Standarized all fatal error messages with "Error:" prefix.
1.1.3	1997.311
	Fixed read_ms_hdr to properly fill in hdr with frame_count and
	extended time from blocktte 1001.
1.1.2	1997.231
	Fixed time_interval2 when sample rate == 0.
1.1.1	1997.206
	Changed calls from add_time to add_dtime to prevent integer overflow.
1.1	1997.168
	Added sample_rate_mult to data_hdr structure.
1.0	1997.132
	Added swab8() function.
1.0	1997.124    
	Added support for proposed opaque data blockette 2000.
*/
/************************************************************************/
