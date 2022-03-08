tinisat: tinisat_standalone.cc
	mpic++ -O3 -o tinisat -g tinisat_standalone.cc -static-libgcc -DNDEBUG -fno-strict-aliasing -fomit-frame-pointer -funroll-all-loops -fexpensive-optimizations -malign-double -Wall -march=nocona -ffast-math -lglog -lstdc++fs
clean:
	-rm tinisat
