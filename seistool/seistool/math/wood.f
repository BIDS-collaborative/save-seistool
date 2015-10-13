c	$Id: wood.f,v 1.1 2001/12/21 18:39:08 lombard Exp $
      complex function wood(w)
c
c     compute nominal Wood Anderson response
c 
c     w is angular frequency in rad/s
c
      complex*8 cz(2),cp(2), ck, s, t
c
c     poles and zero of Wood Anderson response
c     --from the file seisp.stf
c
      data ck /(2.08000E+06,0.)/
      data cz /2*(0.,0.)/
      data cp /(-0.541925E+01, -0.568479E+01),
     1         (-0.541925E+01,  0.568479E+01)/
c
      s = cmplx(0.,w)
c
      t = ck
c
c     loop over zeros and poles
c
      do ii = 1, 2
        t = t * ((s - cz(ii))/(s - cp(ii)))
      end do
c
      wood = t
c
      return
      end
