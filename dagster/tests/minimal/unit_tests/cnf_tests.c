

TEST(CnfTests, cnf_succeeds_to_load_good_file) {
  Cnf("./good/c1.txt");
}

TEST(CnfTests, cnf_successfully_data_of_good_file) {
	Cnf* a = new Cnf("./good/c1.txt");
	ASSERT_EQ(a->cc,8);
	ASSERT_EQ(a->clauses[0][0],1);
	ASSERT_EQ(a->clauses[0][1],2);
	ASSERT_EQ(a->clauses[0][2],3);
	ASSERT_EQ(a->clauses[4][0],-2);
	ASSERT_EQ(a->clauses[4][1],3);
	ASSERT_EQ(a->clauses[4][2],-4);
	ASSERT_EQ(a->vc,6);
	ASSERT_EQ(a->cl[3],2);
}

TEST(CnfTests, cnf_fails_to_load_nonexisting_file) {
	ASSERT_EXIT(Cnf("hunkydory.txt"), ::testing::ExitedWithCode(1),"");
}


TEST(CnfTests, cnf_fails_to_load_bad_header_args2) {
	EXPECT_THROW( {
		try {
			Cnf("./bad/cnfs/bad_header_args2.txt");
		} 
		catch ( const ParsingException& e) {
            EXPECT_STREQ("Parsing File Failed: CNF has header that dosnt match its body - bad variable count or clause count \n", e.what());
			throw;
		}
	}, ParsingException);
}
TEST(CnfTests, cnf_fails_to_load_bad_header_args3) {
	EXPECT_THROW( {
		try {
			Cnf("./bad/cnfs/bad_header_args3.txt");
		} 
		catch ( const ParsingException& e) {
            EXPECT_STREQ("Parsing File Failed: CNF has header that dosnt match its body - bad variable count or clause count \n", e.what());
			throw;
		}
	}, ParsingException);
}
TEST(CnfTests, cnf_fails_to_load_bad_header_args4) {
	EXPECT_THROW( {
		try {
			Cnf("./bad/cnfs/bad_header_args4.txt");
		} 
		catch ( const ParsingException& e) {
            EXPECT_STREQ("Parsing File Failed: CNF has header that dosnt match its body - bad variable count or clause count \n", e.what());
			throw;
		}
	}, ParsingException);
}

TEST(CnfTests, cnf_fails_to_load_bad_header_args5) {
	EXPECT_THROW( {
		try {
			Cnf("./bad/cnfs/bad_header_args5.txt");
		} 
		catch ( const ParsingException& e) {
            EXPECT_STREQ("Parsing File Failed:  Invalid CNF file\n", e.what());
			throw;
		}
	}, ParsingException);
}

TEST(CnfTests, cnf_fails_to_load_bad_header_args) {
	EXPECT_THROW( {
		try {
			Cnf("./bad/cnfs/bad_header_args.txt");
		} 
		catch ( const ParsingException& e) {
            EXPECT_STREQ("Parsing File Failed: CNF has header that dosnt match its body - bad variable count or clause count \n", e.what());
			throw;
		}
	}, ParsingException);
}

TEST(CnfTests, cnf_fails_to_load_bad_header) {
	EXPECT_THROW( {
		try {
			Cnf("./bad/cnfs/bad_header.txt");
		} 
		catch ( const ParsingException& e) {
            EXPECT_STREQ("Parsing File Failed:  Invalid CNF file\n", e.what());
			throw;
		}
	}, ParsingException);
}

TEST(CnfTests, cnf_fails_to_load_garbage_literals2) {
	EXPECT_THROW( {
		try {
			Cnf* c = new Cnf("./bad/cnfs/garbage_literals2.txt");
		} 
		catch ( const ParsingException& e) {
            EXPECT_STREQ("Parsing File Failed: CNF has header that dosnt match its body - bad variable count or clause count \n", e.what());
			throw;
		}
	}, ParsingException);
}


TEST(CnfTests, cnf_successfully_loads_garbage_literals3) {
	Cnf* a = new Cnf("./bad/cnfs/garbage_literals3.txt");
	ASSERT_EQ(a->cc,8);
}

