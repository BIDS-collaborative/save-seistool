c	$Id: wwsp.f,v 1.1 2001/12/21 18:39:08 lombard Exp $
      complex function wwsp (w)
c
c .....WWSSN SP - Blacknest specified poles and zeros.....
c
c  based on the sac subroutine wwspbn - Blacknest
c
      complex*8 cz(3), cp(5), ck, s, t
c
c   .....Set poles and zeros.....
c
      data ck /(432.83395,0.0)/
      data cz /3*(0.,0.)/
      data cp /( -4.04094,  6.47935 ),
     2         ( -4.04094, -6.47935 ),
     3         ( -9.25238,  0.0 ),
     4         ( -7.67430,  0.0 ),
     5         ( -16.72981, 0.0 )/
c
      s = cmplx(0.,w)
c
      t = ck
c
c     loop over common zeros and poles
c
      do ii = 1, 3
        t = t * ((s - cz(ii))/(s - cp(ii)))
      end do
c
c     loop over remaining poles
c
      do ii = 4, 5
        t = t / (s - cp(ii))
      end do
c
      wwsp = t
c
      return
      end
