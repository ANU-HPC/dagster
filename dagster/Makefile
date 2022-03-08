
#Default variables
MPICXX                  = mpic++
CXXFLAGS                += -ggdb -O3 -std=c++17 -DVERSION=1.0.0 -DEMAIL=mark.burgess@anu.edu.au -I.
LDFLAGS                 += -lglog -lstdc++fs -lcudd -lz


OBJS 				= Cnf.o CnfManager.o SatSolver.o Dag.o message.o TableSolutions.o BDDSolutions.o main.o Master.o Worker.o MPICommsInterface.o utilities.o arguments.o CnfHolder.o MasterOrganiser.o

.PHONY: gnovelty

dagster: $(OBJS) gnovelty/*.cc gnovelty/*.hh strengthener/*.cc strengthener/*.h
	$(MAKE) -C strengthener
	$(MAKE) -C gnovelty
	$(MPICXX) $(CPPFLAGS) $(CXXFLAGS) ./*.o gnovelty/*.o strengthener/*.o* minisat/*/*.o* $(LDFLAGS) -o dagster

%.o: %.cpp %.h
	$(MPICXX) $(CPPFLAGS) $(CXXFLAGS) $(INCLUDES) -c $< -o $@ 

clean:
	$(MAKE) -C strengthener clean
	$(MAKE) -C gnovelty clean
	-rm dagster *.o
	-rm ./dag_out.txt
