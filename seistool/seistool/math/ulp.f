c	$Id: ulp.f,v 1.1 2001/12/21 18:39:08 lombard Exp $
      complex function ulp(w)
c
c     compute BKS ultra-long period Transfer Function
c 
c     w is angular frequency in rad/s
c
      complex*8 cz(2),cp(7), ck, s, t
c
c     poles and zero of Benioff response
c
      data ck /(9.46127E+17,0.)/
      data cz /2*(0.,0.)/
      data cp / (-4.398230E-02, 4.487092E-02),
     2          (-4.398230E-02,-4.487092E-02),
     3          (-4.854028E+01,-1.493916E+02),
     4          (-4.854028E+01, 1.493916E+02),
     5          (-1.270801E+02,-9.232909E+01),
     6          (-1.270801E+02, 9.232909E+01),
     7          (-1.570796E+02, 0.000000E+00)/
c
      s = cmplx(0.,w)
c
      t = ck
c
c     loop over common zeros and poles
c
      do ii = 1, 2
        t = t * ((s - cz(ii))/(s - cp(ii)))
      end do
c
c     loop over remaining poles
c
      do ii = 3, 7
        t = t / (s - cp(ii))
      end do
c
      ulp = t
c
      return
      end
