#include "../include/unittest.h"

void test1 () {
	ASSERT_EQUAL ( 5, 10, "Test assertion should be false" );
}

void test2 () {
	ASSERT_EQUAL ( 10, 10, "Test assertion should be true" );
}

TEST ( "UnitTest1", test1 );
TEST ( "UnitTest2", test2 );
