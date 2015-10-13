c	$Id: spectrum.f,v 1.2 2005/02/11 17:17:10 lombard Exp $
c stolen from gfs
c
c lsoph- sophisticated mode
c
      subroutine spectrum (c1, y, aa, bb, npts, dt, lsoph)
      include 'fftw3.f'
      real*4 c1(*), y(*), aa(*), bb(*), dt
      integer*4 npts, lsoph
      integer*8 planf
c
c     compute useful numbers
c
      pi = 4.*atan(1.)
      tpi = 2.0*pi
      pi2 = 0.5*pi
      pi4 = 0.25*pi
c
      if (lsoph.eq.1) then
c
c     detrend and taper 10%
c
c     call detrend(c1, npts, dt, c1)
c
c     actually demean and taper
c
      call demean(c1,npts)
      call taper(c1, npts, 5.0)
      endif
c
c     determine the power of 2 for fourier transform
c

      n2 = ipow(npts)
c
c     pad the array with zeros out to n2 points 
c
      do ii = npts + 1,  n2
        c1(ii) = 0.0
      end do
c
c     frequency increment
c
      dw = tpi / (float(n2) * dt)
c
c     number of points in spectrum
c
      nt2h = (n2 + 2) / 2
c
c     print*,' n2, dw, nt2h:  ', n2, dw, nt2h
c
c     forward transform - (-iwt) forward convention
c
      ierr = 0
      call sfftw_plan_dft_r2c_1d(planf,n2,c1,c1,FFTW_ESTIMATE)
      call sfftw_execute(planf)
c      call fftl (c1, n2, -1, dt, ierr)
c      if (ierr .ne. 0) then
c        iere = 2
c        print*, 'error in forward transform'
c      endif

      jj = 0
      scale = 1.0 / n2
      do ii = 2, nt2h
        jj = jj + 1 
        kk = 2*(ii - 1) + 1
        a = c1(kk) * scale
        b = c1(kk+1) * scale
	aa(jj)= a
	bb(jj)= b
        y(jj) = sqrt(a**2 + b**2)
      enddo

      return
      end


c plot
c        jj = 0
c        do ii = 2, nt2h
c          tau = tpi/(float(ii-1)*dw)
c          if (tau .le. tmax .and. tau .ge. tmin) then
c            jj = jj + 1 
c            a = real(cw(ii))
c            b = aimag(cw(ii))
c            if (llin) then
c              y1(jj) = sqrt(a**2 + b**2)
c              x(jj)  = tau
c            else  
c              y1(jj) = alog10(sqrt(a**2 + b**2))
c              x(jj)  = alog10(tau)
c            end if
c            y2(jj) = atan2(b,a)
c          endif
c        end do
