c  	$Id: xcor.f,v 1.2 2005/02/11 17:17:10 lombard Exp $
	subroutine xcor(npts,dt,a,b,c,iere)
c
c     subroutine to cross-correlate timeseries a with timeseries b
c
c     input:  
c             npts   - number of points in timeseries power of 2
c             dt     - sample spacing in sec
c             a      - first input timeseries
c             b      - second input timeseries
c     output: 
c             c      - output cross-correlation between a and b
c                      c(w) = b(w)*conj(a(w))
c
c      include 'parameter.f'
c
      integer*4 id(6),nt2h
c
      real*4 a(*), b(*), c(*)
c      real*4 m0, m1, m2, centroid, sigma
      real*4 dw

c
c      complex*8 aw(*), bw(*), cw(*)
c
c      character*40 string
c 
c      logical lspec
c      logical lfilt, ltape, ldeci, linstr, ldecon, lderiv, lshift
c      logical ldfilt, ltrend, lneith, lhilb, lcoin, lsave
c
c      equivalence (aw,a), (bw,b), (cw,c)
c
	include 'fftw3.f'
	integer*8 plana,planb,planc
c
c     include some useful numerical constants
c
      include 'numerical.inc'
c
c     initialize error return
c
      iere = 0
c
      do ii = 1, 6
        id(ii) = 0
      end do
c
c     demean and taper a & b - apply a 5% taper
c
      call demean(a, npts)
c
      call taper(a, npts, 5.0)
c
      call demean(b, npts)
c
      call taper(b, npts, 5.0)
c
      n2 = ipow(npts)
c      if (n2 .gt. npmax) then
c        print*,' parameter npmax exceeded'
c        iere = 1
c        return
c      endif

c
c     forward transform
c
      ierr = 0
      call sfftw_plan_dft_r2c_1d(plana,n2,a,a,FFTW_ESTIMATE)
      call sfftw_execute(plana)
c      call fftl (a, n2, -1, dt, ierr)
c      if (ierr .ne. 0) then
c        iere = 2
c        print*, ' error in forward transform'
c      endif
c
      ierr = 0
      call sfftw_plan_dft_r2c_1d(planb,n2,b,b,FFTW_ESTIMATE)
      call sfftw_execute(planb)
c      call fftl (b, n2, -1, dt, ierr)
c      if (ierr .ne. 0) then
c        iere = 2
c        print*, ' error in forward transform'
c      endif
c
c     frequency increment
c
      dw = tpi / (float(n2) * dt)
      nt2h = (n2 + 2) / 2
c
c     loop to form cross-correlation function
c      --save the spectrum of the correlation function in bw
c      -- not done (I have no need for it now --
c
      scale = 1.0 / (n2 * n2)
      do ii = 1, npts, 2
	 c(ii) = (b(ii) * a(ii) + b(ii+1) * a(ii+1)) * scale
	 c(ii+1)=(b(ii+1) *a(ii) - b(ii) *a (ii+1)) * scale
c        bw(ii) = cw(ii)
      end do
      c(npts+1) = a(npts+1) * b(npts+1) * scale
      c(npts+2) = 0.0
c
c     estimate centroid and half-width of the spectral amplitude
c
c      m0 = 0.0
c      do ii = 1, nt2h
c        a1(ii) = sqrt(real(cw(ii))**2 + aimag(cw(ii))**2)
c        m0 = m0 + a1(ii)
c      end do
c      m0 = m0*dw
c      do ii = 1, nt2h
c        a1(ii) = a1(ii)/m0
c      end do
c      m0 = 0.0
c      m1 = 0.0
c      do ii = 1, nt2h
c        f = real(ii - 1)*dw
c        m0 = m0 + a1(ii)
c        m1 = m1 + a1(ii)*f
c      end do
c      centroid = m1/m0
c      m2 = 0.0
c      do ii = 1, nt2h
c        f = real(ii - 1)*dw
c        m2 = m2 + a1(ii)*(f - centroid)**2
c      end do
c      sigma = sqrt(m2/m0)
c
c     inverse transform cross-correlation function
c
      ierr = 0
      call sfftw_plan_dft_c2r_1d(planc,n2,c,c,FFTW_ESTIMATE)
      call sfftw_execute(planc)
      call sfftw_destroy_plan(plana)
      call sfftw_destroy_plan(planb)
      call sfftw_destroy_plan(planc)
c      call fftl (c, n2, -2, dt, ierr)
c      if (ierr .ne. 0) then
c        iere = 2
c        print*, ' error in inverse transform'
c      endif
c
c     make the correlation function symmetric
c
      ndum2 = npts/2
      do ii = 1, ndum2
        a(ii) = c(n2 - ndum2 + ii)
        a(ndum2 + ii) = c(ii)
      end do
      do ii = 1, npts
        c(ii) = a(ii)
      end do
c
c     all done
c
      return
      end
c
