c	$Id: sdr_read.f,v 1.1 2002/04/05 00:40:40 lombard Exp $
	subroutine sdr_read(file,maxp,data,head,iere)
c
	implicit none
 	include "/usr/local/include/qlib2.inc"
c structure for output header
	include 'station_hdr.inc'

	character*(*) file
	integer data(*)
	integer maxp
	integer iere

	integer kfile, icode, io

	double precision THRESHOLD
	parameter (THRESHOLD = .5)

	record /DATA_HDR/ hdr, hdr1
	record /INT_TIME/ endtime
	record /EXT_TIME/ et1, et2
	integer fp, nreq, nread, n, date_fmt, i
	integer seconds, usecs, islew
	double precision slew, thresh

c external functions and subroutines
	integer iargc
	external getarg, iargc
	integer lnblnk
	external lnblnk 
c functions in most fortran libraries.
	double precision tslew
	external tslew
c functions in library fio.
	integer ifopen, ifclose
	external ifopen, ifclose

c initialize
      iere = 0
	io = 6

c determine length of file name
	kfile = lnblnk(file)
	
c open file
	fp = ifopen (file, "r")
	if (fp .lt. 0) then
		write(io,*) "Error opening file", file(1:kfile)
                icode = ifclose(fp)
		iere = 1000
		return
	endif

c set number of requested data points.
c NOTE -- buffer must be large enough to handle this many points.
	nreq = maxp

c read first block, and establish station, channel and timing thresholds.
	date_fmt = 0
	slew = 0
	nread = 0
	n = f_read_ms (hdr, data(nread+1), nreq, fp)
	if (n .gt. 0) then
		call f_copy_data_hdr (hdr1, hdr)
		call f_time_interval2 (1, hdr.sample_rate, 
     1			hdr.sample_rate_mult, seconds, usecs)
		thresh = (seconds * USECS_PER_SEC + usecs) * THRESHOLD
	else
		write(io,*) "Error reading first block ",file(1:kfile)
                icode = ifclose(fp)
		iere = 999
		return
	endif

c update number of points remaining and continue read more blocks.
c check to ensure the data stream is for the same station and channel,
c and check for time tears.
	do while ((n .gt. 0))
	    nread = nread + n
	    nreq = nreq - n
	    if (nreq .gt. 0) then
		call f_delete_blockette (hdr, -1)
		n = f_read_ms (hdr, data(nread+1), nreq, fp)
		if (n .gt. 0) then
		    if (hdr1.station_id .ne. hdr.station_id .or.
     1			hdr1.network_id .ne. hdr.network_id .or.
     2			hdr1.channel_id .ne. hdr.channel_id .or.
     3			hdr1.location_id .ne. hdr.location_id) then
			write (io,*) "Data block for wrong channel ",
     1				file(1:kfile)
			iere = 997
			goto 100
		    endif
		    slew = tslew(hdr1.begtime, hdr.begtime, nread,
     1			hdr.sample_rate, hdr.sample_rate_mult)
		    if (abs(slew) .gt. thresh) then
			write (io,*) "Unacceptable time slew ",file(1:kfile)
			write (io,120) nread, slew, thresh
			iere = 998
			goto 100
		    endif
		endif
	    else
		n = 0
	    endif
	end do


c we are finished reading data. check for error reading data.
100	if (n .lt. -1) then
		write(io,*) "Error reading file", file(1:kfile)
		iere = 999
	endif
	
c output data points.
c convert times to printable forms.
c as an example, convert times to strings and to ext_time.
c	call f_int_to_ext(hdr1.begtime, et1)
c	call f_time_to_str(hdr1.begtime, date_fmt, s1)

c compute ending time (eg expected time of next data point).
	call f_time_interval2 (nread, hdr.sample_rate, 
     1		hdr.sample_rate_mult, seconds, usecs)
	call f_add_time(hdr1.begtime, seconds, usecs, endtime)
	call f_int_to_ext(endtime, et2)
c	call f_time_to_str(endtime, date_fmt, s2)

c output header and data
	islew = slew

c put header together for return
      head.station_id = hdr1.station_id
      head.channel_id = hdr1.channel_id
      head.network_id = hdr1.network_id
      head.location_id = hdr1.location_id
      head.begtime = hdr1.begtime
      head.endtime = endtime
      head.num_pts = nread
      head.sample_rate = hdr1.sample_rate
      head.slew = slew
      head.thresh = thresh

c error checking
c	write (*,*) "station ", 
c     1		hdr1.station_id(1:lnblnk(hdr1.station_id)),
c     2		" channel ",
c     3		hdr1.channel_id(1:lnblnk(hdr1.channel_id)),
c     4		" time ", s1(1:lnblnk(s1)),
c     5		" points ", nread, " slew ", islew
c	do i = 1, nread
c		write (*,*) data(i)
c	end do

	call f_delete_blockette (hdr, -1)
	call f_delete_blockette (hdr1, -1)
	i = ifclose (fp)
120	format("Time error after ", i, " points -- slew ",f6.0, " gt", f4.0)
	end
c
c function to compute the total time slew from the beginning of data
c
	double precision function tslew (begtime, newtime, npts, rate, mult)
	implicit none
 	include "/usr/local/include/qlib2.inc"
	record /INT_TIME/ begtime, newtime
	integer npts, rate, mult
	double precision f_tdiff
	record /INT_TIME/ exptime
	integer seconds, usecs

	call f_time_interval2 (npts, rate, mult, seconds, usecs)
	call f_add_time(begtime, seconds, usecs, exptime)
	tslew = f_tdiff(newtime, exptime)
	return
	end

