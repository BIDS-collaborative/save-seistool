C ************************************************************
C * Structures & API's Specifications for the resplib        *
C * library. This library gives to the user functions        *
C * to access instrument response and coordinate files.      *
C *                                                          *
C * NCEDC @2000 -- Berkeley Seismological Laboratory         *
C * Bugs/Suggestions/Questions: stephane@seismo.berkeley.edu *
C *                                                          *
C ************************************************************

C	$Id: resplib.inc,v 1.2 2002/04/05 00:43:50 lombard Exp $ 

C **************************
C * Parameters Definitions *
C **************************

      integer RL_DEFAULT_LENGTH, RL_DATE_LENGTH, RL_UNIT_LENGTH,
     1 RL_MAX_POLES, RL_MAX_ZEROES, RL_MAX_COEFFS
      parameter (RL_DEFAULT_LENGTH = 8)
      parameter (RL_DATE_LENGTH    = 24)
      parameter (RL_UNIT_LENGTH    = 16)
      parameter (RL_MAX_POLES      = 32)
      parameter (RL_MAX_ZEROES     = 32)
      parameter (RL_MAX_COEFFS     = 32)

      integer RL_VERSION
      parameter ( RL_VERSION = 2 )

      integer RL_OK, RL_EOF, RL_VERSION_ERROR, RL_IO_ERROR, 
     1 RL_FORMAT_ERROR, RL_NOT_FOUND
      parameter (RL_OK 		  = 0)
      parameter (RL_EOF 	  = -1)
      parameter (RL_VERSION_ERROR = 1)
      parameter (RL_IO_ERROR 	  = 2)
      parameter (RL_FORMAT_ERROR  = 3)
      parameter (RL_NOT_FOUND	  = RL_EOF)

      character*256 RL_DEFAULT_RESP_FILE
      character*256 RL_DEFAULT_COORD_FILE
      parameter (RL_DEFAULT_RESP_FILE =
     1    '/usr/contrib/data/bdsn/instr.db.resp')
      parameter (RL_DEFAULT_COORD_FILE = 
     1    '/usr/contrib/data/bdsn/stat.db.coord')

C ************************** 
C * Structures Definitions * 
C ************************** 

C * Station Definition *
      structure /station/
	integer				version
	integer				pad
	character*(RL_DEFAULT_LENGTH)	net
	character*(RL_DEFAULT_LENGTH)	sta
	character*32			staname
	character*(RL_DATE_LENGTH)	ondate
	character*(RL_DATE_LENGTH)	offdate
	real*8				lat
	real*8				lon
	real*8				elev
      end structure

C * Channel definition *
      structure /channel/
	integer				version
	integer				pad
	character*(RL_DEFAULT_LENGTH)	net
	character*(RL_DEFAULT_LENGTH)	sta
	character*(RL_DEFAULT_LENGTH)	seedchan
	character*(RL_DEFAULT_LENGTH)	location
	character*(RL_DEFAULT_LENGTH)	geoschan
	character*(RL_DATE_LENGTH)	ondate
	character*(RL_DATE_LENGTH)	offdate
	character*(RL_UNIT_LENGTH)	units
	real*8				lat
	real*8				lon
	real*8				elev
	real*8				depth
	real*8				dip
	real*8				azimuth
      end structure

C * Poles&Zeroes Response Definition *
      structure /pz_resp/
	integer				version
	integer				pad
	real*8				gain
	integer				nbpole
	integer				nbzero
	complex*16			pole(RL_MAX_POLES)
	complex*16			zero(RL_MAX_ZEROES)
      end structure

C * Polynomial Response Definition *
      structure /pn_resp/
	integer				version
	integer				nbcoeff
	character*(RL_DEFAULT_LENGTH)	type
	real*8				maxerror
	real*8				value(RL_MAX_COEFFS)
      end structure

C * Instrument Response Definition *
      structure /response/
	integer				version
	integer				pad
	record /channel/		resp_chan
	character*(RL_DEFAULT_LENGTH)	resp_type
	record /pz_resp/		resp_pz
	record /pn_resp/		resp_pn
      end structure

C**********************************************************************
C   Functions Definitions
C
C	Note: Pointers are currently implemented integer*4.
C**********************************************************************

