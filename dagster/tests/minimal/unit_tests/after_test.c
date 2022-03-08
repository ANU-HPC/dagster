#include <gtest/gtest.h>
//using ::testing::internal::posix::GetEnv;

// a hacky test that checks if the PREMATURE_EXIT_FILE exists after calling main battery of tests
// if the file exists, then the main battery exited early - which it shouldnt have.
TEST(TestAbnormalExit, if_specified_TEST_PREMATURE_EXIT_FILE_exists) {
	//const char* filepath = GetEnv("TEST_PREMATURE_EXIT_FILE");
	const char* filepath = "EXIT_FILE.txt";
	if (filepath != nullptr && *filepath != '\0') {
		FILE* f = fopen(filepath,"r");
		ASSERT_EQ((long)f,NULL);
	}
}

int main(int argc, char *argv[]) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
