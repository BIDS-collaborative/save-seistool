/*	$Id: bis_header.h,v 1.1 2001/12/21 18:39:03 lombard Exp $	*/

#ifndef BIS_HEADER_H
#define BIS_HEADER_H

/*********  Structure information for BIS header and data.	*********/

/*
 * Bring in the substructures and define statements from the version 3 
 * header; comment out duplicate define statements in this file.
 */
#include "bis3_header.h"

/************************************************************************/

/*
 *   BIS Header Structure.
 */
#define	    BVERSION1		1

typedef	struct bishead {

    /********************************************************************
     ** Magic number and version number.			       **
     ********************************************************************/
    int	    magic;		/*  magic number for this type of file.	*/
    int	    version;		/*  version number of this header.	*/

    /********************************************************************
     ** Station and channel (or stream) identification.                **
     ********************************************************************/
    /* #define	STATSIZE	8 */
    /* #define LOCSIZE		4 */
    /* #define CHSIZE		4 */

    char    station[STATSIZE+1];  /* station name.			*/
    char    location[LOCSIZE+1];       /* station location.		*/
    char    channel[CHSIZE+1];    /* SEED channel name.		*/

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
    STI_TIME start_it;		/* starting date of data stream.	*/
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
    /* #define NUMPOLES    60 */
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
    /* #define MAGSIZE	    4 */
    /* #define SRCDSIZE    40 */
    /* #define EVAGYSIZE   4 */
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
    /* #define NUMOMENT    6 */
    /* #define NMOMAGY	   40 */
    /* #define NUMARES	   30 */
    float   moment[NUMOMENT];	/* moment tensor (cartesian coordinates)*/
    FAULT_PLANE	fault_plane;	/* fault plane solution.		*/
    int	    source_duration;	/* duration of rupture in ticks.	*/
    int	    centroid_time_offset;   /*	(in ticks)			*/
				/* time offset for event centroid	*/
				/* relative to event_origin_time.	*/
    char    moment_agency[NMOMAGY+1];/*agency suppling moment tensor info.*/

} BIS_HEADER;


#endif /* BIS_HEADER_H */
