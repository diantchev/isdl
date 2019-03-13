/**
 * Runs unittests for all the object files that are linked together with this file
 * The only expectations is that the test is a void function and the name starts with test
 */


#include <unittest>
#include <iostream>

isdl::test_runner _test_runner;

namespace isdl {


void test::add_assertion ( const assertion& assert ) {
	_assertions.push_back ( assert );
}



void test_runner::add_test ( const char *file_name, const char *test_name, void ( *f ) () ) {
	_tests[std::string(test_name)] = test ( file_name, test_name, f );
}


void test_runner::run () {
	std::vector < std::string > run_list;
	for ( auto it : _tests ) {
		run_list.push_back ( it.first );
	}
	run ( run_list );
}

void test_runner::run ( const std::vector<std::string>& run_list ) {
	std::cout << "Runnig: " << run_list.size() << " tests. "<< std::endl;
	for ( std::string curr_test : run_list ) {
		_current_test = _tests.find ( curr_test );
		if ( _current_test != _tests.end() ) {
			std::cout << "Running test: " << _current_test->second._test_name << std::endl;
			_current_test->second._start_time = std::chrono::system_clock::now();
              		_current_test->second._test_function();
			_current_test->second._end_time = std::chrono::system_clock::now();
		} else {
			std::cout << __RED << "Test: "<< curr_test << " is not a valid test name" << __NORMAL << std::endl;
		}

        }
}

void test_runner::add_assertion ( const assertion& assert ) {
	_current_test->second.add_assertion ( assert );
	assert._result?++_current_test->second._successful:
		++_current_test->second._failed;
} 


std::ostream& operator << ( std::ostream& str, const test& tst ) {
	str << "  Test : " << tst._test_name << std::endl;
	str << "  Run time: " << std::chrono::duration_cast < std::chrono::microseconds >
			(tst._end_time -tst._start_time).count() << "us";
	str << "  Successful : "<< tst._successful << ", Failed : " << tst._failed << std::endl;
	for ( auto assert : tst._assertions ) {
		str << "    "<< tst._file_name << "," << assert._src_line << " , " << assert._text << std::endl; 
	}
	return str;
}


std::ostream& operator << ( std::ostream& str, const test_runner& runner ) {
	str << "Number of test: " << runner._tests.size() << std::endl;
	for ( auto test : runner._tests ) {
		str << test.second << std::endl;
	}
	return str;
}


register_test::register_test ( const char *file_name, const char *test_name, void (*test_function)() ) {
        _test_runner.add_test ( file_name, test_name, test_function );
}



}




int main ( int argc, char* argv[] ) {

	std::cout << "Start running unittests " << std::endl;

	if ( argc > 1 ) {
		std::vector < std::string > tests_to_run;
		for ( int i = 1; i< argc; ++i ) {
			tests_to_run.push_back ( std::string( argv[i] ) );
		}
		_test_runner.run ( tests_to_run );
	} else {
		_test_runner.run();
	}

	std::cout << _test_runner << std::endl;
}

