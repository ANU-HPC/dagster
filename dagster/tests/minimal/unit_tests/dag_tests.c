

TEST(DagTests, dag_succeeds_to_load_good_file) {
  EXPECT_NO_THROW(Dag("./good/d1.txt"));
}

TEST(DagTests, dag_succeeds_to_load_good_file_depth_and_status) {
  Dag* d;
  EXPECT_NO_THROW(d = new Dag("./good/d1.txt"));
  EXPECT_EQ(d->no_nodes,6);
  EXPECT_EQ(d->node_status[0],1);
  EXPECT_EQ(d->node_status[1],0);
  EXPECT_EQ(d->node_status[2],1);
  EXPECT_EQ(d->node_status[3],0);
  EXPECT_EQ(d->node_status[4],0);
  EXPECT_EQ(d->node_status[5],1);

  EXPECT_EQ(d->depth_for_each_node[0],0);
  EXPECT_EQ(d->depth_for_each_node[1],1);
  EXPECT_EQ(d->depth_for_each_node[2],0);
  EXPECT_EQ(d->depth_for_each_node[3],1);
  EXPECT_EQ(d->depth_for_each_node[4],2);
  EXPECT_EQ(d->depth_for_each_node[5],1);

  EXPECT_EQ(d->max_depth,2);
}

TEST(DagTests, dag_succeeds_dehydrate) {
  Dag* d;
  EXPECT_NO_THROW(d = new Dag("./good/d1.txt"));

  int correct_dehydrate[] = {68, 6, 1, 2, 3, 4, 5, 6, -1, 0, 4, 6, -1, 0, 1, 3, 4, 6, -1, 1, 3, -1, 1, 3, 5, 6, 7, -1, 0, 1, 2, 3, 4, 5, 6, 7, -1, 2, 3, 7, -1, 0, 1, 1, 2, -1, 1, 4, 1, 2, 3, -1, 2, 3, 3, 4, -1, 3, 4, 3, 4, 5, -1, 5, 4, 5, 6, -1};

  int dehydrated_size = d->get_dehydrated_size();
  EXPECT_EQ(dehydrated_size,68);
  int* dehydrated_data = (int*)calloc(sizeof(int),1000);

  d->dehydrate(dehydrated_data);
  EXPECT_EQ(dehydrated_data[0],68);

  for (int i=0; i<68; i++) {
    EXPECT_EQ(dehydrated_data[i],correct_dehydrate[i]);
  }
  free(dehydrated_data);
}



  std::vector<int> node_status;
  std::vector<std::vector<int>> forward_connections;
  std::vector<std::vector<std::vector<int>>> forward_connection_literals;
  std::vector<std::vector<int>> amalgamated_forward_connection_literals;
  std::vector<std::vector<int>> reverse_connections;
  std::vector<std::vector<int>> clause_indices_for_each_node;
  std::vector<int> reporting;

int sum_vec(std::vector<int> a) {
  int s = 0;
  for (auto it2 = a.begin(); it2 != a.end(); it2++)
    s += *it2;
  return s;
}
int mul_vec(std::vector<int> a) {
  int s = 1;
  for (auto it2 = a.begin(); it2 != a.end(); it2++)
    s *= (*it2+1);
  return s;
}

int sum_range(RangeSet& a) {
  int s = 0;
  for (auto it2 = a.buffer.begin(); it2 != a.buffer.end(); it2++)
    for (int c = (*it2).first; c<=(*it2).second; c++)
      s += c;
  return s;
}
int mul_range(RangeSet& a) {
  int s = 1;
  for (auto it2 = a.buffer.begin(); it2 != a.buffer.end(); it2++)
    for (int c = (*it2).first; c<=(*it2).second; c++)
      s *= (c+1);
  return s;
}

