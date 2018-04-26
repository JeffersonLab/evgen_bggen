# bggeb
bggen is the Hall D photoproduction code that Rakitha Beminiwattha <rakithab@jlab.org> imported for 
SoLID background simulations. The modifications are made to allow electroproduction event generation 
with this code. For now bggen can only produce events for proton target only.

Jixie Zhang (jixie@jlab.org) changed it to be compiled and run in 64-bit mode. Jixie also made
some improvments as below.

1) remove Hall-D software dependence
2) self create random seed if given seed is zero
3) add beam current and target parameters into input file
4) add radiator thickness into input file
5) enable coherence brem. only if ratiator thickness> 1.0E-6
6) clean up code, add more comment and make it looks cinsistant.

####################################################
Note that CERN & ROOT must be installed before compilation.

To compile use Makefile:
    make;

To compile use CMake:    
    mkdir build && cd build
    cmake ../code
    make


####################################################
To Running the code:
All input files have been placed in run dir. 
    cd run;
    ./bggen

####################################################
To analyze the output:
    h2root bggen.nt bggen.root
    h2root bggen.hit bggen.his.root

    Use scripts/HallD_LH_xs.cc and scripts/HallD_gen_lund.cc to generate LUND files and
    kinematics plots.
    
