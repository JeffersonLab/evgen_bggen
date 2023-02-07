# Makefile for QwPromptSummary.cc  Code and read_run_range.cc code.
# (modified version of $(ROOTSYS)/test/Makefile)
#

#Tue Nov 26 11:16:00 EST 2013         imported from Qweak scripts to use in SoLID : Rakitha S Beminiwattha

ObjSuf        = o
SrcSuf        = cc
ExeSuf        =
DllSuf        = so
EVENTLIB      = $(EVENTO)
OutPutOpt     = -o  

ROOTCFLAGS    = $(shell root-config --cflags)
ROOTLIBS      = $(shell root-config --libs)   -lNew  -lMinuit
ROOTGLIBS     = $(shell root-config --glibs)


MYOS := $(subst -,,$(shell uname))

CPP_FILES := $(wildcard src/*.cpp)
OBJ_FILES := $(addprefix obj/,$(notdir $(CPP_FILES:.cpp=.o)))


EXE := HallD_gen_lund

# Linux with egcs	
#ifeq ($(MYOS),Linux)
	CXX           = g++ 
	CXXFLAGS      = -O2 -Wall -fPIC 
#-I$(G0ANALYSIS)/Analysis/include
	LD            = g++
	LDFLAGS       = -O
	SOFLAGS       = -shared

	CXXFLAGS     += $(ROOTCFLAGS)
	CXXFLAGS     += -I./include
	LIBS          = $(ROOTLIBS)
#-L$(G0ANALYSIS)/lib -lG0 -lmyevio -L$(CODA_LIB) -let
	GLIBS         = $(ROOTGLIBS)
#endif

all:   $(EXE) 
	@echo "---------------------> $(EXE) done <---------------------- "

HallD_gen_lund:HallD_gen_lund.o
	$(CXX) $(CXXFLAGS)  $(LIBS) $(GLIBS) -o $@ $<


%.o: %.C  %.cc 
	$(CXX) -c  $(CXXFLAGS)   -o $@ $<

obj/%.o: src/%.cpp
	 $(CXX)  $(CXXFLAGS) -c -o $@ $<

#depend .depend : $(SRCS)
#	$(CXX) $(CXXFLAGS) -M $(MAIN)  >./.depend

clean:
	@rm -rf *.o
	@rm -rf *~
	@rm -rf  core

cleanall:
	@rm -f $(OBJS) $(EXE) *~  core

.PHONY : clean cleanall all
