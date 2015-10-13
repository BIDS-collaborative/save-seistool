c	$Id: test_instr.f,v 1.1 2002/04/05 00:39:59 lombard Exp $
	program test_instr
c
      character*256 files, filec, filem

      include '/usr/local/include/qlib2.inc'
      parameter (maxps = 32767)
      integer*4 idata(maxps)
      include 'station_hdr.inc'
c
	filem = 'BKS.BHZ.D.2001.056.2318'
	filen = 'instr.coord'
 	files = 'instr.coord'
	filec = 'instr.resp'
c
      call sdr_read(filem,maxps,idata,head,iere)
      print*,' Return from sdr_read: ', iere
c fetch station info
      call fetch_response(files,head,0,iere)
      print*,' Return from fetch_response: ', iere
c fetch channel info
      call fetch_response(filec,head,1,iere)
      print*,' Return from fetch_response: ', iere

200   continue
      stop
      end

 
