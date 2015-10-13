c	$Id: dresponse.f,v 1.2 2003/02/21 21:52:41 lombard Exp $
      complex*16 function dresponse(w,itype)
c
c     This is the double-precision version of response
c     The input frequency, poles and gain are real*4, the output is complex*16
c
c     resp calculates the actual response curve for a
c       given set of poles and zeros
c
c     the poles and zeros are expected to be stored in the
c       common block instr
c
c     w is circular frequency in rad/sec
c     itype specifies the desired response
c
c     itype = 0: default units
c             1: displacement response
c             2: velocity response
c             3: acceleration response
c
c     assumes the exp(-iwt) transform convention
c
      common /instr/ iunit, ds, gain, npoles, nzeros, poles
c
      complex*16 s, t
      complex*8 poles(60)
      real*8 a0
c
c     decide what to do
c
      if (itype .eq. 0) then
c
c       don't do anything
c
        iaction = 0
c
      else
c
c       figure it out
c
        iaction = iunit - (itype - 1)
c
      end if
c
c     check for zero divide
c
      if ((w .lt. 0.0001) .and. (iaction .ne. 0))then
        w = 0.0001
      endif
c
      s = dcmplx(0.d0,w)
c
c     scale the instrument response by the digitial sensitivity and gain
c
      a0 = ds * gain
      t = dcmplx(a0,0.d0)
c
c     loop over zeros
c
      do ii = 1, nzeros
        jj = npoles + ii
        t = t * (s - poles(jj))
      end do
c
c     loop over poles
c
      do ii = 1, npoles
        t = t / (s - poles(ii))
      end do
c
c     determine type of response
c
      if (iaction .eq. 0) then
        dresponse = t
      else
        dresponse = t * (s)**iaction
      endif
c
c     return
c
      return
      end
