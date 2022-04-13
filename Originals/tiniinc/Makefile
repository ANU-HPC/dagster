CC = g++ -Wall -O3 -DUPDEBUG
HEADERS = Cnf.h CnfManager.h SatSolver.h
OBJS = Cnf.o CnfManager.o SatSolver.o main.o

tinisat: $(OBJS) 
	$(CC) $(OBJS) -o tinisat  

$(OBJS): $(HEADERS) Makefile

.cpp.o: 
	$(CC) -c $< 