TEST(CnfTests, cnf_fails_to_load_garbage_litterals) {
	EXPECT_THROW( {
		try {
			Cnf("./bad/cnfs/garbage_litterals.txt");
		} 
		catch ( const ParsingException& e) {
            EXPECT_STREQ("Parsing File Failed: Invalid CNF file - cnf lines must involve digits and terminate with zero\n", e.what());
			throw;
		}
	}, ParsingException);
}

TEST(CnfTests, cnf_fails_to_load_duplicate_literals) {
	EXPECT_THROW( {
		try {
			Cnf* a = new Cnf("./bad/cnfs/duplicate_literals.txt");
		} 
		catch ( const ParsingException& e) {
            EXPECT_STREQ("Parsing File Failed: duplicate literals in clause in CNF file \n", e.what());
			throw;
		}
	}, ParsingException);
}

TEST(CnfTests, cnf_fails_to_load_contradicting_clauses) {
	EXPECT_THROW( {
		try {
			Cnf* a = new Cnf("./bad/cnfs/skip_clauses.txt");
		} 
		catch ( const ParsingException& e) {
            EXPECT_STREQ("Parsing File Failed: contradicting literals in clause in CNF file \n", e.what());
			throw;
		}
	}, ParsingException);
}


TEST(CnfTests, cnf_occurence_buffers) {
	Cnf* a = new Cnf("./good/c1.txt");
	a->compute_occurance_buffers();
	ASSERT_EQ(a->numOccurence[a->vc+1],2);
	ASSERT_EQ(a->numOccurence[a->vc+2],2);
	ASSERT_EQ(a->numOccurence[a->vc+3],4);
	ASSERT_EQ(a->numOccurence[a->vc+4],3);
	ASSERT_EQ(a->numOccurence[a->vc-1],0);
	ASSERT_EQ(a->numOccurence[a->vc-2],1);
	ASSERT_EQ(a->numOccurence[a->vc-3],1);
	ASSERT_EQ(a->numOccurence[a->vc-4],2);

	EXPECT_TRUE( (a->occurence[a->vc+1][0]==6) || (a->occurence[a->vc+1][0]==0) );
	EXPECT_TRUE( (a->occurence[a->vc+1][1]==6) || (a->occurence[a->vc+1][1]==0) );
}


TEST(CnfTests, cnf_neighborhoods) {
	Cnf* a = new Cnf("./good/c1.txt");
	a->compute_occurance_buffers();
	a->compute_variable_neighborhoods();
	int j=0;
	for (int* k=a->neighbourVar[1]; *k != 0; k++)
		j+=*k;
	ASSERT_EQ(j,2+3+4);
	j=0;
	for (int* k=a->neighbourVar[2]; *k != 0; k++)
		j+=*k;
	ASSERT_EQ(j,1+3+4);
	j=0;
	for (int* k=a->neighbourVar[3]; *k != 0; k++)
		j+=*k;
	ASSERT_EQ(j,1+2+4+5+6);
	j=0;
	for (int* k=a->neighbourVar[4]; *k != 0; k++)
		j+=*k;
	ASSERT_EQ(j,1+2+3+5+6);
	j=0;
	
}


TEST(CnfTests, cnf_neighborhoods_2) {
	Cnf* a = new Cnf("./good/c1.txt");
	a->compute_occurance_buffers();
	a->compute_variable_neighborhoods__DEPRECATED();
	int j=0;
	for (int* k=a->neighbourVar[1]; *k != 0; k++)
		j+=*k;
	ASSERT_EQ(j,2+3+4);
	j=0;
	for (int* k=a->neighbourVar[2]; *k != 0; k++)
		j+=*k;
	ASSERT_EQ(j,1+3+4);
	j=0;
	for (int* k=a->neighbourVar[3]; *k != 0; k++)
		j+=*k;
	ASSERT_EQ(j,1+2+4+5+6);
	j=0;
	for (int* k=a->neighbourVar[4]; *k != 0; k++)
		j+=*k;
	ASSERT_EQ(j,1+2+3+5+6);
	j=0;
	
}


