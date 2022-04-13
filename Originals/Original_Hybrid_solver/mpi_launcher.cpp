#include<iostream>
#include<mpi.h>
#include"mpi_global.h"
#include<unistd.h>
#include<stdlib.h>

int main(int argc, char** argv){

	if(argc < 2){
		//Usage information
		return -1;
	}
	
	MPI_Init(NULL, NULL);
	int world_size;
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);

	int world_rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

	if(world_rank == 0){
		//Spin up tinisat
		char *args[argc+1];
		args[0] = "./tinisat0.22/tinisat";
		for(int i = 1; i<argc; i++){
			args[i] = argv[i];
		}
		args[argc] = NULL;
		execvp(args[0], args);
	}
	else{
		//Spin up SLS processes
		char *args[argc+1];
		args[0] = "./gnovelty+-T/gnovelty+";
		for(int i = 1; i<argc; i++){
			args[i] = argv[i];
		}
		args[argc] = NULL;
		execvp(args[0], args);
	}
	MPI_Finalize();
	return 0;
}
