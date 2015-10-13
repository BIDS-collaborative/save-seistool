c	$Id: sts1.f,v 1.1 2001/12/21 18:39:08 lombard Exp $
      complex function sts1 (w)
c
c  .....10 HZ STS1 nominal response
c  .....Displacement response for MHC HHZ
c
      complex*8 cz(3), cp(4), ck, s, t
c
c   .....Set poles and zeros.....
c
      data ck /(4.18033E+12,0.0)/
      data cz /( 0.000000E+00,  0.000000E+00),
     2         ( 0.000000E+00,  0.000000E+00),
     3         ( 0.000000E+00,  0.000000E+00)/
c
      data cp /(-1.234122E-02,  1.234146E-02),
     2         (-1.234122E-02, -1.234146E-02),
     3         (-3.917566E+01,  4.912341E+01),
     4         (-3.917566E+01, -4.912341E+01)/
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
      sts1 = t
c
      return
      end