TEST(CnfTests, cnf_neighborhoods_3) {
	Cnf* a = new Cnf("./good/c1.txt");
	a->compute_occurance_buffers();
	a->compute_variable_neighborhoods__DEPRECATED_2();
	int j=0;
	for (int* k=a->neighbourVar[1]; *k != 0; k++)
		j+=*k;
	ASSERT_EQ(j,2+3+4);
	j=0;
	for (int* k=a->neighbourVar[2]; *k != 0; k++)
		j+=*k;
	ASSERT_EQ(j,1+3+4);
	j=0;
	for (int* k=a->neighbourVar[3]; *k != 0; k++)
		j+=*k;
	ASSERT_EQ(j,1+2+4+5+6);
	j=0;
	for (int* k=a->neighbourVar[4]; *k != 0; k++)
		j+=*k;
	ASSERT_EQ(j,1+2+3+5+6);
	j=0;
	
}


TEST(CnfTests, hydrate_dehydrate_NULL) {
	EXPECT_THROW( {
		try {
  int** additional_clauses = NULL;
  Cnf* a = new Cnf(additional_clauses);
		} 
		catch ( const BadParameterException& e) {
            EXPECT_STREQ("Bad parameter in Dagster: CNF cannot be instantiated from null data", e.what());
			throw;
		}
	}, BadParameterException);
}



TEST(CnfTests, test_dehydrate) {
  int** additional_clauses = (int**)calloc(sizeof(int*),4);
  additional_clauses[0] = (int*)calloc(sizeof(int),4);
  additional_clauses[0][0] = 55;
  additional_clauses[0][1] = 56;
  additional_clauses[0][2] = -57;
  additional_clauses[1] = (int*)calloc(sizeof(int),4);
  additional_clauses[1][0] = 50;
  additional_clauses[1][1] = 99;
  additional_clauses[1][2] = -52;

  Cnf* a = new Cnf(additional_clauses);
  int* dehydrated_clauses = (int*)calloc(sizeof(int),99999);
  EXPECT_EQ(a->get_dehydrated_size(),13);
  EXPECT_EQ(a->dehydrate(dehydrated_clauses),13);

  static int referecne_data[] = {13, 2, 3, 3, 0, 55, 56, -57, 0, 50, 99, -52, 0};
  
  for (int i=0; i<13; i++)
    EXPECT_EQ(dehydrated_clauses[i],referecne_data[i]);
}


TEST(CnfTests, test_hydrate) {
  int** additional_clauses = (int**)calloc(sizeof(int*),4);
  additional_clauses[0] = (int*)calloc(sizeof(int),4);
  additional_clauses[0][0] = 1;
  additional_clauses[0][1] = 2;
  additional_clauses[0][2] = -3;
  additional_clauses[1] = (int*)calloc(sizeof(int),4);
  additional_clauses[1][0] = 3;
  additional_clauses[1][1] = 2;
  additional_clauses[1][2] = -1;

  int reference_data[] = {13, 2, 3, 3, 0, 1, 2, -3, 0, 3, 2, -1, 0};
  Cnf* b = new Cnf(reference_data);
  for (int i=0; b->clauses[i]; i++) {
    for (int j=0; b->clauses[i][j]; j++) {
      EXPECT_EQ(b->clauses[i][j],additional_clauses[i][j]);
    }
  }
  for (int i=0; additional_clauses[i]; i++) {
    for (int j=0; additional_clauses[i][j]; j++) {
      EXPECT_EQ(b->clauses[i][j],additional_clauses[i][j]);
    }
  }
  EXPECT_EQ(b->cc,2);
  EXPECT_EQ(b->cl[0],3);
  EXPECT_EQ(b->cl[1],3);
  EXPECT_EQ(b->vc,3);
}



TEST(CnfTests, hydrate_dehydrate) {
  int** additional_clauses = (int**)calloc(sizeof(int*),4);
  additional_clauses[0] = (int*)calloc(sizeof(int),4);
  additional_clauses[0][0] = 55;
  additional_clauses[0][1] = 56;
  additional_clauses[0][2] = -57;
  additional_clauses[1] = (int*)calloc(sizeof(int),4);
  additional_clauses[1][0] = 50;
  additional_clauses[1][1] = 99;
  additional_clauses[1][2] = -52;

  Cnf* a = new Cnf(additional_clauses);
  int* dehydrated_clauses = (int*)calloc(sizeof(int),99999);
  a->dehydrate(dehydrated_clauses);
  Cnf* b = new Cnf(dehydrated_clauses);

  for (int i=0; b->clauses[i]; i++) {
    for (int j=0; b->clauses[i][j]; j++) {
      EXPECT_EQ(b->clauses[i][j],additional_clauses[i][j]);
    }
  }
  for (int i=0; additional_clauses[i]; i++) {
    for (int j=0; additional_clauses[i][j]; j++) {
      EXPECT_EQ(b->clauses[i][j],additional_clauses[i][j]);
    }
  }
}









