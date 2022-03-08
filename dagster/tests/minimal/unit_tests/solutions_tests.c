



TEST(MasterTests, basic_loading) {
  Dag* d = new Dag("./good/d1.txt");
  CnfHolder* cnf_holder = new CnfHolder(d,"./good/c1.txt");
  cnf_holder->generate_decomposition();
  BDDSolutions* m;
  m = new BDDSolutions(d,cnf_holder->max_vc);
}


TEST(MasterTests, basic_loading_initial_messages) {
  Dag* d = new Dag("./good/d1.txt");
  CnfHolder* cnf_holder = new CnfHolder(d,"./good/c1.txt");
  cnf_holder->generate_decomposition();
  BDDSolutions* m;
  m = new BDDSolutions(d,cnf_holder->max_vc);
  Message *mess;
  mess = new Message(0,0);
  m->add_message(mess);
  mess = new Message(2,2);
  m->add_message(mess);
  mess = new Message(5,5);
  m->add_message(mess);
  mess = m->get_new_message_combination(0);
  if ((mess->to == 0) || (mess->to == 2)) {} else { FAIL();}
  mess = m->get_new_message_combination(1);
  ASSERT_NE(NULL, (long)mess);
  EXPECT_EQ(mess->to,5);
}


TEST(MasterTests, test_basic_message_retrieval) {
  Dag* d = new Dag("./good/d1.txt");
  CnfHolder* cnf_holder = new CnfHolder(d,"./good/c1.txt");
  cnf_holder->generate_decomposition();
  BDDSolutions* m;
  m = new BDDSolutions(d,cnf_holder->max_vc);
  Message *mess;
  mess = new Message(0,0);
  m->add_message(mess);
  mess = new Message(2,2);
  m->add_message(mess);
  mess = new Message(5,5);
  m->add_message(mess);
  mess = new Message(1,0);
  mess->assignments.push_back(1);
  mess->assignments.push_back(2);
  m->add_message(mess);
  mess = m->get_new_message_combination(1);
  ASSERT_NE(NULL, (long)mess);
  if (mess->to == 1) {
    EXPECT_EQ(mess->assignments[0],1);
    EXPECT_EQ(mess->assignments[1],2);
  } else if (mess->to == 5) {
    EXPECT_EQ(mess->assignments.size(),0);
  } else {
    FAIL();
  }
  mess = m->get_new_message_combination(1);
  if (mess->to == 1) {
    EXPECT_EQ(mess->assignments[0],1);
    EXPECT_EQ(mess->assignments[1],2);
  } else if (mess->to == 5) {
    EXPECT_EQ(mess->assignments.size(),0);
  } else {
    FAIL();
  }
}


TEST(MasterTests, test_literal_compatability_positive) {
  Dag* d = new Dag("./good/d1.txt");
  CnfHolder* cnf_holder = new CnfHolder(d,"./good/c1.txt");
  cnf_holder->generate_decomposition();
  BDDSolutions* m;
  m = new BDDSolutions(d,cnf_holder->max_vc);
  Message *mess;
  mess = new Message(4,1);
  mess->assignments.push_back(1);
  mess->assignments.push_back(2);
  m->add_message(mess);
  mess->from = 3;
  mess->assignments.clear();
  mess->assignments.push_back(2);
  mess->assignments.push_back(3);
  m->add_message(mess);
  mess->from = 5;
  mess->assignments.clear();
  mess->assignments.push_back(3);
  mess->assignments.push_back(4);
  m->add_message(mess);
  mess = m->get_new_message_combination(2);
  ASSERT_NE(NULL, (long)mess);
  EXPECT_EQ(mess->to,4);
  EXPECT_EQ(mess->assignments.size(),4);
  for (int i=0; i<mess->assignments.size(); i++) {
    if ((mess->assignments[i] >= 1) && (mess->assignments[i] <= 4)) {} else {FAIL();}
  }
}

TEST(MasterTests, test_literal_compatability_negative) {
  Dag* d = new Dag("./good/d1.txt");
  CnfHolder* cnf_holder = new CnfHolder(d,"./good/c1.txt");
  cnf_holder->generate_decomposition();
  BDDSolutions* m;
  m = new BDDSolutions(d,cnf_holder->max_vc);
  Message *mess;
  mess = new Message(4,1);
  mess->assignments.push_back(1);
  mess->assignments.push_back(2);
  m->add_message(mess);
  mess->from = 3;
  mess->assignments.clear();
  mess->assignments.push_back(2);
  mess->assignments.push_back(3);
  m->add_message(mess);
  mess->from = 0;
  mess->assignments.clear();
  mess->assignments.push_back(3);
  mess->assignments.push_back(4);
  m->add_message(mess);
  mess = m->get_new_message_combination(2);
  EXPECT_EQ((long)mess,NULL);
}

TEST(MasterTests, test_literal_compatability_positive2) {
  Dag* d = new Dag("./good/d1.txt");
  CnfHolder* cnf_holder = new CnfHolder(d,"./good/c1.txt");
  cnf_holder->generate_decomposition();
  BDDSolutions* m;
  m = new BDDSolutions(d,cnf_holder->max_vc);
  Message *mess;
  mess = new Message(4,1);
  mess->assignments.push_back(1);
  mess->assignments.push_back(2);
  m->add_message(mess);
  mess->from = 3;
  mess->assignments.clear();
  // 2 isn't on 3->4 arc in d1.txt - problem if we want
  // to make bdds as small as possible
  mess->assignments.push_back(2);
  mess->assignments.push_back(3);
  m->add_message(mess);
  mess->from = 5;
  mess->assignments.clear();
  mess->assignments.push_back(5);
  // 4 isn't on 5->4 arc in d1.txt
  mess->assignments.push_back(4);
  m->add_message(mess);
  mess = m->get_new_message_combination(2);
  ASSERT_NE(NULL, (long)mess);
  EXPECT_EQ(mess->to,4);
  EXPECT_EQ(mess->assignments.size(),5);
  for (int i=0; i<mess->assignments.size(); i++) {
    if ((mess->assignments[i] >= 1) && (mess->assignments[i] <= 5)) {} else {FAIL();}
  }
}

