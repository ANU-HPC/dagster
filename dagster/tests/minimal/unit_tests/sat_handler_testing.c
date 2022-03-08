

int world_rank=99;

TEST(SatHandlerTests, handler_succeeds_to_load_good_dag_file) {
  Dag* d = new Dag("./good/d1.txt");
  Cnf* c = new Cnf("./good/c1.txt");
  d->reference_clauses(c);
  SatHandler *sathandler;
  EXPECT_NO_THROW(sathandler = new SatHandler(d, NULL));
}

TEST(SatHandlerTests, handler_fails_on_null_dag) {
	Dag* d = NULL;
	EXPECT_THROW(new SatHandler(d, NULL),BadInitialisationException);
}
TEST(SatHandlerTests, handler_fails_on_unreferenced_dag) {
	Dag* d = new Dag("./good/d1.txt");
	EXPECT_THROW(new SatHandler(d, NULL),BadInitialisationException);
}

TEST(SatHandlerTests, handler_loads_basic_message) {
  Dag* d = new Dag("./good/d1.txt");
  Cnf* c = new Cnf("./good/c1.txt");
  d->reference_clauses(c);
  d->generate_node_CNF_files_and_mappings();
  SatHandler *sathandler;
  EXPECT_NO_THROW(sathandler = new SatHandler(d, NULL));
  Message* m;
  m = new Message(1,1);
  sathandler->load_from_message(m);
}

TEST(SatHandlerTests, handler_generates_basic_solution) {
  Dag* d = new Dag("./good/d1.txt");
  Cnf* c = new Cnf("./good/c1.txt");
  d->reference_clauses(c);
  d->generate_node_CNF_files_and_mappings();
  SatHandler *sathandler;
  EXPECT_NO_THROW(sathandler = new SatHandler(d, NULL));
  Message* m;
  m = new Message(1,1);
  sathandler->load_from_message(m);
  sathandler->solve_and_generate_message(m);
  for (int i=0; i<d->clauses_length_for_each_node[1].size(); i++) {
    bool satisfies = false;
    for (int j=0; j<d->clauses_length_for_each_node[1][i]; j++)
      for (int k=0; k<m->assignments.size(); k++)
        if (m->assignments[k] == d->clauses_for_each_node[1][i][j])
          satisfies = true;
    if (satisfies == true)
      continue;
    FAIL();
  }
}

TEST(SatHandlerTests, handler_generates_multiple_valid_solutions) {
  Dag* d = new Dag("./good/d1.txt");
  Cnf* c = new Cnf("./good/c1.txt");
  d->reference_clauses(c);
  d->generate_node_CNF_files_and_mappings();
  SatHandler *sathandler;
  EXPECT_NO_THROW(sathandler = new SatHandler(d, NULL));
  Message* m;
  m = new Message(1,1);
  sathandler->load_from_message(m);
  bool zoz = true;
  zoz = sathandler->solve_and_generate_message(m);
  while (zoz == true) {
    for (int i=0; i<d->clauses_length_for_each_node[1].size(); i++) {
      bool satisfies = false;
      for (int j=0; j<d->clauses_length_for_each_node[1][i]; j++)
        for (int k=0; k<m->assignments.size(); k++)
          if (m->assignments[k] == d->clauses_for_each_node[1][i][j])
            satisfies = true;
      if (satisfies == true)
        continue;
      FAIL();
    }
    sathandler->reset_for_next();
    zoz = sathandler->solve_and_generate_message(m);
  }
}


template <typename T>
std::ostream &operator<<(std::ostream &strm, const std::unordered_set<T> &v) {
  std::copy(v.begin(), v.end(), std::ostream_iterator<int>(strm, " "));
  return strm;
}

TEST(SatHandlerTests, handler_generates_multiple_unique_valid_solutions) {
  Dag* d = new Dag("./good/d1.txt");
  Cnf* c = new Cnf("./good/c1.txt");
  d->reference_clauses(c);
  d->generate_node_CNF_files_and_mappings();
  SatHandler *sathandler;
  EXPECT_NO_THROW(sathandler = new SatHandler(d, NULL));
  Message* m;
  m = new Message(1,1);
  sathandler->load_from_message(m);

  vector<unordered_set<int>> memory;
  unordered_set<int> short_memory;
  unordered_set<int>* temp;

  bool zoz = true;
  zoz = sathandler->solve_and_generate_message(m);
  while (zoz == true) {
    short_memory.clear();
    //short_memory.insert(2);
    //cout << short_memory;
    for (int i=0; i<m->assignments.size(); i++) {
      short_memory.insert(m->assignments[i]);
    }
    if (find(memory.begin(), memory.end(), short_memory) != memory.end())
      FAIL();
    //temp = new unordered_set<int>(short_memory);
    //memory.push_back(*temp);
    memory.push_back(short_memory);

/*printf("ss\n");
for (int i=0;i<memory.size(); i++) {
	printf("\t");
	cout << memory[i];
}
printf("\n");*/

    //cout << memory;
    for (int i=0; i<d->clauses_length_for_each_node[1].size(); i++) {
      bool satisfies = false;
      for (int j=0; j<d->clauses_length_for_each_node[1][i]; j++)
        for (int k=0; k<m->assignments.size(); k++)
          if (m->assignments[k] == d->clauses_for_each_node[1][i][j])
            satisfies = true;
      if (satisfies == true)
        continue;
      FAIL();
    }
    sathandler->reset_for_next();
    zoz = sathandler->solve_and_generate_message(m);
  }
}


