TARGET=gnovelty+

RANLIB	= ranlib

#CXX = g++ -DUSE_setOfStates  -Wall -g -pg -ggdb -pthread # 
#CXX = g++ -O3 -static -fno-strict-aliasing -pthread
#CXX = mpic++ -g -DUSE_setOfStates -static-libgcc -DNDEBUG -O3 -fno-strict-aliasing -fomit-frame-pointer -funroll-all-loops -fexpensive-optimizations -malign-double -Wall -march=nocona -ffast-math -pthread #-s -DOLD_FLEX -DNDEBUG -O3 -fno-strict-aliasing -fomit-frame-pointer -funroll-all-loops -fexpensive-optimizations   -mno-align-stringops  -minline-all-stringops -pipe  -mfpmath=sse -m32 -msse3 -falign-functions=64 -fforce-addr -ffast-math -mpush-args  -maccumulate-outgoing-args -momit-leaf-frame-pointer -pthread
CXX = mpic++ -g -static-libgcc -DNDEBUG -O3 -fno-strict-aliasing -fomit-frame-pointer -funroll-all-loops -fexpensive-optimizations -malign-double -Wall -march=nocona -ffast-math -pthread #-s -DOLD_FLEX -DNDEBUG -O3 -fno-strict-aliasing -fomit-frame-pointer -funroll-all-loops -fexpensive-optimizations   -mno-align-stringops  -minline-all-stringops -pipe  -mfpmath=sse -m32 -msse3 -falign-functions=64 -fforce-addr -ffast-math -mpush-args  -maccumulate-outgoing-args -momit-leaf-frame-pointer -pthread


MODULES= State PartialState rtime gnovelty+

OBJECTS=$(MODULES:=.o) $(GENERATED:=.o) 


LDFLAGS=$(FLEX_LIB) -lz \
# 	-L/data/private/grettonc/downloads/FRAMEWAVE_1.3_SRC/./Framewave/build/tmp/fwBase_release_static_32/ \
# 	-L/data/private/grettonc/downloads/FRAMEWAVE_1.3_SRC/./Framewave/build/tmp/fwSignal_release_static_32/ \
# 	-L/data/private/grettonc/downloads/FRAMEWAVE_1.3_SRC/./Framewave/build/tmp/fwImage_release_static_32/ \
# 	-L/data/private/grettonc/downloads/FRAMEWAVE_1.3_SRC/./Framewave/build/tmp/fwJPEG_release_static_32/ \
# 	-L/data/private/grettonc/downloads/FRAMEWAVE_1.3_SRC/./Framewave/build/tmp/fwVideo_release_static_32/ \
# 	-lfwSignal -lfwBase -lfwVideo -lfwJPEG -lfwImage \
	-lrt -lm -lpthread  

CFLAGS= -I. #-I/home/charlesg/downloads/boost_1_34_0/#  \
# 	-I/data/private/grettonc/downloads/FRAMEWAVE_1.3_SRC/./Framewave/build/include/ 

all: $(GENERATED:=.cc) $(TARGET)

$(TARGET): $(OBJECTS) 
	$(CXX) -o $@ $(OBJECTS) $(CFLAGS) $(LDFLAGS)

%.o: %.cc ../mpi_global.h
	$(CXX) $(CFLAGS) $< -c -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)

test: $(TARGET)
	bash ./run-tests





# SHELL = /bin/sh
# #CFLAGS = -O3 
# #CFLAGS = -O3 -g -pg
# CFLAGS = -O3 -static -fno-strict-aliasing #-Wall -Werror
# #CFLAGS = -ggdb -I/home/charlesg/downloads/boost_1_34_0/ #-O3 -static -fno-strict-aliasing -I/home/charlesg/downloads/boost_1_34_0/
# CC = g++

# all:	gnovelty+

# #resolvent.o: global.h resolvent.h resolvent.c

# rtime.o: rtime.h rtime.c

# #gnovelty+.o: global.h resolvent.h rtime.h gnovelty+.c

# #gnovelty+: global.h resolvent.h resolvent.o rtime.h rtime.o gnovelty+.o
# #	$(CC) $(CFLAGS) gnovelty+.o resolvent.o rtime.o -lm -o gnovelty+

# gnovelty+.o: global.h rtime.h gnovelty+.c

# gnovelty+: global.h rtime.h rtime.o gnovelty+.o
# 	$(CC) $(CFLAGS) gnovelty+.o rtime.o -lm -o gnovelty+

# clean:
# 	-rm -f gnovelty+
# 	-rm -f *.o
# 	-rm -f *~
# 	-rm -f #*#
# 	-rm -f core
