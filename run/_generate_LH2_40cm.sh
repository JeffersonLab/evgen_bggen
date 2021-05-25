#!/bin/tcsh -f 
 setenv CERN_ROOT /apps/cernlib/x86_64_rhel7/2005
 source /group/solid/apps/root/root_v6.20.02.Linux-centos7-x86_64-gcc4.8/bin/thisroot.csh
 cp fort_LH.15 fort.15
 /w/halla-scifs17exp/solid/tianye/bggen_github/evgen_bggen/bggen.Linux.x86_64.exe
 h2root bggen.nt bggen_LH.root
 unsetenv CERN_ROOT
 unsetenv ROOTSYS
# source /work/halla/solid/tianye/solid_simulation/solid/set_solid 
source /group/solid/solid_github/JeffersonLab/solid_gemc/set_solid 1.3 /group/solid/apps/jlab_root /group/solid/solid_github/JeffersonLab/solid_gemc
 /w/halla-scifs17exp/solid/tianye/bggen_jixie/bggen/scripts/HallD_gen_lund bggen_LH.root 0 1 fileN
 rm bggen_LH.root
#cook
#ln -s $1 gen.lund
solid_gemc solid_fourpi.gcard -INPUT_GEN_FILE="LUND,hallD_LH_all_tracks_10k_fileN.lund" -N=1e4 -OUTPUT="evio,output.evio" -PRINT_EVENT=1 -USE_GUI=0
evio2root -INPUTF=output.evio -B="/group/solid/solid_github/JeffersonLab/solid_gemc/geometry/ec_segmented_moved/solid_PVDIS_ec_forwardangle"

