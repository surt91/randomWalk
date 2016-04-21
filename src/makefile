TARGET	= randomWalk

DOC 	= manual.pdf

CPP	 := main.cpp \
        Cmd.cpp \
        Benchmark.cpp \
        Walker.cpp \
        LatticeWalker.cpp \
        LoopErasedWalker.cpp \
        SelfAvoidingWalker.cpp \
        RealWalker.cpp \
        Simulation.cpp \
        Metropolis.cpp \
        WangLandau.cpp \
        FastWangLandau.cpp \
        RNG.cpp \
        Histogram.cpp \
        DensityOfStates.cpp \
        io.cpp \
        Svg.cpp \
        Povray.cpp \
        stat.cpp

OBJ	 = $(CPP:%.cpp=obj/%.o)
DEP	 = $(CPP:%.cpp=dep/%.d)

QHULL = qhull/lib/libqhullcpp.a qhull/lib/libqhullstatic_r.a

CXX	= g++
#CXX	= clang++
#CXX	= icpc

#COPT = -g
COPT = -O3 -flto -mtune=native -fomit-frame-pointer
#COPT += -DNLOG
COPT += -fopenmp
COPT += -std=c++11 -m64 -ffast-math -fno-strict-aliasing -fexceptions -pipe -fdiagnostics-color

WARNLEVEL= -Wall -Wextra -pedantic

LNDIRS  = -L$(PWD)/lib \
          -L$(PWD)/qhull/lib

INCLUDES = -I$(PWD)/include \
           -I$(PWD)/qhull/src \
           -I$(PWD)/qhull/src/libqhullcpp

CXXFLAGS = $(COPT) $(INCLUDES)

LIBS = -lqhullcpp -lqhullstatic_r -lm
LFLAGS	= $(LNDIRS) $(LIBS)

all: $(DEP) $(TARGET)

.DELETE_ON_ERROR:
.PHONY: clean proper

MAKEFILE_TARGETS_WITHOUT_INCLUDE := clean proper
ifeq ($(filter $(MAKECMDGOALS),$(MAKEFILE_TARGETS_WITHOUT_INCLUDE)),)
-include $(DEP)
endif

obj dep:
	mkdir $@

# perfect dependencies, see https://www.gnu.org/software/make/manual/make.pdf
dep/%.d: %.cpp | dep
	@echo Dep: $@
	@set -e; rm -f $@; \
	$(CXX) $(WARNLEVEL) $(CXXFLAGS) -MM $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,obj/\1.o $@: ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

obj/%.o: %.cpp | obj
	$(CXX) -c $(WARNLEVEL) $(CXXFLAGS) $< -o $@

$(QHULL):
	$(MAKE) -j1 -C qhull cleanall
	$(MAKE) -j1 -C qhull lib/libqhullstatic_r.a lib/libqhullcpp.a

$(TARGET): $(OBJ) $(QHULL)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJ) $(LFLAGS)

doc: $(DOC)
$(DOC): doc.conf *.cpp *.hpp
	doxygen doc.conf
	$(MAKE) -C doc/latex clean
	$(MAKE) -C doc/latex
	cp doc/latex/refman.pdf $@

proper:
	rm -rf $(OBJ)

clean: proper
	rm -rf dep
	rm -rf $(TARGET)

cleanall: clean
	$(MAKE) -C qhull cleanall