TEST(DagTests, dag_dehydrate_rehydrate) {
  Dag* d;
  EXPECT_NO_THROW(d = new Dag("./good/d1.txt"));
  int dehydrated_size = d->get_dehydrated_size();
  int* dehydrated_data = (int*)calloc(sizeof(int),dehydrated_size);

  d->dehydrate(dehydrated_data);
  Dag* d2 = new Dag(dehydrated_data);
  free(dehydrated_data);

  EXPECT_EQ(d->no_nodes,d2->no_nodes);
  EXPECT_EQ(d->node_status.size(),d2->node_status.size());
  EXPECT_EQ(d->forward_connections.size(),d2->forward_connections.size());
  EXPECT_EQ(d->forward_connection_literals.size(),d2->forward_connection_literals.size());
  EXPECT_EQ(d->amalgamated_forward_connection_literals.size(),d2->amalgamated_forward_connection_literals.size());
  EXPECT_EQ(d->reverse_connections.size(),d2->reverse_connections.size());
  EXPECT_EQ(d->reporting.size(),d2->reporting.size());

  EXPECT_EQ(d->node_status.size(),d2->no_nodes);
  EXPECT_EQ(d->forward_connections.size(),d2->no_nodes);
  EXPECT_EQ(d->forward_connection_literals.size(),d2->no_nodes);
  EXPECT_EQ(d->amalgamated_forward_connection_literals.size(),d2->no_nodes);
  EXPECT_EQ(d->reverse_connections.size(),d2->no_nodes);

  for (int i = 0; i<d2->no_nodes; i++) {
    EXPECT_EQ(d->node_status[i],d2->node_status[i]);

    EXPECT_EQ(sum_vec(d->forward_connections[i]),sum_vec(d2->forward_connections[i]));
    EXPECT_EQ(mul_vec(d->forward_connections[i]),mul_vec(d2->forward_connections[i]));

    EXPECT_EQ(sum_vec(d->reverse_connections[i]),sum_vec(d2->reverse_connections[i]));
    EXPECT_EQ(mul_vec(d->reverse_connections[i]),mul_vec(d2->reverse_connections[i]));

    EXPECT_EQ(sum_range(d->amalgamated_forward_connection_literals[i]),sum_range(d2->amalgamated_forward_connection_literals[i]));
    EXPECT_EQ(mul_range(d->amalgamated_forward_connection_literals[i]),mul_range(d2->amalgamated_forward_connection_literals[i]));

    for (int j=0 ; j<d2->no_nodes; j++) {
      EXPECT_EQ(sum_range(d->forward_connection_literals[i][j]),sum_range(d2->forward_connection_literals[i][j]));
      EXPECT_EQ(mul_range(d->forward_connection_literals[i][j]),mul_range(d2->forward_connection_literals[i][j]));
    }
  }

  EXPECT_EQ(sum_range(d->reporting),sum_range(d2->reporting));
  EXPECT_EQ(mul_range(d->reporting),mul_range(d2->reporting));
}





TEST(DagTests, dag_succeeds_to_load_good_file_connections) {
  Dag* d;
  EXPECT_NO_THROW(d = new Dag("./good/d1.txt"));

EXPECT_EQ(d->forward_connections[0].size(),1);
EXPECT_EQ(d->forward_connections[1].size(),1);
EXPECT_EQ(d->forward_connections[2].size(),1);
EXPECT_EQ(d->forward_connections[3].size(),1);
EXPECT_EQ(d->forward_connections[4].size(),0);
EXPECT_EQ(d->forward_connections[5].size(),1);

EXPECT_EQ(d->forward_connections[0][0],1);
EXPECT_EQ(d->forward_connections[1][0],4);
EXPECT_EQ(d->forward_connections[2][0],3);
EXPECT_EQ(d->forward_connections[3][0],4);
EXPECT_EQ(d->forward_connections[5][0],4);

EXPECT_EQ(d->reverse_connections[0].size(),1);
EXPECT_EQ(d->reverse_connections[1].size(),1);
EXPECT_EQ(d->reverse_connections[2].size(),1);
EXPECT_EQ(d->reverse_connections[3].size(),1);
EXPECT_EQ(d->reverse_connections[4].size(),3);
EXPECT_EQ(d->reverse_connections[5].size(),1);

EXPECT_EQ(d->reverse_connections[1][0],0);
EXPECT_EQ(d->reverse_connections[3][0],2);

EXPECT_EQ(d->reverse_connections[0][0],0);
EXPECT_EQ(d->reverse_connections[2][0],2);
EXPECT_EQ(d->reverse_connections[5][0],5);

EXPECT_EQ(count(d->reverse_connections[4].begin(),d->reverse_connections[4].end(),1),1);
EXPECT_EQ(count(d->reverse_connections[4].begin(),d->reverse_connections[4].end(),3),1);
EXPECT_EQ(count(d->reverse_connections[4].begin(),d->reverse_connections[4].end(),5),1);

}

