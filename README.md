# evgen_bggen

bggen is the Hall D photoproduction code that Rakitha Beminiwattha <rakithab@jlab.org> imported for 
SoLID background simulations. The modifications were made to allow electroproduction event generation 
with this code. For now bggen can only produce events for proton target.

Jixie Zhang (jixie@jlab.org) changed it to be compiled and run in 64-bit mode. Jixie also made
some improvements as below.

1) remove Hall-D software dependence
2) self create random seed if given seed is zero
3) add beam current and target parameters into input file
4) add radiator thickness into input file
5) enable coherence brem. only if radiator thickness > 1.0E-6
6) clean up code, add more comment and make it look consistent.

Check HISTORY.txt for details.

==============================================================
# To compile this program:

Note that CERN & ROOT must be installed before compilation.

## setup evn on jlab ifarm
    setenv CERN_ROOT /apps/cernlib/x86_64_rhel7/2005
    source /group/solid/apps/root/root_v6.20.02.Linux-centos7-x86_64-gcc4.8/bin/thisroot.csh
    ('h2root' from root v5.28 or v6.10 and up can convert the hbook file into root file in the above. root v5.30 - v5.34
    can not)
    
## To compile use Makefile 

    make
    bggen.Linux_x86_64.exe will be created.

## To compile use CMake on jlab ifarm: (need more instruction to configure env)
    source setup
    mkdir build && cd build
    cmake ../code
    make
    bggen will be created.
    
==============================================================

# To run this program:

All input files have been placed in run dir.

    cd run
    modify input file "fort.15"
    (just run ./go_bggen.csh or the line below)
    ../bggen.Linux_x86_64.exe 
    h2root bggen.nt bggen.root
    h2root bggen.hit bggen.his.root    
    
==============================================================

# To analyze the output:

    Use scripts/HallD_LH_xs.cc and scripts/HallD_gen_lund.cc to
    generate LUND files and kinematics plots.

    for example
    root scripts/HallD_gen_lund.cc bggen.root targ_index   nLundEntries_index    fFile_index
    
    refer to 
    TString stgt[__TARGETS]={"H1","D","He","H2","H3","H4"};//PVDIS H and D, SIDIS He, J/psi, SIDIS Proton, MOLLER proton
    TString stgt_lundfile_stem[__TARGETS]={"LH","LD","3He","JPsi","SIDIS_LH","MOLLER"};//PVDIS H and D, SIDIS He, J/psi, SIDIS Proton, MOLLER proton    
    
==============================================================

# old repo

 at first, it was in https://github.com/JeffersonLab/remoll_solid/tree/master/generators/halld/
 refer README.Rakitha and README.HallD
 
 Then it was in https://github.com/jixie/bggen, before it was moved the current location at JeffersonLab/evgen_bggen with all history preserved on 2020/11/02
