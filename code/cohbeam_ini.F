C
      SUBROUTINE COHBEAM_INI(ID,E0,EP,ELIM)
C
C---   Photoproduction by the coherent Brem. beam
C---   ID - histogram with the dN/dE*sigma(E), 
C              where dN/dE - coh. Brem., sigma(E) - total photoprod. on protons 
C      E0 - e- energy in GeV
C      EP - coherent peak energy  in GeV
C      ELIM - energy limits  in GeV
C
      IMPLICIT NONE
      INTEGER ID
      REAL E0,EP,ELIM(2)
C
      INCLUDE 'bg_ctrl.inc'
C
      REAL DNIDX,DNCDX,GPXSECT
C
      INTEGER i,nb,ibrem
      REAL emn,emx,flx,xsec,dx,de,e,xstot,xlum,ecurr,targth,frate,vv
      REAL density
      REAL A,X_0
C      
      A = 1.0 !atomic weight
C
C
C---calculate TRUE rad length and luminosity
C
C     X_0 = 63.0470;//g/cm**2 unit rad. length for H from Tsai RevModPhys.46.815
C     LH2 target rad.length is 0.071*40 g/cm2 where H density is 0.071 g/cm3 and lenght is 40 cm
C     by Jixie @ 20180417: use values from input file fort.15

      targth=TGPARA(1)         ! target thickness in cm (30)
      density=TGPARA(2)        ! target density in g/cm^3  (0.071)
      X_0=TGPARA(3)            ! target material rad. length in g/cm^2 (63.0470)
      ecurr=CURRENT*1.0E-6     ! turn beam current from uA to A (2.25uA)
C
C luminosity L= I/C * thickness*density*N_A/A and N_A/A = 0.6022 units are in inverse barns 
      xlum=ecurr/1.6E-19*targth*density*0.6022/A ! luminosity factor for LH2 target: (in [1/b] units) 

C
C end of rad length and luminosity

C
C---   Initialize the coherent Bremsstrahlung 
C
 
      ibrem=1
      IF(ELIM(2).LT.ELIM(1)*1.006) ibrem=0
C
      IF(ibrem.NE.0) THEN
C --  subroutine COBREMS() needs all inputs in GeV or m
         CALL COBREMS(E0,EP,ZCOLL/100.,DCOLL/100.,RADTHICK/100.) 
      ENDIF
C
      emn=ELIM(1)
      emx=ELIM(2)
      de=MIN(0.005,emx-emn)
      dx=de/E0
      nb=INT((emx-emn)/de+0.001)
      NHBEA=nb
C
      CALL HBOOK1(ID   ,'Beam flux dn/dE*sigma(E)' ,nb,emn,emx,0.)
      CALL HBOOK1(ID+10,'Total cross section in mb',nb,emn,emx,0.)
      CALL HBOOK1(ID+11,'Beam flux dn/dE'          ,nb,emn,emx,0.)
C
      frate=0.
      DO i=1,nb
         e=ELIM(1)+(i-0.5)*de
         xstot=GPXSECT(e)*1.E-3 ! x-sec in b
         IF(ibrem.NE.0) THEN
            flx=(DNIDX(e/E0)+DNCDX(e/E0))*dx/de
            vv=xstot*flx*xlum
            frate=frate+vv*de
            CALL HF1(ID+11,e,flx)
         ELSE
            vv=1.
         ENDIF
         CALL HF1(ID   ,e,vv)
         CALL HF1(ID+10,e,xstot)
      ENDDO
C      CALL HPRINT(ID)
C
      RATESEC=0.
      IF(ibrem.NE.0) THEN
         RATESEC=frate
         WRITE(6,FMT='(//10X,''Rates:'')')
         WRITE(6,1000) ecurr*1.E6,emn,emx
 1000    FORMAT(10X,'Beam: ',F8.2,' uA e-, gamma in ',2F6.2,' GeV') 
         WRITE(6,1005) targth
 1005    FORMAT(10X,'Target: ',F8.2,' cm LH2')
         WRITE(6,1010) frate/1000.
 1010    FORMAT(10X,'Interaction rate: ',F8.1,' kHz')
      ENDIF 
C
      CALL HCOPY(ID,ID+1,'    ')  ! a copy of the final histogram to be used for HRNDM1
      Call hrput(0,'cohbrems.hbk','N')
C
 999  RETURN
      END
