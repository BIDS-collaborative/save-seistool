/************************************************************************/
/*  Structure definitions for holding and request records.		*/
/*									*/
/*	Douglas Neuhauser						*/
/*	Seismographic Station						*/
/*	University of California, Berkeley				*/
/*	doug@seismo.berkeley.edu					*/
/*									*/
/************************************************************************/

/*
 * Copyright (c) 1996 The Regents of the University of California.
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

/*	$Id: holdings.h,v 1.6 2010/06/01 21:03:13 doug Exp $ 	*/

#define	HR_STATION_LEN		5
#define	HR_NETWORK_LEN		2
#define	HR_CHANNEL_LEN		3
#define	HR_LOCATION_LEN		2
#define	HR_SPS_LEN		10
#define	HR_MODE_LEN		1
#define	HR_NUM_SEGMENTS_LEN	3
#define	HR_START_TIME_LEN	22
#define	HR_END_TIME_LEN		22
#define	HR_UNIX_START_LEN	10
#define	HR_UNIX_END_LEN		10
#define HR_FILE_KB_LEN		5
#define	HR_DCC_LEN		2
#define	HR_MEDIUM_LEN		1
#define	HR_DATA_FILE_LEN	30
#define	HR_HEADER_DIR_LEN	30
#define	HR_OPERATOR_LEN		2

typedef struct _holding_rec {
    char    station[HR_STATION_LEN+1];		/* name of station.			*/
    char    network[HR_NETWORK_LEN+1];		/* station location.			*/
    char    channel[HR_CHANNEL_LEN+1];		/* SEED channel name.			*/
    char    samples_per_sec[HR_SPS_LEN+1];	/* samples/second [10.4].		*/
    char    mode[HR_MODE_LEN+1];		/* C (continuous) or T (triggered).	*/
    char    num_segments[HR_NUM_SEGMENTS_LEN+1];/* no longer used -- set to 0.		*/
    char    start_time[HR_START_TIME_LEN+1];	/* start time: yyyy,ddd,hh:mm:ss.tttt	*/
    char    end_time[HR_END_TIME_LEN+1];	/* end time: yyyy,ddd,hh:mm:ss.tttt	*/
    char    unix_start_time[HR_UNIX_START_LEN+1];/* start time in seconds from 1/1/70.	*/
    char    unix_end_time[HR_UNIX_END_LEN+1];	/* end time in seconds from 1/1/70.	*/
    char    file_kb[HR_FILE_KB_LEN+1];		/* file length in KB.			*/
    char    dcc[HR_DCC_LEN+1];			/* Data Collection Center.		*/
    char    incoming_medium[HR_MEDIUM_LEN+1];	/* ??					*/
    char    data_filename[HR_DATA_FILE_LEN+1];	/* filename containing data records.	*/
    char    header_dir[HR_HEADER_DIR_LEN+1];	/* header directory name (only use 6).	*/
    char    operator[HR_OPERATOR_LEN+1];	/* operator's initials.			*/
} HOLDING_REC;

typedef struct _request_rec {
    char    station[HR_STATION_LEN+1];		/* name of station.			*/
    char    network[HR_NETWORK_LEN+1];		/* station network.			*/
    char    channel[HR_CHANNEL_LEN+1];		/* SEED channel name.			*/
    char    samples_per_sec[HR_SPS_LEN+1];	/* samples/second [10.4].		*/
    char    mode[HR_MODE_LEN+1];		/* C (continuous) or T (triggered).	*/
    char    num_segments[HR_NUM_SEGMENTS_LEN+1];/* no longer used -- set to 0.		*/
    char    start_time[HR_START_TIME_LEN+1];	/* start time: yyyy,ddd,hh:mm:ss.tttt	*/
    char    end_time[HR_END_TIME_LEN+1];		/* end time: yyyy,ddd,hh:mm:ss.tttt	*/
    char    unix_start_time[HR_UNIX_START_LEN+1];/* start time in seconds from 1/1/70.	*/
    char    unix_end_time[HR_UNIX_END_LEN+1];	/* end time in seconds from 1/1/70.	*/
    char    file_kb[HR_FILE_KB_LEN+1];		/* file length in KB.			*/
    char    dcc[HR_DCC_LEN+1];			/* Data Collection Center.		*/
    char    incoming_medium[HR_MEDIUM_LEN+1];	/* ??					*/
    char    data_filename[HR_DATA_FILE_LEN+1];	/* filename containing data records.	*/
    char    header_dir[HR_HEADER_DIR_LEN+1];	/* header directory name (only use 6).	*/
    char    operator[HR_OPERATOR_LEN+1];	/* operator's initials.			*/
    char    req_start_time[HR_START_TIME_LEN+1];/* request start time.			*/
    char    req_end_time[HR_END_TIME_LEN+1];	/* request end time.			*/
} REQUEST_REC;

