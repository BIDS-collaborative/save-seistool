/************************************************************************/
/*  Structures & API's Specifications for the resplib library.		*/
/*  This library gives to the user functions to access			*/
/*  instrument response and coordinate files.				*/
/*									*/
/*  BSL @2000 -- Berkeley Seismological Laboratory			*/
/*  Bugs/Suggestions/Questions: stephane@seismo.berkeley.edu		*/
/************************************************************************/

/************************************************************************/
/*
Modification History.
Ver	Date	    Whoe    Action
------------------------------------------------------------------------
2	2001.063    DSN	    Initial working version.
1	2000.xxx    SZ	    Never installed.
/*
/************************************************************************/

#ifndef lint
static char sccsid[] = "$Id: resplib.c,v 1.1 2001/03/04 16:42:56 doug Exp $ ";
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/param.h>

#include "resplib.h"

/* The fortran interface is currently implemented for 32-bit pointers.	*/
/* If 64-bit pointers are required, changed to "long long int".		*/
typedef	long int    fort_ptr;

#define	LINELEN	256

/************************************************************************/
/*  StaOpen:	Open a station file.					*/
/*									*/
/*	Input:	Filename; if NULL or empty, open default station file.	*/
/*	Return:	Station file descriptor; NULL if an error occurred.	*/
/************************************************************************/
RL_DESC StaOpen (char* Filename)
{
    char	name[MAXPATHLEN];
    FILE	*fp;

    if ((Filename == NULL) || (strlen (Filename) == 0))
	strcpy (name, RL_DEFAULT_COORD_FILE);
    else	strcpy (name, Filename);
    fp = fopen (name, "rt");
    return ((RL_DESC)fp);
}

/************************************************************************/
/*  StaClose:	Close a station file.					*/
/*									*/
/*	Input:	Station file descriptor.				*/
/*	Return:	RL_OK on success.					*/
/*		RL_IO_ERROR on error.					*/
/************************************************************************/
int StaClose (RL_DESC StaDesc)
{
    int status = fclose ((FILE *)StaDesc);
    status = (status == 0) ? RL_OK : RL_IO_ERROR;
    return (status);
}

/************************************************************************/
/*  StaRewind:	Rewind a station file.					*/
/*									*/
/*	Input:	Station file descriptor.				*/
/*	Return:	RL_OK on success.					*/
/*		RL_IO_ERROR on error.					*/
/************************************************************************/
int StaRewind (RL_DESC StaDesc)
{
    int status = fseek (StaDesc, 0L, SEEK_SET);
    status = (status == 0) ? RL_OK : RL_IO_ERROR;
    return (status);
}

/************************************************************************/
/*  NextLine:	Return the next line in a file.				*/
/*									*/
/*	Input:	File descriptor, Result string.				*/
/*	Return:	RL_OK on success.					*/
/*		RL_EOF on EOF.						*/
/*		RL_IO_ERROR on error.					*/
/************************************************************************/
static int NextLine (RL_DESC FileDesc, char* line)
{
    char	data[LINELEN];
    char	*error;

    /* Reading next line */
    if (fgets (data, LINELEN, (FILE *)FileDesc) == NULL) 
    {
	return ((feof((FILE *)FileDesc)) ? RL_EOF : RL_IO_ERROR);
    }
    strcpy (line, data);
    return (RL_OK);
}

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
int StaNext (RL_DESC StaDesc, station *StaInfo)
{
    char	line[LINELEN];	/* Next line */
    int		status, n;
    station	sta;
    char	tmp[LINELEN];

    if (StaInfo->version != RL_VERSION) return (RL_VERSION_ERROR);

    /* Get next entry, skipping blank or comment lines. */
    if ((status = NextLine (StaDesc, line)) != RL_OK) return (status);
    while (strlen (line) == 0 || line[0] == '#')
    {
	if ((status = NextLine (StaDesc, line)) != RL_OK) return (status);
    }

    /* Decoding line */
    memset (&sta, 0, sizeof(station));
    sta.version = RL_VERSION;
    n = sscanf (line, "%s %lf %lf %lf %s %s %s %s",
	    sta.sta, &sta.lat,
	    &sta.lon, &sta.elev,
	    sta.ondate, sta.offdate,
	    sta.net, tmp);
    if (n != 8) return (RL_FORMAT_ERROR);

    /* Decoding station name */
    strncpy (sta.staname, strstr (line, tmp), RL_STRING_LENGTH);
    sta.staname[RL_STRING_LENGTH-1] = '\0';
    *StaInfo = sta;
    return (RL_OK);
}

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
int StaSearch (RL_DESC StaDesc, station *UserSta, station *StaInfo)
{
    int	status;

    if (UserSta->version != RL_VERSION) return (RL_VERSION_ERROR);
    if ((status = StaRewind (StaDesc)) != RL_OK) return (status);
    StaInfo->version = RL_VERSION;

    /* Search for the station entry */
    while ((status = StaNext (StaDesc, StaInfo)) == RL_OK) {
	if (strcmp (StaInfo->sta, UserSta->sta) == 0 &&
	    strcmp (StaInfo->net, UserSta->net) == 0) return (RL_OK);
    }
    return (status);
}

