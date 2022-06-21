#include <gtest/gtest.h>

#include <unordered_set>
#include <algorithm> 
#include <stdlib.h>

#include "../../../exceptions.h"
#include "../../../utilities.cpp"
#include "../../../message.h"
#include "../../../message.cpp"
#include "../../../Cnf.h"
#include "../../../Cnf.cpp"
#include "../../../Dag.cpp"
#include "../../../CnfHolder.cpp"
#include "../../../CnfManager.cpp"
#include "../../../SatSolver.cpp"
//#include "../../SatHandler.cpp"
//#include "../../SatHandlerInterface.cpp"
#include "../../../SatSolverInterface.h"
#include "../../../SatSolverInterface.cpp"
#include "../../../BDDSolutions.cpp"
#include "../../../TableSolutions.cpp"
#include "../../../DisorderedArray.h"
#include "../../../RangeSet.h"
#include "../../../arguments.h"
#include "../../../arguments.cpp"
#include "../../../MasterOrganiser.h"
#include "../../../MasterOrganiser.cpp"
#include "../../../MPICommsInterface.h"
#include "../../../MPICommsInterface.cpp"
#include "../../../Master.h"
#include "../../../Master.cpp"

#include "../../../strengthener/MpiBuffer.h"
#include "../../../strengthener/MpiBuffer.cc"
#include "../../../strengthener/Work.h"
#include "../../../strengthener/Work.cc"



#include "dag_tests.c"
#include "cnf_tests.c"
#include "cnf_holder_tests.c"
#include "dag_cnf_tests.c"
#include "message_tests.c"
//#include "sat_handler_testing.c"
#include "solutions_tests.c"
#include "disorganised_array_tests.c"
#include "rangeSet_tests.c"
#include "other_tests.c"
#include "checkpointing_tests.c"


Arguments command_line_arguments;
CnfHolder* cnf_holder;

int main(int argc, char *argv[]) {
	command_line_arguments.cnf_filename = "wonkywonk";
	command_line_arguments.dag_filename = "donkeydonk";
  //MPI_Init(NULL, NULL);
	FILE* f = fopen("EXIT_FILE.txt","w");
	fprintf(f,"hello\n");
	fclose(f);
	::testing::InitGoogleTest(&argc, argv);
	int output = RUN_ALL_TESTS();
	system("rm EXIT_FILE.txt");
	
  //MPI_Finalize();
	return output;
}
