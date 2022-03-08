

TEST(CnfHolderTests, basic_holder_construction) {
  Dag* dag = new Dag("./good/d1.txt");
  CnfHolder *holder = new CnfHolder(dag, "./good/c1.txt");
}


TEST(CnfHolderTests, basic_holder_cnf_generation) {
  Dag* dag = new Dag("./good/d1.txt");
  CnfHolder *holder = new CnfHolder(dag, "./good/c1.txt");
  holder->generate_decomposition();
}

TEST(CnfHolderTests, basic_holder_cnf_generation_check) {
  Dag* dag = new Dag("./good/d1.txt");
  CnfHolder *holder = new CnfHolder(dag, "./good/c1.txt");
  holder->generate_decomposition();

  int** clauses = (int**)calloc(sizeof(int*),7);
  clauses[0] = (int*)calloc(sizeof(int),5);
  clauses[1] = (int*)calloc(sizeof(int),5);
  clauses[2] = (int*)calloc(sizeof(int),5);
  clauses[3] = (int*)calloc(sizeof(int),5);
  clauses[4] = (int*)calloc(sizeof(int),5);
  clauses[5] = (int*)calloc(sizeof(int),5);
  clauses[0][0] = 1;
  clauses[0][1] = 2;
  clauses[0][2] = 3;
  clauses[1][0] = -2;
  clauses[1][1] = 3;
  clauses[1][2] = -4;
  clauses[2][0] = 1;
  clauses[2][1] = 2;
  clauses[2][2] = 4;
  
  Cnf* c = holder->get_Cnf(0);
  EXPECT_EQ(c->cc,3);
  EXPECT_EQ(c->vc,4);
  for (int i=0; clauses[i]; i++) {
    for (int j=0; clauses[i][j]; j++) {
      EXPECT_EQ(c->clauses[i][j],clauses[i][j]);
    }
  }

  c = holder->get_Cnf(1);
  EXPECT_EQ(c->cc,5);
  EXPECT_EQ(c->vc,5);
  clauses[0][0] = 1;
  clauses[0][1] = 2;
  clauses[0][2] = 3;
  clauses[0][3] = 0;
  clauses[1][0] = 3;
  clauses[1][1] = 4;
  clauses[1][2] = 5;
  clauses[1][3] = 0;
  clauses[2][0] = -3;
  clauses[2][1] = -5;
  clauses[2][2] = 0;
  clauses[3][0] = -2;
  clauses[3][1] = 3;
  clauses[3][2] = -4;
  clauses[3][3] = 0;
  clauses[4][0] = 1;
  clauses[4][1] = 2;
  clauses[4][2] = 4;
  clauses[4][3] = 0;
  for (int i=0; clauses[i]; i++) {
    for (int j=0; clauses[i][j]; j++) {
      EXPECT_EQ(c->clauses[i][j],clauses[i][j]);
    }
  }

  c = holder->get_Cnf(2);
  EXPECT_EQ(c->cc,2);
  EXPECT_EQ(c->vc,5);
  clauses[0][0] = 3;
  clauses[0][1] = 4;
  clauses[0][2] = 5;
  clauses[0][3] = 0;
  clauses[1][0] = -3;
  clauses[1][1] = -5;
  clauses[1][2] = 0;
  clauses[2][0] = 0;
  clauses[3][0] = 0;
  clauses[4][0] = 0;
  for (int i=0; clauses[i]; i++) {
    for (int j=0; clauses[i][j]; j++) {
      EXPECT_EQ(c->clauses[i][j],clauses[i][j]);
    }
  }

}


TEST(CnfHolderTests, basic_holder_cnf_generation_load_fail) {
  Dag* dag = new Dag("./good/d1.txt");
  CnfHolder *holder = new CnfHolder(dag, "./good/c1.txt");
  holder->generate_decomposition();


	EXPECT_THROW( {
		try {
Cnf* c = holder->get_Cnf(99);
		} 
		catch ( const BadParameterException& e) {
            EXPECT_STREQ("Bad parameter in Dagster: CnfHolder called with node not in dag", e.what());
			throw;
		}
	}, BadParameterException);

	EXPECT_THROW( {
		try {
Cnf* c = holder->get_Cnf(-1);
		} 
		catch ( const BadParameterException& e) {
            EXPECT_STREQ("Bad parameter in Dagster: CnfHolder called with node not in dag", e.what());
			throw;
		}
	}, BadParameterException);
  
	EXPECT_THROW( {
		try {
Cnf* c = holder->get_Cnf(-55657);
		} 
		catch ( const BadParameterException& e) {
            EXPECT_STREQ("Bad parameter in Dagster: CnfHolder called with node not in dag", e.what());
			throw;
		}
	}, BadParameterException);
}



char *readFile(char *fileName) {
  FILE *file = fopen(fileName, "r");
  char *code;
  size_t n = 0;
  int c;
  if (file == NULL)
    return NULL; //could not open file
  code = (char*)malloc(1000);
  int count = 0;
  while ((c = fgetc(file)) != EOF) {
    char cc = (char)c;
    if (cc == ' ') {
      count++;
    } else {
      count=0;
    }
    if (count <= 1) {
      code[n++] = cc;
    }
  }
  code[n] = '\0';      
  return code;
}



