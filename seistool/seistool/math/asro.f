c	$Id: asro.f,v 1.1 2001/12/21 18:39:07 lombard Exp $
      complex function asro (w)
c
c  .....ASRO LP response
c  .....KONO LP response
c
      complex*8 cz(4), cp(14), ck, s, t
c
c   .....Set poles and zeros.....
c
      data ck /(4.636600E+07,0.0)/
      data cz /( 0.000000E+00,  0.000000E+00),
     2         ( 0.000000E+00,  0.000000E+00),
     3         ( 0.000000E+00,  0.000000E+00),
     4         ( 0.000000E+00,  0.000000E+00)/
c
      data cp /( -0.209000E+00, 3.410000E-03),
     2         ( -0.209000E+00,-3.410000E-03),
     3         ( -0.129000E+00, 0.270000E+00),
     4         ( -0.129000E+00,-0.270000E+00),
     5         ( -5.950000E-02, 2.360000E-02),
     6         ( -5.950000E-02,-2.360000E-02),
     7         ( -0.159000E+00, 0.594000E+00),
     8         ( -0.159000E+00,-0.594000E+00),
     9         ( -0.856000E+00, 0.255000E+00),
     &         ( -0.856000E+00,-0.255000E+00),
     &         ( -0.541000E+00,-0.683000E+00),
     &         ( -0.541000E+00, 0.683000E+00),
     &         ( -0.630000E+00, 0.000000E+00),
     &         ( -8.850000E-02, 0.000000E+00)/
c
      s = cmplx(0.,w)
c
      t = ck
c
c     loop over common zeros and poles
c
      do ii = 1, 4
        t = t * ((s - cz(ii))/(s - cp(ii)))
      end do
c
c     loop over remaining poles
c
      do ii = 5, 14
        t = t / (s - cp(ii))
      end do
c
      asro = t
c
      return
      end
