      SUBROUTINE ITSUM(D, G, IV, LIV, LV, P, V, X)
C
C  ***  PRINT ITERATION SUMMARY FOR ***SOL (VERSION 2.3)  ***
C
C  ***  PARAMETER DECLARATIONS  ***
C
      INTEGER LIV, LV, P
      INTEGER IV(LIV)
      REAL D(P), G(P), V(LV), X(P)
C
C+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
C
C  ***  LOCAL VARIABLES  ***
C
      INTEGER ALG, I, IV1, M, NF, NG, OL, PU
C/6S
C     REAL MODEL1(6), MODEL2(6)
C/7S
      CHARACTER*4 MODEL1(6), MODEL2(6)
C/
      REAL NRELDF, OLDF, PRELDF, RELDF, ZERO
C
C  ***  NO EXTERNAL FUNCTIONS OR SUBROUTINES  ***
C
C  ***  SUBSCRIPTS FOR IV AND V  ***
C
      INTEGER ALGSAV, DSTNRM, F, FDIF, F0, NEEDHD, NFCALL, NFCOV, NGCOV,
     1        NGCALL, NITER, NREDUC, OUTLEV, PREDUC, PRNTIT, PRUNIT,
     2        RELDX, SOLPRT, STATPR, STPPAR, SUSED, X0PRT
C
C  ***  IV SUBSCRIPT VALUES  ***
C
C/6
C     DATA ALGSAV/51/, NEEDHD/36/, NFCALL/6/, NFCOV/52/, NGCALL/30/,
C    1     NGCOV/53/, NITER/31/, OUTLEV/19/, PRNTIT/39/, PRUNIT/21/,
C    2     SOLPRT/22/, STATPR/23/, SUSED/64/, X0PRT/24/
C/7
      PARAMETER (ALGSAV=51, NEEDHD=36, NFCALL=6, NFCOV=52, NGCALL=30,
     1           NGCOV=53, NITER=31, OUTLEV=19, PRNTIT=39, PRUNIT=21,
     2           SOLPRT=22, STATPR=23, SUSED=64, X0PRT=24)
C/
C
C  ***  V SUBSCRIPT VALUES  ***
C
C/6
C     DATA DSTNRM/2/, F/10/, F0/13/, FDIF/11/, NREDUC/6/, PREDUC/7/,
C    1     RELDX/17/, STPPAR/5/
C/7
      PARAMETER (DSTNRM=2, F=10, F0=13, FDIF=11, NREDUC=6, PREDUC=7,
     1           RELDX=17, STPPAR=5)
C/
C
C/6
C     DATA ZERO/0.E+0/
C/7
      PARAMETER (ZERO=0.E+0)
C/
C/6S
C     DATA MODEL1(1)/4H    /, MODEL1(2)/4H    /, MODEL1(3)/4H    /,
C    1     MODEL1(4)/4H    /, MODEL1(5)/4H  G /, MODEL1(6)/4H  S /,
C    2     MODEL2(1)/4H G  /, MODEL2(2)/4H S  /, MODEL2(3)/4HG-S /,
C    3     MODEL2(4)/4HS-G /, MODEL2(5)/4H-S-G/, MODEL2(6)/4H-G-S/
C/7S
      DATA MODEL1/'    ','    ','    ','    ','  G ','  S '/,
     1     MODEL2/' G  ',' S  ','G-S ','S-G ','-S-G','-G-S'/
C/
C
C-------------------------------  BODY  --------------------------------
C
      PU = IV(PRUNIT)
      IF (PU .EQ. 0) GO TO 999
      IV1 = IV(1)
      IF (IV1 .GT. 62) IV1 = IV1 - 51
      OL = IV(OUTLEV)
      ALG = MOD(IV(ALGSAV)-1,2) + 1
      IF (IV1 .LT. 2 .OR. IV1 .GT. 15) GO TO 370
      IF (IV1 .GE. 12) GO TO 120
      IF (IV1 .EQ. 2 .AND. IV(NITER) .EQ. 0) GO TO 390
      IF (OL .EQ. 0) GO TO 120
      IF (IV1 .GE. 10 .AND. IV(PRNTIT) .EQ. 0) GO TO 120
      IF (IV1 .GT. 2) GO TO 10
         IV(PRNTIT) = IV(PRNTIT) + 1
         IF (IV(PRNTIT) .LT. IABS(OL)) GO TO 999
 10   NF = IV(NFCALL) - IABS(IV(NFCOV))
      IV(PRNTIT) = 0
      RELDF = ZERO
      PRELDF = ZERO
      OLDF = AMAX1( ABS(V(F0)),  ABS(V(F)))
      IF (OLDF .LE. ZERO) GO TO 20
         RELDF = V(FDIF) / OLDF
         PRELDF = V(PREDUC) / OLDF
 20   IF (OL .GT. 0) GO TO 60
