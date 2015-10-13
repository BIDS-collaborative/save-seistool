#ifndef __resplib_h
#define	__resplib_h

/************************************************************
 * Structures & API's Specifications for the resplib        *
 * library. This library gives to the user functions        *
 * to access instrument response and coordinate files.      *
 *                                                          *
 * NCEDC @2000 -- Berkeley Seismological Laboratory         *
 * Bugs/Suggestions/Questions: stephane@seismo.berkeley.edu *
 *                                                          *
 ************************************************************/

/*	$Id: resplib.h,v 1.2 2002/04/05 00:44:05 lombard Exp $ 	*/

#include	<stdio.h>

#define	RL_VERSION		 2

#define	RL_DEFAULT_LENGTH	 8
#define	RL_STRING_LENGTH	32
#define	RL_DATE_LENGTH		24
#define	RL_UNIT_LENGTH		16
#define	RL_MAX_POLES		32
#define	RL_MAX_ZEROES		32
#define	RL_MAX_COEFFS		32

#define	RL_OK			 0 
#define	RL_EOF			-1
#define	RL_VERSION_ERROR	 1
#define	RL_IO_ERROR		 2
#define	RL_FORMAT_ERROR		 3
#define	RL_NOT_FOUND		RL_EOF

#define	RL_DEFAULT_RESP_FILE	"/usr/contrib/data/bdsn/instr.db.resp"
#define	RL_DEFAULT_COORD_FILE	"/usr/contrib/data/bdsn/stat.db.coord"

typedef void *	RL_DESC;

/************************************************************************/
/*  Structures Definitions						*/
/************************************************************************/

/* Station Definition */
typedef struct {
    int	    version;			/* Version */
    int	    pad;			/* Future expansion */
    char    net[RL_DEFAULT_LENGTH];	/* Network code */
    char    sta[RL_DEFAULT_LENGTH];	/* Station code */
    char    staname[RL_STRING_LENGTH];	/* Station name */
    char    ondate[RL_DATE_LENGTH];	/* Start date */
    char    offdate[RL_DATE_LENGTH];	/* End date */
    double  lat;			/* Latitude */
    double  lon;			/* Longitude */
    double  elev;			/* Elevation */
    } station;

/* Channel definition */
typedef struct {
    int	version;			/* Version */
    int	pad;				/* Future expansion */
    char    net[RL_DEFAULT_LENGTH];	/* Network code */
    char    sta[RL_DEFAULT_LENGTH];	/* Station code */
    char    seedchan[RL_DEFAULT_LENGTH];/* SEED channel code */
    char    location[RL_DEFAULT_LENGTH];/* SEED location code */
    char    geoschan[RL_DEFAULT_LENGTH];/* Geoscope channel code */
    char    ondate[RL_DATE_LENGTH];	/* Start date */
    char    offdate[RL_DATE_LENGTH];	/* End date */
    char    units[RL_UNIT_LENGTH];	/* Input units */
    double  lat;			/* Latitude */
    double  lon;			/* Longitude */
    double  elev;			/* Elevation */
    double  depth;			/* Depth */
    double  dip;			/* Dip */
    double  azimuth;			/* Azimuth */
} channel;

/* Complex Number Definition */
typedef struct {
    double  real;			/* Real part */
    double  imag;			/* Imaginary part */
} dcomplex;

/* Poles&Zeroes Response Definition */
typedef struct {
    int	    version;			/* Version */
    int	    pad;			/* Future expansion */
    double  gain;			/* Overall gain */
    int	    nbpole;			/* Number of poles */
    int	    nbzero;			/* Number of zeroes */
    dcomplex pole[RL_MAX_POLES];	/* Complex poles */
    dcomplex zero[RL_MAX_ZEROES];	/* Complex zeroes */
} pz_resp;

/* Polynomial Response Definition */
typedef struct {
    int	    version;			/* Version */
    int	nbcoeff;			/* Number of coeff. */
					/* polynomial approx. */
    char    type[RL_DEFAULT_LENGTH];	/* Polynomial type */
    double  maxerror;			/* Maximum error of   */
					/* M = MacLaurin   */
    double  value[RL_MAX_COEFFS];	/* Coefficients */
} pn_resp;

/* Instrument Response Definition */
typedef struct {
    int	    version;			/* Version */
    int	    pad;			/* Future expansion */
    channel resp_chan;			/* Channel information */
    char    resp_type[RL_DEFAULT_LENGTH];/* Response type     */
					/* PZ = Poles&Zeroes */
					/* PN = Polynomial   */
    pz_resp resp_pz;			/* Poles&Zeroes response */
    pn_resp resp_pn;			/* Polynomial response */
} response;