/************************************************************************/
/*  93/06/20	Doug Neuhauser, Seismographic Station, UC Berkeley	*/
/*									*/
/*  After discussion with Tim Ahern at IRIS, we have changed the	*/
/*  location field to the network field in the holding and request	*/
/*  file.  This will hold the 2 character NETWORK_CODE associated with	*/
/*  SEED V2.3 Fixed Data Headers.					*/
/*									*/
/*  98/09/23	Doug Neuhauser, Seismological Laboratory, UC Berkeley	*/
/*									*/
/*  Add location, offset and length fields for database support.	*/
/*  Reorganzied fields							*/
/*									*/
/*  2010/06/01	Doug Neuhauser, Seismological Laboratory, UC Berkeley	*/
/*									*/
/*  Added record_type field and padding.				*/
/************************************************************************/

#define	HOLDING_SHORT_VERSION	5
typedef struct _holding_short {								    /*off len*/
    char    len;				/* structure length.			*/  /*  0  1 */
    char    version;				/* version.				*/  /*  1  1 */
    char    blkexp;				/* blksize = 2**blkexp			*/  /*  2  1 */
    char    data_type;				/* MiniSEED data type.			*/  /*  3  1 */
    char    record_type;			/* MiniSEED record type.		*/  /*  4  1 */
    char    pad[3];				/* Padding.				*/  /*  5  3 */
    char    station[HR_STATION_LEN+1];		/* SEED station name.			*/  /*  8  6 */
    char    network[HR_NETWORK_LEN+1];		/* SEED network name.			*/  /* 14  3 */
    char    channel[HR_CHANNEL_LEN+1];		/* SEED channel name.			*/  /* 17  4 */
    char    location[HR_LOCATION_LEN+1];	/* SEED location name.			*/  /* 21  3 */
    short int sample_rate;			/* sample rate (+=samp/sec, -=sec/samp)	*/  /* 24  2 */
    short int sample_rate_mult;			/* padding.				*/  /* 26  2 */
    SDR_TIME start_time;			/* start time: yyyy,ddd,hh:mm:ss.tttt	*/  /* 28 10 */
    SDR_TIME end_time;				/* end time: yyyy,ddd,hh:mm:ss.tttt	*/  /* 38 10 */
    int	    offset;				/* initial offset of timeseries in file.*/  /* 48  4 */
    int	    length;				/* length in bytes of timeseries.	*/  /* 52  4 */
} HOLDING_SHORT;

/*:: #define	HOLDING_SHORT_VERSION	4
/*:: typedef struct _holding_short {								    /*off len*/
/*::    char    len;				/* structure length.			*/  /*  0  1 */
/*::    char    version;				/* version.				*/  /*  1  1 */
/*::    char    blkexp;				/* blksize = 2**blkexp			*/  /*  2  1 */
/*::    char    data_type;				/* MiniSEED data type.			*/  /*  3  1 */
/*::    char    station[HR_STATION_LEN+1];		/* SEED station name.			*/  /*  4  6 */
/*::    char    network[HR_NETWORK_LEN+1];		/* SEED network name.			*/  /* 10  3 */
/*::    char    channel[HR_CHANNEL_LEN+1];		/* SEED channel name.			*/  /* 13  4 */
/*::    char    location[HR_LOCATION_LEN+1];	/* SEED location name.			*/  /* 17  3 */
/*::    short int sample_rate;			/* sample rate (+=samp/sec, -=sec/samp)	*/  /* 20  2 */
/*::    short int sample_rate_mult;			/* padding.				*/  /* 22  2 */
/*::    SDR_TIME start_time;			/* start time: yyyy,ddd,hh:mm:ss.tttt	*/  /* 24 10 */
/*::    SDR_TIME end_time;				/* end time: yyyy,ddd,hh:mm:ss.tttt	*/  /* 34 10 */
/*::    int	    offset;				/* initial offset of timeseries in file.*/  /* 44  4 */
/*::    int	    length;				/* length in bytes of timeseries.	*/  /* 48  4 */
/*::} HOLDING_SHORT;
::*/