TEST(CnfTests, test_create_dereferenced) {
  int** additional_clauses = (int**)calloc(sizeof(int*),4);
  additional_clauses[0] = (int*)calloc(sizeof(int),4);
  additional_clauses[0][0] = 55;
  additional_clauses[0][1] = 56;
  additional_clauses[0][2] = -57;
  additional_clauses[1] = (int*)calloc(sizeof(int),4);
  additional_clauses[1][0] = 50;
  additional_clauses[1][1] = 99;
  additional_clauses[1][2] = -52;

  Cnf* a = new Cnf(additional_clauses, true);
  Cnf* b = new Cnf(additional_clauses, false);

  for (int i=0; b->clauses[i]; i++) {
    for (int j=0; b->clauses[i][j]; j++) {
      EXPECT_EQ(b->clauses[i][j],a->clauses[i][j]);
    }
  }
  for (int i=0; a->clauses[i]; i++) {
    for (int j=0; a->clauses[i][j]; j++) {
      EXPECT_EQ(b->clauses[i][j],a->clauses[i][j]);
    }
  }

  additional_clauses[0][1] = 992;

  for (int i=0; b->clauses[i]; i++) {
    for (int j=0; b->clauses[i][j]; j++) {
      if ((i!=0)||(j!=1))
        EXPECT_EQ(b->clauses[i][j],a->clauses[i][j]);
      else
        EXPECT_TRUE(b->clauses[i][j]!=a->clauses[i][j]);
    }
  }
  for (int i=0; a->clauses[i]; i++) {
    for (int j=0; a->clauses[i][j]; j++) {
      if ((i!=0)||(j!=1))
        EXPECT_EQ(b->clauses[i][j],a->clauses[i][j]);
      else
        EXPECT_TRUE(b->clauses[i][j]!=a->clauses[i][j]);
    }
  }
}



TEST(CnfTests, test_join) {
  int** ac1 = (int**)calloc(sizeof(int*),4);
  ac1[0] = (int*)calloc(sizeof(int),4);
  ac1[0][0] = 1;
  ac1[0][1] = 2;
  ac1[0][2] = -3;
  ac1[1] = (int*)calloc(sizeof(int),4);
  ac1[1][0] = 3;
  ac1[1][1] = 2;
  ac1[1][2] = -1;


  int** ac2 = (int**)calloc(sizeof(int*),4);
  ac2[0] = (int*)calloc(sizeof(int),4);
  ac2[0][0] = 2;
  ac2[0][1] = 3;
  ac2[0][2] = -5;
  ac2[1] = (int*)calloc(sizeof(int),4);
  ac2[1][0] = 4;
  ac2[1][1] = 5;
  ac2[1][2] = -2;


  int** ac3 = (int**)calloc(sizeof(int*),5);
  ac3[0] = (int*)calloc(sizeof(int),4);
  ac3[0][0] = 1;
  ac3[0][1] = 2;
  ac3[0][2] = -3;
  ac3[1] = (int*)calloc(sizeof(int),4);
  ac3[1][0] = 3;
  ac3[1][1] = 2;
  ac3[1][2] = -1;
  ac3[2] = (int*)calloc(sizeof(int),4);
  ac3[2][0] = 2;
  ac3[2][1] = 3;
  ac3[2][2] = -5;
  ac3[3] = (int*)calloc(sizeof(int),4);
  ac3[3][0] = 4;
  ac3[3][1] = 5;
  ac3[3][2] = -2;

  Cnf* a = new Cnf(ac1);
  Cnf* b = new Cnf(ac2);
  a->join(b);

  for (int i=0; a->clauses[i]; i++) {
    for (int j=0; a->clauses[i][j]; j++) {
      EXPECT_EQ(a->clauses[i][j],ac3[i][j]);
    }
  }
  for (int i=0; ac3[i]; i++) {
    for (int j=0; ac3[i][j]; j++) {
      EXPECT_EQ(a->clauses[i][j],ac3[i][j]);
    }
  }

  EXPECT_EQ(a->cc,4);
  EXPECT_EQ(a->cl[0],3);
  EXPECT_EQ(a->cl[1],3);
  EXPECT_EQ(a->cl[2],3);
  EXPECT_EQ(a->cl[3],3);
  EXPECT_EQ(a->vc,5);

  // test that the dereferencing also works
  ac2[1][1]=99;
  for (int i=0; a->clauses[i]; i++) {
    for (int j=0; a->clauses[i][j]; j++) {
      EXPECT_EQ(a->clauses[i][j],ac3[i][j]);
    }
  }
}



