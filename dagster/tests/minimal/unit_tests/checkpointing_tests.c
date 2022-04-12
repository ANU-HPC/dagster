


TEST(CheckpointingTests, OrganiserTestDumpLoadEmpty) {
  MasterOrganiser* m = new MasterOrganiser(5);
  
  FILE* fout = fopen("zog.tcnf","w");
  m->dump_checkpoint(fout);
  fclose(fout);
  
  delete m;
  
  MasterOrganiser* mm = new MasterOrganiser(5);
  FILE* fin = fopen("zog.tcnf","r");
  mm->load_checkpoint(fin);
  fclose(fin);
  
  
  EXPECT_EQ(mm->message_buffer.length,0);
  
}


TEST(CheckpointingTests, OrganiserTestDumpLoadEquivalence) {
  MasterOrganiser* m = new MasterOrganiser(5);
  
  Message* m1 = new Message();
  m1->from=5;
  m1->to=4;
  m1->assignments.push_back(99);
  m1->assignments.push_back(999);
  m1->assignments.push_back(9999);
  
  Message* m2 = new Message();
  m2->from=9;
  m2->to=1;
  m2->assignments.push_back(199);
  m2->assignments.push_back(1999);
  m2->assignments.push_back(19999);
  
  m->add_message(m1);
  m->add_message(m2);
  
  FILE* fout = fopen("zog.tcnf","w");
  m->dump_checkpoint(fout);
  fclose(fout);
  
  m->remove_message(m1);
  m->remove_message(m2);
  
  delete m1;
  delete m2;
  delete m;
  
  MasterOrganiser* mm = new MasterOrganiser(5);
  FILE* fin = fopen("zog.tcnf","r");
  mm->load_checkpoint(fin);
  fclose(fin);
  
  
  EXPECT_EQ(mm->message_buffer[0]->from,5);
  EXPECT_EQ(mm->message_buffer[0]->to,4);
  EXPECT_EQ(mm->message_buffer[0]->assignments[0],99);
  EXPECT_EQ(mm->message_buffer[0]->assignments[1],999);
  EXPECT_EQ(mm->message_buffer[0]->assignments[2],9999);
  EXPECT_EQ(mm->message_buffer[0]->assignments.size(),3);
  
  EXPECT_EQ(mm->message_buffer[1]->from,9);
  EXPECT_EQ(mm->message_buffer[1]->to,1);
  EXPECT_EQ(mm->message_buffer[1]->assignments[0],199);
  EXPECT_EQ(mm->message_buffer[1]->assignments[1],1999);
  EXPECT_EQ(mm->message_buffer[1]->assignments[2],19999);
  EXPECT_EQ(mm->message_buffer[1]->assignments.size(),3);
  
}




TEST(CheckpointingTests, TableMasterDumpLoadEmpty) {
  Dag* d = new Dag("./good/d1.txt");
  TableSolutions* master = new TableSolutions(d,true);
  
  FILE* fout = fopen("zog.tcnf","w");
  master->dump_checkpoint(fout);
  fclose(fout);
  
  delete master;
  
  TableSolutions* master2 = new TableSolutions(d,true);
  FILE* fin = fopen("zog.tcnf","r");
  master2->load_checkpoint(fin);
  fclose(fin);
  
  for (int i=0; i<d->no_nodes; i++) {
    for (int j=0; j<d->no_nodes; j++) {
      EXPECT_EQ(master2->messages[i][j].size(),0);
    }
    EXPECT_EQ(master2->additional_clauses[i]->cc,0);
    EXPECT_EQ(master2->completed_combinations[i].size(),0);
  }
}


TEST(CheckpointingTests, BDDMasterDumpLoadEmpty) {
  Dag* d = new Dag("./good/d1.txt");
  BDDSolutions* master = new BDDSolutions(d,55);
  
  FILE* fout = fopen("zog.tcnf","w");
  master->dump_checkpoint(fout);
  fclose(fout);
  
  delete master;
  
  BDDSolutions* master2 = new BDDSolutions(d,55);
  FILE* fin = fopen("zog.tcnf","r");
  master2->load_checkpoint(fin);
  fclose(fin);
  
  for (int i=0; i<d->no_nodes; i++) {
    for (int j=0; j<d->no_nodes; j++) {
      EXPECT_EQ(master2->bdds[i][j], Cudd_ReadLogicZero(master2->ddmgr));
    }
    EXPECT_EQ(master2->communicated[i],Cudd_ReadLogicZero(master2->ddmgr));
    EXPECT_EQ(master2->completed[i],Cudd_ReadLogicZero(master2->ddmgr));
  }
  EXPECT_EQ(master2->initial_messages.size(),0);
  EXPECT_EQ(master2->vc,55);
}