/************************************************************************/
/*  ChaOpen:	Open a channel file.					*/
/*									*/
/*	Input:	Filename; if null then default channel file is opened.	*/
/*	Return:	Channel file descriptor; NULL if an error occurred.	*/
/************************************************************************/
RL_DESC ChaOpen (char* Filename)
{
    char	name[MAXPATHLEN];
    FILE	*fp;

    if ((Filename == NULL) || (strlen (Filename) == 0))
	strcpy (name, RL_DEFAULT_RESP_FILE);
    else	strcpy (name, Filename);
    fp = fopen (name, "rt");
    return ((RL_DESC)fp);
}

/************************************************************************/
/*  ChaClose:	Close a channel file.					*/
/*									*/
/*	Input:	Channel file descriptor.				*/
/*	Return:	RL_OK on success.					*/
/*		RL_IO_ERROR on error.					*/
/************************************************************************/
int ChaClose (RL_DESC ChaDesc)
{
    int status = fclose ((FILE *)ChaDesc);
    status = (status == 0) ? RL_OK : RL_IO_ERROR;
    return (status);
}

/************************************************************************/
/*  ChaRewind:	Rewind a channel file.					*/
/*									*/
/*	Input:	Channel file descriptor.				*/
/*	Return:	RL_OK on success.					*/
/*		RL_IO_ERROR on error.					*/
/************************************************************************/
int ChaRewind (RL_DESC ChaDesc)
{
    int status = fseek (ChaDesc, 0L, SEEK_SET);
    status = (status == 0) ? RL_OK : RL_IO_ERROR;
    return (status);
}

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
int ChaNext (RL_DESC ChaDesc, response* ChaInfo)
{
    char	line[LINELEN];	/* Next line */
    int		status;
    char	tmp[LINELEN];
    channel	cha;
    pn_resp	pnr;
    pz_resp	pzr;
    int		i, n;
    int		new_nbcoeff;
    int		new_nbpole;

    if (ChaInfo->version != RL_VERSION) return (-1);
    memset (&cha, 0, sizeof(channel));
    cha.version = RL_VERSION;

    /* Get next entry, skipping blank or comment lines. */
    if ((status = NextLine (ChaDesc, line)) != RL_OK) return (status);
    while (strlen (line) == 0 || line[0] == '#')
    {
	if ((status = NextLine (ChaDesc, line)) != RL_OK) return (status);
    }

    /* Decoding header line */
    n = sscanf (line, "%4s%4s %s %s %lf %lf %s %s %s %s %lf %lf %lf %lf",
		cha.sta, cha.geoschan, cha.ondate, cha.offdate,
		&cha.azimuth, &cha.dip, cha.units, cha.net,
		cha.seedchan, cha.location, &cha.lat, &cha.lon,
		&cha.elev, &cha.depth);
    if (n != 14) return (RL_FORMAT_ERROR);
    if (cha.location[0] == '-') memset(cha.location,0,RL_DEFAULT_LENGTH);

    /* Setting version number */
    cha.version = RL_VERSION;

    /* Read type of response */
    if ((status = NextLine (ChaDesc, line)) != RL_OK) return (status);
    n = sscanf (line, "%*s %s", tmp);
    if (n != 1) return (RL_FORMAT_ERROR);

    memset (&pnr, 0, sizeof(pn_resp));
    memset (&pzr, 0, sizeof(pz_resp));
    /* If the second token in the line is non-numeric, it is a polynomial response. */
    if (strchr ("ABCDEFGHIJKLMNOPQRSTUVWXYZ", tmp[0]) != NULL)	
    {
	/* Polynomial response */
	/* Decoding line */
	n = sscanf (line, "%lf %c %d", &pnr.maxerror, &pnr.type, &pnr.nbcoeff);
	if (n != 3) return (RL_FORMAT_ERROR);

	/* Reading polynomial coefficients */
	if ((pnr.nbcoeff%2) == 1)
    	    new_nbcoeff = pnr.nbcoeff-1;
	else   new_nbcoeff = pnr.nbcoeff;

	for (i = 0; i < new_nbcoeff; i+=2)
	{
	    /* Reading next line */
	    if ((status = NextLine (ChaDesc, line)) != RL_OK) return (status);
	    n = sscanf (line, "%lf %lf", &pnr.value[i], &pnr.value[i+1]);
	    if (n != 2) return (RL_FORMAT_ERROR);
	}

	if ((pnr.nbcoeff%2) == 1)	
	{
	    /* Reading next line */
	    if ((status = NextLine (ChaDesc, line)) != RL_OK) return (status);
	    n = sscanf (line, "%lf", &pnr.value[pnr.nbcoeff-1]);
	    if (n != 1) return (RL_FORMAT_ERROR);
	}

	/* Setting version number */
	pnr.version = RL_VERSION;

	/* Updating result response */
	ChaInfo->version = RL_VERSION;
	ChaInfo->resp_chan = cha;
	strcpy (ChaInfo->resp_type, "PN");
	ChaInfo->resp_pn = pnr;
    }
    else
    {
	/* Poles&Zeroes response */
	/* Decoding line */
	n = sscanf (line, "%lE %d %d", &pzr.gain, &pzr.nbzero, &pzr.nbpole);
	if (n != 3) return (RL_FORMAT_ERROR);

	/* Reading zeros */
	for (i=0;i<pzr.nbzero;i++)
	{
	    /* Reading next line */
	    if ((status = NextLine (ChaDesc, line)) != RL_OK) return (status);
	    n = sscanf (line, "%lE %lE", &pzr.zero[i].real, &pzr.zero[i].imag);
	    if (n != 2) return (RL_FORMAT_ERROR);
	}
         
	/* Reading poles */
	new_nbpole = pzr.nbpole - (pzr.nbpole % 2);
	for (i=0;i<new_nbpole;i+=2)
	{
	    if ((status = NextLine (ChaDesc, line)) != RL_OK) return (status);
	    n = sscanf (line, "%lE %lE %lE %lE", &pzr.pole[i].real, &pzr.pole[i].imag, 
			&pzr.pole[i+1].real, &pzr.pole[i+1].imag);
	    if (n != 4) return (RL_FORMAT_ERROR);
	}
         
	if (pzr.nbpole%2)
	{
	    if ((status = NextLine (ChaDesc, line)) != RL_OK) return (status);
	    n = sscanf (line, "%lE %lE", &pzr.pole[pzr.nbpole-1].real, &pzr.pole[pzr.nbpole-1].imag);
	    if (n != 2) return (RL_FORMAT_ERROR);
	}

	/* Setting version number */
	pzr.version = RL_VERSION;

	/* Updating result response */
	ChaInfo->version = RL_VERSION;
	ChaInfo->resp_chan = cha;
	strcpy (ChaInfo->resp_type, "PZ");
	ChaInfo->resp_pz = pzr;
    }

    return (RL_OK);
}

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
int ChaSearch (RL_DESC ChaDesc, channel *UserCha, response* ChaInfo)
{
    int		status;
    int		HaveResponseForChannel = 0;
    response	OldResp;
    char	location[RL_DEFAULT_LENGTH];

    if (UserCha->version != RL_VERSION) return (-1);
    /* A location that begins with "-" is equivalent to a blank location. */
    strcpy (location, UserCha->location);
    if (UserCha->location[0] == '-') memset(location,0,RL_DEFAULT_LENGTH);
    if ((status = ChaRewind (ChaDesc)) != RL_OK) return (status);
    ChaInfo->version = RL_VERSION;

    /* Search for the station entry */
    while ((status = ChaNext (ChaDesc, ChaInfo)) == RL_OK) {
	/* Comparing data */
	if ((!strcmp (ChaInfo->resp_chan.sta, UserCha->sta)) &&
	    (!strcmp (ChaInfo->resp_chan.net, UserCha->net)) &&
	    (!strcmp (ChaInfo->resp_chan.seedchan, UserCha->seedchan)) &&
	    (!strcmp (ChaInfo->resp_chan.location, location)))
	{
	    /* Testing optional date */
	    if (strlen (UserCha->ondate) == 0)
	    {
		if (! HaveResponseForChannel)
		{
		    /* Save the first response for this channel. */
		    OldResp = *ChaInfo;
		    HaveResponseForChannel = 1;
		}
		else
		{
		    /* Save ths response if it is later than the old response. */
		    if (strcmp (ChaInfo->resp_chan.ondate, OldResp.resp_chan.ondate) > 0)
			OldResp = *ChaInfo;
		}
	    }
	    else 
	    {
		/* See if this is the time interval we want. */
		if ((strcmp (UserCha->ondate, ChaInfo->resp_chan.ondate) >= 0) &&
		     (strcmp (UserCha->ondate, ChaInfo->resp_chan.offdate) <= 0))
		{
		    return (RL_OK);
		}
	    }
	}
    }

    if (HaveResponseForChannel)
    {
	*ChaInfo = OldResp;
	return (RL_OK);
    }
    return (status);
}


