TARGET	= randomWalk

DOC 	= manual.pdf

CPP	 := main.cpp \
        Cmd.cpp \
        RNG.cpp \
        io.cpp \
        stat.cpp \
        Benchmark.cpp \
        walker/Walker.cpp \
        walker/LatticeWalker.cpp \
        walker/LoopErasedWalker.cpp \
        walker/SelfAvoidingWalker.cpp \
        walker/RealWalker.cpp \
        walker/GaussWalker.cpp \
        walker/LevyWalker.cpp \
        walker/CorrelatedWalker.cpp \
        simulation/Simulation.cpp \
        simulation/Metropolis.cpp \
        simulation/WangLandau.cpp \
        simulation/FastWangLandau.cpp \
        simulation/Histogram.cpp \
        visualization/Svg.cpp \
        visualization/Povray.cpp \
        visualization/Gnuplot2D.cpp \
        visualization/Gnuplot3D.cpp \

OBJ	 = $(CPP:%.cpp=obj/%.o)
DEP	 = $(CPP:%.cpp=dep/%.d)

QHULL = qhull/lib/libqhullcpp.a qhull/lib/libqhullstatic_r.a

CXXFLAGS = -std=c++11 -fexceptions -pipe
#CXXFLAGS += -fopenmp

# diagnostics color is introduced with gcc 4.9, test if our gcc knows it
# http://stackoverflow.com/a/17947005/1698412
GCC_GTEQ_490 := $(shell expr `gcc -dumpversion | sed -e 's/\.\([0-9][0-9]\)/\1/g' -e 's/\.\([0-9]\)/0\1/g' -e 's/^[0-9]\{3,4\}$$/&00/'` \>= 40900)
ifeq "$(GCC_GTEQ_490)" "1"
    CXXFLAGS += -fdiagnostics-color=auto
endif

release: CXXFLAGS += -O3 -flto -mtune=native -mtune=corei7 -ffast-math -fno-strict-aliasing
release: all
silent: CXXFLAGS += -DNLOG
silent: release
debug: CXXFLAGS += -g -Og
debug: all CXXFLAGS += -g -Og

# for clang sanitizers
#CXX = clang++
#CXXFLAGS = -O1 -g -fsanitize=address -fno-omit-frame-pointer
#CXXFLAGS = -fsanitize=memory -fno-omit-frame-pointer -g -O2 -fsanitize-memory-track-origins
#CXXFLAGS = -fsanitize=undefined -O2

WARNLEVEL= -Wall -Wextra -Wpedantic -Werror -Wno-unknown-pragmas
#WARNLEVEL += -Wfloat-equal -Wshadow -Wextra-semi -Wdocumentation -Wdeprecated
#WANRLEVEL += -Weverything -Wno-c++98-compat -Wno-c++98-compat-bind-to-temporary-copy -Wno-padded

LNDIRS  = -Lqhull/lib

INCLUDES = -Itclap/include \
           -Iqhull/src \
           -Iqhull/src/libqhullcpp

CXXFLAGS += $(INCLUDES)

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
	@mkdir -p $(@D)
	@set -e; rm -f $@; \
	$(CXX) $(WARNLEVEL) $(CXXFLAGS) -MM $< > $@.$$$$; \
	sed 's,\($(*F)\)\.o[ :]*,obj/$*.o $@: ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

obj/%.o: %.cpp | obj
	@mkdir -p $(@D)
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
