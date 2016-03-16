TARGET	= randomWalk

CPP	 := main.cpp \
        Cmd.cpp \
        Walker.cpp \
        LoopErasedWalker.cpp \
        ConvexHull.cpp \
        RNG.cpp \
        Svg.cpp

OBJ	 = $(CPP:%.cpp=obj/%.o)
DEP	 = $(CPP:%.cpp=dep/%.d)

QHULL = qhull/lib/libqhullcpp.a qhull/lib/libqhullstatic_r.a

#CC	= g++ -g
CC	= g++ -O3 -flto -mtune=native -ffast-math
#CC	= clang++ -O3 -flto -march=native -ffast-math
WARNLEVEL= -Wall -Wextra -Wno-unused-parameter -pedantic

LNDIRS  = -L$(PWD)/lib \
          -L$(PWD)/qhull/lib

INCLUDES = -I$(PWD)/include \
           -I$(PWD)/qhull/src \

COPT = -std=c++11 -m64 -O -fPIC -fno-strict-aliasing -fomit-frame-pointer -fexceptions -pipe
CFLAGS	= $(COPT) $(INCLUDES)

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
	$(CC) $(WARNLEVEL) $(CFLAGS) -MM $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,obj/\1.o $@: ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

obj/%.o: %.cpp | obj
	$(CC) -c $(WARNLEVEL) $(CFLAGS) $< -o $@

$(QHULL):
	# needed three tries the last time I tried
	$(MAKE) -i -C qhull
	$(MAKE) -i -C qhull
	$(MAKE) -i -C qhull

$(TARGET): $(OBJ) $(QHULL)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ) $(LFLAGS)

proper:
	rm -rf $(OBJ)

clean: proper
	rm -rf dep
	rm -rf $(TARGET)