/************************************************************************/
/* Fortran <==> C conversion for station and channel structures.	*/
/************************************************************************/
static int trimlen (char *str, int maxlen)
{
    int len = maxlen - 1;
    while ( (len > 0) && (str[len-1] == ' ') ) 	len--;
    return (len);
}

static void fpad (char *str, int maxlen)
{
    int i;
    for (i=strlen(str); i<maxlen; i++) str[i] = ' ';
}


static void f2c_station (station *p, int all) 
{
    p->net[trimlen(p->net, RL_DEFAULT_LENGTH)] = '\0';
    p->sta[trimlen(p->sta, RL_DEFAULT_LENGTH)] = '\0';
    if (! all) return;
    p->staname[trimlen(p->staname, RL_STRING_LENGTH)] = '\0';
    p->ondate[trimlen(p->ondate, RL_DATE_LENGTH)] = '\0';
    p->offdate[trimlen(p->offdate, RL_DATE_LENGTH)] = '\0';
}

static void c2f_station (station *p)
{
    fpad(p->net, RL_DEFAULT_LENGTH);
    fpad(p->sta, RL_DEFAULT_LENGTH);
    fpad(p->staname, RL_STRING_LENGTH);
    fpad(p->ondate, RL_DATE_LENGTH);
    fpad(p->offdate, RL_DATE_LENGTH);
}