/************************************************************************/
/*  Station and Channel Functions Declarations				*/
/************************************************************************/

/************************************************************************/
/*  StaOpen:	Open a station file.					*/
/*									*/
/*	Input:	Filename; if NULL or empty, open default station file.	*/
/*	Return:	void ptr  ; NULL if an error occurred.	*/
/************************************************************************
RL_DESC StaOpen (char* Filename);

/************************************************************************/
/*  StaClose:	Close a station file.					*/
/*									*/
/*	Input:	Station file descriptor.				*/
/*	Return:	RL_OK on success.					*/
/*		RL_IO_ERROR on error.					*/
/************************************************************************/
int StaClose (RL_DESC StaDesc);

/************************************************************************/
/*  StaRewind:	Rewind a station file.					*/
/*									*/
/*	Input:	Station file descriptor.				*/
/*	Return:	RL_OK on success.					*/
/*		RL_IO_ERROR on error.					*/
/************************************************************************/
int StaRewind (RL_DESC StaDesc);

/************************************************************************/
/*  StaNext:	Return the next station entry.				*/
/*									*/
/*	Input:	File descriptor, Result station structure.		*/
/*	Return:	RL_OK on success.					*/
/*		RL_EOF on EOF.						*/
/*		RL_IO_ERROR on error.					*/
/*		RL_VERSION_ERROR on version error.			*/
/*		RL_FORMAT_ERROR on station entry format error.		*/
/************************************************************************/
int StaNext (RL_DESC StaDesc, station *StaInfo);

/************************************************************************/
/*  StaSearch:	Search a station file.					*/
/*									*/
/*	Input:	File descriptor, Search station structure,		*/
/*		Result station structure.				*/
/*									*/
/*	Return:	RL_OK on success.					*/
/*		RL_EOF on EOF.						*/
/*		RL_IO_ERROR on error.					*/
/*		RL_VERSION_ERROR on version error.			*/
/*		RL_FORMAT_ERROR on station entry format error.		*/
/************************************************************************/
int StaSearch (RL_DESC StaDesc, station *UserSta, station *StaInfo);

/************************************************************************/
/*  ChaOpen:	Open a channel file.					*/
/*									*/
/*	Input:	Filename; if null then default channel file is opened.	*/
/*	Return:	Channel file descriptor; NULL if an error occurred.	*/
/************************************************************************
RL_DESC ChaOpen (char* Filename);

/************************************************************************/
/*  ChaClose:	Close a channel file.					*/
/*									*/
/*	Input:	Channel file descriptor.				*/
/*	Return:	RL_OK on success.					*/
/*		RL_IO_ERROR on error.					*/
/************************************************************************/
int ChaClose (RL_DESC ChaDesc);

/************************************************************************/
/*  ChaRewind:	Rewind a channel file.					*/
/*									*/
/*	Input:	Channel file descriptor.				*/
/*	Return:	RL_OK on success.					*/
/*		RL_IO_ERROR on error.					*/
/************************************************************************/
int ChaRewind (RL_DESC ChaDesc);

/************************************************************************/
/*  ChaNext:	Return the next channel entry.				*/
/*									*/
/*	Input:	File descriptor, Result channel structure.		*/
/*	Return:	RL_OK on success.					*/
/*		RL_EOF on EOF.						*/
/*		RL_IO_ERROR on error.					*/
/*		RL_VERSION_ERROR on version error.			*/
/*		RL_FORMAT_ERROR on station entry format error.		*/
/************************************************************************/
int ChaNext (RL_DESC ChaDesc, response* ChaInfo);

/************************************************************************/
/*  ChaSearch:	Search a channel file.					*/
/*									*/
/*	Input:	File descriptor, Search channel structure,		*/
/*		Result channel structure.				*/
/*									*/
/*	Return:	RL_OK on success.					*/
/*		RL_EOF on EOF.						*/
/*		RL_IO_ERROR on error.					*/
/*		RL_VERSION_ERROR on version error.			*/
/*		RL_FORMAT_ERROR on station entry format error.		*/
/*	Remark:	Date format is yyyy.doy.hhmm;				*/
/*		If ommited, return the most recent entry for station.	*/
/************************************************************************/
int ChaSearch (RL_DESC ChaDesc, channel *UserCha, response* ChaInfo);

#endif
