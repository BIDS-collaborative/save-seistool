c  	$Id: coinst.f,v 1.5 2005/02/11 17:17:10 lombard Exp $
	subroutine coinst(npts,dt,ind,ind2,c1,cw,fl,fh,iere)
	include 'fftw3.f'
c       
c       subroutine for Fourier Domain operations in process_waveform
c       
c       input:  
c       npts   - number of points in timeseries
c       dt     - sample spacing in sec
c       ind    - type of instrument operation to perform
c       1 - Benioff 100 kg
c       2 - 17s-23s 10 pole Butterworth
c       3 - Wood Anderson
c       4 - ULP instrument
c       5 - SP WWSSN instrument
c       6 - LP WWSSN instrument
c       7 - LP HGLP instrument
c       8 - Nominal STS1 response
c       9 - Benioff 14kg 
c       10 - ASRO LP response
c       11 - STS2 LP response
c       20 - deconvolve to displacement
c       21 - deconvolve to velocity
c       22 - deconvolve to acceleration
c       ind2  - type of filter operation to perform
c       1 - apply 6 pole Butterworth bandpass filter between fl and fh
c       2 - apply 6 pole Butterworth phaseless bandpass filter between fl and fh
c       3 - apply 6 pole Butterworth lowpass  filter with cutoff fl
c       4 - apply 6 pole Butterworth phaseless lowpass  filter with cutoff fl
c       5 - apply 6 pole Butterworth hipass   filter with cutoff fh
c       6 - apply 6 pole Butterworth phaseless hipass   filter with cutoff fh
c       c1    - input and output timeseries
c       cw    - input and output dummy array for storing spectra
c       fl    - input low  freq corner of Butterworth filters (input in Hz)
c       fh    - input high freq corner of Butterworth filters (input in Hz)
c       iere  - error return code
c       
	real*4 c1(*)
c       
	complex*16 dresponse, drw, daw
	complex*8 cw(*), aw
	complex*8 ben100, lpbptf_b, wood
	complex*8 ulp, wwsp, wwlp
	complex*8 hglp, sts1, ben14, asro, sts2
	complex*8 lobwth, hibwth, bpbwth
c       
	integer*4 ipow
	integer*8 planf, planb
c   
	common /instr/ iunit, ds, gain, npoles, nzeros, poles
	complex poles(60)
c       
c       compute useful numbers
c       
	pi  = 4.0*atan(1.0)
	tpi = 2.0*pi
	io = 6
c       
c       initialize error flag
c       
	iere = 0
c
ccccc lsg 06/17/97
c       
c       detrend and taper
c       
c	call detrend(c1, npts, dt, c1)
c
c       OR
c
c       demean and taper
c
 	call demean(c1, npts)
c
ccccc lsg
c	
c       
c       Seistool option
	call taper(c1, npts, 10.0)
c       
c       call taper(c1, npts, 5.0)
c       
c       compute the number of points
c       
c       Seistool option
        n2 = ipow(npts)
c       
c       n2 = ipow(npts*2)
c       
c       frequency increment & number of frequency samples
c       
	dw = tpi / (float(n2) * dt)
	nt2h = (n2 + 2) / 2
c       
c       zero excess points
c       
	do ii = npts + 1,  n2
	   c1(ii) = 0.0
	end do
c       
c       forward transform
c       
C Original FFT call
c	ierr = 0
c	call fftl (c1, n2, -1, dt, ierr)
c	if (ierr .ne. 0) then
c	   iere = 2
c	   write(io,*) 'Error in forward transform'
c	   return
c	endif
	call sfftw_plan_dft_r2c_1d(planf,n2,c1,cw,FFTW_ESTIMATE)
	call sfftw_execute(planf)
	
c       form complex array
c       
c	do ii = 1, nt2h
c	   jj = 2*(ii - 1) + 1
c	   cw(ii) = cmplx(c1(jj),c1(jj+1))
c	end do
c       
c       Perform instrument operations
c       
	itype = 1

c a0 is used only to test for real*4 overflow.
c But we need to provide more space in this test, since products
c of gain*ds times other things may overflow.

c This is a terrible kludge that hass caused to many headdaches.
c I am eliminating it, and using double precision for all cases; PNL 7/20/2003
c	a0= gain*ds* 1.0d12

c	if(ir_isnormal(a0).eq.0 .and. ind .ne. 0) then
c	   print*, 'DOUBLE PRECISION response used.'
c	endif

	if (ind .eq. 1) then
c       
c       convolve with the Benioff 100kg response
c       
	   cw(1) = (0.0,0.0)