static void f2c_channel (channel *p, int all)
{
    p->net[trimlen(p->net, RL_DEFAULT_LENGTH)] = '\0';
    p->sta[trimlen(p->sta, RL_DEFAULT_LENGTH)] = '\0';
    p->seedchan[trimlen(p->seedchan, RL_DEFAULT_LENGTH)] = '\0';
    p->location[trimlen(p->location, RL_DEFAULT_LENGTH)] = '\0';
    p->ondate[trimlen(p->ondate, RL_DATE_LENGTH)] = '\0';
    p->offdate[trimlen(p->offdate, RL_DATE_LENGTH)] = '\0';
    if (! all) return;
    p->geoschan[trimlen(p->geoschan, RL_DEFAULT_LENGTH)] = '\0';
    p->units[trimlen(p->units, RL_UNIT_LENGTH)] = '\0';
}

static void c2f_channel (channel *p) 
{
    fpad(p->net, RL_DEFAULT_LENGTH);
    fpad(p->sta, RL_DEFAULT_LENGTH);
    fpad(p->seedchan, RL_DEFAULT_LENGTH);
    fpad(p->location, RL_DEFAULT_LENGTH);
    fpad(p->geoschan, RL_DEFAULT_LENGTH);
    fpad(p->ondate, RL_DATE_LENGTH);
    fpad(p->offdate, RL_DATE_LENGTH);
    fpad(p->units, RL_UNIT_LENGTH);
}

