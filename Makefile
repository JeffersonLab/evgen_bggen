#   This Makefile shows how to compile all C++, C and Fortran
#   files found in $(SRCDIR) directory.
#   Linking is done with g++. Need to have $CERN_ROOT and $ROOTSYS defined
###################################################################
MYOS     := $(shell uname)
ARCH     := $(shell arch)
USER     := $(shell whoami)
MYHOST   := $(shell hostname -s)
##################################################################
VERSION   = 1.0
TARGET    = bggen
EXECFILE  = bggen.$(MYOS).$(ARCH).exe

SRCDIR   := code
INCDIR   := code/include 

OTHERINC := #-I/work/halla/solid/jixie/sim-recon/Linux_RHEL6-x86_64-gcc4.4.7/include
OTHERLIBS := #-L/work/halla/solid/jixie/sim-recon/Linux_RHEL6-x86_64-gcc4.4.7/lib -lHDDM
###################################################################
TARGETLIB = $(TARGET)_v$(VERSION)
OBJDIR   := obj.${MYOS}.$(ARCH)
###################################################################
#in the darwin machines, ar cmd has something wrong!!!
#I switch to libtool 
#USAGE: libtool -static -o out.lib ObjList.o

AR       := ar r
ifeq ($(MYOS),Darwin)
AR       := libtool -static -o
endif
###################################################################
ifeq ($(ARCH),i686) 
####
MODE     := -m32
GPPFLAGS := -M
CXX      := g++
FF       := g77
CXXFLAGS := -Wall -O3 -g -Wno-deprecated  $(MODE) -I$(INCDIR) $(OTHERINC) 
CFLAGS   := -Wall -O3 -g  $(MODE) -I$(INCDIR) $(OTHERINC) 
FFLAGS   := -Wall -O3 -g  $(MODE) -I$(INCDIR) $(OTHERINC) 
FFLAGS   += -fno-automatic -finit-local-zero -fno-f2c \
 -fbounds-check -funroll-all-loops -fdollar-ok \
 -ffixed-line-length-none -fno-second-underscore \
 -Wunused -Wuninitialized 
LD       := g++
LDFLAGS  := -O3 -g $(MODE)  
SYSLIBS  := -lstdc++ 
else 
###################################################################
###take care of 64 bit compiler stuff, for -m64 use gfortran while g77 for -m32
###########ifeq ($(ARCH),x86_64)############
ifeq ($(COMPILER_BIT),32)
MODE     := -m32
FF       := g77
else
MODE     := -m64
FF       := gfortran
endif

GPPFLAGS := -M
CXX      := g++
CXXFLAGS := -Wall -O3 -g -Wno-deprecated  $(MODE) -I$(INCDIR) $(OTHERINC) 
CFLAGS   := -Wall -O3 -g  $(MODE) -I$(INCDIR) $(OTHERINC) 
FFLAGS   := -Wall -O3 -g  $(MODE) -I$(INCDIR) $(OTHERINC)  
FFLAGS   += -fno-automatic -finit-local-zero -fno-f2c \
 -fbounds-check -funroll-all-loops -fdollar-ok \
 -ffixed-line-length-none -fno-second-underscore \
 -Wunused -Wuninitialized 
