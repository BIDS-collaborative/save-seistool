c	$Id: demean.f,v 1.1 2001/12/21 18:39:07 lombard Exp $
      SUBROUTINE demean(A,N)
C
C $$$$$ CALLS NO OTHER ROUTINE $$$$$
C
C   DEMEAN REMOVES THE MEAN FROM THE N POINT SERIES STORED IN
C   ARRAY A.
C
C                                                    
      INTEGER*4 N,I
      DIMENSION A(*)
      XM=0.
      DO 1 I=1,N
 1    XM=XM+A(I)
      XM=XM/N
      DO 2 I=1,N
 2    A(I)=A(I)-XM
      RETURN
      END