TEST(MasterTests, test_literal_compatability_negative2) {
  Dag* d = new Dag("./good/d1.txt");
  CnfHolder* cnf_holder = new CnfHolder(d,"./good/c1.txt");
  cnf_holder->generate_decomposition();
  BDDSolutions* m;
  m = new BDDSolutions(d,cnf_holder->max_vc);
  Message *mess;
  mess = new Message(4,1);
  mess->assignments.push_back(1);
  mess->assignments.push_back(2);
  m->add_message(mess);
  mess->from = 3;
  mess->assignments.clear();
  mess->assignments.push_back(2);
  mess->assignments.push_back(3);
  m->add_message(mess);
  mess->from = 5;
  mess->assignments.clear();
  mess->assignments.push_back(-3);
  mess->assignments.push_back(4);
  m->add_message(mess);
  mess = m->get_new_message_combination(2);
  EXPECT_EQ((long)mess,NULL);
}


TEST(MasterTests, test_no_duplicates1) {
  Dag* d = new Dag("./good/d1.txt");
  CnfHolder* cnf_holder = new CnfHolder(d,"./good/c1.txt");
  cnf_holder->generate_decomposition();
  BDDSolutions* m;
  m = new BDDSolutions(d,cnf_holder->max_vc);
  Message *mess;
  mess = new Message(4,1);
  mess->assignments.push_back(1);
  mess->assignments.push_back(2);
  m->add_message(mess);
  mess->from = 3;
  mess->assignments.clear();
  mess->assignments.push_back(2);
  mess->assignments.push_back(3);
  m->add_message(mess);
  mess->from = 5;
  mess->assignments.clear();
  mess->assignments.push_back(5);
  mess->assignments.push_back(4);
  m->add_message(mess);
  mess = m->get_new_message_combination(2);
  ASSERT_NE(NULL, (long)mess);
  EXPECT_EQ(mess->to,4);
  EXPECT_EQ(mess->assignments.size(),5);
  for (int i=0; i<mess->assignments.size(); i++) {
    if ((mess->assignments[i] >= 1) && (mess->assignments[i] <= 5)) {} else {FAIL();}
  }
  mess = m->get_new_message_combination(2);
  EXPECT_EQ((long)mess,NULL);
}

// some BDDSolutions specific tests

// test what happens when you call get_new_message_combination multiple times
// look at communicated and bdd


// test that bdd master deals with initial messages appropriately
TEST(MasterTests, test_BDDSolutions_initial_messages) {  
  Dag* d = new Dag("./good/d1.txt");
  CnfHolder* cnf_holder = new CnfHolder(d,"./good/c1.txt");
  cnf_holder->generate_decomposition();
  BDDSolutions* m;
  m = new BDDSolutions(d,cnf_holder->max_vc);
  Message *mess;
  mess = new Message(0,0);
  m->add_message(mess);
  mess = new Message(2,2);
  m->add_message(mess);
  mess = new Message(5,5);
  m->add_message(mess);
  mess = m->get_new_message_combination(0);
  ASSERT_NE(NULL, (long)mess);
  
  // expect that the message is to a base node and that the assignments are empty
  EXPECT_EQ(1, d->node_status[mess->to]);
  EXPECT_EQ(0, mess->assignments.size());
  
  delete mess;
  delete m;
  delete d;
  
}


TEST(MasterTests, test_add_assignments_unsat) {
  // adding two contradictory assignments gives 1 bdd and 
  // get_new_messge_combination should return empty message - anything possible
  Dag* d = new Dag("./good/d1.txt");
  CnfHolder* cnf_holder = new CnfHolder(d,"./good/c1.txt");
  cnf_holder->generate_decomposition();
  BDDSolutions* m;
  m = new BDDSolutions(d,cnf_holder->max_vc);
  Message *mess;
  mess = new Message(4,1);
  mess->assignments.push_back(1);
  m->add_message(mess);
  mess->assignments.clear();
  mess->assignments.push_back(-1);
  m->add_message(mess);

  // add message from 3 and 5 to 4 to stop get_new_message_combination from being empty
  mess->assignments.clear();
  mess->from = 3;
  m->add_message(mess);
  mess->from = 5;
  m->add_message(mess);

  // test that get_new_message_combination returns a message with empty assignments
  mess = m->get_new_message_combination(2);
  ASSERT_NE(NULL, (long)mess);
  EXPECT_EQ(0, mess->assignments.size());
  
  delete mess;
  delete m;
  delete d;
  
}