static void c2f_response (response *p)
{
    c2f_channel (&(p->resp_chan));
    fpad(p->resp_type, RL_DEFAULT_LENGTH);
}

/************************************************************************/
/*  Fortran callable functions.						*/
/************************************************************************/

/************************************************************************/
/*  f_staopen:	Open a station file.					*/
/*									*/
/*	Input:  Filename; if blank then default station file is opened.	*/
/*	Return:	Station file descriptor (int); 0 if an error occurred.	*/
/************************************************************************/
#ifdef fortran_suffix
fort_ptr f_staopen_
#else
fort_ptr f_staopen
#endif
	(char* Filename, int Length)
{
    char cpath[MAXPATHLEN];
    int len;
    int i;

    len = trimlen (Filename, Length);
    if ( len >= MAXPATHLEN ) {
	fprintf(stderr, "iopen: Filename too long (%d)\n", len);
	fflush(stderr);
	return(-1);
    }
    bcopy(Filename, cpath, len);
    cpath[len] = 0;

    return ((int)StaOpen (cpath));
}

/************************************************************************/
/*  f_staclose:	Close a station file.					*/
/*									*/
/*	Input:  Station file descriptor.				*/
/*	Return: RL_OK on success.					*/
/*		RL_IO_ERROR on error.					*/
/************************************************************************/
#ifdef fortran_suffix
int f_staclose_
#else
int f_staclose
#endif
	(fort_ptr *StaDesc)
{
    return (StaClose ((RL_DESC)StaDesc));
}

/************************************************************************/
/*  f_starewind: Rewind a station file.					*/
/*									*/
/*	Input:  Station file descriptor.				*/
/*	Return: RL_OK on success.					*/
/*		RL_IO_ERROR on error.					*/
/************************************************************************/
#ifdef fortran_suffix
long int f_starewind_
#else
long int f_starewind
#endif
	(fort_ptr *StaDesc)
{
    return (StaRewind ((RL_DESC)*StaDesc));
}

/************************************************************************/
/*  f_stanext:	Return the next station entry.				*/
/*									*/
/*	Input:  File descriptor, Result station structure.		*/
/*	Return: RL_OK on success.					*/
/*		RL_EOF on EOF.						*/
/*		RL_IO_ERROR on error.					*/
/*		RL_VERSION_ERROR on version error.			*/
/*		RL_FORMAT_ERROR on station entry format error.		*/
/************************************************************************/
#ifdef fortran_suffix
int f_stanext_
#else
int f_stanext
#endif
	(fort_ptr *StaDesc, station *StaInfo)
{
    station Sta;
    int status;
    memcpy (&Sta, StaInfo, sizeof(station));
    f2c_station (&Sta, 0);
    status = StaNext ((RL_DESC)*StaDesc, &Sta);
    if (status == 0) {
	c2f_station (&Sta);
	memcpy (StaInfo, &Sta, sizeof(station));
    }
    return (status);
}

/************************************************************************/
/*  f_stasearch: Search a station file.					*/
/*									*/
/*	Input:  File descriptor, Search station structure,		*/
/*		Result station structure.				*/
/*									*/
/*	Return: RL_OK on success.					*/
/*		RL_EOF on EOF.						*/
/*		RL_IO_ERROR on error.					*/
/*		RL_VERSION_ERROR on version error.			*/
/*		RL_FORMAT_ERROR on station entry format error.		*/
/************************************************************************/
#ifdef fortran_suffix
int f_stasearch_
#else
int f_stasearch
#endif
	(fort_ptr *StaDesc, station *UserSta, station *StaInfo)
{
    station USta;
    int status;
    memcpy (&USta, UserSta, sizeof(station));
    f2c_station (&USta, 0);
    status = StaSearch ((RL_DESC)*StaDesc, &USta, StaInfo);
    if (status == 0) {
	c2f_station (StaInfo);
    }
    return (status);
}

