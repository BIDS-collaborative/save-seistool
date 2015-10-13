c	$Id: taper.f,v 1.1 2001/12/21 18:39:08 lombard Exp $
      SUBROUTINE TAPER(A,LEN,PER)
C
C  TAPER INPUT SERIES WITH A PER% COS**2 TAPER
C  PER=100 IS A FULL TAPER
C
      real*4 PER
      real*4 A(*)
c
      FL=FLOAT(LEN)
      FP=PER*.005
      FLS=FP*FL
      FLM=FL-FLS
      DO 1 I=1,LEN
    1 A(I)=A(I)*BPCOS(FLOAT(I),0.,FLS,FLM,FL)
      a(1) = 0.0
      a(len) = 0.0
      RETURN
      END