TEST(CheckpointingTests, TableMasterDumpLoadIdentity) {
  Dag* d = new Dag("./good/d1.txt");
  TableSolutions* master = new TableSolutions(d,true);

  Message* m = new Message();
  m->to = 0;
  m->from = 0;
  m->assignments.push_back(43);
  master->add_message(m);
  
  Message* m2 = new Message();
  m2->to = 4;
  m2->from=3;
  m2->assignments.push_back(102);
  master->register_message_completion(m2);
  
  vector<int> message_indexes;
  message_indexes.push_back(1);
  message_indexes.push_back(2);
  message_indexes.push_back(4);
  message_indexes.push_back(44);
  master->completed_combinations[2].emplace(message_indexes);
  
  FILE* fout = fopen("zog.tcnf","w");
  master->dump_checkpoint(fout);
  fclose(fout);
  
  TableSolutions* master2 = new TableSolutions(d,true);
  FILE* fin = fopen("zog.tcnf","r");
  master2->load_checkpoint(fin);
  fclose(fin);
  
  for (int i=0; i<d->no_nodes; i++) {
    for (int j=0; j<d->no_nodes; j++) {
      EXPECT_EQ(master2->messages[i][j].size(),master->messages[i][j].size());
      for (int k=0; k<master2->messages[i][j].size(); k++) {
        EXPECT_EQ(master2->messages[i][j][k].from, master->messages[i][j][k].from);
        EXPECT_EQ(master2->messages[i][j][k].to, master->messages[i][j][k].to);
        EXPECT_EQ(master2->messages[i][j][k].assignments.size(), master->messages[i][j][k].assignments.size());
        for (int l=0; l<master2->messages[i][j][k].assignments.size(); l++) {
          EXPECT_EQ(master2->messages[i][j][k].assignments[l], master->messages[i][j][k].assignments[l]);
        }
      }
    }
    EXPECT_EQ(master2->additional_clauses[i]->cc,master->additional_clauses[i]->cc);
    EXPECT_EQ(master2->additional_clauses[i]->vc,master->additional_clauses[i]->vc);
    for (int c=0; c<master2->additional_clauses[i]->cc; c++) {
      EXPECT_EQ(master->additional_clauses[i]->cl[c],master2->additional_clauses[i]->cl[c]);
      for (int cc=0; cc<master->additional_clauses[i]->cl[c]; cc++) {
        int v1 = master->additional_clauses[i]->clauses[c][cc];
        int v2 = master2->additional_clauses[i]->clauses[c][cc];
        EXPECT_EQ(v1,v2);
      }
    }
    EXPECT_EQ(master2->completed_combinations[i].size(),master->completed_combinations[i].size());
  }
}






vector<set<int>> get_cubes(DdManager* ddmgr, DdNode* tmp) {
  int numvars = Cudd_ReadSize(ddmgr);
  int* cube;
  CUDD_VALUE_TYPE value;
  DdGen* gen;
  vector<set<int>> cubes;
  Cudd_ForeachCube(ddmgr, tmp, gen, cube, value) {
    set<int> s;
    s.clear();
    for (int i=1; i<numvars; i++) {
      if (cube[i]==2) continue;
      if (cube[i]==0)
        s.insert(i);
      else if (cube[i]==1)
        s.insert(-i);
    }
    cubes.push_back(s);
  }
  return cubes;
}

bool equality_of_cubes(vector<set<int>>& cubes1,vector<set<int>>& cubes2) {
  for (auto c1 : cubes1) {
    bool continued = false;
    for (auto c2 : cubes2) {
      if (c1==c2) {
        continued = true;
        break;
      }
    }
    if (continued==false)
      return false;
  }
  return true;
}



