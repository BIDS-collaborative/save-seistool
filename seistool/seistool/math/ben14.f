c	$Id: ben14.f,v 1.2 2013/02/28 21:24:55 lombard Exp $
      complex function ben14(w)
c
c     compute 14kg Benioff Transfer Function
c 
c     w is angular frequency in rad/s
c
      complex*8 cz(5), cp(14), ck, s, t
c
c     poles and zero of Benioff response
c
      data ck /(1.29181E+24,0.0)/
      data cz /5*(0.,0.)/
      data cp /(-.371674E+01, .000000E+00),
     1         (-.141722E+02,-.117587E+02),
     1         (-.141722E+02, .117587E+02),
     1         (-.219911E+02,-.224355E+02),
     1         (-.219911E+02, .224355E+02),
     1         (-.162621E+02,-.606909E+02),
     1         (-.162621E+02, .606909E+02),
     1         (-.444288E+02,-.444288E+02),
     1         (-.444288E+02, .444288E+02),
     1         (-.606909E+02,-.162621E+02),
     1         (-.606909E+02, .162621E+02),
     1         (-.444288E+02,-.444288E+02),
     1         (-.444288E+02, .444288E+02),
     1         (-.628319E-01, .000000E+00)/
c
      s = cmplx(0.,w)
c
      t = ck
c
c     loop over common zeros and poles
c
      do ii = 1, 5
        t = t * ((s - cz(ii))/(s - cp(ii)))
      end do
c
c     loop over remaining poles
c
      do ii = 6, 14
        t = t / (s - cp(ii))
      end do
c
      ben14 = t
c
      return
      end
