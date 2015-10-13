c	$Id: ben100.f,v 1.1 2001/12/21 18:39:07 lombard Exp $
      complex function ben100(w)
c
c     compute 100kg Benioff Transfer Function
c 
c     w is angular frequency in rad/s
c
      complex*8 cz(5),cp(9), ck, s, t
c
c     poles and zero of Benioff response
c
      data ck /(2.87910E+14,0.)/
      data cz /5*(0.,0.)/
      data cp /(-0.403300E+01, 0.637000E+01),
     1         (-0.403300E+01,-0.637000E+01),
     2         (-0.180300E+02, 0.000000E+00),
     3         (-0.444200E-01, 0.444400E-01),
     4         (-0.444200E-01,-0.444400E-01),
     5         (-0.111100E+03, 0.111100E+03),
     6	   (-0.111100E+03,-0.111100E+03),
     7         (-0.841500E+01, 0.357000E+00),
     8         (-0.841500E+01,-0.357000E+00)/
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
      do ii = 6, 9
        t = t / (s - cp(ii))
      end do
c
      ben100 = t
c
      return
      end
