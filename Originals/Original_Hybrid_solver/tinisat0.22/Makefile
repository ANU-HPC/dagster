CC = mpic++ -g -Wall -O3 -DNDEBUG
HEADERS = Cnf.h CnfManager.h SatSolver.h ../mpi_global.h
OBJS = Cnf.o CnfManager.o SatSolver.o main.o

tinisat: $(OBJS) 
	$(CC) $(OBJS) -o tinisat  

$(OBJS): $(HEADERS) Makefile

.cpp.o: 
	$(CC) -c $< 

clean:
	rm tinisat *.o