C
C        ***  PRINT SHORT SUMMARY LINE  ***
C
         IF (IV(NEEDHD) .EQ. 1 .AND. ALG .EQ. 1) WRITE(PU,30)
 30   FORMAT(/10H   IT   NF,6X,1HF,7X,5HRELDF,3X,6HPRELDF,3X,5HRELDX,
     1       2X,13HMODEL  STPPAR)
         IF (IV(NEEDHD) .EQ. 1 .AND. ALG .EQ. 2) WRITE(PU,40)
 40   FORMAT(/11H    IT   NF,7X,1HF,8X,5HRELDF,4X,6HPRELDF,4X,5HRELDX,
     1       3X,6HSTPPAR)
         IV(NEEDHD) = 0
         IF (ALG .EQ. 2) GO TO 50
         M = IV(SUSED)
         WRITE(PU,100) IV(NITER), NF, V(F), RELDF, PRELDF, V(RELDX),
     1                 MODEL1(M), MODEL2(M), V(STPPAR)
         GO TO 120
C
 50      WRITE(PU,110) IV(NITER), NF, V(F), RELDF, PRELDF, V(RELDX),
     1                 V(STPPAR)
         GO TO 120
C
C     ***  PRINT LONG SUMMARY LINE  ***
C
 60   IF (IV(NEEDHD) .EQ. 1 .AND. ALG .EQ. 1) WRITE(PU,70)
 70   FORMAT(/11H    IT   NF,6X,1HF,7X,5HRELDF,3X,6HPRELDF,3X,5HRELDX,
     1       2X,13HMODEL  STPPAR,2X,6HD*STEP,2X,7HNPRELDF)
      IF (IV(NEEDHD) .EQ. 1 .AND. ALG .EQ. 2) WRITE(PU,80)
 80   FORMAT(/11H    IT   NF,7X,1HF,8X,5HRELDF,4X,6HPRELDF,4X,5HRELDX,
     1       3X,6HSTPPAR,3X,6HD*STEP,3X,7HNPRELDF)
      IV(NEEDHD) = 0
      NRELDF = ZERO
      IF (OLDF .GT. ZERO) NRELDF = V(NREDUC) / OLDF
      IF (ALG .EQ. 2) GO TO 90
      M = IV(SUSED)
      WRITE(PU,100) IV(NITER), NF, V(F), RELDF, PRELDF, V(RELDX),
     1             MODEL1(M), MODEL2(M), V(STPPAR), V(DSTNRM), NRELDF
      GO TO 120
C
 90   WRITE(PU,110) IV(NITER), NF, V(F), RELDF, PRELDF,
     1             V(RELDX), V(STPPAR), V(DSTNRM), NRELDF
 100  FORMAT(I6,I5,E10.3,2E9.2,E8.1,A3,A4,2E8.1,E9.2)
 110  FORMAT(I6,I5,E11.3,2E10.2,3E9.1,E10.2)
C
 120  IF (IV1 .LE. 2) GO TO 999
      I = IV(STATPR)
      IF (I .EQ. (-1)) GO TO 460
      IF (I + IV1 .LT. 0) GO TO 460
      GO TO (999, 999, 130, 150, 170, 190, 210, 230, 250, 270, 290, 310,
     1       330, 350, 500),  IV1
C
 130  WRITE(PU,140)
 140  FORMAT(/26H ***** X-CONVERGENCE *****)
      GO TO 430
C
 150  WRITE(PU,160)
 160  FORMAT(/42H ***** RELATIVE FUNCTION CONVERGENCE *****)
      GO TO 430
C
 170  WRITE(PU,180)
 180  FORMAT(/49H ***** X- AND RELATIVE FUNCTION CONVERGENCE *****)
      GO TO 430
C
 190  WRITE(PU,200)
 200  FORMAT(/42H ***** ABSOLUTE FUNCTION CONVERGENCE *****)
      GO TO 430
C
 210  WRITE(PU,220)
 220  FORMAT(/33H ***** SINGULAR CONVERGENCE *****)
      GO TO 430
C
 230  WRITE(PU,240)
 240  FORMAT(/30H ***** FALSE CONVERGENCE *****)
      GO TO 430
