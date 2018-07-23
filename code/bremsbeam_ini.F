C
      SUBROUTINE BREMSBEAM_INI(ID,E0,EP,ELIM)
C
C---   Photoproduction by the normal Brem. beam
C---   ID - histogram with the dN/dE*sigma(E), 
C           where dN/dE - Brem. Photon, sigma(E) - total photoprod. on protons 
C      E0 - e- energy in GeV
C      EP - coherent peak energy in GeV
C      ELIM - energy limits in GeV
C
      IMPLICIT NONE
      INTEGER ID
      REAL E0,EP,ELIM(2)
C
      INCLUDE 'bg_ctrl.inc'
C
      REAL GPXSECT
C
      INTEGER i,nb,ibrem
      REAL emn,emx,flx,xsec,dx,de,e,xstot,xlum,ecurr,targth,frate,vv
      REAL density

C parameters to compute EPA and BREMS Rakitha Fri Sep  4 11:37:06 EDT 2015
C DOUBLE PRECISION
      REAL pi,amp,ame,alpha
      REAL X,omega,e1
      REAL q2min,q2max,t1,t2,t3,res
      REAL N_omega,N_omega_EPA,N_omega_BREM
      REAL A,N_A,X_0,rad_len

C     for the ntuple following parameters are used
      CHARACTER*2 CHTAGS(5)
      DATA CHTAGS/'nk','xs','n1','n2','n3'/ 
      REAL EVENT(5)

      pi=DACOS(0.D0)*2.0
      alpha =1.0/137.036
      A = 1.0 !atomic weight
      N_A = 6.022E+23 !avogadro's number
      ame=0.511E-3
      amp=0.9383
 
C
C---calculate TRUE rad length and luminosity
C
C     X_0 = 63.0470;//g/cm**2 unit rad. length for H from Tsai RevModPhys.46.815
C     LH2 target rad.length is 0.071*40 g/cm2 where H density is 0.071 g/cm3 and lenght is 40 cm
C     by Jixie @ 20180417: use values from input file fort.15

      targth=TGPARA(1)         ! target thickness for PVDIS and SIDIS 40 cm for JPsi 15 cm for MOLLER 150 cm
      density=TGPARA(2)        ! target density in g/cm^3 
      X_0=TGPARA(3)            ! target material rad. length in g/cm^2
      ecurr=CURRENT*1.0E-6     ! turn beam current from uA to A: PVDIS (50 uA), JPsi (3 uA), SIDIS (15 uA) and MOLLER (85 uA)
C true thickness in unit of rad length
      rad_len=density*targth/X_0