TEST(CnfTests, test_DIMACS_output_and_load) {
  int** additional_clauses = (int**)calloc(sizeof(int*),4);
  additional_clauses[0] = (int*)calloc(sizeof(int),4);
  additional_clauses[0][0] = 55;
  additional_clauses[0][1] = 56;
  additional_clauses[0][2] = -57;
  additional_clauses[1] = (int*)calloc(sizeof(int),4);
  additional_clauses[1][0] = 50;
  additional_clauses[1][1] = 99;
  additional_clauses[1][2] = -52;

  Cnf* a = new Cnf(additional_clauses);
  a->output_dimacs("donkey1223.tcnf");
  Cnf* b = new Cnf("donkey1223.tcnf");

  for (int i=0; b->clauses[i]; i++) {
    for (int j=0; b->clauses[i][j]; j++) {
      EXPECT_EQ(b->clauses[i][j],additional_clauses[i][j]);
    }
  }
  for (int i=0; additional_clauses[i]; i++) {
    for (int j=0; additional_clauses[i][j]; j++) {
      EXPECT_EQ(b->clauses[i][j],additional_clauses[i][j]);
    }
  }
  EXPECT_EQ(b->cc,2);
  EXPECT_EQ(b->cl[0],3);
  EXPECT_EQ(b->cl[1],3);
  EXPECT_EQ(b->vc,99);
}



TEST(CnfTests, cnf_neighborhoods_unit_add) {
	Cnf* a = new Cnf("./good/c1.txt");
	a->compute_variable_neighborhoods();
	a->add_unitary_clause(5);
	a->add_unitary_clause(-3);
	
	int j=0;
	for (int* k=a->neighbourVar[1]; *k != 0; k++)
		j+=*k;
	ASSERT_EQ(j,2+3+4);
	j=0;
	for (int* k=a->neighbourVar[2]; *k != 0; k++)
		j+=*k;
	ASSERT_EQ(j,1+3+4);
	j=0;
	for (int* k=a->neighbourVar[3]; *k != 0; k++)
		j+=*k;
	ASSERT_EQ(j,1+2+4+5+6);
	j=0;
	for (int* k=a->neighbourVar[4]; *k != 0; k++)
		j+=*k;
	ASSERT_EQ(j,1+2+3+5+6);
	j=0;
	for (int* k=a->neighbourVar[5]; *k != 0; k++)
		j+=*k;
	ASSERT_EQ(j,3+4+6);
	j=0;
	for (int* k=a->neighbourVar[6]; *k != 0; k++)
		j+=*k;
	ASSERT_EQ(j,3+4+5);
	j=0;
	
	for (int i=0; i<a->numOccurence[a->vc+1]; i++) {
		j+=a->occurence[a->vc+1][i];
	}
	ASSERT_EQ(j,0+6);
	j=0;
	for (int i=0; i<a->numOccurence[a->vc-1]; i++) {
		j+=a->occurence[a->vc-1][i];
	}
	ASSERT_EQ(j,0);
	j=0;
	
	for (int i=0; i<a->numOccurence[a->vc+2]; i++) {
		j+=a->occurence[a->vc+2][i];
	}
	ASSERT_EQ(j,0+6);
	j=0;
	for (int i=0; i<a->numOccurence[a->vc-2]; i++) {
		j+=a->occurence[a->vc-2][i];
	}
	ASSERT_EQ(j,4);
	j=0;
	
	for (int i=0; i<a->numOccurence[a->vc+3]; i++) {
		j+=a->occurence[a->vc+3][i];
	}
	ASSERT_EQ(j,0+1+4+7);
	j=0;
	for (int i=0; i<a->numOccurence[a->vc-3]; i++) {
		j+=a->occurence[a->vc-3][i];
	}
	ASSERT_EQ(j,3+9);
	j=0;
	
	for (int i=0; i<a->numOccurence[a->vc+4]; i++) {
		j+=a->occurence[a->vc+4][i];
	}
	ASSERT_EQ(j,1+6+7);
	j=0;
	for (int i=0; i<a->numOccurence[a->vc-4]; i++) {
		j+=a->occurence[a->vc-4][i];
	}
	ASSERT_EQ(j,4+5);
	j=0;
	
	for (int i=0; i<a->numOccurence[a->vc+5]; i++) {
		j+=a->occurence[a->vc+5][i];
	}
	ASSERT_EQ(j,1+2+5+8);
	j=0;
	for (int i=0; i<a->numOccurence[a->vc-5]; i++) {
		j+=a->occurence[a->vc-5][i];
	}
	ASSERT_EQ(j,3);
	j=0;
	
	for (int i=0; i<a->numOccurence[a->vc+6]; i++) {
		j+=a->occurence[a->vc+6][i];
	}
	ASSERT_EQ(j,2);
	j=0;
	for (int i=0; i<a->numOccurence[a->vc-6]; i++) {
		j+=a->occurence[a->vc-6][i];
	}
	ASSERT_EQ(j,5+7);
	j=0;
}



