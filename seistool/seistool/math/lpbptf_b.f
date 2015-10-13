c	$Id: lpbptf_b.f,v 1.1 2001/12/21 18:39:07 lombard Exp $
      complex function lpbptf_b(f)
c
c ...... compute 17-23 sec Butterworth 10PBP Transfer Function
c
      complex*8 citf
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
      ns = 10
      wc2 = tpi/17.
      wc3 = 1.1*wc2
      wc1 = tpi/23.
      wc0 = wc1/1.1
      wc = f
c
      if(wc.le.wc0) then
        citf = cmplx(0.,0.)
      elseif(wc.lt.wc1) then
        arg = pi*(wc-wc0)/(wc1-wc0)
        q = 0.5*(1.-cos(arg))
        citf = cmplx(q,0.)
      elseif(wc.le.wc2) then
        citf = cmplx(1.,0.)
      elseif(wc.lt.wc3) then
        arg = pi*(wc3-wc)/(wc3-wc2)
        q = 0.5*(1.-cos(arg))
        citf = cmplx(q,0.)
      else
        citf = cmplx(0.,0.)
      endif
c
      lpbptf_b = citf 
c
      return
      end