LD       := g++
LDFLAGS  := -O3 -g $(MODE)
SYSLIBS  := -lstdc++   
endif
###################################################################
#you can specify the .SUFFIXES
.SUFFIXES: .C .c .CC .cc .CPP .cpp .CXX .cxx .F .f
.PHONY: all clean test
VPATH := $(SRCDIR):$(INCDIR):$(OTHERINC) 
###################################################################
#generate obj file list
FSOURCES  := $(wildcard $(SRCDIR)/*.[Ff])
CSOURCES  := $(wildcard $(SRCDIR)/*.[Cc])
CSOURCES  += $(wildcard $(SRCDIR)/*.[Cc][Cc])
CSOURCES  += $(wildcard $(SRCDIR)/*.[Cc][XxPp][XxPp])
SOURCES   := $(FSOURCES) $(CSOURCES)
#add .o to all the source files
#an example of replace/patrern replace statement
#######OBJS  = $(OBJS:.f=.o)#######
OBJS  := $(addsuffix .o, $(basename $(SOURCES)))
OBJS  := $(patsubst  $(SRCDIR)/%.o,$(OBJDIR)/%.o,$(OBJS))
#do not make dependence for fortran code
COBJS := $(addsuffix .o, $(basename $(CSOURCES)))
COBJS := $(patsubst  $(SRCDIR)/%.o,$(OBJDIR)/%.o,$(COBJS))
DEPS  := $(subst .o,.d,$(COBJS))
###################################################################
ifneq ($(FSOURCES),)
ifeq ($(FF),gfortran)
SYSLIBS   += -lgfortran 
else
SYSLIBS   += -lg2c 
endif
endif
###################################################################
ifndef ROOTSYS
$(error ROOTSYS is not set)
endif
ROOTCFLAGS   := $(shell root-config --cflags)
ROOTLIBS     := $(shell root-config --libs) -lMinuit
ROOTGLIBS    := $(shell root-config --glibs) -lMinuit

LIBS          = $(ROOTLIBS) $(SYSLIBS)
GLIBS         = $(ROOTGLIBS) $(SYSLIBS) 

ifneq ($(FSOURCES),)
ifdef CERN_ROOT
FFLAGS       += -I$(CERN_ROOT)/include 
GLIBS        += -L$(CERN_ROOT)/lib -lpdflib804 -lmathlib -lphtools \
 -lgeant321 -lpawlib -lgraflib -lgrafX11 -lpacklib -lkernlib 
else
$(error CERN_ROOT is not set)
endif
endif

CXXFLAGS     += $(ROOTCFLAGS) 
LDFLAGS      += $(GLIBS) $(OTHERLIBS) 
###################################################################

all: exe

# Make the $(TARGET).d file and include it.
$(OBJDIR)/%.d: %.c 
	@echo Making dependency for file $< ......
	@set -e;\
	$(CXX) $(GPPFLAGS) $(CXXFLAGS) -w $< |\
	sed 's!$*\.o!$(OBJDIR)/& $@!' >$@;\
	[ -s $@ ] || rm -f $@
$(OBJDIR)/%.d: %.C 
	@echo Making dependency for file $< ......
	@set -e;\
	$(CXX) $(GPPFLAGS) $(CXXFLAGS) -w $< |\
	sed 's!$*\.o!$(OBJDIR)/& $@!' >$@;\
	[ -s $@ ] || rm -f $@
$(OBJDIR)/%.d: %.cc
	@echo Making dependency for file $< ......
	@set -e;\
	$(CXX) $(GPPFLAGS) $(CXXFLAGS) -w $< |\
	sed 's!$*\.o!$(OBJDIR)/& $@!' >$@;\
	[ -s $@ ] || rm -f $@
$(OBJDIR)/%.d: %.cpp 
	@echo Making dependency for file $< ......
	@set -e;\
	$(CXX) $(GPPFLAGS) $(CXXFLAGS) -w $< |\
	sed 's!$*\.o!$(OBJDIR)/& $@!' >$@;\
	[ -s $@ ] || rm -f $@
$(OBJDIR)/%.d: %.cxx
	@echo Making dependency for file $< ......
	@set -e;\
	$(CXX) $(GPPFLAGS) $(CXXFLAGS) -w $< |\
	sed 's!$*\.o!$(OBJDIR)/& $@!' >$@;\
	[ -s $@ ] || rm -f $@
#there is a problem to build dependency for fortran code...
$(OBJDIR)/%.d: %.f 
	@echo Making dependency for file $< ......
	@set -e;\
	$(FF) $(GPPFLAGS) -cpp $(FFLAGS) -w  $< |\
	sed 's!$*\.o!$(OBJDIR)/& $@!' >$@;\
	[ -s $@ ] || rm -f $@ 
	@-rm -f $*.o 
$(OBJDIR)/%.d: %.F 
	@echo Making dependency for file $< ......
	@set -e;\
	$(FF) $(GPPFLAGS) -cpp $(FFLAGS) -w $< |\
	sed 's!$*\.o!$(OBJDIR)/& $@!' >$@;\
	[ -s $@ ] || rm -f $@ 
	@-rm -f $*.o 
ifneq ($(DEPS),)
-include $(DEPS)
endif

##########################################################

lib: $(OBJDIR) $(OBJS)
	@rm -f $(OBJDIR)/lib$(TARGETLIB).a
	@$(AR) $(OBJDIR)/lib$(TARGETLIB).a $(OBJS)
	@if [ -e $(OBJDIR)/lib$(TARGETLIB).a ] ; then echo "Finish creating target lib"  $(OBJDIR)/lib$(TARGETLIB).a ;fi
#	@libtool -static  -o $(OBJDIR)/lib$(TARGETLIB).a $(OBJS)
#	@ar r $(OBJDIR)/lib$(TARGETLIB).a $(OBJS)

exe: lib
#	@if [ ! -d Graph ]; then mkdir -p Graph; fi
	$(LD) -o $(EXECFILE) -L$(OBJDIR) -l$(TARGETLIB) $(LDFLAGS)
	@echo "Linking $(EXECFILE) ...done!"

##########################################################
$(OBJDIR)/%.o: %.F
	@echo Compiling $< ......
	@$(FF) -c $< -o $@  $(FFLAGS)

$(OBJDIR)/%.o: %.f
	@echo Compiling $< ......
	@$(FF) -c $< -o $@  $(FFLAGS)

$(OBJDIR)/%.o: %.c
	@echo Compiling $< ......
	@$(CXX) -c $< -o $@  $(CXXFLAGS)

$(OBJDIR)/%.o: %.C
	@echo Compiling $< ......
	@$(CXX) -c $< -o $@  $(CXXFLAGS)

$(OBJDIR)/%.o: %.cxx
	@echo Compiling $< ......
	@$(CXX) -c $< -o $@  $(CXXFLAGS)

$(OBJDIR)/%.o: %.cc
	@echo Compiling $< ......
	@$(CXX) -c $< -o $@  $(CXXFLAGS)

$(OBJDIR)/%.o: %.cpp
	@echo Compiling $< ......
	@$(CXX) -c $< -o $@  $(CXXFLAGS)

##########################################################
$(OBJDIR):
	@if [ ! -d $(OBJDIR) ] ; then mkdir -p $(OBJDIR) ;fi

clean:
	@rm -f $(OBJS) $(DEPS) $(OBJDIR)/lib$(TARGETLIB).a
	@rm -f *~ *# $(EXECFILE) */*~ */*#

test:	
	@echo \\MYOS\:$(MYOS) \\ARCH\:$(ARCH)
	@echo \\CFLAGS\:$(CFLAGS)	
	@echo \\CXXFLAGS\:$(CXXFLAGS)        
	@echo \\FFLAGS\:$(FFLAGS)
	@echo \\LDFLAGS\:$(LDFLAGS)
	@echo \\sources\: $(SOURCES)	
	@echo \\objs\: $(OBJS)	
	@echo \\dependencies: \$(DEPS)
	@echo \\SYSLIBS: \$(SYSLIBS)

help: test

##########################################################
print-%:
	@echo $* = $($*)
