c	$Id: wwlp.f,v 1.1 2001/12/21 18:39:08 lombard Exp $
      complex function wwlp (w)
c
c  .....WWSSN LP - Blacknest specified poles and zeros.....
c
c  based on the sac subroutine wwspbn - Blacknest
c
      complex*8 cz(3), cp(4), ck, s, t
c
c   .....Set poles and zeros.....
c
      data ck /(0.5985275,0.0)/
      data cz /3*(0.,0.)/
      data cp /( -0.25700,  0.3376),
     2         ( -0.25700, -0.3376),
     3         ( -0.06283,  0.0),
     4         ( -0.06283,  0.0) /
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
      do ii = 4, 4
        t = t / (s - cp(ii))
      end do
c
      wwlp = t
c
      return
      end
