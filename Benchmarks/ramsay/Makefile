TARGET=generate_ramsey_NM

CXX=g++

CXXPREFIX= 

OPTIMISE := -O3 -DNDEBUG -march=native -mno-align-stringops  -minline-all-stringops -ffast-math #-fsee -fsched-spec-load-dangerous -funsafe-loop-optimizations -fprefetch-loop-arrays -ftracer -fipa-matrix-reorg -ftree-parallelize-loops=4  -fmodulo-sched -fmodulo-sched-allow-regmoves -fomit-frame-pointer -finline-functions -funroll-all-loops -mpush-args  -maccumulate-outgoing-args 

WARNINGS := -Wall -Wextra

CPPFLAGS := -ggdb -pg -pedantic -Wall $(OPTIMISE)


LDFLAGS := # -lm -lpthread -lz -lglpk -lltdl -lClp -lCoinUtils

CXXFLAGS :=  # -march=native $(WARNINGS) $(OPTIMISE)

SOURCES := $(wildcard *.cc)
DEPENDS := $(SOURCES:.cc=.d)
OBJECTS := $(SOURCES:.cc=.o)
LOBJECTS := $(SOURCES:.cc=.lo)

.PHONY: all clean

all: $(DEPENDS) $(OBJECTS) $(TARGET) 

$(TARGET): $(OBJECTS)
	$(CXXPREFIX) $(CXX) -o $@ $(OBJECTS) $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) 

%.o: %.cc
	$(CXXPREFIX) $(CXX) $(CPPFLAGS) $< -c -o $@

%.d: %.cc
	$(CXXPREFIX) $(CXX) $(CPPFLAGS) -MM -MT $@ $< -o $@

%: %.cc %.d
	$(CXXPREFIX) $(CXX) $(CPPFLAGS) $(CXXFLAGS) $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET) $(DEPENDS) $(LOBJECTS)
