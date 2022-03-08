

TEST(DagCnfTests, dag_cnf_succeeds_linking_good_file) {
  Dag* d;
  Cnf* c;
  EXPECT_NO_THROW(d = new Dag("./good/d1.txt"));
  CnfHolder* cnf_holder = new CnfHolder(d,"./good/c1.txt");
  cnf_holder->generate_decomposition();
  EXPECT_EQ(cnf_holder->get_Cnf(2)->clauses[1][1],-5);
}



TEST(DagCnfTests, dag_cnf_fails_linking_bad_file) {
  Dag* d;
  Cnf* c;
  EXPECT_NO_THROW(d = new Dag("./bad/dagcnfs/d1.txt"));
  CnfHolder* cnf_holder;
  EXPECT_NO_THROW(cnf_holder = new CnfHolder(d,"./bad/dagcnfs/c1.txt"));
  ASSERT_THROW(cnf_holder->generate_decomposition() ,ParsingException);
}
