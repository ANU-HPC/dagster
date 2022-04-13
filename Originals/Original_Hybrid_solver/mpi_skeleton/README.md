# MPI Skeleton

This program simulates a team of workers running cooperatively to solve a SAT problem.
The workers use MPI_Isend, MPI_Irecv, MPI_Test and MPI_Wait to communicate asynchronously.

To build and run the Docker image:
```
docker build -t milthorpe/async-neighbours .
docker run -it --mount src=`pwd`,target=/home/appuser,type=bind milthorpe/async-neighbours
```

To build and run the skeleton:
```
mpicc -o skeleton skeleton.c
mpirun -np 4 ./skeleton
```