c	   if (ir_isnormal(a0).eq.1) then	
c	      do ii = 2, nt2h
c		 f = float(ii - 1)*dw
c		 rw = response(f, itype)
c		 aw = ben100(f)    
c		 cw(ii) = cw(ii) * (aw/rw)
c	      end do
c	      write(io,*) 'Benioff'
c	   else
	      do ii = 2, nt2h
		 f = float(ii - 1)*dw
		 drw = dresponse(f, itype)
		 daw = dcmplx(ben100(f))    
		 cw(ii) = cmplx(cw(ii) * (daw/drw))
	      end do
c	      write(io,*) 'Benioff'
c	   endif
	elseif (ind .eq. 2) then
c       
c       convolve with a 20 s filter
c       
	   cw(1) = (0.0,0.0)
c	   if (ir_isnormal(a0).eq.1) then	
c	      do ii = 2, nt2h 
c		 f = float(ii - 1)*dw
c		 rw = response(f, itype)
c		 aw = lpbptf_b(f)
c		 cw(ii) = cw(ii)*(aw/rw)
c	      end do
c	   else
	      do ii = 2, nt2h
		 f = float(ii - 1)*dw
		 drw = dresponse(f, itype)
		 daw = dcmplx(lpbptf_b(f))
		 cw(ii) = cmplx(cw(ii) * (daw/drw))
	      end do
c	   endif

	elseif (ind .eq. 3) then
c       
c       convolve with the Wood Anderson response
c       
	   cw(1) = (0.0,0.0)
c	   if (ir_isnormal(a0).eq.1) then	
c	      do ii = 2, nt2h 
c		 f = float(ii - 1)*dw
c		 rw = response(f, itype)
c		 aw = wood(f)    
c		 cw(ii) = cw(ii)*(aw/rw)
c	      end do
c	   else
	      do ii = 2, nt2h
		 f = float(ii - 1)*dw
		 drw = dresponse(f, itype)
		 daw = dcmplx(wood(f))
		 cw(ii) = cmplx(cw(ii) * (daw/drw))
	      end do
c	   endif
	elseif (ind .eq. 4) then
c       
c       convolve with the ULP response
c       
	   cw(1) = (0.0,0.0)
c	   if (ir_isnormal(a0).eq.1) then	
c	      do ii = 2, nt2h
c		 f = float(ii - 1)*dw
c		 rw = response(f, itype)
c		 aw = ulp(f)
c		 cw(ii) = cw(ii)*(aw/rw)
c	      end do
c	   else
	      do ii = 2, nt2h
		 f = float(ii - 1)*dw
		 drw = dresponse(f, itype)
		 daw = dcmplx(ulp(f))
		 cw(ii) = cmplx(cw(ii) * (daw/drw))
	      end do
c	   endif
	elseif (ind .eq. 5) then
c       
c       convolve with the SP WWSSN response
c       
	   cw(1) = (0.0,0.0)
c	   if (ir_isnormal(a0).eq.1) then	
c	      do ii = 2, nt2h
c		 f = float(ii - 1)*dw
c		 rw = response(f, itype)
c		 aw = wwsp(f)
c		 cw(ii) = cw(ii)*(aw/rw)
c	      end do
c	   else
	      do ii = 2, nt2h
		 f = float(ii - 1)*dw
		 drw = dresponse(f, itype)
		 daw = dcmplx(wwsp(f))
		 cw(ii) = cmplx(cw(ii) * (daw/drw))
	      end do
c	   endif
	elseif (ind .eq. 6) then
c       
c       convolve with the LP WWSSN response
c       
	   cw(1) = (0.0,0.0)
c	   if (ir_isnormal(a0).eq.1) then	
c	      do ii = 2, nt2h
c		 f = float(ii - 1)*dw
c		 rw = response(f, itype)
c		 aw = wwlp(f)
c		 cw(ii) = cw(ii)*(aw/rw)
c	      end do
c	   else
	      do ii = 2, nt2h
		 f = float(ii - 1)*dw
		 drw = dresponse(f, itype)
		 daw = dcmplx(wwlp(f))
		 cw(ii) = cmplx(cw(ii) * (daw/drw))
	      end do
c	   endif
	elseif (ind .eq. 7) then
c       
c       convolve with the LP HGLP response
c       
	   cw(1) = (0.0,0.0)
