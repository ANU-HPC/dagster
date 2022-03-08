#include "stdio.h"
#include "stdlib.h"

TEST(DisorganisedArrayTests, basic_functionality) {
  DisorderedArray<int> a(5);
  a.add(5);
  a.add(8);
  a.add(5);
  a.add(8);
  a.add(8);
  a.add(90);

  EXPECT_EQ(a[0],5);
  EXPECT_EQ(a[1],8);
  EXPECT_EQ(a[2],5);
  EXPECT_EQ(a[3],8);
  EXPECT_EQ(a[4],8);
  EXPECT_EQ(a[5],90);
  EXPECT_EQ(a.length,6);

  a.remove(5);

  EXPECT_EQ(a[0],90);
  EXPECT_EQ(a[1],8);
  EXPECT_EQ(a[2],8);
  EXPECT_EQ(a[3],8);
  EXPECT_EQ(a.length,4);

  a.remove(8);

  EXPECT_EQ(a[0],90);
  EXPECT_EQ(a.length,1);

  a.remove(90);
  EXPECT_EQ(a.length,0);
}

