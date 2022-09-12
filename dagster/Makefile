UNAME                           = $(shell uname -a)
CRAY_PLATFORM_NAME              = nakuru

ifeq (${CRAY_PLATFORM_NAME}, $(findstring ${CRAY_PLATFORM_NAME}, ${UNAME}))
        #Variables for Nakuru Cray system
        MPICXX                  = CC
        CXXFLAGS                += -O3 -std=c++17 -DVERSION=1.0.0 -DNO_PROVENANCE 
        INCLUDES                = -I/sw/UNCLASSIFIED/glog-master/include -I/sw/UNCLASSIFIED/cudd-release/include \
                                  -I/sw/UNCLASSIFIED/zlib-1.2.11/include
        LDFLAGS                 = -L/sw/UNCLASSIFIED/glog-master/lib -L/sw/UNCLASSIFIED/cudd-release/lib \
                                  -L/sw/UNCLASSIFIED/zlib-1.2.11/lib -lglog -lstdc++fs -lcudd -lz
	#For some reason main.cpp is attempted to be compiled with CXX and no includes rather than the
	#MPICXX line below, can't quite work out where make is coming up with this change
	CXX			= CC $(INCLUDES) 
else
	#Default variables
	MPICXX                  = mpic++
	CXXFLAGS                += -ggdb -O3 -std=c++17 -I. -Iminisat
	LDFLAGS                 += -lglog -lstdc++fs -lcudd -lz
endif
#Profiling
#       LDFLAGS                 += -L/usr/local/lib/openmpi -lmpiP -lm -lbfd -liberty -lunwind -L.. -ldummy_mpi1


OBJS 				= Cnf.o CnfManager.o SatSolver.o Dag.o Message.o TableSolutions.o BDDSolutions.o main.o Master.o Worker.o MPICommsInterface.o utilities.o Arguments.o CnfHolder.o MasterOrganiser.o SatSolverInterface.o

.PHONY: gnovelty

dagster: $(OBJS) gnovelty/*.cc gnovelty/*.hh strengthener/*.cc strengthener/*.h minisat_solver/*.cc minisat_solver/*.h
	$(MAKE) -C strengthener
	$(MAKE) -C gnovelty
	$(MAKE) -C minisat_solver
	$(MPICXX) $(CPPFLAGS) $(CXXFLAGS) ./*.o gnovelty/*.o strengthener/*.o* minisat/*/*.o* minisat_solver/*.o* $(LDFLAGS) -o dagster

%.o: %.cpp %.h
	$(MPICXX) $(CPPFLAGS) $(CXXFLAGS) $(INCLUDES) -c $< -o $@ 

clean:
	$(MAKE) -C strengthener clean
	$(MAKE) -C gnovelty clean
	$(MAKE) -C minisat_solver clean
	-rm dagster *.o
	-rm ./dag_out.txt