C
 250  WRITE(PU,260)
 260  FORMAT(/38H ***** FUNCTION EVALUATION LIMIT *****)
      GO TO 430
C
 270  WRITE(PU,280)
 280  FORMAT(/28H ***** ITERATION LIMIT *****)
      GO TO 430
C
 290  WRITE(PU,300)
 300  FORMAT(/18H ***** STOPX *****)
      GO TO 430
C
 310  WRITE(PU,320)
 320  FORMAT(/44H ***** INITIAL F(X) CANNOT BE COMPUTED *****)
C
      GO TO 390
C
 330  WRITE(PU,340)
 340  FORMAT(/37H ***** BAD PARAMETERS TO ASSESS *****)
      GO TO 999
C
 350  WRITE(PU,360)
 360  FORMAT(/43H ***** GRADIENT COULD NOT BE COMPUTED *****)
      IF (IV(NITER) .GT. 0) GO TO 460
      GO TO 390
C
 370  WRITE(PU,380) IV(1)
 380  FORMAT(/14H ***** IV(1) =,I5,6H *****)
      GO TO 999
C
C  ***  INITIAL CALL ON ITSUM  ***
C
 390  IF (IV(X0PRT) .NE. 0) WRITE(PU,400) (I, X(I), D(I), I = 1, P)
 400  FORMAT(/23H     I     INITIAL X(I),8X,4HD(I)//(1X,I5,E17.6,E14.3))
C     *** THE FOLLOWING ARE TO AVOID UNDEFINED VARIABLES WHEN THE
C     *** FUNCTION EVALUATION LIMIT IS 1...
      V(DSTNRM) = ZERO
      V(FDIF) = ZERO
      V(NREDUC) = ZERO
      V(PREDUC) = ZERO
      V(RELDX) = ZERO
      IF (IV1 .GE. 12) GO TO 999
      IV(NEEDHD) = 0
      IV(PRNTIT) = 0
      IF (OL .EQ. 0) GO TO 999
      IF (OL .LT. 0 .AND. ALG .EQ. 1) WRITE(PU,30)
      IF (OL .LT. 0 .AND. ALG .EQ. 2) WRITE(PU,40)
      IF (OL .GT. 0 .AND. ALG .EQ. 1) WRITE(PU,70)
      IF (OL .GT. 0 .AND. ALG .EQ. 2) WRITE(PU,80)
      IF (ALG .EQ. 1) WRITE(PU,410) IV(NFCALL), V(F)
      IF (ALG .EQ. 2) WRITE(PU,420) IV(NFCALL), V(F)
 410  FORMAT(/6H     0,I5,E10.3)
 420  FORMAT(/6H     0,I5,E11.3)
      GO TO 999
C
C  ***  PRINT VARIOUS INFORMATION REQUESTED ON SOLUTION  ***
C
 430  IV(NEEDHD) = 1
      IF (IV(STATPR) .LE. 0) GO TO 460
         OLDF = AMAX1( ABS(V(F0)),  ABS(V(F)))
         PRELDF = ZERO
         NRELDF = ZERO
         IF (OLDF .LE. ZERO) GO TO 440
              PRELDF = V(PREDUC) / OLDF
              NRELDF = V(NREDUC) / OLDF
 440     NF = IV(NFCALL) - IV(NFCOV)
         NG = IV(NGCALL) - IV(NGCOV)
         WRITE(PU,450) V(F), V(RELDX), NF, NG, PRELDF, NRELDF
 450  FORMAT(/9H FUNCTION,E17.6,8H   RELDX,E17.3/12H FUNC. EVALS,
     1   I8,9X,11HGRAD. EVALS,I8/7H PRELDF,E16.3,6X,7HNPRELDF,E15.3)
C
 460  IF (IV(SOLPRT) .EQ. 0) GO TO 999
         IV(NEEDHD) = 1
         IF (IV(ALGSAV) .GT. 2) GO TO 999
         WRITE(PU,470)
 470  FORMAT(/22H     I      FINAL X(I),8X,4HD(I),10X,4HG(I)/)
         DO 480 I = 1, P
 480          WRITE(PU,490) I, X(I), D(I), G(I)
 490     FORMAT(1X,I5,E16.6,2E14.3)
      GO TO 999
C
 500  WRITE(PU,510)
 510  FORMAT(/24H INCONSISTENT DIMENSIONS)
 999  RETURN
C  ***  LAST CARD OF ITSUM FOLLOWS  ***
      END