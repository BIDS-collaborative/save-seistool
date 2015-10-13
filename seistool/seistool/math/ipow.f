c	$Id: ipow.f,v 1.1 2001/12/21 18:39:07 lombard Exp $
      INTEGER*4 FUNCTION IPOW(N)
C
C  FINDS POWER OF 2 GREATER THAN N
C
      IN=2
    1 IN=IN*2
      IF(IN.GE.N) GO TO 2
      GO TO 1
    2 IPOW=IN
      RETURN
      END
