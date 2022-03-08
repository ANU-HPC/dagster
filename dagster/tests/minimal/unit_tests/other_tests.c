



TEST(OtherTests, resolve_sorted_vectors) {
	vector<vector<int>> a = {
		{ 4, -5, -6, 7 },
		{ 1, -2, 3, 4, -5 },
		{ -6, 7, -8, 9 }
	};
	vector<int> result;
	resolve_sorted_vectors(result,a);
	vector<int> result_expected = {1, -2, 3, 4, -5, -6, 7, -8, 9};
	EXPECT_EQ(result_expected.size(), result.size());
	for (int i=0; i<result.size(); i++) {
		EXPECT_EQ(result[i],result_expected[i]);
	}
}


TEST(OtherTests, resolve_sorted_vectors2) {
	vector<vector<int>> a = {
		{ },
		{ 1, -2, 3, 4, -5 },
		{ -6, 7, -8, 9 }
	};
	vector<int> result;
	resolve_sorted_vectors(result,a);
	vector<int> result_expected = {1, -2, 3, 4, -5, -6, 7, -8, 9};
	EXPECT_EQ(result_expected.size(), result.size());
	for (int i=0; i<result.size(); i++) {
		EXPECT_EQ(result[i],result_expected[i]);
	}
}


TEST(OtherTests, resolve_sorted_vectors3) {
	vector<vector<int>> a = {
		{ },
		{ },
		{ -6, 7, -8, 9 }
	};
	vector<int> result;
	resolve_sorted_vectors(result,a);
	vector<int> result_expected = {-6, 7, -8, 9};
	EXPECT_EQ(result_expected.size(), result.size());
	for (int i=0; i<result.size(); i++) {
		EXPECT_EQ(result[i],result_expected[i]);
	}
}



TEST(OtherTests, resolve_sorted_vectors4) {
	vector<vector<int>> a = {
		{ },
		{ },
		{ }
	};
	vector<int> result;
	resolve_sorted_vectors(result,a);
	vector<int> result_expected = {};
	EXPECT_EQ(result_expected.size(), result.size());
	for (int i=0; i<result.size(); i++) {
		EXPECT_EQ(result[i],result_expected[i]);
	}
}




TEST(OtherTests, resolve_sorted_vectors5) {
	vector<vector<int>> a = {
		{ 4, -5, 6, 7 },
		{ 1, -2, 3, 4, -5 },
		{ -6, 7, -8, 9 }
	};
	vector<int> result;
	
	EXPECT_THROW( {
		try {
			resolve_sorted_vectors(result,a);
		} 
		catch ( const ConsistencyException& e) {
            EXPECT_STREQ("Dagster is made Inconsitent:  trying to resolve together inconsistent messages\n", e.what());
			throw;
		}
	}, ConsistencyException);
}


