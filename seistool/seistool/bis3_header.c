#ifndef lint
static char id[] = "$Id: bis3_header.c,v 1.2 2013/02/28 21:25:00 lombard Exp $";
#endif

/*
 * bis3_header.c--
 *     XDR routines for reading/writing of BIS3_HEADER; taken from
 *     the BIS3 package.
 *
 * Copyright (c) 1992-3 Andrew K. Yu, Lawrence Berkeley Laboratory
 *      and University of California, Berkeley.
 * All rights reserved.
 */
#include <rpc/types.h>
#include <rpc/xdr.h>
#include <stdio.h>
#include "bis_proto.h"

#ifdef DEBUG
#define CHECKPOINT(str,okay) \
    fprintf(stderr,"checkpt %s: ok= %s \n",str,(okay?"TRUE":"FALSE"));
#else
#define CHECKPOINT(s,ok)
#endif

/*
 * Read/write the date separately from the rest of the header.
 * This is needed becuase we no longer use a DATE structure in the 
 * BIS3_HEADER; we have to convert to and from an INT_TIME struct. 
 */
bool_t xdr_DATE(XDR *xdrs, DATE *dp)
{
    bool_t ok;

    ok= xdr_int(xdrs, &dp->year) &&
	xdr_int(xdrs, &dp->day) &&
	xdr_int(xdrs, &dp->hour) &&
	xdr_int(xdrs, &dp->minute) &&
	xdr_int(xdrs, &dp->second) &&
	xdr_int(xdrs, &dp->ticks);
    return ok;
}

bool_t xdr_COMPLEX(XDR *xdrs, COMPLEX *cp)
{
    bool_t ok;

    ok= xdr_float(xdrs, &cp->real) &&
	xdr_float(xdrs, &cp->imag);
    return ok;
}

bool_t xdr_MAGNITUDE(XDR *xdrs, MAGNITUDE *mp)
{
    bool_t ok;
    char *cp;

    cp= mp->type;
    ok= xdr_float(xdrs,&mp->value) &&
	xdr_string(xdrs,&cp,MAGTYPESIZE);
    return ok;
}

bool_t xdr_FAULT_PLANE(XDR *xdrs, FAULT_PLANE *fp)
{
    bool_t ok;

    ok= xdr_int(xdrs, &fp->strike) &&
	xdr_int(xdrs, &fp->dip) &&
	xdr_int(xdrs, &fp->rake) &&
	xdr_float(xdrs, &fp->m0);
    return ok;
}

bool_t xdr_UVAL(XDR *xdrs, int format, UVAL *up)
{
    bool_t ok;

    switch(format) {
    case I16_FORMAT:
	ok=xdr_short(xdrs, &up->sflag);
	break;
    case I32_FORMAT:
	ok=xdr_int(xdrs, &up->iflag);
	break;
    case R32_FORMAT:
	ok=xdr_float(xdrs, &up->fflag);
	break;
    case R64_FORMAT:
	ok=xdr_double(xdrs, &up->dflag);
	break;
    default:
	ok=FALSE;
	break;
    }
    return ok;
}

bool_t xdr_MAGIC (XDR *xdrs, BIS3_HEADER *bp)
{
    bool_t ok;
    
    /*	Magic number and version number.				*/
    ok=  xdr_int(xdrs, &bp->magic);
    ok&= xdr_int(xdrs, &bp->version);

    CHECKPOINT("magic #",ok);
    if (!ok) return FALSE;	/* check-point */

    return ok;
}


/* Read/write the part of header before the date */
bool_t xdr_BIS3_HEADER_before (XDR *xdrs, BIS3_HEADER *bp)
{
    bool_t ok;
    int i;
    char *cp;
    
    /*	Station and channel (or stream) identification.			*/
    cp= bp->station;
    ok&= xdr_string(xdrs, &cp, STATSIZE);
    cp= bp->network;
    ok&= xdr_string(xdrs, &cp, NETSIZE);
    cp= bp->channel;
    ok&= xdr_string(xdrs, &cp, CHSIZE);
    cp= bp->location;
    ok&= xdr_string(xdrs, &cp, LOCSIZE);
    ok&= xdr_float(xdrs, &bp->dip);
    ok&= xdr_float(xdrs, &bp->azimuth);
    ok&= xdr_float(xdrs, &bp->latitude);
    ok&= xdr_float(xdrs, &bp->longitude);
    ok&= xdr_float(xdrs, &bp->elevation);
    ok&= xdr_float(xdrs, &bp->depth);

    CHECKPOINT("stat & chan",ok);
    if (!ok) return FALSE;	/* check-point */

    return ok;
}

