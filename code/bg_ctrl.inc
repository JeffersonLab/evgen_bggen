C
C--- Flags to control the job
C
C
      INTEGER    MXPGEANT,MXPKC
      PARAMETER (MXPGEANT=1000,MXPKC=1000)
C
      COMMON/PHCTRL/ NEVENT,RUNNO,EPH_LIM(2),EELEC,EPEAK,EPYMIN,ELOWMIN
     +              ,ZCOLL,DCOLL,RADTHICK
     +              ,IWROUT(3),LUNWR(3),IRND_SEQ,NPRIEV
     +              ,IDBEAM,NHBEA
     +              ,IFPYTH
     +              ,IDLOWEN
     +              ,IPLUND(MXPGEANT),IDECLUND(MXPGEANT)
     +              ,KCGEAN(-MXPKC:MXPKC)
     +              ,RATESEC
     +              ,VERTEX(3)
     +              ,CURRENT
     +              ,TGPARA(3)
      INTEGER   NEVENT    ! number of events to simulate
     +         ,RUNNO     ! run number to use
     +         ,IWROUT    ! (1)>0 - write out the HDDS file (events)
C                           (2)>0 - write out a sequential file
C                           (3)>0 - write out an ntuple file
     +         ,LUNWR     ! (1) LUN for HDDS file
C                           (2) LUN for the sequential file
C                           (3) LUN for the ntuple file
     +         ,IRND_SEQ  ! the random number seed (each integer number gives a different sequence)
C                           Added By Jixie: if =0, will use process id and system time to form this seed
     +         ,NPRIEV    ! number of events to print
     +         ,IDBEAM    ! histogram ID for the beam (=0 - fixed energy)
     +         ,NHBEA     ! number of bins in IDBEAM
     +         ,IFPYTH    ! PYTHIA is used
     +         ,IDLOWEN   ! <>0 - starting ID of histograms for the low energy generator
      INTEGER   IPLUND    ! PYTHIA particle codes (KF) 
     +         ,IDECLUND  !  =0 - forbid the decays of this particle in PYTHIA
      INTEGER   KCGEAN    ! GEANT code for the PYTHIA internal code KC (with sign)
      REAL      EPH_LIM   ! limits on the photon beam energy GeV
     +         ,EELEC     ! electron beam energy
     +         ,EPEAK     ! energy of the coherent peak (the right edge) 
     +         ,EPYMIN    ! minimal energy for PYTHIA,
C                           below that the low energy generator is used
C                           the value may be adjusted to the bin boundary of IDBEA
     +         ,ELOWMIN   ! minimal energy for the low energy generator 
     +         ,ZCOLL     ! distance to the collimator (cm)
     +         ,DCOLL     ! collimator diameter (cm), Jixie changed it from m to cm to be consistant
     +         ,RADTHICK  ! Added By Jixie: radiator thichness in cm 
     +         ,RATESEC   ! reference interation rate (Hz), calculated
     +         ,VERTEX    ! Vertex set in HDDM output file (cm), 0,0,0 is the default 
C                           CURRENT and array TGPARA(3) added by Jixie @ 20180417
     +         ,CURRENT   ! electron beam current in uA   
     +         ,TGPARA    ! (1) target length in cm
C                           (2) target density in g/cm^3
C                           (3) target material rad. length in g/cm^2, not true thickness
