default:
	mpic++ -g -O3 mpi_launcher.cpp -o mpi_launcher 
	make -C tinisat0.22 
	make -C gnovelty+-T
clean: 
	rm mpi_launcher
	make clean -C tinisat0.22 
	make clean -C gnovelty+-T
