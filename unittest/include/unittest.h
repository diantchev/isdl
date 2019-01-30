#pragma once
#include <utility>
#include <vector>
#include <sstream>
#include <chrono>

namespace isdl {

struct assertion {
	int _src_line;
	std::string _text;
	bool _result;

public:
	template < typename Actual, typename Expected > assertion ( int src_line, const char *text,
		 Actual actual, Expected expected, bool result ) : _src_line {src_line}, _result ( result ) {
		std::ostringstream str;
		if ( _result ) {
			str << "OK, " << text << ", Expected: " << expected << " Actual: " << actual; 
		} else {
			str << "FAILED, " << text << ", Expected: " << expected << " Actual: " << actual; 
		}
		_text = str.str();
	}
	
};


struct test {

	const char *_file_name;
	const char *_test_name;
	std::chrono::system_clock::time_point _start_time;
	std::chrono::system_clock::time_point _end_time;
	int _successful;
	int _failed;
	void ( *_test_function ) ();
	std::vector<assertion> _assertions;
public:
	test ( const char *file_name, const char *test_name , void (*function) ()) : _file_name {file_name},
		_test_name {test_name}, _successful( 0 ), _failed ( 0 ),
		_test_function { function } {} 

	void add_assertion ( const assertion& assert );

};


std::ostream& operator << (std::ostream& str, const test& tst );

/**
 * Runner class runs all the tests in the program
 */
class test_runner {
	friend std::ostream& operator << ( std::ostream&, const test_runner&);
	std::vector < test > _tests;
	
	int _current_test;

public:
	void add_test ( const char *file_name, const char *test_name, void ( *f ) () );

	void add_assertion ( const assertion& assert );

	void run ();
};

std::ostream& operator << ( std::ostream& ostream, const test_runner& runner );


/**
 * Helper class to register unit test to be executed
 */
class register_test {
public: 
	register_test ( const char *file_name, const char *test_name, void (*test_function)() );
};

/**
 * Defines a void function to be executed as unit test under the specified name
 */
#define TEST( TEST_NAME, FUNCTION ) isdl::register_test class_##FUNCTION ( __FILE__, TEST_NAME, FUNCTION );

#define ASSERT_EQUAL( ACTUAL, EXPECTED, TEXT ) \
	_test_runner.add_assertion ( isdl::assertion ( __LINE__, TEXT, ACTUAL, EXPECTED, ACTUAL==EXPECTED ) );
#define ASSERT_GREATER( ACTUAL, EXPECTED, TEXT ) \
	_test_runner.add_assertion ( ids::ssertion ( __LINE__, TEXT, ACTUAL, EXPECTED, ACTUAL>EXPECTED ) );
#define ASSERT_LESS( ACTUAL, EXPECTED, TEXT ) \
	_test_runner.add_assertion ( isdl::assertion ( __LINE__, TEXT, ACTUAL, EXPECTED, ACTUAL<EXPECTED ) );

}

/**
 * Defines a single instance to run all the tests in the program
 */
extern isdl::test_runner _test_runner;