TEST(MasterTests, test_add_assignments_two_messages) {
  // adding two assignments
  Dag* d = new Dag("./good/d1.txt");
  CnfHolder* cnf_holder = new CnfHolder(d,"./good/c1.txt");
  cnf_holder->generate_decomposition();
  BDDSolutions* m;
  m = new BDDSolutions(d,cnf_holder->max_vc);
  Message *mess;
  mess = new Message(4,1);
  mess->assignments.push_back(1);
  mess->assignments.push_back(2);
  m->add_message(mess);
  mess->assignments.clear();
  mess->assignments.push_back(-1);
  mess->assignments.push_back(-2);
  m->add_message(mess);

  // add message from 3 and 5 to 4 to stop get_new_message_combination from being empty
  mess->assignments.clear();
  mess->from = 3;
  m->add_message(mess);
  mess->from = 5;
  m->add_message(mess);

  mess = m->get_new_message_combination(2);
  ASSERT_NE(NULL, (long)mess);
  
  // EXPECT things
  EXPECT_EQ(2, mess->assignments.size());
  EXPECT_EQ(4, mess->to);
  // flag: bool describing which message was produced first
  bool flag;
  for (int i=0; i<mess->assignments.size(); i++) {
    if (mess->assignments[i] == 1) {
      EXPECT_EQ(2, mess->assignments[1-i]);
      flag = false;
    } else if (mess->assignments[i] == 2) {
      EXPECT_EQ(1, mess->assignments[1-i]);
      flag = false;
    } else if (mess->assignments[i] == -1) {
      EXPECT_EQ(-2, mess->assignments[1-i]);
      flag = true;
    } else if (mess->assignments[i] == -2) {
      EXPECT_EQ(-1, mess->assignments[1-i]);
      flag = true;
    } else {
      FAIL();
    }
  }
  // get another new message - expect the other assignment
  mess = m->get_new_message_combination(2);

  EXPECT_EQ(2, mess->assignments.size());
  EXPECT_EQ(4, mess->to);
  if (flag) {
    for (int i=0; i<mess->assignments.size(); i++) {
      EXPECT_TRUE(mess->assignments[i] == 1 || mess->assignments[i] == 2);
      if (mess->assignments[i] == 1) {
        EXPECT_EQ(2, mess->assignments[1-i]);
      } else if (mess->assignments[i] == 2) {
        EXPECT_EQ(1, mess->assignments[1-i]);
      } else {
        FAIL();
      }
    }
  }
  else {
    for (int i=0; i<mess->assignments.size(); i++) {
      EXPECT_TRUE(mess->assignments[i] == -1 || mess->assignments[i] == -2);
      if (mess->assignments[i] == -1) {
        EXPECT_EQ(-2, mess->assignments[1-i]);
      } else if (mess->assignments[i] == -2) {
        EXPECT_EQ(-1, mess->assignments[1-i]);
      } else {
        FAIL();
      }
    }
  }

  delete mess;
  delete m;
  delete d;
  
}



// test for register completion
// look at all 3 bdds after adding one assignment, 
// getting a fresh assignment
// and registering its completion
TEST(MasterTests, test_cubes_register_completion_simple) {
  Dag* d = new Dag("./good/d1.txt");
  CnfHolder* cnf_holder = new CnfHolder(d,"./good/c1.txt");
  cnf_holder->generate_decomposition();
  BDDSolutions* m;
  m = new BDDSolutions(d,cnf_holder->max_vc);
  m->set__BDD_compilation_scheme("cubes");
  // add a simple message
  Message *mess;
  mess = new Message(4,1);
  mess->assignments.push_back(1);
  mess->assignments.push_back(2);
  m->add_message(mess);
  mess->assignments.clear();

  // add message from 3 and 5 to 4 to stop get_new_message_combination from being empty
  mess->from = 3;
  m->add_message(mess);
  mess->from = 5;
  m->add_message(mess);

  // get a new message and check it is what
  // we expect
  mess = m->get_new_message_combination(2);
  ASSERT_NE(NULL, (long)mess);
  EXPECT_EQ(2, mess->assignments.size());
  EXPECT_EQ(4, mess->to);

  for (int i=0; i<mess->assignments.size(); i++) {
    EXPECT_NE(3, mess->assignments[i]);
    EXPECT_NE(-1, mess->assignments[i]);
    EXPECT_NE(-2, mess->assignments[i]);
    if (mess->assignments[i] != 1) {
      EXPECT_EQ(2, mess->assignments[i]);
    }
    if (mess->assignments[i] != 2) {
      EXPECT_EQ(1, mess->assignments[i]);
    }
  }
  mess->from = 1;
  // now register the completion and check
  // that the completed bdd is correct
  // by calling get_additional_clauses
  // additional_clauses should be -1, -2
  m->register_completion(mess);
  int** additional_clauses;
  additional_clauses = m->get_additional_clauses(1);
  EXPECT_NE(NULL, (long)additional_clauses);
  EXPECT_NE(NULL, (long)additional_clauses[0]);
  EXPECT_EQ(NULL, additional_clauses[1]);
  for (int i=0; additional_clauses[i]; i++) {
    for (int j=0; additional_clauses[i][j]; j++) {
      EXPECT_NE(3, additional_clauses[i][j]);
      EXPECT_NE(1, additional_clauses[i][j]);
      EXPECT_NE(2, additional_clauses[i][j]);
      if (additional_clauses[i][j] != -1) {
        EXPECT_EQ(-2, additional_clauses[i][j]);
      }
      if (additional_clauses[i][j] != -2) {
        EXPECT_EQ(-1, additional_clauses[i][j]);
      }
    }
  }

  // clean up
  for (int i=0; additional_clauses[i]; i++) {
    free(additional_clauses[i]);
  }
  free(additional_clauses);
  delete m;
  delete mess;
  
  delete d;
}