TEST(CnfTests, cnf_neighborhoods_clause_add) {
	Cnf* a = new Cnf("./good/c1.txt");
	a->compute_variable_neighborhoods();
	
	Cnf* b = new Cnf("./good/c3.txt");
	a->add_clause(b->clauses[0]);
	a->add_clause(b->clauses[1]);
	
	int j=0;
	for (int* k=a->neighbourVar[1]; *k != 0; k++)
		j+=*k;
	ASSERT_EQ(j,2+3+4+5);
	j=0;
	for (int* k=a->neighbourVar[2]; *k != 0; k++)
		j+=*k;
	ASSERT_EQ(j,1+3+4+6);
	j=0;
	for (int* k=a->neighbourVar[3]; *k != 0; k++)
		j+=*k;
	ASSERT_EQ(j,1+2+4+5+6);
	j=0;
	for (int* k=a->neighbourVar[4]; *k != 0; k++)
		j+=*k;
	ASSERT_EQ(j,1+2+3+5+6);
	j=0;
	for (int* k=a->neighbourVar[5]; *k != 0; k++)
		j+=*k;
	ASSERT_EQ(j,3+4+6+1);
	j=0;
	for (int* k=a->neighbourVar[6]; *k != 0; k++)
		j+=*k;
	ASSERT_EQ(j,3+4+5+2);
	j=0;
	
	for (int i=0; i<a->numOccurence[a->vc+1]; i++) {
		j+=a->occurence[a->vc+1][i];
	}
	ASSERT_EQ(j,0+6+8);
	j=0;
	for (int i=0; i<a->numOccurence[a->vc-1]; i++) {
		j+=a->occurence[a->vc-1][i];
	}
	ASSERT_EQ(j,0);
	j=0;
	
	for (int i=0; i<a->numOccurence[a->vc+2]; i++) {
		j+=a->occurence[a->vc+2][i];
	}
	ASSERT_EQ(j,0+6+9);
	j=0;
	for (int i=0; i<a->numOccurence[a->vc-2]; i++) {
		j+=a->occurence[a->vc-2][i];
	}
	ASSERT_EQ(j,4);
	j=0;
	
	for (int i=0; i<a->numOccurence[a->vc+3]; i++) {
		j+=a->occurence[a->vc+3][i];
	}
	ASSERT_EQ(j,0+1+4+7+9);
	j=0;
	for (int i=0; i<a->numOccurence[a->vc-3]; i++) {
		j+=a->occurence[a->vc-3][i];
	}
	ASSERT_EQ(j,3);
	j=0;
	
	for (int i=0; i<a->numOccurence[a->vc+4]; i++) {
		j+=a->occurence[a->vc+4][i];
	}
	ASSERT_EQ(j,1+6+7);
	j=0;
	for (int i=0; i<a->numOccurence[a->vc-4]; i++) {
		j+=a->occurence[a->vc-4][i];
	}
	ASSERT_EQ(j,4+5);
	j=0;
	
	for (int i=0; i<a->numOccurence[a->vc+5]; i++) {
		j+=a->occurence[a->vc+5][i];
	}
	ASSERT_EQ(j,1+2+5+8);
	j=0;
	for (int i=0; i<a->numOccurence[a->vc-5]; i++) {
		j+=a->occurence[a->vc-5][i];
	}
	ASSERT_EQ(j,3);
	j=0;
	
	for (int i=0; i<a->numOccurence[a->vc+6]; i++) {
		j+=a->occurence[a->vc+6][i];
	}
	ASSERT_EQ(j,2+9);
	j=0;
	for (int i=0; i<a->numOccurence[a->vc-6]; i++) {
		j+=a->occurence[a->vc-6][i];
	}
	ASSERT_EQ(j,5+7);
	j=0;
}

