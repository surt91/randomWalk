TARGET	= randomWalk
DEBUG	= randomWalkD

DOC 	= manual.pdf
TEST	= test
TESTD	= testD
BENCH	= bench

# max dimension
# 0 for arbitrary (needs heap allocations, may be slower)
# do a "make clean" after changing this
D_MAX = 4

# enable or disable MPI
MPI ?= 0

# enable or disable OpenMP
OMP ?= 0

CXXFLAGS = -std=c++11 -fexceptions -pipe

CPP	 := $(wildcard *.cpp) $(wildcard **/*.cpp)
HPP	 := $(wildcard *.hpp) $(wildcard **/*.hpp)

# handle MPI
ifeq "$(MPI)" "1"
MPICXX ?= mpic++
CXX = $(MPICXX)
CXXFLAGS += -D_MPI
else
CPP := $(filter-out simulation/MetropolisParallelTemperingMPI.cpp, $(CPP))
HPP := $(filter-out simulation/MetropolisParallelTemperingMPI.hpp, $(HPP))
endif

ifeq "$(OMP)" "1"
ifeq "$(CXX)" "clang++"
CXXFLAGS += -fopenmp=libomp
else
CXXFLAGS += -fopenmp
endif
endif

OBJ	 = $(CPP:%.cpp=obj/%.o)
DBG	 = $(CPP:%.cpp=dbg/%.o)
GCH	 = $(HPP:%.hpp=%.hpp.gch)
DEP	 = $(CPP:%.cpp=dep/%.d)

