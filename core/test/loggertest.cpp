/*************************************************************************************
 * Testing logger
 */
#include <unittest>
#include <logger>
#include <iostream>
#include <cstring>

void loggertest1 () {
        ASSERT_EQUAL ( isdl::get_params ("Test string { } hello {{}"), 1, "One parameter folloewed by escaped start" );
        ASSERT_EQUAL ( isdl::get_params ("Test string { } hello {}"), 2, "Two parameters" );

}

void loggertest2 () {
	constexpr const char *fmt = "Test string {  } hello {{} test";
	constexpr const char *fmt1 = "Test string {  } hello {} test";

	isdl::fmt_segments< isdl::get_params ( fmt ) >fgmnts ( fmt );
	
        ASSERT_EQUAL ( fgmnts._count, 2, "Two segments" );
        ASSERT_EQUAL ( fgmnts._segments[0]._start, fmt,  "Make sure the pointer is correct" );
        ASSERT_EQUAL ( fgmnts._segments[0]._length, 12,  "Make sure the size is correct" );
        ASSERT_EQUAL ( fgmnts._segments[1]._start, &fmt[16],  "Make sure the pointer is correct" );
        ASSERT_EQUAL ( fgmnts._segments[1]._length, 15,  "Make sure the size is correct" );
        ASSERT_EQUAL ( (void *) fgmnts._format_end, (void *) (fmt + 31),  "Make sure the last pointer is correct" );

	isdl::fmt_segments< isdl::get_params ( fmt1 ) >fgmnts1 ( fmt1 );

        ASSERT_EQUAL ( fgmnts1._count,3, "Three segments" );
        ASSERT_EQUAL ( fgmnts1._segments[0]._start, fmt1,  "Make sure the pointer is correct" );
        ASSERT_EQUAL ( fgmnts1._segments[0]._length, 12,  "Make sure the size is correct" );
        ASSERT_EQUAL ( fgmnts1._segments[1]._start, &fmt1[16],  "Make sure the pointer is correct" );
        ASSERT_EQUAL ( fgmnts1._segments[1]._length, 7,  "Make sure the size is correct" );
        ASSERT_EQUAL ( fgmnts1._segments[2]._start, &fmt1[25],  "Make sure the pointer is correct" );
        ASSERT_EQUAL ( fgmnts1._segments[2]._length, 5,  "Make sure the size is correct" );
        ASSERT_EQUAL ( (void *) fgmnts1._format_end, (void *) ( fmt1 + 30 ),  "Make sure the last point is correct" );


}



struct test_logback : public isdl::log_back {
	char *_message_buffer;
	int _buffer_position;
	isdl::log_level _lvl;
	const char *_file_name;
	int _line_number;
	isdl::timestamp _timestamp;
	bool _completed;
	
	
	test_logback ( size_t buffer_size ) : _message_buffer { new char [buffer_size] },
	_buffer_position ( 0 ) {}
	virtual void add ( isdl::log_level __v, const char *__f, int __l, isdl::timestamp __t,
		const char *__m, size_t __s, bool __b, bool __e ) {
		if ( __b ) {
			_buffer_position = 0;
			_lvl = __v; _file_name = __f; _line_number = __l;  _timestamp = __t;
		}
		std::memcpy ( &_message_buffer[_buffer_position], __m, __s );
		_buffer_position += __s;
		if ( __e ) {
			_message_buffer[_buffer_position] = '\0';
			_completed = true;
		}
	}

	std::string message () { return std::string ( _message_buffer ); }

	~test_logback () {
		delete [] _message_buffer;
	}


		
};

/**
 * Test logger 
 */
void loggertest3 () {
	test_logback test_back ( 1024 );
	isdl::log_factory->add_logger ( "testlogger", &test_back, isdl::log_level::info ); 
	isdl::basic_logger& log = isdl::log_factory->get_logger ( "testlogger" );
	test_back._completed = false;
	LOG ( log, isdl::log_level::info, "Test logging {{{}}}", 5 );
	int line_number = __LINE__ - 1;
	while ( !test_back._completed );
	ASSERT_EQUAL ( test_back.message(), std::string ( "Test logging {{5}}" ), "Check that the log message is correct" );
	ASSERT_EQUAL ( test_back._file_name, __FILE__, "Check if the file name is logged correctly" );
	ASSERT_EQUAL ( test_back._lvl, isdl::log_level::info, "Check if the log level is recorded correctly" );
	ASSERT_EQUAL ( test_back._line_number, line_number, "Check if the line number is recorded correctly" );

	int a = 5;
	std::string b = "tst";

	test_back._completed = false;
	LOG ( log, isdl::log_level::info, "Test string value4: {}, value2: {} hello1 ", a, b );
	while ( !test_back._completed );
	ASSERT_EQUAL ( test_back.message(), std::string ( "Test string value4: 5, value2: tst hello1 "  ), 
		"Check message with two parameters " );

}

/**
 * Test messages that fit in more than one queue elements
 */
void loggertest4 () {
	test_logback test_back ( 1024 );
	isdl::log_factory->add_logger ( "testlogger", &test_back, isdl::log_level::info ); 
	isdl::basic_logger& log = isdl::log_factory->get_logger ( "testlogger" );
	test_back._completed = false;
	LOG ( log, isdl::log_level::info, "*********----------**********----------{}*********----------**********----------{}*********----------**********----------{}*********----------**********----------{}", 1, 2, 3, 4)
	
	while ( !test_back._completed );
	ASSERT_EQUAL ( test_back.message(), std::string ( "*********----------**********----------1*********----------**********----------2*********----------**********----------3*********----------**********----------4") , "Check if long message is logged correctly" );
}



TEST ( " Test constexpr correctly identifys parameters placeholders", loggertest1 )
TEST ( " Test constexpr parses log messages segments correctly", loggertest2 )
TEST ( " Test information recorded by the logger", loggertest3 )
TEST ( " Test message longer than queue element size", loggertest4 )