TEST(CnfTests, cnf_neighborhoods_join) {
	Cnf* a = new Cnf("./good/c1.txt");
	a->compute_variable_neighborhoods();
	
	Cnf* b = new Cnf("./good/c3.txt");
	b->compute_variable_neighborhoods();
	a->join(b);
	
	int j=0;
	for (int* k=a->neighbourVar[1]; *k != 0; k++)
		j+=*k;
	ASSERT_EQ(j,2+3+4+5);
	j=0;
	for (int* k=a->neighbourVar[2]; *k != 0; k++)
		j+=*k;
	ASSERT_EQ(j,1+3+4+6);
	j=0;
	for (int* k=a->neighbourVar[3]; *k != 0; k++)
		j+=*k;
	ASSERT_EQ(j,1+2+4+5+6);
	j=0;
	for (int* k=a->neighbourVar[4]; *k != 0; k++)
		j+=*k;
	ASSERT_EQ(j,1+2+3+5+6);
	j=0;
	for (int* k=a->neighbourVar[5]; *k != 0; k++)
		j+=*k;
	ASSERT_EQ(j,3+4+6+1);
	j=0;
	for (int* k=a->neighbourVar[6]; *k != 0; k++)
		j+=*k;
	ASSERT_EQ(j,3+4+5+2);
	j=0;
	
	for (int i=0; i<a->numOccurence[a->vc+1]; i++) {
		j+=a->occurence[a->vc+1][i];
	}
	ASSERT_EQ(j,0+6+8);
	j=0;
	for (int i=0; i<a->numOccurence[a->vc-1]; i++) {
		j+=a->occurence[a->vc-1][i];
	}
	ASSERT_EQ(j,0);
	j=0;
	
	for (int i=0; i<a->numOccurence[a->vc+2]; i++) {
		j+=a->occurence[a->vc+2][i];
	}
	ASSERT_EQ(j,0+6+9);
	j=0;
	for (int i=0; i<a->numOccurence[a->vc-2]; i++) {
		j+=a->occurence[a->vc-2][i];
	}
	ASSERT_EQ(j,4);
	j=0;
	
	for (int i=0; i<a->numOccurence[a->vc+3]; i++) {
		j+=a->occurence[a->vc+3][i];
	}
	ASSERT_EQ(j,0+1+4+7+9);
	j=0;
	for (int i=0; i<a->numOccurence[a->vc-3]; i++) {
		j+=a->occurence[a->vc-3][i];
	}
	ASSERT_EQ(j,3);
	j=0;
	
	for (int i=0; i<a->numOccurence[a->vc+4]; i++) {
		j+=a->occurence[a->vc+4][i];
	}
	ASSERT_EQ(j,1+6+7);
	j=0;
	for (int i=0; i<a->numOccurence[a->vc-4]; i++) {
		j+=a->occurence[a->vc-4][i];
	}
	ASSERT_EQ(j,4+5);
	j=0;
	
	for (int i=0; i<a->numOccurence[a->vc+5]; i++) {
		j+=a->occurence[a->vc+5][i];
	}
	ASSERT_EQ(j,1+2+5+8);
	j=0;
	for (int i=0; i<a->numOccurence[a->vc-5]; i++) {
		j+=a->occurence[a->vc-5][i];
	}
	ASSERT_EQ(j,3);
	j=0;
	
	for (int i=0; i<a->numOccurence[a->vc+6]; i++) {
		j+=a->occurence[a->vc+6][i];
	}
	ASSERT_EQ(j,2+9);
	j=0;
	for (int i=0; i<a->numOccurence[a->vc-6]; i++) {
		j+=a->occurence[a->vc-6][i];
	}
	ASSERT_EQ(j,5+7);
	j=0;
}


