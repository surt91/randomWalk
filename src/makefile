TARGET	= randomWalk

DOC 	= manual.pdf

# max dimension
# 0 for arbitrary (needs heap allocations, may be slower)
# do a "make clean" after changing this
D_MAX = 5

CXXFLAGS = -std=c++11 -fexceptions -pipe
CXXFLAGS += -fopenmp

CPP	 := main.cpp \
        Cmd.cpp \
        Logging.cpp \
        RNG.cpp \
        io.cpp \
        Hypercube.cpp \
        stat/runtime.cpp \
        stat/RollingMean.cpp \
        Benchmark.cpp \
        walker/Walker.cpp \
        walker/SpecWalker.cpp \
        walker/LatticeWalker.cpp \
        walker/LoopErasedWalker.cpp \
        walker/SelfAvoidingWalker.cpp \
        walker/RealWalker.cpp \
        walker/GaussWalker.cpp \
        walker/LevyWalker.cpp \
        walker/CorrelatedWalker.cpp \
        walker/EscapeWalker.cpp \
        walker/ScentWalker.cpp \
        simulation/Simulation.cpp \
        simulation/SimpleSampling.cpp \
        simulation/Metropolis.cpp \
        simulation/MetropolisParallelTempering.cpp \
        simulation/WangLandau.cpp \
        simulation/FastWLEntropic.cpp \
        simulation/Histogram.cpp \
        visualization/Svg.cpp \
        visualization/Povray.cpp \
        visualization/Gnuplot2D.cpp \
        visualization/Gnuplot3D.cpp \
        visualization/Threejs.cpp \

OBJ	 = $(CPP:%.cpp=obj/%.o)
DEP	 = $(CPP:%.cpp=dep/%.d)

QHULL = qhull/lib/libqhullcpp.a qhull/lib/libqhullstatic_r.a

# diagnostics color is introduced with gcc 4.9, test if our gcc knows it
# http://stackoverflow.com/a/17947005/1698412
GCC_GTEQ_490 := $(shell expr `gcc -dumpversion | sed -e 's/\.\([0-9][0-9]\)/\1/g' -e 's/\.\([0-9]\)/0\1/g' -e 's/^[0-9]\{3,4\}$$/&00/'` \>= 40900)
ifeq "$(GCC_GTEQ_490)" "1"
    CXXFLAGS += -fdiagnostics-color=auto
endif

VERSION := $(shell git describe --tags --always)

release: CXXFLAGS += -O3 -flto -mtune=native -mtune=corei7 -ffast-math -fno-strict-aliasing
release: VERSION += release
release: all
silent: CXXFLAGS += -DNLOG
silent: VERSION += silent
silent: release
debug: CXXFLAGS += -g -Og
debug: VERSION += debug
debug: all

CXXFLAGS += -DVERSION="\"$(VERSION)\""
CXXFLAGS += -DD_MAX=$(D_MAX)

# for clang sanitizers
#CXX = clang++
#CXXFLAGS = -O1 -g -fsanitize=address -fno-omit-frame-pointer
#CXXFLAGS = -fsanitize=memory -fno-omit-frame-pointer -g -O2 -fsanitize-memory-track-origins
#CXXFLAGS = -fsanitize=undefined -O2

WARNLEVEL = -Wall -Wextra -Wpedantic -Werror
WARNLEVEL += -Wno-unknown-pragmas -Wno-unused-result -Wno-unused-parameter
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

# compile the parts containing the version new, after changes to git
# compare http://stackoverflow.com/a/6526504/1698412
obj/simulation/Simulation.o obj/Cmd.o: ../.git/HEAD ../.git/index

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
	# usually works after trying 3 times
	$(MAKE) -j1 -C qhull
	$(MAKE) -j1 -C qhull
	$(MAKE) -j1 -C qhull

$(TARGET): $(OBJ) $(QHULL)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJ) $(LFLAGS)

manual.pdf: doc
	$(MAKE) -C doc/latex clean
	$(MAKE) -C doc/latex
	cp doc/latex/refman.pdf $@

doc: doc.conf *.cpp *.hpp cli.md
	doxygen doc.conf > doxy.log

cli.md: $(TARGET) ../README.md makefile
	@echo documentation for cli
	@sed ' 1 s/.*/&     {#mainpage}/' ../README.md > cli.md
	@printf "\n    " > version.dat
	@./randomWalk --version | tr -d '\n' >> version.dat
	@printf "\n\n" >> version.dat
	@sed -i "4 r version.dat" cli.md
	@rm version.dat
	@echo '' >> cli.md
	@echo '## CLI Usage' >> cli.md
	@echo '' >> cli.md
	@echo '~~~' >> cli.md
	@./randomWalk -h >> cli.md
	@echo '~~~' >> cli.md

proper:
	rm -rf $(OBJ)

clean: proper
	rm -rf dep
	rm -rf $(TARGET)
	rm -rf doc

cleanall: clean
	$(MAKE) -C qhull cleanall
