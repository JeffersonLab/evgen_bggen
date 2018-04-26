# bggeb

bggen is the Hall D photoproduction code that Rakitha Beminiwattha <rakithab@jlab.org> imported for 
SoLID background simulations. The modifications were made to allow electroproduction event generation 
with this code. For now bggen can only produce events for proton target.

Jixie Zhang (jixie@jlab.org) changed it to be compiled and run in 64-bit mode. Jixie also made
some improvments as below.

1) remove Hall-D software dependence
2) self create random seed if given seed is zero
3) add beam current and target parameters into input file
4) add radiator thickness into input file
5) enable coherence brem. only if ratiator thickness> 1.0E-6
6) clean up code, add more comment and make it looks cinsistant.

Check HISTORY.txt for details.

==============================================================
# To compile this program:

Note that CERN & ROOT must be installed before compilation.

## To compile use Makefile:

    make;
    bggen.Linux_x86_64.exe will be created.

## To compile use CMake:

    mkdir build && cd build
    cmake ../code
    make
    bggen will be created.
    
==============================================================

# To run this program:

All input files have been placed in run dir.

    cd run;
    ../bggen.Linux_x86_64.exe 

==============================================================

# To analyze the output:

    h2root bggen.nt bggen.root
    h2root bggen.hit bggen.his.root

    I happened to find that 'h2root' from root v5.28 or v6.10 and up can
    convert the hbook file into root file in the above. root v5.30 - v5.34
    can not.
     
    Use scripts/HallD_LH_xs.cc and scripts/HallD_gen_lund.cc to
    generate LUND files and kinematics plots.

    
