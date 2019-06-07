#include <unittest>
#include <disruptor>
#include <chrono>
#include <iostream>

/**
 * Testing basic disruptor methods
 */
void disruptor1 () {
	struct test_event {
		int _value1;
		int _value2;
	};

	struct my_wait_strategy {

		void wait () {
			std::this_thread::sleep_for ( std::chrono::nanoseconds ( 1 ) );
		}
		
		void notify () {
		}

	};


	struct EventHandler {
		int64_t seq_num = 0;
		bool event ( int64_t seq, test_event& event ) {
			ASSERT_EQUAL( seq, seq_num, "Check if receiving the right sequence number" );
			ASSERT_EQUAL( event._value1, 5, "Make sure that the event is properly received" );
			ASSERT_EQUAL( event._value2, 6, "Make sure that the event is porperly received" );
			return true;
		}
	} handler;
	

	{

		isdl::disruptor< test_event, int64_t, my_wait_strategy > testdisruptor ( 2048, handler );
	
		testdisruptor.start ();

		int64_t next_seq = testdisruptor.next();
       

		ASSERT_EQUAL ( next_seq , 0, "Make sure that the next sequence number is 0" );
       		ASSERT_EQUAL ( testdisruptor.size() - testdisruptor.allocated(), 2047, 
			"Make sure available size is decremented after allocation" );
       		ASSERT_EQUAL (  testdisruptor.allocated(), 1, 
			"Allocated size is 1" );

		test_event& event = testdisruptor[next_seq];

		event._value1 = 5;
	
		event._value2 = 6;

		testdisruptor.publish ( next_seq ); 
	}


	
}


TEST ( "Basic test the constructor and the parameters", disruptor1 );




void disruptor2 () {
	struct my_wait_strategy {

		void wait () {
			std::this_thread::sleep_for ( std::chrono::nanoseconds ( 1 ) );
		}
		
		void notify () {
		}

	};


	struct EventHandler {
		int64_t accumulated = 0;
		bool event ( int64_t seq, int64_t& value ) {
			accumulated += value;
			return true;
		}
	} handler1 , handler2 , handler3 ;
	
	/// Block to create and destroy the disruptor
	{
		isdl::disruptor< int64_t, int64_t, my_wait_strategy > testdisruptor ( 2<<16, handler1, handler2, handler3 );
		
		testdisruptor.start();


		for ( int64_t i = 1; i < 100000001; ++i ) {
			int64_t seq = testdisruptor.next();
			testdisruptor[seq]=i;
			testdisruptor.publish ( seq ); 
		}


	}
	ASSERT_EQUAL( handler1.accumulated, (100000001L*50000000L), "Check accumulated value for the first handler" );
	ASSERT_EQUAL( handler2.accumulated, (100000001L*50000000L), "Check accumulated value for the second handler" );
	ASSERT_EQUAL( handler3.accumulated, (100000001L*50000000L), "Check accumulated value for the third handler" );

}

TEST ( "Test one producer three consumers", disruptor2 );



void disruptor3 () {
	struct my_wait_strategy {

		void wait () {
			std::this_thread::sleep_for ( std::chrono::nanoseconds ( 1 ) );
		}
		
		void notify () {
		}

	};


	struct EventHandler {
		int64_t accumulated = 0;
		const int _count;
		const int _index;
		bool event ( int64_t seq, int64_t& value ) {
			if ( seq % _count == _index ) 
				accumulated += value;
			return true;
		}
		EventHandler ( int count, int index ) : _count {count}, _index {index} {}
	} handler1 ( 3, 0 ) , handler2 ( 3, 1 )  , handler3 ( 3, 2 )  ;

	struct Publisher {
		const int _start;
		const int _end;
		const int _skip;
		isdl::disruptor < int64_t, int64_t, my_wait_strategy>& _disruptor;
		Publisher ( int64_t start, int64_t end, int skip, isdl::disruptor< int64_t, int64_t, my_wait_strategy>& disruptor ) 
			: _start ( start ), _end ( end ), _skip ( skip ), _disruptor ( disruptor )  {}

		void operator () ()  {
			for ( auto index = _start; index < _end; index+= _skip ) {
				int64_t seq = _disruptor.next();
				_disruptor[seq] = index;
				_disruptor.publish ( seq );
				
			}
		}
	};

	
	/// Block to create and destroy the disruptor
	{
		isdl::disruptor< int64_t, int64_t, my_wait_strategy > testdisruptor (1048576, handler1, handler2, handler3 );
		
		testdisruptor.start();

		std::thread publisher1 ( Publisher ( 1, 100000001, 2, testdisruptor ));
		std::thread publisher2 ( Publisher ( 2, 100000001, 2, testdisruptor )); 
		publisher1.join();
		publisher2.join();

	}
	ASSERT_EQUAL( handler3.accumulated+handler2.accumulated+handler1.accumulated, (100000001L*50000000L), "Check accumulated count" );

}

TEST ( "Test two producer three consumers", disruptor3 );

void disruptor4 () {
	struct my_wait_strategy {

		void wait () {
			std::this_thread::sleep_for ( std::chrono::nanoseconds ( 1 ) );
		}
		
		void notify () {
		}

	};

	struct my_event {
		int64_t _value;
		int64_t _final;
	};


	struct EventHandler {
		int64_t accumulated = 0;
		const int _count;
		const int _index;
		bool event ( int64_t seq, my_event& event ) {
			if ( seq % _count == _index ) 
				event._final = event._value;
			return true;
		}
		EventHandler ( int count, int index ) : _count {count}, _index {index} {}
	} handler1 ( 3, 0 ) , handler2 ( 3, 1 )  , handler3 ( 3, 2 )  ;


	struct EventHandlerAfter  {
		int64_t accumulated = 0;
		bool event ( int64_t seq, my_event& event ) {
			accumulated += event._final;
			return true;
		}
	} after_handler1, after_handler2, after_handler3;

	struct Publisher {
		const int _start;
		const int _end;
		const int _skip;
		isdl::disruptor < my_event, int64_t, my_wait_strategy >& _disruptor;
		Publisher ( int64_t start, int64_t end, int skip, isdl::disruptor< my_event, int64_t, my_wait_strategy>& disruptor ) 
			: _start ( start ), _end ( end ), _skip ( skip ), _disruptor ( disruptor )  {}

		void operator () ()  {
			for ( auto index = _start; index < _end; index+= _skip ) {
				int64_t seq = _disruptor.next();
				_disruptor[seq]._value = index;
				_disruptor.publish ( seq );
				
			}
		}
	};

	
	/// Block to create and destroy the disruptor
	{
		isdl::disruptor< my_event, int64_t, my_wait_strategy > testdisruptor (1048576, handler1, handler2, handler3 );

		testdisruptor.then ( after_handler1, after_handler2, after_handler3 ); 
		
		testdisruptor.start();

		std::thread publisher1 ( Publisher ( 1, 100000001, 2, testdisruptor ));
		std::thread publisher2 ( Publisher ( 2, 100000001, 2, testdisruptor )); 
		publisher1.join();
		publisher2.join();

	}

	ASSERT_EQUAL( after_handler1.accumulated, (100000001L*50000000L), "Check accumulated count of after handler 1" );
	ASSERT_EQUAL( after_handler2.accumulated, (100000001L*50000000L), "Check accumulated count of after handler 2" );
	ASSERT_EQUAL( after_handler3.accumulated, (100000001L*50000000L), "Check accumulated count of after handler 3" );

}

TEST ( "Test after handler", disruptor4 );