// same as previous test except add more assignments
TEST(MasterTests, test_cubes_register_completion_simple_more_assignmnets) {
  Dag* d = new Dag("./good/d1.txt");
  CnfHolder* cnf_holder = new CnfHolder(d,"./good/c1.txt");
  cnf_holder->generate_decomposition();
  BDDSolutions* m;
  m = new BDDSolutions(d,cnf_holder->max_vc);
  m->set__BDD_compilation_scheme("cubes");
  // add a simple message
  Message *mess;
  mess = new Message(4,1);
  mess->assignments.push_back(1);
  mess->assignments.push_back(2);
  m->add_message(mess);
  mess->assignments.clear();
  // get a new message and check it is what
  // we expect

  // add message from 3 and 5 to 4 to stop get_new_message_combination from being empty
  mess->assignments.clear();
  mess->from = 3;
  m->add_message(mess);
  mess->from = 5;
  m->add_message(mess);

  mess = m->get_new_message_combination(2);
  ASSERT_NE(NULL, (long)mess);
  EXPECT_EQ(2, mess->assignments.size());
  EXPECT_EQ(4, mess->to);

  for (int i=0; i<mess->assignments.size(); i++) {
    EXPECT_NE(3, mess->assignments[i]);
    EXPECT_NE(-1, mess->assignments[i]);
    EXPECT_NE(-2, mess->assignments[i]);
    if (mess->assignments[i] != 1) {
      EXPECT_EQ(2, mess->assignments[i]);
    }
    if (mess->assignments[i] != 2) {
      EXPECT_EQ(1, mess->assignments[i]);
    }
  }
  mess->from = 1;
  // now register the completion and check
  // that the completed bdd is correct
  // by calling get_additional_clauses
  // additional_clauses should be -1, -2
  m->register_completion(mess);
  int** additional_clauses;
  additional_clauses = m->get_additional_clauses(1);
  EXPECT_NE(NULL, (long)additional_clauses);
  EXPECT_NE(NULL, (long)additional_clauses[0]);
  EXPECT_EQ(NULL, additional_clauses[1]);
  for (int i=0; additional_clauses[i]; i++) {
    for (int j=0; additional_clauses[i][j]; j++) {
      EXPECT_NE(3, additional_clauses[i][j]);
      EXPECT_NE(1, additional_clauses[i][j]);
      EXPECT_NE(2, additional_clauses[i][j]);
      if (additional_clauses[i][j] != -1) {
        EXPECT_EQ(-2, additional_clauses[i][j]);
      }
      if (additional_clauses[i][j] != -2) {
        EXPECT_EQ(-1, additional_clauses[i][j]);
      }
    }
  }

  // clean up
  for (int i=0; additional_clauses[i]; i++) {
    free(additional_clauses[i]);
  }
  free(additional_clauses);
  delete m;
  delete mess;
  
  delete d;
}


// test that it doesnt add all variables if they are not needed
TEST(MasterTests, test_cubes_register_completion_selects_correct_variables) {
  Dag* d = new Dag("./good/d1.txt");
  CnfHolder* cnf_holder = new CnfHolder(d,"./good/c1.txt");
  cnf_holder->generate_decomposition();
  BDDSolutions* m;
  m = new BDDSolutions(d,cnf_holder->max_vc);
  m->set__BDD_compilation_scheme("cubes");
  Message *mess;
  mess = new Message(4, 1);
  mess->assignments.push_back(1);
  mess->assignments.push_back(4); // not on the forward connections for node 1
  m->register_completion(mess);
  int** additional_clauses = m->get_additional_clauses(1);
  ASSERT_NE(NULL, (long)additional_clauses);
  // 4 should not be in additional_clauses
  for (int i=0; additional_clauses[i]; i++) {
    for (int j=0; additional_clauses[j]; j++) {
      EXPECT_NE(4, abs(additional_clauses[i][j]));
    }
  }
  // additional_clauses should just be -1
  EXPECT_EQ(-1, additional_clauses[0][0]);
  EXPECT_EQ(0, additional_clauses[0][1]);
  EXPECT_EQ(NULL, additional_clauses[1]);
}


// test for register completion on the final node - need to deal with
// the variables on outgoing arcs differently
// NOTE: solutions (at final node) aren't recorded in messages or by master
TEST(MasterTests, test_cubes_register_completion_final_node) {
  Dag* d = new Dag("./good/d1.txt");
  CnfHolder* cnf_holder = new CnfHolder(d,"./good/c1.txt");
  cnf_holder->generate_decomposition();
  BDDSolutions* m;
  m = new BDDSolutions(d,cnf_holder->max_vc);
  m->set__BDD_compilation_scheme("cubes");
  // make an artificial message to and from the final node, node 4
  Message *mess;
  mess = new Message(4, 4);
  // add assignments to message
  for (int i=1; i<=cnf_holder->max_vc; i++) {
    mess->assignments.push_back(i);
  }
  // now register the completiong of this message (not done in loops.cpp)
  m->register_completion(mess);

  // now compile negated - we want all variables from the completed message
  int** neg_rep = m->get_additional_clauses(4);
  ASSERT_NE(NULL, (long)neg_rep);
  ASSERT_NE(NULL, (long)neg_rep[0]);
  int count;
  for (count=0; neg_rep[0][count]; count++) {
    EXPECT_TRUE(neg_rep[0][count] < 0 && neg_rep[0][count] > -7);
  }
  EXPECT_EQ(6, count);

  delete mess;
  delete m;
  delete d;
  
  for (int i=0; neg_rep[i]; i++) {
    free(neg_rep[i]);
  }
  free(neg_rep);
}

