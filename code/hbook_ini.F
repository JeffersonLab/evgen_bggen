      SUBROUTINE HBOOK_INI
C
C---       Initialize the HBOOK
C
C
      IMPLICIT NONE
C
      INTEGER mxpawc
      PARAMETER (mxpawc=10000000)
      COMMON/PAWC/ HMEM(mxpawc)
      REAL HMEM
C
C
      CALL HLIMIT(mxpawc)
C
      RETURN
      END