CPP := $(filter-out test.cpp, $(CPP))
CPP := $(filter-out benchmark.cpp, $(CPP))
CPP := $(filter-out $(wildcard tests/*.cpp), $(CPP))
CPP := $(filter-out $(wildcard benchmarks/*.cpp), $(CPP))

# for CATCH unit tests
TEST_CPP := $(filter-out main.cpp, $(CPP))
TEST_CPP += test.cpp $(wildcard tests/*.cpp)
TEST_OBJ := $(TEST_CPP:%.cpp=obj/%.o)
TEST_DBG := $(TEST_CPP:%.cpp=dbg/%.o)
TEST_GCH := $(GCH)
TEST_GCH += Catch/single_include/catch.hpp
TEST_DEP := $(TEST_CPP:%.cpp=dep/%.d)

# for benchmarks
BENCH_CPP := $(filter-out main.cpp, $(CPP))
BENCH_CPP += benchmark.cpp $(wildcard benchmarks/*.cpp)
BENCH_OBJ := $(BENCH_CPP:%.cpp=obj/%.o)
BENCH_GCH := $(GCH)
BENCH_GCH += benchmark/include/benchmark/benchmark.h
BENCH_DEP := $(BENCH_CPP:%.cpp=dep/%.d)

QHULL_CPP = qhull/lib/libqhullcpp.a
QHULL = qhull/lib/libqhullstatic_r.a

LIBBENCHMARK := benchmark/build/src/libbenchmark.a

# diagnostics color is introduced with gcc 4.9, test if our gcc knows it
# http://stackoverflow.com/a/17947005/1698412
GCC_GTEQ_490 := $(shell expr `gcc -dumpversion | sed -e 's/\.\([0-9][0-9]\)/\1/g' -e 's/\.\([0-9]\)/0\1/g' -e 's/^[0-9]\{3,4\}$$/&00/'` \>= 40900)
ifeq "$(GCC_GTEQ_490)" "1"
    CXXFLAGS += -fdiagnostics-color=auto
endif

# enable strict warnings
WARNLEVEL = -Wall -Wextra -Werror
ifeq "$(CXX)" "icc"
WARNLEVEL += -diag-disable=11074
else
WARNLEVEL += -Wpedantic -Wno-unused-result
endif
ifeq "$(CXX)" "clang++"
WARNLEVEL += -Wextra-semi -Wdocumentation -Wdeprecated
endif
# relax strictness for OpenMP pragmas
WARNLEVEL += -Wno-unknown-pragmas

LNDIRS  = -Lqhull/lib

INCLUDES = -isystem tclap/include \
           -isystem qhull/src \
           -isystem qhull/src/libqhullcpp \
		   -isystem Catch/single_include \
		   -isystem benchmark/include \

CXXFLAGS += $(INCLUDES)

LIBS = -lqhullcpp -lqhullstatic_r -lm
LFLAGS	= $(LNDIRS) $(LIBS)

VERSION := $(shell git describe --tags --always)
VERSION := $(VERSION) release
DBG_VERSION := $(VERSION) debug

CXXFLAGS += -DD_MAX=$(D_MAX)

DBGFLAGS := $(CXXFLAGS)
CXXFLAGS += -DVERSION="\"$(VERSION)\""
DBGFLAGS += -DVERSION="\"$(DBG_VERSION)\""

CXXFLAGS += -O3 -mtune=native -ffast-math -fno-strict-aliasing -fomit-frame-pointer -DNDEBUG
DBGFLAGS += -g -fprofile-arcs -ftest-coverage

# add some gcc specific flags (at least clang does not understand them or they caused issues (in the last tested version on arch respectively travis ubuntu trusty))
ifeq "$(CXX)" "g++"
CXXFLAGS += -flto
DBGFLAGS += -Og
else
DBGFLAGS += -O1
endif

all: $(DEP) $(TARGET)

.DELETE_ON_ERROR:
.PHONY: clean proper cleanall

MAKEFILE_TARGETS_WITHOUT_INCLUDE := clean proper cleanall
ifeq ($(filter $(MAKECMDGOALS),$(MAKEFILE_TARGETS_WITHOUT_INCLUDE)),)
-include $(DEP)
endif

obj dep dbg:
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
	sed 's,\($(*F)\)\.o[ :]*,obj/$*.o dbg/$*.o $@: ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

obj/%.o: %.cpp | obj
	@mkdir -p $(@D)
	$(CXX) -c $(WARNLEVEL) $(CXXFLAGS) $< -o $@

dbg/%.o: %.cpp | dbg
	@mkdir -p $(@D)
	$(CXX) -c $(WARNLEVEL) $(DBGFLAGS) $< -o $@

%.hpp.gch: %.hpp
	$(CXX) -c $(WARNLEVEL) $(CXXFLAGS) $< -o $@

$(QHULL_CPP): $(QHULL)
$(QHULL) $(QHULL_CPP):
	@echo "Building $@"
	@$(MAKE) -C qhull $(@:qhull/%=%)

$(LIBBENCHMARK):
	@echo "Building $@"
	cd benchmark; mkdir -p build; cd build; cmake -DCMAKE_BUILD_TYPE=Release -DBENCHMARK_ENABLE_LTO=true ..; $(MAKE)

$(TARGET): $(QHULL_CPP) $(OBJ)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJ) $(LFLAGS)

$(DEBUG): $(QHULL_CPP) $(DBG)
	$(CXX) $(DBGFLAGS) -o $(DEBUG) $(DBG) $(LFLAGS)

$(TEST): $(QHULL_CPP) $(TEST_OBJ)
	$(CXX) $(CXXFLAGS) -o $(TEST) $(TEST_OBJ) $(LFLAGS)

$(TESTD): $(QHULL_CPP) $(TEST_DBG)
	$(CXX) $(DBGFLAGS) -o $(TESTD) $(TEST_DBG) $(LFLAGS)

$(BENCH): $(QHULL_CPP) $(BENCH_OBJ) $(LIBBENCHMARK)
	$(CXX) $(CXXFLAGS) -o $(BENCH) $(BENCH_OBJ) $(LIBBENCHMARK) -pthread $(LFLAGS)

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
	rm -rf obj dbg $(GCH)

clean: proper
	rm -rf dep
	rm -rf $(TARGET) $(DEBUG)
	rm -rf $(TEST) $(TESTD)
	rm -rf doc

cleanall: clean
	$(MAKE) -C qhull cleanall