// test that there are no additional clauses when the completed bdd is the 0 bdd
TEST(MasterTests, test_completed_zero) {
  Dag* d = new Dag("./good/d1.txt");
  CnfHolder* cnf_holder = new CnfHolder(d,"./good/c1.txt");
  cnf_holder->generate_decomposition();
  BDDSolutions* m;
  m = new BDDSolutions(d,cnf_holder->max_vc);
  m->set__BDD_compilation_scheme("cubes");
  Message *mess;
  // check that no additional clauses are added before register_completion
  // has been called

  // add and retreive some message to check that this doesn't change completed bdd
  mess = new Message(4, 1);
  mess->assignments.push_back(1);
  mess->assignments.push_back(2);
  m->add_message(mess);
  mess->assignments.clear();
  mess->assignments.push_back(-2);
  mess->assignments.push_back(-3);
  m->add_message(mess);

  // add message from 3 and 5 to 4 to stop get_new_message_combination from being empty
  mess->assignments.clear();
  mess->from = 3;
  m->add_message(mess);
  mess->from = 5;
  m->add_message(mess);

  mess = m->get_new_message_combination(2);
  delete mess;
  mess =  new Message(4, 3);
  mess->assignments.push_back(3);
  mess->assignments.push_back(-5);
  m->add_message(mess);
  mess->assignments.clear();
  mess->from = 5;
  mess->assignments.push_back(5);
  mess->assignments.push_back(-6);
  m->add_message(mess);
  mess = m->get_new_message_combination(2);
  mess = m->get_new_message_combination(2);
  mess = m->get_new_message_combination(1);


  // now check that getting additional clauses at each of the nodes gives nothing
  int** additional_clauses;
  for (int node=1; node<6; node++) {
    additional_clauses = m->get_additional_clauses(node);
    EXPECT_EQ(NULL, additional_clauses);
  }

  // check that making a new additional clauses object and dehydrating it works as expected
  //Cnf* a = new Cnf(additional_clauses);
  //EXPECT_EQ(NULL, a->clauses);
  
  delete m;
  delete mess;
  //delete a;
  delete d;
  
}

// test when completed is 1 (true) (no more possible assignments)
TEST(MasterTests, test_completed_no_more_possible_assignents) {
  Dag* d = new Dag("./good/d1.txt");
  CnfHolder* cnf_holder = new CnfHolder(d,"./good/c1.txt");
  cnf_holder->generate_decomposition();
  BDDSolutions* m;
  m = new BDDSolutions(d,cnf_holder->max_vc);
  m->set__BDD_compilation_scheme("cubes");
  // register completion so that all possible combinations are given
  Message *mess;
  mess = new Message(4, 1);
  mess->assignments.push_back(1);
  m->register_completion(mess);
  mess->assignments.clear();
  mess->assignments.push_back(-1);
  m->register_completion(mess);
  int** additional_clauses = m->get_additional_clauses(1);

  // tests on additional_clauses
  // expect empty cnf - just one 0
  ASSERT_NE(NULL, (long)additional_clauses);
  ASSERT_NE(NULL, (long)additional_clauses[0]);
  EXPECT_EQ(0, additional_clauses[0][0]);
  EXPECT_EQ(NULL, additional_clauses[1]);
  
  delete m;
  delete mess;
  delete d;
  
  if (additional_clauses) {
    for (int i=0; additional_clauses[i]; i++) {
      free(additional_clauses[i]);
    }
    free(additional_clauses);
  }
}

// test that get_additional_clauses with each of the different compilation schemes
// gives the same number of solutions
/*TEST(MasterTests, test_get_additional_clauses_solution_counts_simple) {
  Dag* d = new Dag("./good/d1.txt");
  Cnf* c = new Cnf("./good/c1.txt");
  d->reference_clauses(c);

  BDDSolutions* m = new BDDSolutions(d);
  m->set__BDD_compilation_scheme("cubes");

  // make a message to register completion
  Message* mess = new Message(4, 1);  // the variables on the 1->4 interface are 1,2,3
  mess->assignments.push_back(1);
  mess->assignments.push_back(2);
  m->register_completion(mess);
  
  // now get the neg rep from the three compilations
  int** cubes_additional_clauses = m->get_additional_clauses(1);
  ASSERT_NE(NULL, (long)cubes_additional_clauses);
  m->set__BDD_compilation_scheme("minisat");
  int** minisat_additional_clauses = m->get_additional_clauses(1);
  ASSERT_NE(NULL, (long)minisat_additional_clauses);
  m->set__BDD_compilation_scheme("paths");
  int** paths_additional_clauses = m->get_additional_clauses(1);
  ASSERT_NE(NULL, (long)paths_additional_clauses);

  // count the models
  SatHandler* sat_handler = new SatHandler(d, NULL);
  Message* empty_message = new Message();
  Message* mess_tmp = new Message();
  sat_handler->load_from_message(empty_message, cubes_additional_clauses);
  int cubes_count = 0;
  // copy logic from loops.cpp, don't quite understand the result == 2 condition
  int result = 2;
  while (true) {
    while (result == 2) {
      result = sat_handler->solve_and_generate_message(mess_tmp);
    }
    if (result == 0) 
      break;
    cubes_count++;
    if (!sat_handler->reset_for_next())
      break;
  }

  sat_handler->load_from_message(empty_message, minisat_additional_clauses);
  int minisat_count = 0;
  result = 2;
  while (true) {
    while (result == 2) {
      result = sat_handler->solve_and_generate_message(mess_tmp);
    }
    if (result == 0) 
      break;
    minisat_count++;
    if (!sat_handler->reset_for_next())
      break;
  }

  sat_handler->load_from_message(empty_message, paths_additional_clauses);
  int paths_count = 0;
  result = 2;
  while (true) {
    while (result == 2) {
      result = sat_handler->solve_and_generate_message(mess_tmp);
    }
    if (result == 0) 
      break;
    paths_count++;
    if (!sat_handler->reset_for_next())
      break;
  }

  // now compare the counts
  EXPECT_EQ(cubes_count, minisat_count);
  EXPECT_EQ(minisat_count, paths_count);


  // clean up
  if (cubes_additional_clauses) {
    for (int i=0; cubes_additional_clauses[i]; i++) {
      free(cubes_additional_clauses[i]);
    }
    free(cubes_additional_clauses);
  }
  if (minisat_additional_clauses) {
    for (int i=0; minisat_additional_clauses[i]; i++) {
      free(minisat_additional_clauses[i]);
    }
    free(minisat_additional_clauses);
  }
  if (paths_additional_clauses) {
    for (int i=0; paths_additional_clauses[i]; i++) {
      free(paths_additional_clauses[i]);
    }
    free(paths_additional_clauses);
  }

  delete mess;
  delete d;
  

}*/

