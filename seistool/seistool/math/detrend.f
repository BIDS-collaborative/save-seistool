c	$Id: detrend.f,v 1.1 2001/12/21 18:39:07 lombard Exp $
      subroutine detrend (xx, n, dt, yy)
c
c     remove the best-fitting line from a set of time-points
c
      real*4 xx(*),yy(*)
c
c     sx = 0.0
c     sxx = 0.0
c     sy = 0.0
c     sxy = 0.0
c     s = float(n)
c     do i = 1, n
c       x = (i - 1)*dt
c       sx = sx + x
c       sxx = sxx + x**2
c       sy = sy + xx(i)
c       sxy = sxy + xx(i)*x
c     end do
c     delta = s*sxx - sx**2
c     a = (sxx*sy - sx*sxy)/delta
c     b = (s*sxy - sx*sy)/delta
c
c     apparently this is numerically unstable; let's try another approach
c
      sx = 0.0
      sy = 0.0
      s = float(n)
      do i = 1, n
        x = (i - 1)*dt
        sx = sx + x
        sy = sy + xx(i)
      end do
      sxos = sx/s
      st2 = 0.0
      b = 0.0
      do i = 1, n
        x = (i - 1)*dt
        t = x - sxos
        st2 = st2 + t*t
        b = b + t * xx(i)
      end do
      b = b/st2
      a = sy/s - sxos*b
c
c     if (abs(b) .gt. 0.00001) then
c       print*,' removing linear trend (int and slope): ', a, b
c     endif
c
      do i = 1, n
        x = (i - 1)*dt
        yy(i) = xx(i) - a - b * x
      end do
c
      return
      end
