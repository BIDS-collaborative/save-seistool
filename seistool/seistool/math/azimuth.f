c	$Id: azimuth.f,v 1.1 2001/12/21 18:39:07 lombard Exp $
c
c  taken from Lind's utilities.
c  -- AY.
c
      subroutine azimth(slat,slon,rlat,rlon,delta,azim,bazim)
c
c   This routine uses Euler angles to find the geocentric 
c   distance, azimuth, and back azimuth for a source-reciever
c   pair.
c
c                      Input
c   slat  - source geographic latitude in decimal degrees
c   slon  - source longitude in decimal degrees
c   rlat  - receiver geographic latitude in decimal degrees
c   rlon  - receiver longitude in decimal degrees
c
c                     Output
c   delta - source-reciever distance in decimal degrees of arc
c   azim  - azimuth from the source to the reciever
c   bazim - back azimuth from the reciever to the source
c
c   Mark Riedesel, January 30, 1986
c
      data dtor/.0174532/,flt/298.25/
      e = 1./flt
c
c    convert to geocentric coordinates and from latitude to 
c    colatitude
c
      slatra = dtor*slat
      w = sin(slatra)
      s = ((2.-e)*w +4.*e*(w**3))*e*cos(slatra)
      scolat = 1.5707963 - slatra + s
c
      rlatra = dtor*rlat
      w = sin(rlatra)
      s = ((2.-e)*w +4.*e*(w**3))*e*cos(rlatra)
      rcolat = 1.5707963 - rlatra + s
c
      slonra=slon*dtor
      rlonra=rlon*dtor
      c2=cos(scolat)
      s2=sin(scolat)
      c1=cos(slonra)
      s1=sin(slonra)
      slatrc=sin(rcolat)
c
c  find the azimuth and distance by rotating the source to the
c  North pole

      x0=slatrc*cos(rlonra)
      y0=slatrc*sin(rlonra)
      z0=cos(rcolat)
      x1=c1*x0+s1*y0
      y1=-s1*x0+c1*y0
      z1=z0
      x2=c2*x1-s2*z1
      y2=y1
      z2=c2*z1+s2*x1
      call angles(x2,y2,z2,delta,azim)
      azim=180.-azim
c
c  find the back azimuth by rotating the reciever to the 
c  North pole
c
      c2=cos(rcolat)
      s2=sin(rcolat)
      c1=cos(rlonra)
      s1=sin(rlonra)
      slatrc=sin(scolat)
      x0=slatrc*cos(slonra)
      y0=slatrc*sin(slonra)
      z0=cos(scolat)
      x1=c1*x0+s1*y0
      y1=-s1*x0+c1*y0
      z1=z0
      x2=c2*x1-s2*z1
      y2=y1
      z2=c2*z1+s2*x1
      call angles(x2,y2,z2,delta,bazim)
      bazim=180.-bazim
      return
      end
      subroutine angles(x,y,z,theta,phi)
c
c  finds the angles theta and phi of a spherical polar coordinate
c  system from the cartesion coordinates x, y, and z.
c
c  Mark Riedesel, 1983
c
      parameter (pi=3.1415926,eps=1.e-4)
      rtod=180./pi
      arg1=sqrt(x*x+y*y)
      theta=atan2(arg1,z)
      if(abs(x).le.eps.and.abs(y).le.eps) then
         phi=0.
      else
         phi=atan2(y,x)
      end if
      phi=phi*rtod
      theta=theta*rtod
      return
      end