// now a bigger test to check model counts
// test on some costas 7 assignments
/*TEST(MasterTests, test_get_additional_clauses_solution_counts_costas) {
  Dag* d = new Dag("../../Benchmarks/costas/debugging/costas_7.dag");
  Cnf* c = new Cnf("../../Benchmarks/costas/debugging/costas_7.cnf");
  d->reference_clauses(c);

  BDDSolutions* m = new BDDSolutions(d);
  m->set__BDD_compilation_scheme("cubes");

  // make a message to register completion
  Message* mess = new Message(2, 1);  
  // the variables on the 1->2 interface are 561-572,549-560, others get filtered out
  // make some messages with random lengths and variables and signs
  int num_messages = std::rand() % 10;
  for (int i=0; i< num_messages; i++) {
    int message_length = std::rand() % 5;
    for (int j=0; j<message_length; j++) {
      int sign = std::rand() % 2 == 0 ? -1 : 1;
      mess->assignments.push_back(sign * std::rand() % 23 + 549);
    }
    m->register_completion(mess);
    mess->assignments.clear();
  }  
  
  // now get the neg rep from the three compilations
  int** cubes_additional_clauses = m->get_additional_clauses(1);
  ASSERT_NE(NULL, (long)cubes_additional_clauses);
  m->set__BDD_compilation_scheme("minisat");
  int** minisat_additional_clauses = m->get_additional_clauses(1);
  ASSERT_NE(NULL, (long)minisat_additional_clauses);
  m->set__BDD_compilation_scheme("paths");
  int** paths_additional_clauses = m->get_additional_clauses(1);
  ASSERT_NE(NULL, (long)paths_additional_clauses);

  // count the models
  SatHandler* sat_handler = new SatHandler(d, NULL);
  Message* empty_message = new Message();
  Message* mess_tmp = new Message();
  sat_handler->load_from_message(empty_message, cubes_additional_clauses);
  int cubes_count = 0;
  // copy logic from loops.cpp, don't quite understand the result == 2 condition
  int result = 2;
  while (true) {
    while (result == 2) {
      result = sat_handler->solve_and_generate_message(mess_tmp);
    }
    if (result == 0) 
      break;
    cubes_count++;
    if (!sat_handler->reset_for_next())
      break;
  }

  sat_handler->load_from_message(empty_message, minisat_additional_clauses);
  int minisat_count = 0;
  result = 2;
  while (true) {
    while (result == 2) {
      result = sat_handler->solve_and_generate_message(mess_tmp);
    }
    if (result == 0) 
      break;
    minisat_count++;
    if (!sat_handler->reset_for_next())
      break;
  }

  sat_handler->load_from_message(empty_message, paths_additional_clauses);
  int paths_count = 0;
  result = 2;
  while (true) {
    while (result == 2) {
      result = sat_handler->solve_and_generate_message(mess_tmp);
    }
    if (result == 0) 
      break;
    paths_count++;
    if (!sat_handler->reset_for_next())
      break;
  }

  // now compare the counts
  EXPECT_EQ(cubes_count, minisat_count);
  EXPECT_EQ(minisat_count, paths_count);


  // clean up
  if (cubes_additional_clauses) {
    for (int i=0; cubes_additional_clauses[i]; i++) {
      free(cubes_additional_clauses[i]);
    }
    free(cubes_additional_clauses);
  }
  if (minisat_additional_clauses) {
    for (int i=0; minisat_additional_clauses[i]; i++) {
      free(minisat_additional_clauses[i]);
    }
    free(minisat_additional_clauses);
  }
  if (paths_additional_clauses) {
    for (int i=0; paths_additional_clauses[i]; i++) {
      free(paths_additional_clauses[i]);
    }
    free(paths_additional_clauses);
  }

  delete mess;
  delete d;
  

}*/

