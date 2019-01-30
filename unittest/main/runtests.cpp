/**
 * Runs unittests for all the object files that are linked together with this file
 * The only expectations is that the test is a void function and the name starts with test
 */


#include "../include/unittest.h"
#include <iostream>

isdl::test_runner _test_runner;

namespace isdl {


void test::add_assertion ( const assertion& assert ) {
	_assertions.push_back ( assert );
}



void test_runner::add_test ( const char *file_name, const char *test_name, void ( *f ) () ) {
	_tests.push_back ( test ( file_name, test_name, f ) );
}


void test_runner::run () {
	std::cout << "Runnig: " << _tests.size() << " tests."<< std::endl;
	for ( _current_test = 0; _current_test < _tests.size(); ++_current_test ) {
		std::cout << "Running test: " << _tests[_current_test]._test_name << std::endl;
		_tests[_current_test]._start_time = std::chrono::system_clock::now();
                _tests[_current_test]._test_function();
		_tests[_current_test]._end_time = std::chrono::system_clock::now();

        }
}

void test_runner::add_assertion ( const assertion& assert ) {
	_tests[_current_test].add_assertion ( assert );
	assert._result?++_tests[_current_test]._successful:
		++_tests[_current_test]._failed;
} 


std::ostream& operator << ( std::ostream& str, const test& tst ) {
	str << "  Test : " << tst._test_name << std::endl;
	str << "  Run time: " << (tst._end_time -tst._start_time).count();
	str << "  Successful : "<< tst._successful << ", Failed : " << tst._failed << std::endl;
	for ( auto assert : tst._assertions ) {
		str << "    "<< tst._file_name << "," << assert._src_line << " , " << assert._text << std::endl; 
	}
	return str;
}


std::ostream& operator << ( std::ostream& str, const test_runner& runner ) {
	str << "Number of test: " << runner._tests.size() << std::endl;
	for ( auto test : runner._tests ) {
		str << test << std::endl;
	}
	return str;
}


register_test::register_test ( const char *file_name, const char *test_name, void (*test_function)() ) {
        _test_runner.add_test ( file_name, test_name, test_function );
}



}




int main ( int argc, char* argv[] ) {

	std::cout << "Start running unittests " << std::endl;

	_test_runner.run();

	std::cout << _test_runner << std::endl;
}