TEST(CnfHolderTests, basic_holder_cnf_basic_file_generation) {
  Dag* dag = new Dag("./good/d1.txt");
  CnfHolder *holder = new CnfHolder(dag, "DAG_CNF_DIRECTORY", "./good/c1.txt");
  holder->generate_decomposition();
}


TEST(CnfHolderTests, basic_holder_cnf_file_generation) {
  Dag* dag = new Dag("./good/d1.txt");
  CnfHolder *holder = new CnfHolder(dag, "DAG_CNF_DIRECTORY", "./good/c1.txt");
  holder->generate_decomposition();

  char* node0data = "p cnf 4 3 \n1 2 3 0\n-2 3 -4 0\n1 2 4 0";
  char* file0data = readFile("DAG_CNF_DIRECTORY/cnf_node0.txt");
  for (int i=0; i<35; i++)
    EXPECT_EQ(node0data[i],file0data[i]);

  char* node1data = "p cnf 5 5 \n1 2 3 0\n3 4 5 0\n-3 -5 0\n-2 3 -4 0\n1 2 4 0";
  char* file1data = readFile("DAG_CNF_DIRECTORY/cnf_node1.txt");
  for (int i=0; i<51; i++)
    EXPECT_EQ(node1data[i],file1data[i]);

  char* node2data = "p cnf 5 2 \n3 4 5 0\n-3 -5 0";
  char* file2data = readFile("DAG_CNF_DIRECTORY/cnf_node2.txt");
  for (int i=0; i<24; i++)
    EXPECT_EQ(node2data[i],file2data[i]);

  char* node3data = "p cnf 6 5 \n3 4 5 0\n-3 -5 0\n-4 5 -6 0\n1 2 4 0\n3 4 -6 0";
  char* file3data = readFile("DAG_CNF_DIRECTORY/cnf_node3.txt");
  for (int i=0; i<52; i++)
    EXPECT_EQ(node3data[i],file3data[i]);
}


TEST(CnfHolderTests, basic_holder_cnf_file_generation_check) {
  Dag* dag = new Dag("./good/d1.txt");
  CnfHolder *holder = new CnfHolder(dag, "DAG_CNF_DIRECTORY", "./good/c1.txt");
  holder->generate_decomposition();

  int** clauses = (int**)calloc(sizeof(int*),7);
  clauses[0] = (int*)calloc(sizeof(int),5);
  clauses[1] = (int*)calloc(sizeof(int),5);
  clauses[2] = (int*)calloc(sizeof(int),5);
  clauses[3] = (int*)calloc(sizeof(int),5);
  clauses[4] = (int*)calloc(sizeof(int),5);
  clauses[5] = (int*)calloc(sizeof(int),5);
  clauses[0][0] = 1;
  clauses[0][1] = 2;
  clauses[0][2] = 3;
  clauses[1][0] = -2;
  clauses[1][1] = 3;
  clauses[1][2] = -4;
  clauses[2][0] = 1;
  clauses[2][1] = 2;
  clauses[2][2] = 4;
  
  Cnf* c = holder->get_Cnf(0);
  EXPECT_EQ(c->cc,3);
  EXPECT_EQ(c->vc,4);
  for (int i=0; clauses[i]; i++) {
    for (int j=0; clauses[i][j]; j++) {
      EXPECT_EQ(c->clauses[i][j],clauses[i][j]);
    }
  }

  c = holder->get_Cnf(1);
  EXPECT_EQ(c->cc,5);
  EXPECT_EQ(c->vc,5);
  clauses[0][0] = 1;
  clauses[0][1] = 2;
  clauses[0][2] = 3;
  clauses[0][3] = 0;
  clauses[1][0] = 3;
  clauses[1][1] = 4;
  clauses[1][2] = 5;
  clauses[1][3] = 0;
  clauses[2][0] = -3;
  clauses[2][1] = -5;
  clauses[2][2] = 0;
  clauses[3][0] = -2;
  clauses[3][1] = 3;
  clauses[3][2] = -4;
  clauses[3][3] = 0;
  clauses[4][0] = 1;
  clauses[4][1] = 2;
  clauses[4][2] = 4;
  clauses[4][3] = 0;
  for (int i=0; clauses[i]; i++) {
    for (int j=0; clauses[i][j]; j++) {
      EXPECT_EQ(c->clauses[i][j],clauses[i][j]);
    }
  }

  c = holder->get_Cnf(2);
  EXPECT_EQ(c->cc,2);
  EXPECT_EQ(c->vc,5);
  clauses[0][0] = 3;
  clauses[0][1] = 4;
  clauses[0][2] = 5;
  clauses[0][3] = 0;
  clauses[1][0] = -3;
  clauses[1][1] = -5;
  clauses[1][2] = 0;
  clauses[2][0] = 0;
  clauses[3][0] = 0;
  clauses[4][0] = 0;
  for (int i=0; clauses[i]; i++) {
    for (int j=0; clauses[i][j]; j++) {
      EXPECT_EQ(c->clauses[i][j],clauses[i][j]);
    }
  }

}