/* Read/write the part of header after the start date, before the event date */
bool_t xdr_BIS3_HEADER_middle (XDR *xdrs, BIS3_HEADER *bp)
{
    bool_t ok;
    int i;

    /*	Time information.					*/
    ok= xdr_int(xdrs, &bp->time_correction);
    ok&= xdr_int(xdrs, &bp->format);
    ok&= xdr_int(xdrs, &bp->n_values);
    ok&= xdr_int(xdrs, &bp->n_flagged);
    ok&= xdr_float(xdrs, &bp->sample_rate);

    CHECKPOINT("time",ok);
    if (!ok) return FALSE;	/* check-point */

    /*	Data type information.						*/
    ok=  xdr_UVAL(xdrs, bp->format, &bp->flag);
    ok&= xdr_UVAL(xdrs, bp->format, &bp->min_value);
    ok&= xdr_UVAL(xdrs, bp->format, &bp->max_value);

    CHECKPOINT("data type",ok);
    if (!ok) return FALSE;	/* check-point */

    /*  Poles and Zeros							*/
    ok=  xdr_int(xdrs, &bp->response);
    ok&= xdr_float(xdrs, &bp->digital_sens);
    ok&= xdr_float(xdrs, &bp->gain_factor);
    ok&= xdr_int(xdrs, &bp->n_poles);
    ok&= xdr_int(xdrs, &bp->n_zeros);
    for(i=0; i<NUMPOLES; i++) {
	ok&= xdr_COMPLEX(xdrs, &bp->poles_zeros[i]);
    }

    CHECKPOINT("poles & zeros", ok);
    if (!ok) return FALSE;	/* check-point */

    /*	Event information 1.						*/
    ok=  xdr_float(xdrs, &bp->event_latitude);
    ok&= xdr_float(xdrs, &bp->event_longitude);
    ok&= xdr_float(xdrs, &bp->event_depth);

    CHECKPOINT("event location", ok);
    if (!ok) return FALSE;	/* check-point */

    return ok;
}

/* Read/write the part of header after the event date */
bool_t xdr_BIS3_HEADER_after (XDR *xdrs, BIS3_HEADER *bp)
{
    bool_t ok;
    int i;
    char *cp;

    cp= bp->event_agency;
    ok= xdr_string(xdrs, &cp, EVAGYSIZE);
    ok&= xdr_float(xdrs, &bp->event_delta);
    ok&= xdr_float(xdrs, &bp->event_azimuth);
    for(i=0; i < MAGSIZE; i++) {
	ok&= xdr_MAGNITUDE(xdrs, &bp->magnitude[i]);
    }
    ok&=  xdr_int(xdrs, &bp->flinn_engdahl_region);
    cp= bp->source_location_description;
    ok&= xdr_string(xdrs, &cp, SRCDSIZE);

    CHECKPOINT("event_1",ok);
    if (!ok) return FALSE;	/* check-point */
    
    /*	Moment tensor information 1.					*/
    for(i=0; i < NUMOMENT; i++) {
	ok&=  xdr_float(xdrs, &bp->moment[i]);
    }
    ok&= xdr_FAULT_PLANE(xdrs, &bp->fault_plane);
    ok&= xdr_int(xdrs, &bp->source_duration);
    ok&= xdr_int(xdrs, &bp->centroid_time_offset);
    cp= bp->moment_agency;
    ok&= xdr_string(xdrs, &cp, NMOMAGY);

    CHECKPOINT("moment_1",ok);
    if (!ok) return FALSE;	/* check-point */

    CHECKPOINT("all", ok);
    return ok;
}
    