// test that get_additional_clauses works with Cnf class
// similar to what happens in loops.cpp
TEST(MasterTests, test_get_additional_clauses_cubes_with_class) {
  // initialisation
  Dag* d = new Dag("./good/d1.txt");
  CnfHolder* cnf_holder = new CnfHolder(d,"./good/c1.txt");
  cnf_holder->generate_decomposition();
  BDDSolutions* m;
  m = new BDDSolutions(d,cnf_holder->max_vc);
  m->set__BDD_compilation_scheme("cubes");
  
  // add some messages
  Message *mess;
  mess = new Message(4, 1);
  mess->assignments.push_back(1);
  mess->assignments.push_back(2);
  m->add_message(mess);

  // add message from 3 and 5 to 4 to stop get_new_message_combination from being empty
  mess->assignments.clear();
  mess->from = 3;
  m->add_message(mess);
  mess->from = 5;
  m->add_message(mess);

  // output_message: mess should be 1,2
  mess = m->get_new_message_combination(2);
  ASSERT_NE(NULL, (long)mess);
  EXPECT_EQ(4, mess->to);
  EXPECT_EQ(2, mess->assignments.size());
  for (int i=0; i<mess->assignments.size(); i++) {
    EXPECT_TRUE(mess->assignments[i] == 1 || mess->assignments[i] == 2);
  }

  // and register the completion - add to completed[4] bdd
  // same as in master loop for when MPI_TAG is 5 (request for an assignment)
  mess->from = 4;
  m->register_completion(mess);
  // new_data should be -1, -2
  int** new_data = m->get_additional_clauses(mess->to);
  ASSERT_NE(NULL, (long)new_data);
  ASSERT_NE(NULL, (long)new_data[0]);
  int count;
  for (count=0; new_data[0][count]; count++) {
    EXPECT_TRUE(new_data[0][count] < 0);
    EXPECT_TRUE(new_data[0][count] > -3);
  }
  EXPECT_EQ(2, count);
  EXPECT_EQ(NULL, new_data[1]);

  // now make a new additional_clauses object and test on it
  Cnf* additional_clauses = new Cnf(new_data);
  ASSERT_NE(NULL, (long)additional_clauses->clauses);
  ASSERT_NE(NULL, (long)additional_clauses->clauses[0]);
  for (count=0; additional_clauses->clauses[0][count]; count++) {
    EXPECT_TRUE(additional_clauses->clauses[0][count] < 0);
    EXPECT_TRUE(additional_clauses->clauses[0][count] > -3);
  }
  EXPECT_EQ(2, count);
  EXPECT_EQ(NULL, additional_clauses->clauses[1]);

  int* dehydrated_additional_clauses = (int*)calloc(sizeof(int),9999);
  additional_clauses->dehydrate(dehydrated_additional_clauses);
  delete additional_clauses;
  // some tests
  ASSERT_NE(NULL, (long)dehydrated_additional_clauses);
  EXPECT_EQ(7, dehydrated_additional_clauses[0]);
  EXPECT_EQ(1, dehydrated_additional_clauses[1]);
  EXPECT_EQ(2, dehydrated_additional_clauses[2]);
  EXPECT_EQ(0, dehydrated_additional_clauses[3]);
  EXPECT_TRUE(dehydrated_additional_clauses[4] < 0);
  EXPECT_TRUE(dehydrated_additional_clauses[4] > -3);
  EXPECT_TRUE(dehydrated_additional_clauses[5] < 0);
  EXPECT_TRUE(dehydrated_additional_clauses[5] > -3);
  EXPECT_EQ(0, dehydrated_additional_clauses[6]);

  for (int i=0; new_data[i]; i++) {
    free(new_data[i]);
  }
  free(new_data);
  free(dehydrated_additional_clauses);
  delete mess;
  delete m;
  
  delete d;
  
}

TEST(MasterTests, TableSolutions_additional_clauses) {
  Dag* dag = new Dag("./good/d1.txt");
  CnfHolder* cnf_holder = new CnfHolder(dag,"./good/c1.txt");
  cnf_holder->generate_decomposition();
  int** additional_clauses;
  TableSolutions* sols = new TableSolutions(dag,false);

  additional_clauses = sols->get_additional_clauses(0);
  EXPECT_EQ(additional_clauses, (int**)NULL);

  Message* m = new Message();
  m->assignments.push_back(-1);
  m->assignments.push_back(2);
  m->assignments.push_back(-3);

  m->from = 0;
  m->to = 1;

  sols->register_completion(m);

  m->assignments.clear();
  m->assignments.push_back(-1);
  m->assignments.push_back(-2);
  m->assignments.push_back(3);

  sols->register_completion(m);

  additional_clauses = sols->get_additional_clauses(0);

  EXPECT_EQ(additional_clauses[2], (int*)NULL);
  EXPECT_EQ(additional_clauses[0][0], 1);
  EXPECT_EQ(additional_clauses[0][1], -2);
  EXPECT_EQ(additional_clauses[0][2], 0);
  EXPECT_EQ(additional_clauses[1][0], 1);
  EXPECT_EQ(additional_clauses[1][1], 2);
  EXPECT_EQ(additional_clauses[1][2], 0);
//exit(55);
//EXPECT_EQ(1,2);
}



TEST(MasterTests, TableSolutions_redundant_messages) {
  Dag* dag = new Dag("./good/d1.txt");
  CnfHolder* cnf_holder = new CnfHolder(dag,"./good/c1.txt");
  cnf_holder->generate_decomposition();
  TableSolutions *t = new TableSolutions(dag,false);
  Message* m1 = new Message(2,3);
  m1->assignments.push_back(-5);
  m1->assignments.push_back(3);
  m1->assignments.push_back(-9);
  m1->assignments.push_back(-1);
  m1->assignments.push_back(2);
  
  Message* m2 = new Message(2,3);
  m2->assignments.push_back(-5);
  m2->assignments.push_back(3);
  m2->assignments.push_back(-9);
  m2->assignments.push_back(-1);
  m2->assignments.push_back(2);
  
  t->add_message(m1);
  t->add_message(m2);
  
  t->get_new_message_combination(1);
  EXPECT_EQ(t->get_new_message_combination(1),(Message*)NULL);
}



TEST(MasterTests, TableSolutions_additional_clauses2) {
  Dag* dag = new Dag("./good/d1.txt");
  CnfHolder* cnf_holder = new CnfHolder(dag,"./good/c1.txt");
  cnf_holder->generate_decomposition();
  int** additional_clauses;
  TableSolutions* sols = new TableSolutions(dag,false);

  additional_clauses = sols->get_additional_clauses(0);
  EXPECT_EQ(additional_clauses, (int**)NULL);

  Message* m = new Message();
  m->assignments.push_back(5);
  m->assignments.push_back(-6);

  m->from = 5;
  m->to = 4;

  sols->register_completion(m);

  m->assignments.clear();
  m->assignments.push_back(-6);
  m->assignments.push_back(-5);

  sols->register_completion(m);

  additional_clauses = sols->get_additional_clauses(5);

  EXPECT_EQ(additional_clauses[2], (int*)NULL);
  EXPECT_EQ(additional_clauses[0][0], -5);
  EXPECT_EQ(additional_clauses[0][1], 6);
  EXPECT_EQ(additional_clauses[0][2], 0);
  EXPECT_EQ(additional_clauses[1][0], 5);
  EXPECT_EQ(additional_clauses[1][1], 6);
  EXPECT_EQ(additional_clauses[1][2], 0);
  //EXPECT_EQ(additional_clauses[0][2], 0);
//exit(55);
//EXPECT_EQ(1,2);
}


