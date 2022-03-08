#include "stdio.h"
#include "stdlib.h"




TEST(RangeSetTests, null_test) {
  RangeSet a;
  EXPECT_EQ(a.find(5),false);
  EXPECT_EQ(a.find(6),false);
  EXPECT_EQ(a.find(7),false);
  EXPECT_EQ(a.find(8),false);
  EXPECT_EQ(a.find(4),false);
  EXPECT_EQ(a.find(400),false);
  EXPECT_EQ(a.buffer_size,0);
}

TEST(RangeSetTests, singleton_add_test) {
  RangeSet a;
  a.add(5);
  EXPECT_EQ(a.find(5),true);
  EXPECT_EQ(a.find(6),false);
  EXPECT_EQ(a.find(4),false);
  EXPECT_EQ(a.find(400),false);
  EXPECT_EQ(a.buffer_size,1);
  
}

TEST(RangeSetTests, double_singleton_add_test) {
  RangeSet a;
  a.add(5);
  a.add(7);
  EXPECT_EQ(a.find(5),true);
  EXPECT_EQ(a.find(6),false);
  EXPECT_EQ(a.find(7),true);
  EXPECT_EQ(a.find(8),false);
  EXPECT_EQ(a.find(4),false);
  EXPECT_EQ(a.find(400),false);
  EXPECT_EQ(a.buffer_size,2);
}


TEST(RangeSetTests, left_extend) {
  RangeSet a;
  a.add(5);
  a.add(6);
  a.add(7);
  EXPECT_EQ(a.find(3),false);
  EXPECT_EQ(a.find(4),false);
  EXPECT_EQ(a.find(5),true);
  EXPECT_EQ(a.find(6),true);
  EXPECT_EQ(a.find(7),true);
  EXPECT_EQ(a.find(8),false);
  EXPECT_EQ(a.find(9),false);
  EXPECT_EQ(a.buffer_size,1);
}



TEST(RangeSetTests, right_extend) {
  RangeSet a;
  a.add(7);
  a.add(6);
  a.add(5);
  EXPECT_EQ(a.find(3),false);
  EXPECT_EQ(a.find(4),false);
  EXPECT_EQ(a.find(5),true);
  EXPECT_EQ(a.find(6),true);
  EXPECT_EQ(a.find(7),true);
  EXPECT_EQ(a.find(8),false);
  EXPECT_EQ(a.find(9),false);
  EXPECT_EQ(a.buffer_size,1);
}


TEST(RangeSetTests, left_left_extend) {
  RangeSet a;
  a.add(3);
  a.add(5);
  a.add(6);
  a.add(7);
  EXPECT_EQ(a.find(2),false);
  EXPECT_EQ(a.find(3),true);
  EXPECT_EQ(a.find(4),false);
  EXPECT_EQ(a.find(5),true);
  EXPECT_EQ(a.find(6),true);
  EXPECT_EQ(a.find(7),true);
  EXPECT_EQ(a.find(8),false);
  EXPECT_EQ(a.find(9),false);
  EXPECT_EQ(a.buffer_size,2);
}



TEST(RangeSetTests, right_right_extend) {
  RangeSet a;
  a.add(9);
  a.add(7);
  a.add(6);
  a.add(5);
  EXPECT_EQ(a.find(3),false);
  EXPECT_EQ(a.find(4),false);
  EXPECT_EQ(a.find(5),true);
  EXPECT_EQ(a.find(6),true);
  EXPECT_EQ(a.find(7),true);
  EXPECT_EQ(a.find(8),false);
  EXPECT_EQ(a.find(9),true);
  EXPECT_EQ(a.find(10),false);
  EXPECT_EQ(a.buffer_size,2);
}



