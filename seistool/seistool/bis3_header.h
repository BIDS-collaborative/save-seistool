/*	$Id: bis3_header.h,v 1.2 2013/02/28 21:25:00 lombard Exp $	*/

#ifndef BIS3_HEADER_H
#define BIS3_HEADER_H

#include "time.h"
#include "math/complex.h"

/*********  Structure information for BIS3 header and data.	*********/

/*
 * The BIS3 header is the first revision after the original BIS version 1.
 * There is no BIS version 2; the changes to seistool include a number of
 * file format changes. The decision was made to bring all these formats 
 * to a common version number - hence version 3.
 */


/*  
 *   Flagged (missing) values 
 */

#define	    I2FLAG		0x7FF8
#define	    I4FLAG		0x7FFFFFF8
#define	    F4FLAG		(1.7e+38)
#define	    F8FLAG		(1.7e+38)

/*
    A BIS3 frame consists of:
	a.  Header
	b.  0 or more points of data.
    A BIS3 file consists of:
	1 or more BIS3 frames.

    Definitions for the data storage formats.
	I16:	16 bit 2-s complement integer, stored in 2 bytes.
	I32:	32-bit 2-s complement integer, stored in 4 bytes.
	R32:	32-bit IEEE floating point, stored in 4 bytes.
	R64:	64-bit IEEE floating point, stored in 8 bytes.
*/

#define	I16_FORMAT	1
#define	I32_FORMAT	2
#define	R32_FORMAT	3
#define	R64_FORMAT	4

/************************************************************************/

/* Date structure for BIS files */
#define MAGTYPESIZE	4
typedef struct _magnitude {
    float   value;		/*  magnitude value.			*/
    char    type[MAGTYPESIZE+1];  /*  magnitude type.			*/
} MAGNITUDE;

/*  Fault plane solution.						*/
typedef struct _fault_plane {
    int strike;			/*  fault strike in degrees.		*/
    int dip;			/*  fault dip in degrees.		*/
    int rake;			/*  fault rake in degrees.		*/
    float   m0;			/*  fault moment in dyne-cm.		*/
} FAULT_PLANE;
    
/*  Union for flag value:  depends on type of data.			*/
typedef union _uval {
    short	sflag;		/*  short integer value		        */
    int		iflag;		/*  integer value			*/
    float	fflag;		/*  float value				*/
    double	dflag;		/*  double value			*/
} UVAL;

/************************************************************************/

/*
 *   BIS3 Header Structure.
 *   Changes from BIS version 1:
 *         Addition of network name
 *         Reduction of station, channel and location names by 1 byte
 *         Version number incremented to 2. 
 */
#define	    BMAGIC		0x42495348    /*   value of 'BISH'   */
#define	    BVERSION3		3

typedef	struct bis3head {

    /********************************************************************
     ** Magic number and version number.			       **
     ********************************************************************/
    int	    magic;		/*  magic number for this type of file.	*/
    int	    version;		/*  version number of this header.	*/

    /********************************************************************
     ** Station and channel (or stream) identification.                **
     ********************************************************************/
#define	STATSIZE	8
#define NETSIZE		4
#define LOCSIZE		4
#define CHSIZE		4

    char    station[STATSIZE];  /* station name.		*/
    char    network[NETSIZE];   /* network name.                */ 
    char    channel[CHSIZE];    /* SEED channel name.		*/
    char    location[LOCSIZE];  /* station location.		*/

    /*	Channel information and Instrument Response:
     *	Can be used with 1 to 3 point-multiplexed channels.
     *	SEED convention:					
     *	    dip:	-90 = up, 90 = down, 0 = horizontal	
     *	    azimuth:	0 = North, 90 = east ...		
     */
    float   dip;		     /* station dip: -90 to 90		*/
    float   azimuth;		     /* station azimuth (0-360)		*/

    /*	Station coordinates:
     *	SEED standards:	
     *	    latitude (degrees):	    +90 = North Pole, -90 = South Pole
     *	    longitude (degrees from Greenwich):	
     *		    negative -> West 
     *		    positive -> East	
     *	    elevation (meters):	elevation of station above sea level.
     *	    depth (meters):	positive value of instrument below
     *				station location.
     */
    float   latitude;		/* station coordinates: latitude.	*/
    float   longitude;		/* station coordinates: longitude.	*/  
    float   elevation;		/* station coordinates: elevation.	*/
    float   depth;		/* instrument depth below surface.	*/
				/* positive -> below station elevation.	*/


    /********************************************************************
     ** Data information.					       **
     ********************************************************************/
    STI_TIME start_it;		/* starting time of data stream.	*/
    int	    time_correction;	/* time correction in ticks.		*/
    int	    format;		/* data storage format number.		*/
    int	    n_values;		/* number of data values.		*/
    int	    n_flagged;		/* number of flagged data pts.		*/
				/* -1 -> unknown.			*/
    float   sample_rate;	/* data sampling rate, in samples/sec.	*/


    /********************************************************************
     ** Data_type info						       **
     ********************************************************************/
    UVAL    flag;		/* flag value.				*/
    UVAL    min_value;		/* smallest data value in dataset.	*/
    UVAL    max_value;		/* largest data value in dataset.	*/
				/* 0 -> no flag value.			*/

    /********************************************************************
     ** Poles & zeros						       **
     ********************************************************************/
#define NUMPOLES    60
    int	    response;		/* type of instrument response		*/
				/* -1=unknown 0= disp  1= vely  2= accl */
                                /* -2= not loaded                       */
    float digital_sens;		/* digital sensitivity			*/
    float gain_factor;		/* gain factor				*/
    int	    n_poles;		/* number of poles.			*/
    int	    n_zeros;		/* number of zeros.			*/
    COMPLEX poles_zeros[NUMPOLES]; /*Instrument poles followed by zeros.*/


    /********************************************************************
     ** Event_1 information.					       **
     ********************************************************************/
#define MAGSIZE	    4
#define SRCDSIZE    40
#define EVAGYSIZE   4
    float   event_latitude;	/* event coordinates:	latitude.	*/
    float   event_longitude;	/* event coordinates:	longitude.	*/  
    float   event_depth;	/* event depth below surface (km).	*/
    STI_TIME event_origin_it;	/* event origin time.			*/
    char    event_agency[EVAGYSIZE+1];  /* agency supplying event info.	*/
    float   event_delta;	/* minor arc distance in degrees.	*/
    float   event_azimuth;	/* minor arc azimute in degrees.	*/
    MAGNITUDE	magnitude[MAGSIZE];
    int	    flinn_engdahl_region;
    char    source_location_description[SRCDSIZE+1];

    /********************************************************************
     ** Moment tensor info_1					       **
     ********************************************************************/
#define NUMOMENT    6
#define NMOMAGY	    40
#define NUMARES	    30
    float   moment[NUMOMENT];	/* moment tensor (cartesian coordinates)*/
    FAULT_PLANE	fault_plane;	/* fault plane solution.		*/
    int	    source_duration;	/* duration of rupture in ticks.	*/
    int	    centroid_time_offset;   /*	(in ticks)			*/
				/* time offset for event centroid	*/
				/* relative to event_origin_time.	*/
    char    moment_agency[NMOMAGY+1];/*agency suppling moment tensor info.*/

} BIS3_HEADER;


#endif /* BIS3_HEADER_H */