TEST(CnfTests, cnf_neighborhoods_join_copy) {
	Cnf* a = new Cnf("./good/c1.txt");
	a->compute_variable_neighborhoods();
	
	Cnf* b = new Cnf("./good/c3.txt");
	b->compute_variable_neighborhoods();
	a->join(b);
	
	a = new Cnf(a);
	
	int j=0;
	for (int* k=a->neighbourVar[1]; *k != 0; k++)
		j+=*k;
	ASSERT_EQ(j,2+3+4+5);
	j=0;
	for (int* k=a->neighbourVar[2]; *k != 0; k++)
		j+=*k;
	ASSERT_EQ(j,1+3+4+6);
	j=0;
	for (int* k=a->neighbourVar[3]; *k != 0; k++)
		j+=*k;
	ASSERT_EQ(j,1+2+4+5+6);
	j=0;
	for (int* k=a->neighbourVar[4]; *k != 0; k++)
		j+=*k;
	ASSERT_EQ(j,1+2+3+5+6);
	j=0;
	for (int* k=a->neighbourVar[5]; *k != 0; k++)
		j+=*k;
	ASSERT_EQ(j,3+4+6+1);
	j=0;
	for (int* k=a->neighbourVar[6]; *k != 0; k++)
		j+=*k;
	ASSERT_EQ(j,3+4+5+2);
	j=0;
	
	for (int i=0; i<a->numOccurence[a->vc+1]; i++) {
		j+=a->occurence[a->vc+1][i];
	}
	ASSERT_EQ(j,0+6+8);
	j=0;
	for (int i=0; i<a->numOccurence[a->vc-1]; i++) {
		j+=a->occurence[a->vc-1][i];
	}
	ASSERT_EQ(j,0);
	j=0;
	
	for (int i=0; i<a->numOccurence[a->vc+2]; i++) {
		j+=a->occurence[a->vc+2][i];
	}
	ASSERT_EQ(j,0+6+9);
	j=0;
	for (int i=0; i<a->numOccurence[a->vc-2]; i++) {
		j+=a->occurence[a->vc-2][i];
	}
	ASSERT_EQ(j,4);
	j=0;
	
	for (int i=0; i<a->numOccurence[a->vc+3]; i++) {
		j+=a->occurence[a->vc+3][i];
	}
	ASSERT_EQ(j,0+1+4+7+9);
	j=0;
	for (int i=0; i<a->numOccurence[a->vc-3]; i++) {
		j+=a->occurence[a->vc-3][i];
	}
	ASSERT_EQ(j,3);
	j=0;
	
	for (int i=0; i<a->numOccurence[a->vc+4]; i++) {
		j+=a->occurence[a->vc+4][i];
	}
	ASSERT_EQ(j,1+6+7);
	j=0;
	for (int i=0; i<a->numOccurence[a->vc-4]; i++) {
		j+=a->occurence[a->vc-4][i];
	}
	ASSERT_EQ(j,4+5);
	j=0;
	
	for (int i=0; i<a->numOccurence[a->vc+5]; i++) {
		j+=a->occurence[a->vc+5][i];
	}
	ASSERT_EQ(j,1+2+5+8);
	j=0;
	for (int i=0; i<a->numOccurence[a->vc-5]; i++) {
		j+=a->occurence[a->vc-5][i];
	}
	ASSERT_EQ(j,3);
	j=0;
	
	for (int i=0; i<a->numOccurence[a->vc+6]; i++) {
		j+=a->occurence[a->vc+6][i];
	}
	ASSERT_EQ(j,2+9);
	j=0;
	for (int i=0; i<a->numOccurence[a->vc-6]; i++) {
		j+=a->occurence[a->vc-6][i];
	}
	ASSERT_EQ(j,5+7);
	j=0;
}