c	   if (ir_isnormal(a0).eq.1) then	
c	      do ii = 2, nt2h
c		 f = float(ii - 1)*dw
c		 rw = response(f, itype)
c		 aw = hglp(f)
c		 cw(ii) = cw(ii)*(aw/rw)
c	      end do
c	   else
	      do ii = 2, nt2h
		 f = float(ii - 1)*dw
		 drw = dresponse(f, itype)
		 daw = dcmplx(hglp(f))
		 cw(ii) = cmplx(cw(ii) * (daw/drw))
	      end do
c	   endif
	elseif (ind .eq. 8) then
c       
c       convolve with the nominal STS1 response
c       
	   cw(1) = (0.0,0.0)
c	   if (ir_isnormal(a0).eq.1) then	
c	      do ii = 2, nt2h
c		 f = float(ii - 1)*dw
c		 rw = response(f, itype)
c		 aw = sts1(f)
c		 cw(ii) = cw(ii)*(aw/rw)
c	      end do
c	   else
	      do ii = 2, nt2h
		 f = float(ii - 1)*dw
		 drw = dresponse(f, itype)
		 daw = dcmplx(sts1(f))
		 cw(ii) = cmplx(cw(ii) * (daw/drw))
	      end do
c	   endif
	elseif (ind .eq. 9) then
c       
c       convolve with a Benioff 14 kg
c       
	   cw(1) = (0.0,0.0) 
c	   if (ir_isnormal(a0).eq.1) then	
c	      do ii = 2, nt2h 
c		 f = float(ii - 1)*dw 
c		 rw = response(f, itype) 
c		 aw = ben14(f)    
c		 cw(ii) = cw(ii)*(aw/rw) 
c	      end do 
c	   else
	      do ii = 2, nt2h
		 f = float(ii - 1)*dw
		 drw = dresponse(f, itype)
		 daw = dcmplx(ben14(f))
		 cw(ii) = cmplx(cw(ii) * (daw/drw))
	      end do
c	   endif
	elseif (ind .eq. 10) then
c       
c       convolve with a ASRO LP response
c       
	   cw(1) = (0.0,0.0)
c	   if (ir_isnormal(a0).eq.1) then	
c	      do ii = 2, nt2h
c		 f = float(ii - 1)*dw
c		 rw = response(f, itype)
c		 aw = asro(f)
c		 cw(ii) = cw(ii)*(aw/rw)
c	      end do
c	   else
	      do ii = 2, nt2h
		 f = float(ii - 1)*dw
		 drw = dresponse(f, itype)
		 daw = dcmplx(asro(f))
		 cw(ii) = cmplx(cw(ii) * (daw/drw))
	      end do
c	   endif
	elseif (ind .eq. 11) then
c       
c       convolve with a STS2 LP response
c       
	   cw(1) = (0.0,0.0)
c	   if (ir_isnormal(a0).eq.1) then	
c	      do ii = 2, nt2h
c		 f = float(ii - 1)*dw
c		 rw = response(f, itype)
c		 aw = sts2(f)
c		 cw(ii) = cw(ii)*(aw/rw)
c	      end do
c	   else
	      do ii = 2, nt2h
		 f = float(ii - 1)*dw
		 drw = dresponse(f, itype)
		 daw = dcmplx(sts2(f))
		 cw(ii) = cmplx(cw(ii) * (daw/drw))
	      end do
c	   endif
	elseif (ind .gt. 11 .and. ind .lt. 20) then
	   write(io,*) 'Non-existent response'
	   iere = 1
	   return
c       
c       remove the instrument only - no reconvolution
c       
	elseif (ind .eq. 20) then
c       
c       remove instrument to displacement
c       
	   itype = 1
	   cw(1) = (0.0,0.0)
c	   if (ir_isnormal(a0).eq.1) then	
c	      do ii = 2, nt2h
c		 f = float(ii - 1)*dw
c		 rw = response(f, itype)
c		 cw(ii) = cw(ii) / rw
c	      end do
c	      write(io,*) 'Displacement'
c	   else
	      do ii = 2, nt2h
		 f = float(ii - 1)*dw
		 drw = dresponse(f, itype)
		 cw(ii) = cmplx(cw(ii) / drw)
	      end do
c	      write(io,*) 'Displacement'
c	   endif
	elseif (ind .eq. 21) then 
c       
c       remove instrument to velocity
c       
	   itype = 2 
	   cw(1) = (0.0,0.0) 
c	   if (ir_isnormal(a0).eq.1) then	
c	      do ii = 2, nt2h 
c		 f = float(ii - 1)*dw 
c		 rw = response(f, itype) 
c		 cw(ii) = cw(ii) / rw 
c	      end do 
c	      write(io,*) 'Velocity'
c	   else
	      do ii = 2, nt2h
		 f = float(ii - 1)*dw
		 drw = dresponse(f, itype)
		 cw(ii) = cmplx(cw(ii) / drw)
	      end do
