

TEST(MessageTests, basic_assignment_assignment) {
  Message* m;
  m = new Message();
  m->to = 1;
  m->from = 1;
  m->assignments.clear();
}

TEST(MessageTests, basic_constructor) {
  Message* m;
  m = new Message(2, 1);
}


TEST(MessageTests, hydrate_check) {
  Message* m;
  m = new Message(2, 1);
  m->assignments.push_back(5);
  m->assignments.push_back(4);
  m->assignments.push_back(2);

  int* data = (int*)calloc(sizeof(int),9999);
  m->dehydrate(data);

  int reference_data[] = {5+3, 2, 1, 0, 3, 5, 4, 2};
  for (int i=0; i<8; i++) {
    EXPECT_EQ(data[i],reference_data[i]);
  }
}

TEST(MessageTests, hydrate_check_with_additional_clauses) {
  Message* m;
  m = new Message(2, 1);
  m->assignments.push_back(5);
  m->assignments.push_back(4);
  m->assignments.push_back(2);

  int** additional_clauses = (int**)calloc(sizeof(int*),4);
  additional_clauses[0] = (int*)calloc(sizeof(int),4);
  additional_clauses[0][0] = 55;
  additional_clauses[0][1] = 56;
  additional_clauses[0][2] = -57;
  additional_clauses[1] = (int*)calloc(sizeof(int),4);
  additional_clauses[1][0] = 50;
  additional_clauses[1][1] = 99;
  additional_clauses[1][2] = -52;

  m->additional_clauses = new Cnf(additional_clauses);


  int* data = (int*)calloc(sizeof(int),9999);
  m->dehydrate(data);

  int reference_data[] = {21, 2, 1, 1, 3, 5, 4, 2, 13, 2, 3, 3, 0, 55, 56, -57, 0, 50, 99, -52, 0};
  for (int i=0; i<8; i++) {
    EXPECT_EQ(data[i],reference_data[i]);
  }
}

TEST(MessageTests, dehydrate_check) {
  int reference_data[] = {5+3, 2, 1, 0, 3, 5, 4, 2};
  Message* m = new Message(reference_data);
  EXPECT_EQ(m->to,2);
  EXPECT_EQ(m->from,1);
  EXPECT_EQ(m->assignments.size(),3);
  EXPECT_TRUE((m->assignments[0]==5) || (m->assignments[0]==4) || (m->assignments[0]==2));
  EXPECT_TRUE((m->assignments[1]==5) || (m->assignments[1]==4) || (m->assignments[1]==2));
  EXPECT_TRUE((m->assignments[2]==5) || (m->assignments[2]==4) || (m->assignments[2]==2));
}


TEST(MessageTests, dehydrate_check_with_additional_clauses) {
  int reference_data[] = {21, 2, 1, 1, 3, 5, 4, 2, 13, 2, 3, 3, 0, 1, 2, -3, 0, 3, 2, -1, 0};
  Message* m = new Message(reference_data);
  EXPECT_EQ(m->to,2);
  EXPECT_EQ(m->from,1);
  EXPECT_EQ(m->assignments.size(),3);
  EXPECT_TRUE((m->assignments[0]==5) || (m->assignments[0]==4) || (m->assignments[0]==2));
  EXPECT_TRUE((m->assignments[1]==5) || (m->assignments[1]==4) || (m->assignments[1]==2));
  EXPECT_TRUE((m->assignments[2]==5) || (m->assignments[2]==4) || (m->assignments[2]==2));
  
  EXPECT_TRUE(m->additional_clauses != NULL);
  EXPECT_EQ(m->additional_clauses->cc,2);
  EXPECT_EQ(m->additional_clauses->vc,3);
  EXPECT_EQ(m->additional_clauses->cl[0],3);
  EXPECT_EQ(m->additional_clauses->cl[1],3);
  
  EXPECT_EQ(m->additional_clauses->clauses[0][0],1);
  EXPECT_EQ(m->additional_clauses->clauses[0][1],2);
  EXPECT_EQ(m->additional_clauses->clauses[0][2],-3);
  EXPECT_EQ(m->additional_clauses->clauses[1][0],3);
  EXPECT_EQ(m->additional_clauses->clauses[1][1],2);
  EXPECT_EQ(m->additional_clauses->clauses[1][2],-1);
}




TEST(MessageTests, hydrate_dehydrate_identity) {
  Message* m;
  m = new Message(2, 1);
  m->assignments.push_back(5);
  m->assignments.push_back(4);
  m->assignments.push_back(2);

  int* data = (int*)calloc(sizeof(int),9999);
  m->dehydrate(data);

  Message* m2;
  m2 = new Message(data);

  EXPECT_EQ(m2->to,2);
  EXPECT_EQ(m2->from,1);
  EXPECT_EQ(m2->assignments.size(),3);
  EXPECT_EQ(m2->assignments[0],5);
  EXPECT_EQ(m2->assignments[1],4);
  EXPECT_EQ(m2->assignments[2],2);
}

