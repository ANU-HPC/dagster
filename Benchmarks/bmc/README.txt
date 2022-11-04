Bounded Model Checking (BMC) benchmarks:
----------------------------------------

These two benchmarks encode how it is possible to verify the possibility of an attack on a protocl, specifically
there are two subfolders (1) and (2) breaking protocols from 


A lightweight security protocol for ultra-low power ASIC implementation for wireless Implantable Medical Devices
 by Saied Hosseini-Khayat
  5th International Symposium on Medical Information and Communication Technology (2011)
  
AND

Block cipher based security for severely resource-constrained implantable medical devices
 by Christoph Beck, Daniel Masny, Willi Geiselmann, Georg Bretthauer

respectively.

These two cases evaluate the performance of solving the bounded model checking attack at a lower fidelity of variables
and then by propagating thoes variable valuations to a higher fidelity, to also verify the attack at a higher fidelity.
In both cases, the interraction between the actors is encoded in C files (in some way) and then these are converted to
SAT CNF files via CBMC.
the variables corresponding to thoes which are abstraction invariant (AI), and whos varlues are ment to be propagated
and linked to the same variables at higher fidelity are marked in the CNF, and the python utility pulls thoes values
out and linkes them between the CNF fidelity files via a generated DAG.


HOW TO RUN:
-----------

* ensure CBMC is installed and accessable from the shell as 'cbmc'
* compile dagster, and run generate.sh from the respective subdirectory, will compile and run all examples