TEST(SatHandlerTests, handler_generates_multiple_unique_valid_solutions_for_many_nodes) {
  Dag* d = new Dag("./good/d1.txt");
  Cnf* c = new Cnf("./good/c1.txt");
  d->reference_clauses(c);
  d->generate_node_CNF_files_and_mappings();
  SatHandler *sathandler;
  EXPECT_NO_THROW(sathandler = new SatHandler(d, NULL));
  Message* m;
  for (int kkk=0; kkk<6; kkk++) {
    m = new Message(kkk,kkk);
    sathandler->load_from_message(m);
  
    vector<unordered_set<int>> memory;
    unordered_set<int> short_memory;
    unordered_set<int>* temp;
  
    bool zoz = true;
    zoz = sathandler->solve_and_generate_message(m);
    while (zoz == true) {
      short_memory.clear();
      //short_memory.insert(2);
      //cout << short_memory;
      for (int i=0; i<m->assignments.size(); i++) {
        short_memory.insert(m->assignments[i]);
      }
      if (find(memory.begin(), memory.end(), short_memory) != memory.end())
        FAIL();
      //temp = new unordered_set<int>(short_memory);
      //memory.push_back(*temp);
      memory.push_back(short_memory);

      //cout << memory;
      for (int i=0; i<d->clauses_length_for_each_node[kkk].size(); i++) {
        bool satisfies = false;
        for (int j=0; j<d->clauses_length_for_each_node[kkk][i]; j++)
          for (int k=0; k<m->assignments.size(); k++)
            if (m->assignments[k] == d->clauses_for_each_node[kkk][i][j])
              satisfies = true;
        if (satisfies == true)
          continue;
        FAIL();
      }
      sathandler->reset_for_next();
      zoz = sathandler->solve_and_generate_message(m);
    }
    sathandler->clear();
  }
}


TEST(SatHandlerTests, handler_generates_multiple_unique_valid_non_overlapping_solutions_for_many_nodes) {
  Dag* d = new Dag("./good/d1.txt");
  Cnf* c = new Cnf("./good/c1.txt");
  d->reference_clauses(c);
  d->generate_node_CNF_files_and_mappings();
  SatHandler *sathandler;
  EXPECT_NO_THROW(sathandler = new SatHandler(d, NULL));
  Message* m;
  m = new Message(0,0);
  for (int kkk=0; kkk<6; kkk++) {
    m->to = kkk;
    m->from = kkk;
    sathandler->load_from_message(m);
  
    vector<unordered_set<int>> memory;
    unordered_set<int> short_memory;
    unordered_set<int>* temp;
  
    bool zoz = true;
    zoz = sathandler->solve_and_generate_message(m);
    while (zoz == true) {
      short_memory.clear();
      //short_memory.insert(2);
      //cout << short_memory;
      for (int i=0; i<m->assignments.size(); i++) {
        short_memory.insert(m->assignments[i]);
      }
      if (find(memory.begin(), memory.end(), short_memory) != memory.end())
        FAIL();
      //temp = new unordered_set<int>(short_memory);
      //memory.push_back(*temp);
      memory.push_back(short_memory);
  


      //cout << memory;
      for (int i=0; i<d->clauses_length_for_each_node[kkk].size(); i++) {
        bool satisfies = false;
        for (int j=0; j<d->clauses_length_for_each_node[kkk][i]; j++)
          for (int k=0; k<m->assignments.size(); k++)
            if (m->assignments[k] == d->clauses_for_each_node[kkk][i][j])
              satisfies = true;
        if (satisfies == true)
          continue;
        FAIL();
      }
      sathandler->reset_for_next();
      zoz = sathandler->solve_and_generate_message(m);
    }
      /*short_memory.clear();
      short_memory.insert(3);
      short_memory.insert(-5);
      short_memory.insert(8);
      memory.push_back(short_memory);
    
printf("Checking non-overlap in \n");
for (int i=0;i<memory.size(); i++) {
	printf("\t");
	cout << memory[i];
}
printf("\n");*/

    for (int i=0; i<memory.size(); i++) {
      for (int j=i+1; j<memory.size(); j++) {
        bool conflict = false;
        for (unordered_set<int>::iterator ik=memory[i].begin(); ik!=memory[i].end(); ik++) {
          if (conflict == false) {
            for (unordered_set<int>::iterator jk=memory[j].begin(); jk!=memory[j].end(); jk++) {
              if (*ik == -(*jk)) {
                conflict = true;
                break;
              }
            }
          }
        }
        if (conflict == false)
          FAIL();
      }
    }
    sathandler->clear();
  }
}

//TODO: only test remaining is to make sure that the SAT handler returns solutions exhaustively.
//TODO: test the sathandler loads and executes additional_clauses properly






