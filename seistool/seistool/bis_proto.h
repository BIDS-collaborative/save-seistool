/*
 * seistool function prototypes for BIS* things.
 */

#ifndef BIS_PROTO_H
#define BIS PROTO_H

#include <rpc/types.h>
#include <rpc/xdr.h>
#include "bis_header.h"

/* bis3_header.c */
bool_t xdr_DATE(XDR *xdrs, DATE *dp);
bool_t xdr_COMPLEX(XDR *xdrs, COMPLEX *cp);
bool_t xdr_MAGNITUDE(XDR *xdrs, MAGNITUDE *mp);
bool_t xdr_FAULT_PLANE(XDR *xdrs, FAULT_PLANE *fp);
bool_t xdr_UVAL(XDR *xdrs, int format, UVAL *up);
bool_t xdr_MAGIC (XDR *xdrs, BIS3_HEADER *bp);
bool_t xdr_BIS3_HEADER_before (XDR *xdrs, BIS3_HEADER *bp);
bool_t xdr_BIS3_HEADER_middle (XDR *xdrs, BIS3_HEADER *bp);
bool_t xdr_BIS3_HEADER_after (XDR *xdrs, BIS3_HEADER *bp);

/* bis_header.c */
bool_t xdr_BIS_HEADER_before (XDR *xdrs, BIS_HEADER *bp);
bool_t xdr_BIS_HEADER_middle (XDR *xdrs, BIS_HEADER *bp);
bool_t xdr_BIS_HEADER_after (XDR *xdrs, BIS_HEADER *bp);

#endif
