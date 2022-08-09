
#include "utilities.h"
#include "utilities.cpp"
#include "Cnf.h"
#include "Cnf.cpp"

#include "message.h"
#include "message.cpp"
#include "MasterOrganiser.h"
#include "MasterOrganiser.cpp"

#include "stdio.h"

int main() {
	Message *m1 = new Message(1,2);
	Message *m2 = new Message(2,3);
	Message *m3 = new Message(3,4);
	Message *m4 = new Message(5,6);

	MasterOrganiser *organiser = new MasterOrganiser(4);
	organiser->debug_message("a");
	organiser->add_message(m1);
	organiser->add_message(m2);
	organiser->add_message(m3);
	organiser->add_message(m4);
	organiser->debug_message("a");
	organiser->allocate_assignments();
	organiser->debug_message("a");
	organiser->dispatch(1);
	organiser->dispatch(2);
	organiser->debug_message("a");
	printf("helo\n");
	return 0;
}
