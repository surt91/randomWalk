TARGET	= randomWalk

DOC 	= manual.pdf
TEST	= test

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
GCH	 = $(HPP:%.hpp=%.hpp.gch)
DEP	 = $(CPP:%.cpp=dep/%.d)

# for CATCH unit tests
CPP := $(filter-out test.cpp, $(CPP))
TEST_CPP := $(filter-out main.cpp, $(CPP))
CPP := $(filter-out $(wildcard tests/*.cpp), $(CPP))
TEST_CPP += test.cpp
TEST_OBJ := $(TEST_CPP:%.cpp=obj/%.o)
TEST_GCH := $(GCH)
TEST_GCH += Catch/single_include/catch.hpp
TEST_DEP := $(TEST_CPP:%.cpp=dep/%.d)

QHULL_CPP = qhull/lib/libqhullcpp.a
QHULL = qhull/lib/libqhullstatic_r.a

# diagnostics color is introduced with gcc 4.9, test if our gcc knows it
# http://stackoverflow.com/a/17947005/1698412
GCC_GTEQ_490 := $(shell expr `gcc -dumpversion | sed -e 's/\.\([0-9][0-9]\)/\1/g' -e 's/\.\([0-9]\)/0\1/g' -e 's/^[0-9]\{3,4\}$$/&00/'` \>= 40900)
ifeq "$(GCC_GTEQ_490)" "1"
    CXXFLAGS += -fdiagnostics-color=auto
endif

VERSION := $(shell git describe --tags --always)

release: CXXFLAGS += -O3 -mtune=native -mtune=corei7 -ffast-math -fno-strict-aliasing -fomit-frame-pointer -DNDEBUG
release: VERSION += release
release: all
silent: CXXFLAGS += -DNLOG
silent: VERSION += silent
silent: release
debug: CXXFLAGS += -g
debug: VERSION += debug
debug: all

# add some gcc specific flags (at least clang does not understand them or they caused issues (in the last tested version on arch respectively travis ubuntu trusty))
ifeq "$(CXX)" "g++"
release: CXXFLAGS += -flto
debug: CXXFLAGS += -Og
else
debug: CXXFLAGS += -O1
endif

CXXFLAGS += -DVERSION="\"$(VERSION)\""
CXXFLAGS += -DD_MAX=$(D_MAX)

# enable strict warnings
WARNLEVEL = -Wall -Wextra -Wpedantic -Werror
ifeq "$(CXX)" "clang++"
WARNLEVEL += -Wextra-semi -Wdocumentation -Wdeprecated
endif
# relax strictness for OpenMP pragmas
WARNLEVEL += -Wno-unknown-pragmas -Wno-unused-result

LNDIRS  = -Lqhull/lib

INCLUDES = -isystem tclap/include \
           -isystem qhull/src \
           -isystem qhull/src/libqhullcpp \
		   -isystem Catch/single_include

CXXFLAGS += $(INCLUDES)

LIBS = -lqhullcpp -lqhullstatic_r -lm
LFLAGS	= $(LNDIRS) $(LIBS)

all: $(DEP) $(TARGET) $(TEST)

.DELETE_ON_ERROR:
.PHONY: clean proper

MAKEFILE_TARGETS_WITHOUT_INCLUDE := clean proper cleanall
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
	sed 's,\($(*F)\)\.o[ :]*,obj/$*.o $@: ,g ; s,.hpp,.hpp.gch,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

obj/%.o: %.cpp | obj
	@mkdir -p $(@D)
	$(CXX) -c $(WARNLEVEL) $(CXXFLAGS) $< -o $@

%.hpp.gch: %.hpp
	$(CXX) -c $(WARNLEVEL) $(CXXFLAGS) $< -o $@

$(QHULL_CPP): $(QHULL)
$(QHULL) $(QHULL_CPP):
	@echo "Building $@"
	@$(MAKE) -C qhull $(@:qhull/%=%)

$(TARGET): $(QHULL_CPP) $(OBJ)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJ) $(LFLAGS)

$(TEST): $(QHULL_CPP) $(TEST_OBJ)
	$(CXX) $(CXXFLAGS) -o $(TEST) $(TEST_OBJ) $(LFLAGS)

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
	rm -rf $(OBJ) $(GCH)

clean: proper
	rm -rf dep
	rm -rf $(TARGET)
	rm -rf doc

cleanall: clean
	$(MAKE) -C qhull cleanall