/************************************************************************/
/*  f_chaopen:	Open a channel file.					*/
/*									*/
/*	Input:  Filename; blank, open default channel file.		*/
/*	Return:	Channel file descriptor (int); 0 if an error occurred.	*/
/************************************************************************/
#ifdef fortran_suffix
int f_chaopen_
#else
int f_chaopen
#endif
	(char* Filename, int Length)
{
    char cpath[MAXPATHLEN];
    int len;
    int i;

    len = trimlen (Filename, Length);
    if ( len >= MAXPATHLEN ) {
	fprintf(stderr, "iopen: Filename too long (%d)\n", len);
	fflush(stderr);
	return(-1);
    }
    bcopy(Filename, cpath, len);
    cpath[len] = 0;

    return ((fort_ptr)ChaOpen (cpath));
}

/************************************************************************/
/*  f_chaclose:	Close a channel file.					*/
/*									*/
/*	Input:  Channel file descriptor.				*/
/*	Return: RL_OK on success.					*/
/*		RL_IO_ERROR on error.					*/
/************************************************************************/
#ifdef fortran_suffix
int f_chaclose_
#else
int f_chaclose
#endif
	(fort_ptr *ChaDesc)
{
    return (ChaClose ((RL_DESC)*ChaDesc));
}

/************************************************************************/
/*  f_charewind: Rewind a channel file.					*/
/*									*/
/*	Input:  Channel file descriptor.				*/
/*	Return: RL_OK on success.					*/
/*		RL_IO_ERROR on error.					*/
/************************************************************************/
#ifdef fortran_suffix
int f_charewind_
#else
int f_charewind
#endif
	(fort_ptr *ChaDesc)
{
    return (ChaRewind ((RL_DESC)*ChaDesc));
}

/************************************************************************/
/*  f_chanext:	Return the next channel entry.				*/
/*									*/
/*	Input:  File descriptor, Result channel structure.		*/
/*	Return: RL_OK on success.					*/
/*		RL_EOF on EOF.						*/
/*		RL_IO_ERROR on error.					*/
/*		RL_VERSION_ERROR on version error.			*/
/*		RL_FORMAT_ERROR on station entry format error.		*/
/************************************************************************/
#ifdef fortran_suffix
int f_chanext_
#else
int f_chanext
#endif
	(fort_ptr *ChaDesc, response* ChaInfo)
{
    response Cha;
    int status;
    memcpy (&Cha, ChaInfo, sizeof(response));
    f2c_channel (&(ChaInfo->resp_chan),0);
    status = ChaNext ((RL_DESC)*ChaDesc, &Cha);
    if (status == 0) {
	c2f_response (&Cha);
	memcpy (ChaInfo, &Cha, sizeof(response));
    }
    return (status);
}

/************************************************************************/
/*  f_chasearch: Search a channel file.					*/
/*									*/
/*	Input:  File descriptor, Search channel structure,		*/
/*		Result channel structure.				*/
/*									*/
/*	Return: RL_OK on success.					*/
/*		RL_EOF on EOF.						*/
/*		RL_IO_ERROR on error.					*/
/*		RL_VERSION_ERROR on version error.			*/
/*		RL_FORMAT_ERROR on station entry format error.		*/
/*	Remark: Date format is yyyy.doy.hhmm;				*/
/*		If ommited, return the most recent entry for station.	*/
/************************************************************************/
#ifdef fortran_suffix
int f_chasearch_
#else
int f_chasearch
#endif
	(fort_ptr *ChaDesc, channel *UserCha, response* ChaInfo)
{
    channel UCha;
    int status;
    memcpy (&UCha, UserCha, sizeof(channel));
    f2c_channel (&UCha,0);
    status = ChaSearch ((RL_DESC)*ChaDesc, &UCha, ChaInfo);;
    if (status == 0) {
	c2f_response (ChaInfo);
    }
    return (status);
}