TEST(MasterTests, TableSolutions_additional_clauses3) {
  Dag* dag = new Dag("./good/d1.txt");
  CnfHolder* cnf_holder = new CnfHolder(dag,"./good/c1.txt");
  cnf_holder->generate_decomposition();
  int** additional_clauses;
  TableSolutions* sols = new TableSolutions(dag,false);

  additional_clauses = sols->get_additional_clauses(0);
  EXPECT_EQ(additional_clauses, (int**)NULL);

  Message* m = new Message();
  m->assignments.push_back(5);
  m->assignments.push_back(-6);

  m->from = 5;
  m->to = 4;

  sols->register_completion(m);

  m->assignments.clear();
  m->assignments.push_back(-6);
  m->assignments.push_back(5);

  sols->register_completion(m);

  additional_clauses = sols->get_additional_clauses(5);

  EXPECT_EQ(additional_clauses[1], (int*)NULL);
  EXPECT_EQ(additional_clauses[0][0], -5);
  EXPECT_EQ(additional_clauses[0][1], 6);
  EXPECT_EQ(additional_clauses[0][2], 0);
  //EXPECT_EQ(additional_clauses[1][0], 5);
  //EXPECT_EQ(additional_clauses[1][1], 6);
  //EXPECT_EQ(additional_clauses[1][2], 0);
  //EXPECT_EQ(additional_clauses[0][2], 0);
//exit(55);
//EXPECT_EQ(1,2);
}




TEST(MasterTests, BDDSolutions_additional_clauses2) {
  Dag* dag = new Dag("./good/d1.txt");
  CnfHolder* cnf_holder = new CnfHolder(dag,"./good/c1.txt");
  cnf_holder->generate_decomposition();
  int** additional_clauses;
  BDDSolutions* sols = new BDDSolutions(dag,cnf_holder->max_vc);
  sols->set__BDD_compilation_scheme("minisat");

  additional_clauses = sols->get_additional_clauses(0);
  EXPECT_EQ(additional_clauses, (int**)NULL);

  Message* m = new Message();
  m->assignments.push_back(5);
  m->assignments.push_back(-6);

  m->from = 5;
  m->to = 4;

  sols->register_completion(m);

  m->assignments.clear();
  m->assignments.push_back(-6);
  m->assignments.push_back(-5);

  sols->register_completion(m);

  additional_clauses = sols->get_additional_clauses(5);
  
/*10 
8 
-9 
-8 -6 10 
8 -6 -10 
-9 6 10 
9 6 -10 
-8 -9 10 
8 9 -10*/

  EXPECT_EQ(additional_clauses[10], (int*)NULL);
  EXPECT_EQ(additional_clauses[0][0], 10);
  EXPECT_EQ(additional_clauses[0][1], 0);
  //EXPECT_EQ(additional_clauses[0][2], 6);
  //EXPECT_EQ(additional_clauses[0][3], 0);
  
  EXPECT_EQ(additional_clauses[1][0], 8);
  EXPECT_EQ(additional_clauses[1][1], 0);
  //EXPECT_EQ(additional_clauses[1][2], 6);
  //EXPECT_EQ(additional_clauses[1][3], 0);
  
  EXPECT_EQ(additional_clauses[2][0], -9);
  EXPECT_EQ(additional_clauses[2][1], 0);
  //EXPECT_EQ(additional_clauses[2][2], 6);
  //EXPECT_EQ(additional_clauses[2][3], 0);
  
  EXPECT_EQ(additional_clauses[3][0], -8);
  EXPECT_EQ(additional_clauses[3][1], -6);
  EXPECT_EQ(additional_clauses[3][2], 10);
  EXPECT_EQ(additional_clauses[3][3], 0);
  
  EXPECT_EQ(additional_clauses[4][0], 8);
  EXPECT_EQ(additional_clauses[4][1], -6);
  EXPECT_EQ(additional_clauses[4][2], -10);
  EXPECT_EQ(additional_clauses[4][3], 0);
  
  EXPECT_EQ(additional_clauses[5][0], -9);
  EXPECT_EQ(additional_clauses[5][1], 6);
  EXPECT_EQ(additional_clauses[5][2], 10);
  EXPECT_EQ(additional_clauses[5][3], 0);
  
  EXPECT_EQ(additional_clauses[6][0], 9);
  EXPECT_EQ(additional_clauses[6][1], 6);
  EXPECT_EQ(additional_clauses[6][2], -10);
  EXPECT_EQ(additional_clauses[6][3], 0);
  
  EXPECT_EQ(additional_clauses[7][0], -8);
  EXPECT_EQ(additional_clauses[7][1], -9);
  EXPECT_EQ(additional_clauses[7][2], 10);
  EXPECT_EQ(additional_clauses[7][3], 0);
  
  EXPECT_EQ(additional_clauses[8][0], 8);
  EXPECT_EQ(additional_clauses[8][1], 9);
  EXPECT_EQ(additional_clauses[8][2], -10);
  EXPECT_EQ(additional_clauses[8][3], 0);
}