c	      write(io,*) 'Velocity'
c	   endif

	elseif (ind .eq. 22) then 
c       
c       remove instrument to acceleration
c       
	   itype = 3 
	   cw(1) = (0.0,0.0) 
c	   if (ir_isnormal(a0).eq.1) then	
c	      do ii = 2, nt2h 
c		 f = float(ii - 1)*dw 
c		 rw = response(f, itype) 
c		 cw(ii) = cw(ii) / rw 
c	      end do 
c	      write(io,*) 'Acceleration'
c	   else
	      do ii = 2, nt2h
		 f = float(ii - 1)*dw
		 drw = dresponse(f, itype)
		 cw(ii) = cmplx(cw(ii) / drw)
	      end do
c	      write(io,*) 'Acceleration'
c	   endif

	elseif (ind .ge. 23) then
	   write(io,*) 'Unknown option'
	   iere = 1
	   return
	end if
c       
c       Check for filter operations
c       
	wl = fl*tpi
	wh = fh*tpi
	if (ind2 .eq. 0) then
c	   write(io,*) 'No filtering'
	elseif (ind2 .eq. 1) then
c       
c       apply a 6 pole bandpass Butterworth filter
c       
	   do ii = 1, nt2h
	      f = float(ii - 1)*dw
	      cw(ii) = cw(ii)*bpbwth(f, wl, wh)
	   end do
c	   write(io,*) "Band Pass causal"
	elseif (ind2 .eq. 2) then
c       
c       apply a 6 pole bandpass phaseless Butterworth filter
c       
	   do ii = 1, nt2h
	      f = float(ii - 1)*dw
	      aw = bpbwth (f, wl, wh) * conjg(bpbwth (f, wl, wh))
	      cw(ii) = cw(ii)*aw
	   end do
c	   write(io,*) "Band Pass non-causal"
	elseif (ind2 .eq. 3) then
c       
c       apply a 6 pole lowpass Butterworth filter
c       
	   do ii = 1, nt2h
	      f = float(ii - 1)*dw
	      cw(ii) = cw(ii)*lobwth(f, wl)
	   end do
c	   write(io,*) "Low Pass causal"
	elseif (ind2 .eq. 4) then
c       
c       apply a 6 pole lowpass phaseless Butterworth filter
c       
	   do ii = 1, nt2h  
	      f = float(ii - 1)*dw
	      aw = lobwth (f, wl) * conjg(lobwth (f, wl))
	      cw(ii) = cw(ii)*aw
	   end do    
c	   write(io,*) "Low Pass non-causal"
	elseif (ind2 .eq. 5) then
c       
c       apply a 6 pole hipass Butterworth filter
c       
	   do ii = 1, nt2h
	      f = float(ii - 1)*dw
	      cw(ii) = cw(ii)*hibwth(f, wh)
	   end do
c	   write(io,*) "high Pass causal"
	elseif (ind2 .eq. 6) then  
c       
c       apply a 6 pole hipass phaseless Butterworth filter
c       
	   do ii = 1, nt2h
	      f = float(ii - 1)*dw
	      aw = hibwth (f, wh) * conjg(hibwth (f, wh))
	      cw(ii) = cw(ii)*aw
	   end do
c	   write(io,*) "high Pass non-causal"
	elseif (ind2 .ge. 7) then
	   write(io,*) 'Unknown option'
	   iere = 1
	   return
	end if
c       
c       reform the complex array into a real
c With the fftw package, we need to scale the data instead of moving it
c       
	scale = 1.0/n2
	do ii = 1, nt2h 
c	   jj = 2*(ii - 1) + 1 
c	   c1(jj) = real(cw(ii))
c	   c1(jj+1) = aimag(cw(ii))
	   cw(ii) = cw(ii) * scale
	end do 
c       
c       inverse transform filtered function
c       
c	ierr = 0
c	call fftl (c1, n2, -2, dt, ierr)
c	if (ierr .ne. 0) then
c	   iere = 2
c	   write(io,*) 'Error in forward transform'
c	   return
c	endif
	call sfftw_plan_dft_c2r_1d(planb,n2,cw,c1,FFTW_ESTIMATE)
	call sfftw_execute(planb)
	call sfftw_destroy_plan(planb)
	call sfftw_destroy_plan(planf)
c       
c       all done
c       
	return
	end
c       