//TODO: non trivial identity comparrison with BDDSolutions interface
TEST(CheckpointingTests, BDDMasterDumpLoadIdentity) {
  Dag* d = new Dag("./good/d1.txt");
  BDDSolutions* master = new BDDSolutions(d,150);

  Message* m = new Message();
  m->to = 0;
  m->from = 0;
  m->assignments.push_back(43);
  m->assignments.push_back(41);
  master->add_message(m);
  
  Message* m2 = new Message();
  m2->to = 4;
  m2->from=3;
  m2->assignments.push_back(102);
  m2->assignments.push_back(132);
  master->register_message_completion(m2);
  
  FILE* fout = fopen("zog2.tcnf","w");
  master->dump_checkpoint(fout);
  fclose(fout);
  
  BDDSolutions* master2 = new BDDSolutions(d,150);
  FILE* fin = fopen("zog2.tcnf","r");
  master2->load_checkpoint(fin);
  fclose(fin);
  
  EXPECT_EQ(master->vc,master2->vc);
  EXPECT_EQ(master->vc,master2->vc);
  for (int i=0; i<d->no_nodes; i++) {
    vector<set<int>> a1,a2;
    a1 = get_cubes(master->ddmgr, master->completed[i]);
    a2 = get_cubes(master2->ddmgr, master2->completed[i]);
    EXPECT_EQ(equality_of_cubes(a1,a2),true);
    a1 = get_cubes(master->ddmgr, master->communicated[i]);
    a2 = get_cubes(master2->ddmgr, master2->communicated[i]);
    EXPECT_EQ(equality_of_cubes(a1,a2),true);
    for (int j=0; j<d->no_nodes; j++) {
      a1 = get_cubes(master->ddmgr, master->bdds[i][j]);
      a2 = get_cubes(master2->ddmgr, master2->bdds[i][j]);
      EXPECT_EQ(equality_of_cubes(a1,a2),true);
    }
  }
  EXPECT_EQ(master->initial_messages.size(), master2->initial_messages.size());
  for (int i=0; i<master->initial_messages.size(); i++) {
    EXPECT_EQ(master->initial_messages[i]->to, master2->initial_messages[i]->to );
    EXPECT_EQ(master->initial_messages[i]->from, master2->initial_messages[i]->from );
    EXPECT_EQ(master->initial_messages[i]->assignments.size(), master2->initial_messages[i]->assignments.size() );
    for (int j=0; j<master->initial_messages[i]->assignments.size(); j++) {
      EXPECT_EQ(master->initial_messages[i]->assignments[j],master2->initial_messages[i]->assignments[j]);
    }
  }
}