C**********************************************************************
C   f_staopen:	Open a station file.
C
C	Input:  Filename; if blank then default station file is opened.
C	Return:	Station file descriptor (int); 0 if an error occurred.
C
C 	integer*4 fp
C 	character*256 filename
C 	fp = f_staopen(filename)
C**********************************************************************
      integer*4 f_staopen
      external f_staopen

C**********************************************************************
C   f_staclose:	Close a station file.
C
C	Input:  Station file descriptor.
C	Return: RL_OK on success.
C		RL_IO_ERROR on error.
C 
c	integer*4 fp
C	integer status
C	status = f_staclose(fp)
C**********************************************************************
      integer f_staclose
      external f_staclose

C**********************************************************************
C   f_starewind: Rewind a station file.
C
C	Input:  Station file descriptor.
C	Return: RL_OK on success.
C		RL_IO_ERROR on error.
C
C	integer*4 fp
C	integer status
C	status = f_starewind(fp)
C**********************************************************************
      integer f_starewind
      external f_starewind

C**********************************************************************
C  f_stanext:	Return the next station entry.
C
C	Input:  File descriptor, Result station structure.
C	Return: RL_OK on success.
C		RL_EOF on EOF.
C		RL_IO_ERROR on error.
C		RL_VERSION_ERROR on version error.
C		RL_FORMAT_ERROR on station entry format error.
C
C	integer*4 fp
C	integer status
C	record /station/ stainfo
C	status = f_stanext(fp, stainfo)
C**********************************************************************
      integer f_stanext
      external f_stanext

C**********************************************************************
C  f_stasearch: Search a station file.
C
C	Input:  File descriptor, Search station structure,
C		Result station structure.
C
C	Return: RL_OK on success.
C		RL_EOF on EOF.
C		RL_IO_ERROR on error.
C		RL_VERSION_ERROR on version error.
C		RL_FORMAT_ERROR on station entry format error.
C
C	integer*4 fp
C	integer status
C	record /station/ Usersta, stainfo
C	status = f_stasearch(fp, Usersta, stainfo )
C**********************************************************************
      integer f_stasearch
      external f_stasearch

C**********************************************************************
C  f_chaopen:	Open a channel file.
C
C	Input:  Filename; blank, open default channel file.
C	Return:	Channel file descriptor (int); 0 if an error occurred.
C
C	integer*4 fp
C	character*256 filenam
C	fp = f_chaopen(filename)
C**********************************************************************
      integer*4 f_chaopen
      external f_chaopen

C**********************************************************************
C  f_chaclose:	Close a channel file.
C
C	Input:  Channel file descriptor.
C	Return: RL_OK on success.
C		RL_IO_ERROR on error.
C
C	integer*4
C	integer status
C	status = f_chaclose(fp)
C**********************************************************************
      integer f_chaclose
      external f_chaclose

C**********************************************************************
C  f_charewind: Rewind a channel file.
C
C	Input:  Channel file descriptor.
C	Return: RL_OK on success.
C		RL_IO_ERROR on error.
C
C	integer*4 fp
C	integer status
C	status = f_charewind(fp)
C**********************************************************************
      integer f_charewind
      external f_charewind

C**********************************************************************
C  f_chanext:	Return the next channel entry.
C
C	Input:  File descriptor, Result channel structure.
C	Return: RL_OK on success.
C		RL_EOF on EOF.
C		RL_IO_ERROR on error.
C		RL_VERSION_ERROR on version error.
C		RL_FORMAT_ERROR on station entry format error.
C
C	integer*4 fp
C	integer status
C	record /response/ chainfo
C	status = f_chanext(fp, chainfo)
C**********************************************************************
      integer f_chanext
      external f_chanext

C**********************************************************************
C   f_chasearch: Search a channel file.
C
C	Input:  File descriptor, Search channel structure,
C		Result channel structure.
C
C	Return: RL_OK on success.
C		RL_EOF on EOF.
C		RL_IO_ERROR on error.
C		RL_VERSION_ERROR on version error.
C		RL_FORMAT_ERROR on station entry format error.
C	Remark: Date format is yyyy.doy.hhmm;
C		If ommited, return the most recent entry for station.
C
C	integer*4 fp
C	integer status, f_chasearch
C	record /channel/ usercha
C	record /response/ chainfo
C	external f_chasearch
C	status = f_chasearch(fp, usercha, chainfo )
C**********************************************************************
      integer f_chasearch
      external f_chasearch