TEST(MessageTests, hydrate_dehydrate_identity_with_additional_clauses) {
  Message* m;
  m = new Message(2, 1);
  m->assignments.push_back(5);
  m->assignments.push_back(4);
  m->assignments.push_back(2);

  int** additional_clauses = (int**)calloc(sizeof(int*),4);
  additional_clauses[0] = (int*)calloc(sizeof(int),4);
  additional_clauses[0][0] = 1;
  additional_clauses[0][1] = 2;
  additional_clauses[0][2] = -3;
  additional_clauses[1] = (int*)calloc(sizeof(int),4);
  additional_clauses[1][0] = 3;
  additional_clauses[1][1] = 2;
  additional_clauses[1][2] = -1;

  m->additional_clauses = new Cnf(additional_clauses);

  int* data = (int*)calloc(sizeof(int),9999);
  m->dehydrate(data);

  Message* m2 = new Message(data);

  EXPECT_EQ(m2->to,2);
  EXPECT_EQ(m2->from,1);
  EXPECT_EQ(m2->assignments.size(),3);
  EXPECT_EQ(m2->assignments[0],5);
  EXPECT_EQ(m2->assignments[1],4);
  EXPECT_EQ(m2->assignments[2],2);
  
  EXPECT_TRUE(m2->additional_clauses != NULL);
  EXPECT_EQ(m2->additional_clauses->cc,2);
  EXPECT_EQ(m2->additional_clauses->vc,3);
  EXPECT_EQ(m2->additional_clauses->cl[0],3);
  EXPECT_EQ(m2->additional_clauses->cl[1],3);
  
  EXPECT_EQ(m2->additional_clauses->clauses[0][0],1);
  EXPECT_EQ(m2->additional_clauses->clauses[0][1],2);
  EXPECT_EQ(m2->additional_clauses->clauses[0][2],-3);
  EXPECT_EQ(m2->additional_clauses->clauses[1][0],3);
  EXPECT_EQ(m2->additional_clauses->clauses[1][1],2);
  EXPECT_EQ(m2->additional_clauses->clauses[1][2],-1);
}




TEST(MessageTests, message_duplicate) {
  Message* m;
  m = new Message(2, 1);
  m->assignments.push_back(5);
  m->assignments.push_back(4);
  m->assignments.push_back(2);

  Message* m2;
  m2 = new Message();
  m2->set(m);

  EXPECT_EQ(m2->to,2);
  EXPECT_EQ(m2->from,1);
  EXPECT_EQ(m2->assignments.size(),3);
  EXPECT_EQ(m2->assignments[0],5);
  EXPECT_EQ(m2->assignments[1],4);
  EXPECT_EQ(m2->assignments[2],2);
}


TEST(MessageTests, message_duplicate_with_additional_clauses) {
  Message* m;
  m = new Message(2, 1);
  m->assignments.push_back(5);
  m->assignments.push_back(4);
  m->assignments.push_back(2);

  int** additional_clauses = (int**)calloc(sizeof(int*),4);
  additional_clauses[0] = (int*)calloc(sizeof(int),4);
  additional_clauses[0][0] = 1;
  additional_clauses[0][1] = 2;
  additional_clauses[0][2] = -3;
  additional_clauses[1] = (int*)calloc(sizeof(int),4);
  additional_clauses[1][0] = 3;
  additional_clauses[1][1] = 2;
  additional_clauses[1][2] = -1;

  m->additional_clauses = new Cnf(additional_clauses);

  Message *m2 = new Message(m);

  EXPECT_EQ(m2->to,2);
  EXPECT_EQ(m2->from,1);
  EXPECT_EQ(m2->assignments.size(),3);
  EXPECT_EQ(m2->assignments[0],5);
  EXPECT_EQ(m2->assignments[1],4);
  EXPECT_EQ(m2->assignments[2],2);
  EXPECT_TRUE(m2->additional_clauses != NULL);
  EXPECT_EQ(m2->additional_clauses->cc,2);
  EXPECT_EQ(m2->additional_clauses->vc,3);
  EXPECT_EQ(m2->additional_clauses->cl[0],3);
  EXPECT_EQ(m2->additional_clauses->cl[1],3);

  for (int i=0; m->additional_clauses->clauses[i]; i++) {
    for (int j=0; m->additional_clauses->clauses[i][j]; j++) {
      EXPECT_EQ(m->additional_clauses->clauses[i][j],m2->additional_clauses->clauses[i][j]);
    }
  }
  for (int i=0; m2->additional_clauses->clauses[i]; i++) {
    for (int j=0; m2->additional_clauses->clauses[i][j]; j++) {
      EXPECT_EQ(m->additional_clauses->clauses[i][j],m2->additional_clauses->clauses[i][j]);
    }
  }

}


TEST(MessageTests, message_dehydrate_size_test) {
  Message* m;
  m = new Message(2, 1);
  m->assignments.push_back(5);
  m->assignments.push_back(4);
  m->assignments.push_back(2);

  EXPECT_EQ(m->get_dehydrated_size(),8);
  int* data = (int*)calloc(sizeof(int),9999);
  m->dehydrate(data);
  EXPECT_EQ(data[0],m->get_dehydrated_size());

  int** additional_clauses = (int**)calloc(sizeof(int*),4);
  additional_clauses[0] = (int*)calloc(sizeof(int),4);
  additional_clauses[0][0] = 1;
  additional_clauses[0][1] = 2;
  additional_clauses[0][2] = -3;
  additional_clauses[1] = (int*)calloc(sizeof(int),4);
  additional_clauses[1][0] = 3;
  additional_clauses[1][1] = 2;
  additional_clauses[1][2] = -1;

  m->additional_clauses = new Cnf(additional_clauses);

  EXPECT_EQ(m->get_dehydrated_size(),21);
  m->dehydrate(data);
  EXPECT_EQ(data[0],m->get_dehydrated_size());

  m = new Message(0,0);
  EXPECT_EQ(m->get_dehydrated_size(),5);
  m->dehydrate(data);
  EXPECT_EQ(data[0],m->get_dehydrated_size());
}