TEST(CheckpointingTests, MasterDumpLoadIdentity) {
  Dag* d = new Dag("./good/d1.txt");
  TableSolutions* solutions = new TableSolutions(d,true);
  Master* master = new Master(55,d->no_nodes, solutions, 0, 0);
  
  Message* m = new Message();
  m->to = 0;
  m->from = 0;
  m->assignments.push_back(43);
  solutions->add_message(m);
  
  Message* m2 = new Message();
  m2->to = 4;
  m2->from=3;
  m2->assignments.push_back(102);
  solutions->register_message_completion(m2);
  
  vector<int> message_indexes;
  message_indexes.push_back(1);
  message_indexes.push_back(2);
  message_indexes.push_back(4);
  message_indexes.push_back(44);
  solutions->completed_combinations[2].emplace(message_indexes);
  
  Message* m11 = new Message();
  m11->from=5;
  m11->to=4;
  m11->assignments.push_back(99);
  m11->assignments.push_back(999);
  m11->assignments.push_back(9999);
  
  Message* m12 = new Message();
  m12->from=9;
  m12->to=1;
  m12->assignments.push_back(199);
  m12->assignments.push_back(1999);
  m12->assignments.push_back(19999);
  
  master->organiser->add_message(m11);
  master->organiser->add_message(m12);
  
  master->dag_nodes_generated_solutions.insert(23);
  master->dag_nodes_given_assignments.insert(13);
  master->subgraph_finished.insert(84);
  
  Message* m21 = new Message();
  m21->from=19;
  m21->to=10;
  m21->assignments.push_back(1299);
  m21->assignments.push_back(12999);
  m21->assignments.push_back(129999);
  
  master->solutions.push_back(m21);
  
  FILE* fout = fopen("zog.tcnf","w");
  master->dump_checkpoint(fout);
  fclose(fout);
  
  TableSolutions* solutions2 = new TableSolutions(d,true);
  Master* master2 = new Master(55,d->no_nodes, solutions, 0, 0);
  FILE* fin = fopen("zog.tcnf","r");
  master2->load_checkpoint(fin);
  fclose(fin);
  
  for (int i=0; i<d->no_nodes; i++) {
    for (int j=0; j<d->no_nodes; j++) {
      EXPECT_EQ(((TableSolutions*)(master2->master))->messages[i][j].size(),((TableSolutions*)(master->master))->messages[i][j].size());
      for (int k=0; k<((TableSolutions*)(master2->master))->messages[i][j].size(); k++) {
        EXPECT_EQ(((TableSolutions*)(master2->master))->messages[i][j][k].from, ((TableSolutions*)(master->master))->messages[i][j][k].from);
        EXPECT_EQ(((TableSolutions*)(master2->master))->messages[i][j][k].to, ((TableSolutions*)(master->master))->messages[i][j][k].to);
        EXPECT_EQ(((TableSolutions*)(master2->master))->messages[i][j][k].assignments.size(), ((TableSolutions*)(master->master))->messages[i][j][k].assignments.size());
        for (int l=0; l<((TableSolutions*)(master2->master))->messages[i][j][k].assignments.size(); l++) {
          EXPECT_EQ(((TableSolutions*)(master2->master))->messages[i][j][k].assignments[l], ((TableSolutions*)(master->master))->messages[i][j][k].assignments[l]);
        }
      }
    }
    EXPECT_EQ(((TableSolutions*)(master2->master))->additional_clauses[i]->cc,((TableSolutions*)(master->master))->additional_clauses[i]->cc);
    EXPECT_EQ(((TableSolutions*)(master2->master))->additional_clauses[i]->vc,((TableSolutions*)(master->master))->additional_clauses[i]->vc);
    for (int c=0; c<((TableSolutions*)(master2->master))->additional_clauses[i]->cc; c++) {
      EXPECT_EQ(((TableSolutions*)(master->master))->additional_clauses[i]->cl[c],((TableSolutions*)(master2->master))->additional_clauses[i]->cl[c]);
      for (int cc=0; cc<((TableSolutions*)(master->master))->additional_clauses[i]->cl[c]; cc++) {
        int v1 = ((TableSolutions*)(master->master))->additional_clauses[i]->clauses[c][cc];
        int v2 = ((TableSolutions*)(master2->master))->additional_clauses[i]->clauses[c][cc];
        EXPECT_EQ(v1,v2);
      }
    }
    EXPECT_EQ(((TableSolutions*)(master2->master))->completed_combinations[i].size(),((TableSolutions*)(master->master))->completed_combinations[i].size());
  }
  EXPECT_EQ(master2->organiser->message_buffer.length,2);
  
  EXPECT_EQ(master2->organiser->message_buffer[0]->from,5);
  EXPECT_EQ(master2->organiser->message_buffer[0]->to,4);
  EXPECT_EQ(master2->organiser->message_buffer[0]->assignments[0],99);
  EXPECT_EQ(master2->organiser->message_buffer[0]->assignments[1],999);
  EXPECT_EQ(master2->organiser->message_buffer[0]->assignments[2],9999);
  EXPECT_EQ(master2->organiser->message_buffer[0]->assignments.size(),3);
  
  EXPECT_EQ(master2->organiser->message_buffer[1]->from,9);
  EXPECT_EQ(master2->organiser->message_buffer[1]->to,1);
  EXPECT_EQ(master2->organiser->message_buffer[1]->assignments[0],199);
  EXPECT_EQ(master2->organiser->message_buffer[1]->assignments[1],1999);
  EXPECT_EQ(master2->organiser->message_buffer[1]->assignments[2],19999);
  EXPECT_EQ(master2->organiser->message_buffer[1]->assignments.size(),3);
}





