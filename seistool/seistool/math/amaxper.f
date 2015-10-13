c	$Id: amaxper.f,v 1.1 2001/12/21 18:39:07 lombard Exp $

      subroutine amaxper(npts,dt,ind,fc,amaxmm,amaxnm,pmax,imax)
c 
c     compute maximum amplitude and its associated period
c
c     input:
c             npts   - number of points in timeseries
c             dt     - sample spacing in sec
c             ind    - type of instrument (for magnitude)
c                      1 - Benioff 100 kg (Mb)
c                      2 - 17s-23s 10 pole butterworth filter (Ms)
c                      3 - Wood Anderson (Ml)
c                     20 - deconvolve only
c                     21 - velocity 
c                     22 - acceleration
c             fc     - input timeseries
c     output:
c             amaxmm - raw maximum
c             amaxnm - maximum in microns of ground motion
c                      unless ind = 3, when output is millimeters of record motion
c             pmax   - period of maximum
c             imax   - index of maximum point
c
      real*4 fc(*)
      complex*8 ben100, lpbptf_b, aw
c
      include 'numerical.inc'
c
      imax = 1
      amax = 0.
      do i = 1,npts
        if (abs(fc(i)) .gt. amax) then
          imax = i
          amax = abs(fc(i))
        endif
      end do
c
c     compute period of maximum
c
      if (fc(imax) .gt. 0.) then
        pp = 0.
        j = imax
    2   j = j+1
        if(fc(j).gt.0.) then
          pp = pp+dt
          go to 2
        endif
        pp = pp+dt*(0.-fc(j-1))/(fc(j)-fc(j-1))
        pm = 0.
        j = imax
    3   j = j-1
        if(fc(j).gt.0.) then
          pm = pm+dt
          go to 3
        endif
        pm = pm+dt*(0.-fc(j+1))/(fc(j)-fc(j+1))
      else
        pp = 0.
        j = imax
    4   j = j+1
        if(fc(j).lt.0.) then
          pp = pp+dt
          go to 4
        endif
        pp = pp+dt*(0.-fc(j-1))/(fc(j)-fc(j-1))
        pm = 0.
        j = imax
    5   j = j-1
        if(fc(j).lt.0.) then
          pm = pm+dt
          go to 5
        endif
        pm = pm+dt*(0.-fc(j+1))/(fc(j)-fc(j+1))
      endif
c
c     correct for new instrument/filter/whatever
c
      pmax = 2.*(pm+pp)
      wmax = tpi / pmax
      if (ind .eq. 0) then
c       now Raw data......
        aw = (1.0,0.0)
        scale = 1.0
      elseif (ind .eq. 1) then
c       Benioff - true ground motion - convert amplitude to microns
        aw = ben100(wmax)
        scale = 1.e6
      elseif (ind .eq. 2) then
c       Band-pass filter - true ground motion - convert amplitude to microns
        aw = lpbptf_b(wmax)
        scale = 1.e6
      elseif (ind .eq. 3) then
c       WAS - record motion - convert amplitude to millimeters
        aw = 1.0
        scale = 1.0
      elseif (ind .eq. 20) then
c       deconvolved - true ground motion - convert amplitude to microns
        aw = (1.0,0.0)
        scale = 1.e6
      elseif (ind .eq. 21) then
c       deconvolved - true ground velocity - convert amplitude to microns/sec
        aw = (1.0,0.0)
        scale = 1.e6
      elseif (ind .eq. 22) then
c       deconvolved - true ground acceleration - convert amplitude to microns/sec ^2
        aw = (1.0,0.0)
        scale = 1.e6
      end if
c
c     two amplitude values are returned
c     --amaxmm - which is the raw peak amplitude
c     --amaxnm - which is peak amplitude corrected for instrument (mostly)
c
      amaxmm = amax
      amaxnm = scale*amax/cabs(aw)
c
      return
      end
