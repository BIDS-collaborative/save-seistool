c	$Id: response.f,v 1.1 2001/12/21 18:39:08 lombard Exp $
      complex function response(w,itype)
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
      complex s, t
      complex poles(60)
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
      if ((w .eq. 0.) .and. (iaction .ne. 0))then
        w = .0001
      endif
c
      s = cmplx(0.0,w)
c
c     scale the instrument response by the digitial sensitivity and gain
c
      a0 = ds * gain
      t = cmplx(a0,0.0)
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
        response = t
      else
        response = t * (s)**iaction
      endif
c
c     return
c
      return
      end