C luminosity L= I/C * thickness*density/A *(p#+n#) *N_A, units are in inverse barns
C (p#+n#) is proton number + neutron number, it cancels with 1/A.  
      xlum=ecurr/1.6E-19*targth*density*0.6022 ! luminosity factor of neucleon (in [1/b] units) 

C
C end of rad length and luminosity

C
C---   Initialize the coherent Bremsstrahlung 
C
      ibrem=1
      IF(ELIM(2).LT.ELIM(1)*1.006) ibrem=0
C
! -- Jixie: COBREMS() will initilize the diamond parameters and ratation matrix, we do not need it
!           but it will not do anything bad here
!      IF(ibrem.NE.0) THEN
! --   subroutine COBREMS() needs all inputs in GeV or m
!         CALL COBREMS(E0,EP,ZCOLL/100.,DCOLL/100.,RADTHICK/100.) 
!      ENDIF
C
      emn=ELIM(1)
      emx=ELIM(2)
      de=MIN(0.005,emx-emn) !dk for integral over photon energy, to get more resolution in ntupes I can change this to 0.0005
      dx=de/E0
      nb=INT((emx-emn)/de+0.001)
      NHBEA=nb
C
      CALL HBOOK1(ID   ,'Beam flux dn/dE*sigma(E)' ,nb,emn,emx,0.)
C      CALL HBOOK1(ID   ,'Total N omega (EPA+Brems)' ,nb,emn,emx,0.)
      CALL HBOOK1(ID+10,'Total cross section in mb',nb,emn,emx,0.)
      CALL HBOOK1(ID+11,'Beam flux dn/dE'          ,nb,emn,emx,0.)
C     Create the ntuple parameters are, ID, name, no of leaves, leave empty to save to memory or else give a file name, intial event bugger and array of leaf names
      CALL HBOOKN(1234,'Ntuple for photoproduction',5,' ',1000,CHTAGS)
C
      frate=0.
      DO i=1,nb
         e=ELIM(1)+(i-0.5)*de
C Start N _omega calculation
         X=e/E0
         omega = X*E0               ! photon energy
         e1 = E0 - omega            ! electron energy after radiation
         res=0.D0
         N_omega_EPA=0.D0
         N_omega_BREM=0.D0
C ---- start (Equivalent Photon Approximation(EPA) calculation
C --  Jixie: details in V.M. Budnev et. al. Physics Reports 15, no.4 (1975) 181-282
         IF(e1.LE.0.) GO TO 100
         q2min = ame**2*omega**2/E0/e1 !Eq. 6.11, V.M. Budnev et. al. 
         q2max = 4.*E0*e1*SIN(pi/2.0*1.0)
         q2max=MIN(q2max,0.80) ! Q2max cut is 0.8 GeV
         t1=(1.0-omega/E0+(omega/E0)**2/2.0)*LOG(q2max/q2min)
         t2=((1.0-omega/E0/2.0)**2)*LOG((omega**2+q2max)/(omega**2+q2min))
         t3=ame**2*omega**2/E0**2/q2min*(1.0-q2min/q2max)
C
         res=alpha/pi*(t1-t2-t3)*de/e   !Eq. 6.17, V.M. Budnev et. al.,
 100     N_omega_EPA=res*e/de           !remove de/e, because N_omega_BREM also not include this 
C ---- end EPA calculation
C
C ---- start BREMS calculation
! --  Jixie: pdg booklet 2016 version,  Eq.33.30, but missing A/X0/N_A and de/e
!     Without coefficent 'A/X0/N_A' it is "units of photons per radiation length"
!     http://pdg.lbl.gov/2016/reviews/rpp2016-rev-passage-particles-matter.pdf
!     Use rad_len/2 here since effectively one half of the whole target will see brem. photon.
! --  Jixie: 20180722, added window thickness here to produce more Brem. photon
         N_omega_BREM = (WINTHICK+rad_len/2)*(4.0/3.0 - 4.0*X/3.0 + X**2)
C ---- end BREMS calculation
C
         xstot=GPXSECT(e)*1.E-3 ! x-sec in b
         IF(ibrem.NE.0) THEN
            flx=(N_omega_EPA + N_omega_BREM)*de/e ! dn_gamma(k) = n_gamma(x)dk/k
            vv=xstot*flx*xlum ! dsigma_ep(k) = sigma_gamma(k)*dn_gamma(k) and drate_ep = dsigma_ep(k)*xlum
            frate=frate+vv
            CALL HF1(ID+11,e,flx)
C     Fill the parameters for the ntuple
            EVENT(1)=e
            EVENT(2)=xstot
            EVENT(3)=N_omega_EPA
            EVENT(4)=N_omega_BREM
            EVENT(5)=N_omega_EPA + N_omega_BREM
C     Fill the ntuple
            CALL HFN(1234,EVENT)
         ELSE
            vv=1.
         ENDIF
         IF(ISNAN(N_omega_EPA) ) THEN
            N_omega = N_omega_BREM
         ELSE
            N_omega = N_omega_EPA  + N_omega_BREM
         ENDIF
C         N_omega = 1.
         CALL HF1(ID   ,e,vv) ! this is diamond cohbrems
C         CALL HF1(ID   ,e,N_omega) ! this is total N_omega 
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
C     All the histograms and ntuples in the memory will be saved to an hbook file
      Call hrput(0,'N_omega.hbk','N')
C
 999  RETURN
      END
