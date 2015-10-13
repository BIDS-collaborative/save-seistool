c	$Id: butter.f,v 1.1 2001/12/21 18:39:07 lombard Exp $
      complex function butter(f,fl,fh,npole)
c
c ...... bandpass filter  (nPBP Butterworth Filter)
c
c        convolve with npole Butterworth Bandpass filter
c
c        stolen from Seisgram - 10/28/92
c        - 6 pole output compared with subroutine bpbw
c          Pretty good, although slight differences exist
c
c        where -
c          f     - frequency of evaluation in rad/s
c          fl    - low frequency corner in rad/s
c          fh    - high frequency corner in rad/s
c          npole - number of poles in filter at each corner
c                  (not more than 20)
c
      integer*4 npole
      complex*8 sph(20),spl(20)
      complex*8 c0, c1,cjw
      complex*8 cph,cpl
c
c     include some useful stuff
c
c      include 'numerical.inc'
c***
c
c     include file for useful constants
c
c     pi = 3.14159265350
c
      pi = 4.*atan(1.)
      tpi = 2.0*pi
      pi2 = 0.5*pi
      pi4 = 0.25*pi
c
c     drad - degrees to radians
c
      drad = pi/180.
c
c     radd - radians to degrees
c
      radd = 180./pi
c
c     rmhz - radians/s to mhz
c
      rmhz = 1000./tpi
c
c     rad - mhz to radians/s
c
      rad = 1.0/rmhz
c
c     rn - earth radius in meters
c
      rn = 6371000.0
c
c     rnk - earth radius in km
c
      rnk = 6371.
c
c     dkm - degrees to km
c
      dkm = rnk*drad
c
      bigg = 6.6732e-11
      rhobar = 5515.0
c
      third = 1.0/3.0
      tthird = 2.0/3.0
      fthird = 4.0/3.0
c
c     for Barbara's mode scalings
c
      gn = pi*bigg*rhobar*rn
      vn2 = gn*rn
      vn = sqrt(vn2)
      wn = vn/rn
c
c     time - for counting seconds
c
      spmin = 60.
      sphr = 3600.
      spday = 86400.
c
c***
c
      nop = npole-2*(npole/2)
      nepp = npole/2
      c0 = cmplx(0.,0.)
      c1 = cmplx(1.,0.)
      w = f
      wch = fh
      wcl = fl
      np = 0
      if (w .eq. 0.0) then
        butter = c0
        return
      end if
      if (nop.gt.0) then
        np = np+1
        sph(np) = c1
      endif
      if (nepp.gt.0) then
        do i = 1, nepp
          ak = 2.*sin((2.*float(i)-1.)*pi/(2.*float(npole)))
          ar = 0.5*ak*wch
          ai = 0.5*wch*sqrt(4.-ak*ak)
          np = np+1
          sph(np) = cmplx(-ar,-ai)
          np = np+1
          sph(np) = cmplx(-ar,+ai)
        end do
      endif
      np = 0
      if (nop.gt.0) then
        np = np+1
        spl(np) = c1
      endif
      if (nepp.gt.0) then
        do i = 1, npole/2
          ak = 2.*sin((2.*float(i)-1.)*pi/(2.*float(npole)))
          ar = 0.5*ak*wcl
          ai = 0.5*wcl*sqrt(4.-ak*ak)
          np = np+1
          spl(np) = cmplx(-ar,-ai)
          np = np+1
          spl(np) = cmplx(-ar,+ai)
        end do
      endif
      cjw = cmplx(0.,-w)
      cph = c1
      cpl = c1
      do j = 1, npole
        cph = cph*sph(j)/(sph(j)+cjw)
        cpl = cpl*cjw/(spl(j)+cjw)
      end do
      butter = cph*cpl
c
      return
      end