TEST(RangeSetTests, left_internal_extend) {
  RangeSet a;
  a.add(9);
  a.add(5);
  a.add(6);
  a.add(7);
  EXPECT_EQ(a.find(2),false);
  EXPECT_EQ(a.find(3),false);
  EXPECT_EQ(a.find(4),false);
  EXPECT_EQ(a.find(5),true);
  EXPECT_EQ(a.find(6),true);
  EXPECT_EQ(a.find(7),true);
  EXPECT_EQ(a.find(8),false);
  EXPECT_EQ(a.find(9),true);
  EXPECT_EQ(a.find(10),false);
  EXPECT_EQ(a.buffer_size,2);
}

TEST(RangeSetTests, right_internal_extend) {
  RangeSet a;
  a.add(3);
  a.add(7);
  a.add(6);
  a.add(5);
  EXPECT_EQ(a.find(2),false);
  EXPECT_EQ(a.find(3),true);
  EXPECT_EQ(a.find(4),false);
  EXPECT_EQ(a.find(5),true);
  EXPECT_EQ(a.find(6),true);
  EXPECT_EQ(a.find(7),true);
  EXPECT_EQ(a.find(8),false);
  EXPECT_EQ(a.find(9),false);
  EXPECT_EQ(a.find(10),false);
  EXPECT_EQ(a.buffer_size,2);
}

TEST(RangeSetTests, internal_singleton) {
  RangeSet a;
  a.add(3);
  a.add(4);
  a.add(8);
  a.add(9);
  a.add(6);
  EXPECT_EQ(a.find(2),false);
  EXPECT_EQ(a.find(3),true);
  EXPECT_EQ(a.find(4),true);
  EXPECT_EQ(a.find(5),false);
  EXPECT_EQ(a.find(6),true);
  EXPECT_EQ(a.find(7),false);
  EXPECT_EQ(a.find(8),true);
  EXPECT_EQ(a.find(9),true);
  EXPECT_EQ(a.find(10),false);
  EXPECT_EQ(a.buffer_size,3);
}


TEST(RangeSetTests, internal_singleton_merge_left) {
  RangeSet a;
  a.add(3);
  a.add(4);
  a.add(8);
  a.add(9);
  a.add(6);
  a.add(5);
  EXPECT_EQ(a.find(2),false);
  EXPECT_EQ(a.find(3),true);
  EXPECT_EQ(a.find(4),true);
  EXPECT_EQ(a.find(5),true);
  EXPECT_EQ(a.find(6),true);
  EXPECT_EQ(a.find(7),false);
  EXPECT_EQ(a.find(8),true);
  EXPECT_EQ(a.find(9),true);
  EXPECT_EQ(a.find(10),false);
  EXPECT_EQ(a.buffer_size,2);
}

TEST(RangeSetTests, internal_singleton_merge_right) {
  RangeSet a;
  a.add(3);
  a.add(4);
  a.add(8);
  a.add(9);
  a.add(6);
  a.add(7);
  EXPECT_EQ(a.find(2),false);
  EXPECT_EQ(a.find(3),true);
  EXPECT_EQ(a.find(4),true);
  EXPECT_EQ(a.find(5),false);
  EXPECT_EQ(a.find(6),true);
  EXPECT_EQ(a.find(7),true);
  EXPECT_EQ(a.find(8),true);
  EXPECT_EQ(a.find(9),true);
  EXPECT_EQ(a.find(10),false);
  EXPECT_EQ(a.buffer_size,2);
}

TEST(RangeSetTests, internal_left_right_merge) {
  RangeSet a;
  a.add(3);
  a.add(4);
  a.add(8);
  a.add(9);
  a.add(6);
  a.add(5);
  a.add(7);
  EXPECT_EQ(a.find(2),false);
  EXPECT_EQ(a.find(3),true);
  EXPECT_EQ(a.find(4),true);
  EXPECT_EQ(a.find(5),true);
  EXPECT_EQ(a.find(6),true);
  EXPECT_EQ(a.find(7),true);
  EXPECT_EQ(a.find(8),true);
  EXPECT_EQ(a.find(9),true);
  EXPECT_EQ(a.find(10),false);
  EXPECT_EQ(a.buffer_size,1);
}