TEST(DagTests, dag_succeeds_to_load_reporting) {
  Dag* d;
  EXPECT_NO_THROW(d = new Dag("./good/d1.txt"));
  EXPECT_EQ(6, d->reporting.size());
  EXPECT_EQ(d->reporting.buffer[0].first,1);
  EXPECT_EQ(d->reporting.buffer[0].second,6);
}

TEST(DagTests, dag_succeeds_to_load__single_reporting) {
  Dag* d;
  EXPECT_NO_THROW(d = new Dag("./good/d2.txt"));
  
  EXPECT_EQ(4, d->reporting.size());
  
  EXPECT_EQ(d->reporting.buffer[0].first,1);
  EXPECT_EQ(d->reporting.buffer[0].second,3);
  EXPECT_EQ(d->reporting.buffer[1].first,5);
  EXPECT_EQ(d->reporting.buffer[1].second,5);
}

TEST(DagTests, dag_fails_to_load_nonexisting_file) {
  ASSERT_THROW(Dag("hunkydory.txt"), FileFailedException);
}

TEST(DagTests, dag_detects_cyclic) {
  ASSERT_THROW(Dag("./bad/dags/cyclic.txt"), ConsistencyException);
}

TEST(DagTests, dag_detects_bad_arrows2) {ASSERT_THROW(Dag("./bad/dags/bad_arrows2.txt"), ParsingException);}
TEST(DagTests, dag_detects_bad_arrows) {ASSERT_THROW(Dag("./bad/dags/bad_arrows.txt"), ParsingException);}
TEST(DagTests, dag_detects_bad_clauses) {ASSERT_THROW(Dag("./bad/dags/bad_clauses.txt"), ParsingException);}
TEST(DagTests, dag_detects_bad_header3) {ASSERT_THROW(Dag("./bad/dags/bad_header3.txt"), ParsingException);}
TEST(DagTests, dag_detects_bad_header5) {ASSERT_THROW(Dag("./bad/dags/bad_header5.txt"), ParsingException);}
TEST(DagTests, dag_detects_bad_header) {ASSERT_THROW(Dag("./bad/dags/bad_header.txt"), ParsingException);}
TEST(DagTests, dag_detects_negative_node_count) {ASSERT_THROW(Dag("./bad/dags/negative_node_count.txt"), ParsingException);}
TEST(DagTests, dag_detects_bad_arrows3) {ASSERT_THROW(Dag("./bad/dags/bad_arrows3.txt"), ParsingException);}
TEST(DagTests, dag_detects_bad_clauses2) {ASSERT_THROW(Dag("./bad/dags/bad_clauses2.txt"), ParsingException);}
TEST(DagTests, dag_detects_bad_header2) {ASSERT_THROW(Dag("./bad/dags/bad_header2.txt"), ParsingException);}
TEST(DagTests, dag_detects_bad_header4) {ASSERT_THROW(Dag("./bad/dags/bad_header4.txt"), ParsingException);}
TEST(DagTests, dag_detects_bad_header6) {ASSERT_THROW(Dag("./bad/dags/bad_header6.txt"), ParsingException);}
TEST(DagTests, dag_detects_extra_bad_stuff) {ASSERT_THROW(Dag("./bad/dags/extra_bad_stuff.txt"), ParsingException);}
TEST(DagTests, dag_detects_small_node_count) {ASSERT_THROW(Dag("./bad/dags/small_node_count.txt"), ParsingException);}
TEST(DagTests, dag_detects_no_reporting) {ASSERT_THROW(Dag("./bad/dags/no_reporting.txt"), ParsingException);}
// TODO: reporting tests

