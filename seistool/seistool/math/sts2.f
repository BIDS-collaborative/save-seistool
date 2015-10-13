c	$Id: sts2.f,v 1.1 2001/12/21 18:39:08 lombard Exp $
      complex function sts2 (w)
c
c  .....STS2 nominal response
c  .....Displacement response for WDC HHZ
c
      complex*8 cz(3), cp(5), ck, s, t
c
c   .....Set poles and zeros.....
c
      data ck /(2.92692E+16,0.0)/
      data cz /( 0.000000E+00,  0.000000E+00),
     2         ( 0.000000E+00,  0.000000E+00),
     3         ( 0.000000E+00,  0.000000E+00)/
c
      data cp /(-3.702367E-02,  3.702438E-02),
     2         (-3.702367E-02, -3.702438E-02),
     3         (-1.186336E+02,  4.230651E+02),
     4         (-1.186336E+02, -4.230651E+02),
     5         (-2.513274E+02,  0.000000E+00)/
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
      sts2 = t
c
      return
      end
