C	$Id: fetch_response.f,v 1.1 2002/04/05 00:41:13 lombard Exp $
      subroutine fetch_response(filen,head,req,ierr)
c
c     subroutine will find response for station stat and channel chan
c     at stime and load it into response common block
c
c     input:
c     	filen - file containing instrument information
c     	head  - station header structure
c     output:
c     	ierr  - integer error flag
c           	= 0 for recovered response
c           	 = 999 for no response found
c
      integer GetStat, GetChan
      parameter (GetStat=0) 
      parameter (GetChan=1)
      include '/usr/local/include/qlib2.inc'
c:::      record /INT_TIME/ stime, bdate, edate
      record /EXT_TIME/ st
c
      include 'station_hdr.inc'
c
      include '/usr/local/include/resplib.inc'
      record /station/ usersta, stainfo
      record /channel/ usercha
      record /response/ chainfo
c
      character*4 cyear
      character*3 cdoy
      character*2 chr, cmin
      character*9 cunit
      integer*4 ierr
c:::      integer*4 iere, daymo
c:::      real*8 td1, td2
      integer*4 req
c
      character*(*) filen
      integer*4 status
c     Fortran equivalent of pointers - change to i*8 for 64-bit ptrs.
      integer*4 f_staopen, fp
c
      integer*4 f_staclose, f_stasearch
      external f_staopen, f_staclose, f_stasearch
c
c     Common block for saving instrument response information
c
      integer*4 iunit
      integer*4 npoles, nzeros
      real*4 ds, gain
      complex*8 poles(60)
      common /instr/ iunit, ds, gain, npoles, nzeros, poles
c
c     Initialize
c
      ierr = 0
      if (req .ne. GetStat .and. req .ne. GetChan) then
         print*, 'fetch_response: invalid request: ', req
         goto 110
      endif
c
      iunit = 999
      npoles = 0
      nzeros = 0
      gain = 0.0
      ds = 1.0
c
      if (req .eq. GetStat) then
         usersta.version   = RL_VERSION
         usersta.sta       = head.station_id
         usersta.net       = head.network_id
         print*, 'Station request info: '
         print*, 'filename = ', filen(1:lnblnk(filen))
         print*, 'usersta.version = ', usersta.version
         print*, 'usersta.station = ', usersta.sta
         print*, 'usersta.network = ', usersta.net
      endif

c
      if (req .eq. GetChan) then
         usercha.version   = RL_VERSION
         usercha.sta       = head.station_id
         usercha.net       = head.network_id
         usercha.location  = head.location_id
         usercha.seedchan  = head.channel_id
         print*, 'Channel request info: '
         print*, 'filename = ', filen(1:lnblnk(filen))
         print*, 'version=  ', usercha.version
         print*, 'station=  ', usercha.sta    
         print*, 'network=  ', usercha.net    
         print*, 'location= ', usercha.location
         print*, 'channel=  ', usercha.seedchan
      endif
c
      call f_int_to_ext(head.begtime,st)
      write(cyear,'(i4)') st.year
      write(cdoy,'(i3.3)') st.doy
      write(chr,'(i2.2)') st.hour
      write(cmin,'(i2.2)') st.minute
      usercha.ondate = cyear//'.'//cdoy//'.'//chr//cmin
      usersta.ondate = cyear//'.'//cdoy//'.'//chr//cmin
      if (req .eq. GetStat)  print*,usersta.ondate
      if (req .eq. GetChan)  print*,usercha.ondate
c
c     Open the channel file
c
      if (req .eq. GetStat) fp = f_staopen(filen)
      if (req .eq. GetChan) fp = f_chaopen(filen)
      if (fp .eq. 0) then
         ierr = 997
         print*,'Error return ', status
         status = f_chaclose(fp)
         goto 110
      end if
c
c     Find the station information
c
      if (req .eq. GetStat) then
         status = f_stasearch(fp, usersta, stainfo)
         if (status .ne. RL_OK) then
            ierr = 998
            print*,'Error return ', status
            status = f_staclose(fp)
            goto 110
         end if
      endif
c
c     Find the channel information
c
      if (req .eq. GetChan) then
         status = f_chasearch(fp, usercha, chainfo)
         if (status .ne. RL_OK) then
            ierr = 998
            print*,'Error return ', status
            status = f_staclose(fp)
            goto 110
         end if
c
         head.azi = chainfo.resp_chan.azimuth
         head.dip = chainfo.resp_chan.dip
         head.gain = chainfo.resp_pz.gain * ds
         head.nzeros = chainfo.resp_pz.nbzero
         head.npoles = chainfo.resp_pz.nbpole
         if (cunit .eq. 'DU/M/S**2') then
            iunit=2
         else if (cunit .eq. 'DU/M/S') then
            iunit=1
         else if (cunit .eq. 'DU/M') then
            iunit=0
         else
            iunit=999
         end if
         print*, 'np=', head.npoles
         print*, 'nz=', head.nzeros
         do jj = 1, head.npoles
            head.poles(jj) = chainfo.resp_pz.pole(jj)
         end do
         do jj = 1, head.nzeros
            head.poles(head.npoles+jj) = chainfo.resp_pz.zero(jj)
         end do
         print*, 'dip=',head.dip
         print*, 'azi=',head.azi
         print*, 'gain=',head.gain
         print*, 'npoles=',head.npoles
         print*, 'nzeros=',head.nzeros
         print*, 'poles=',(head.poles(jj),jj=1,head.npoles)
         print*, 'zeros=',(head.poles(jj),
     1                     jj=head.npoles+1,head.npoles+head.nzeros)
         head.iunit = iunit
      endif
c
c     exit
c
      
 110  return
c
c     format statements
c
 101  format(2a4,1x,i4,1x,i3,1x,i2,i2,2x,i4,
     .     1x,i3,1x,i2,i2,2f8.2,1x,a9,1x,a4,1x,a3)
 102  format(3x,e12.5,i3,i4)
 103  format(1x,2e14.6)
 104  format(4e14.6)
c
      end
