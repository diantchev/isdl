#include <ringqueue>
#include <unittest>
#include <iostream>
#include <thread>
#include <memory>



void test1 () {

	isdl::ringqueue < int, int64_t, 32, 1 > queue;

	int64_t first_seq = queue.allocate(1);

	
	ASSERT_EQUAL ( first_seq, 0, "First sequence should be 0" );

	ASSERT_EQUAL ( queue.available_size(), 31, "Make sure available size is decremented after allocation" );

	ASSERT_EQUAL ( queue.committed_elements(), 0, "Commited elements are unaffected" );

	ASSERT_EQUAL ( queue.allocate_index(), 1, "Allocate index is incremented by 1");

	ASSERT_EQUAL ( queue.commit_index(), 0, "Commit index is untouched" );

	ASSERT_EQUAL ( queue.read_index(), 0, "Read index is the same" );

	queue [ first_seq ] = 5;

	queue.commit ( first_seq, 1 );

	ASSERT_EQUAL ( queue.available_size(), 31, " Allocate index doesn't change by commit operation" );

	ASSERT_EQUAL ( queue.committed_elements(), 1 , "Commited elements count is incremented" );

	ASSERT_EQUAL ( queue.allocate_index(), 1 , "Allocate index is the same");

	ASSERT_EQUAL ( queue.commit_index(), 1 , "Commit index is incremented");

	ASSERT_EQUAL ( queue.read_index(), 0, "Read index is unchanged" );


	queue.free ( 0, first_seq, 1 );

	ASSERT_EQUAL ( queue.available_size(), 32, "Element is freed available size is back to normal" );

	ASSERT_EQUAL ( queue.committed_elements(), 0 , "No commited elements" );

	ASSERT_EQUAL ( queue.allocate_index(), 1 , "Allocate index is unchanged");

	ASSERT_EQUAL ( queue.commit_index(), 1 , "Commit index is unchanged");

	ASSERT_EQUAL ( queue.read_index(), 1 , "Read index is incremented" );

}

TEST ( "Basic Methods Test", test1 )


/***
 * Make sure that multiple threads can get sequence numbers and publish simultaniously 
 * and produce the correct result
 */
void test2 () {

	isdl::ringqueue < int, int64_t, 32, 1 > queue;

	int number_of_threads = 8;
	int64_t sequences[number_of_threads];
	std::vector<std::unique_ptr <std::thread>> allocate_threads(number_of_threads);


	for ( int i = 0; i <number_of_threads; ++i  ) {
		int64_t& seq = sequences[i];
		allocate_threads[i] = std::unique_ptr<std::thread> (
			new std::thread ( [&seq, &queue](){
			seq = queue.allocate(1); }) );
	}


	for ( int i = 0; i < number_of_threads; ++i ) {
		allocate_threads[i]->join();
	}
	
	ASSERT_EQUAL ( queue.available_size(), 24, "Make sure available size is decremented after allocation" );

	ASSERT_EQUAL ( queue.committed_elements(), 0, "Commited elements are unaffected" );

	ASSERT_EQUAL ( queue.allocate_index(), 8, "Allocate index is incremented by 1");

	ASSERT_EQUAL ( queue.commit_index(), 0, "Commit index is untouched" );

	ASSERT_EQUAL ( queue.read_index(), 0, "Read index is the same" );

};

TEST ( "Multi threaded allocation test", test2 )

template < size_t QueueSize, size_t Readers > class Receiver {
	int64_t& _acumulator;
	isdl::ringqueue<int, int64_t, QueueSize, Readers >& _queue;
	int _number;
	int _index;
	

public:
	Receiver ( int64_t& acumulator, isdl::ringqueue<int, int64_t, QueueSize, Readers>& queue, int number_of_receivers, 
		int receiver_index ) :
		_acumulator ( acumulator ), _queue ( queue ), _number {number_of_receivers},
		_index {receiver_index} {}

	void operator () () {
		_acumulator = 0;
		int64_t last_index = 0;
		int64_t value = 0;
		while ( true ) {
			size_t count = _queue.committed ( last_index );
			for (int i = 0; i< count; ++i ) {
				if ( ( last_index +i ) % _number == _index ) {
					value = _queue [ last_index + i  ];
					if ( value >= 0 ) { _acumulator += value; }
					if ( value < 0 ) break;
				}
			
			}
			if ( count > 0 ) {
				_queue.free( _index, last_index, count );
				last_index+= count;
			}
			if ( value < 0 ) break;
			std::this_thread::sleep_for ( std::chrono::nanoseconds ( 10 ) );
		}

	}

	
};


template < size_t QueueSize, size_t Readers > class Publisher {
	int _start;
	int _end;
	int _increment;
	isdl::ringqueue<int, int64_t, QueueSize, Readers >& _queue;
	int _exceptions;

public:
	Publisher ( int start, int end, int increment, isdl::ringqueue<int, int64_t, QueueSize, Readers >& queue ):
	_start{start}, _end{end}, _increment {increment}, _queue { queue }, _exceptions {0}
	 {}

	void operator () () {
		int64_t index = 0;
		for ( int i = _start; i < _end; i += _increment ) {
			/// Wait for slots to become available
			index = _queue.allocate (1);
			_queue [index] = i;
			_queue.commit(index, 1);
		}
	}
};

void test4 () {
	isdl::ringqueue < int, int64_t, 65536, 1> queue;
	int64_t acumulator = 0;
	std::thread receiver ( Receiver < 65536, 1> ( acumulator, queue, 1, 0 ) );
	std::thread publisher ( Publisher < 65536, 1 > ( 1, 100000001, 1, queue) );
	publisher.join();
	int64_t index = queue.allocate(1);
	queue [ index ] = -1;
	queue.commit( index, 1);
	receiver.join();
	ASSERT_EQUAL ( acumulator, 100000001L*(100000000L/2), "Make sure acumulated value is correct" );
	
}

TEST ( "Single Producer Single Consumer Test", test4 )
/**
 * Multiple producers multiple consumers test
 */
void test5 () {
	isdl::ringqueue < int, int64_t, 65536, 3 > queue;
	int64_t acumulator = 0;
	int64_t acumulator1 = 0;
	int64_t acumulator2 = 0;
	std::thread receiver1 ( Receiver<65536, 3> ( acumulator, queue, 3, 0 ) );
	std::thread receiver2 ( Receiver<65536, 3> ( acumulator1, queue, 3, 1 ) );
	std::thread receiver3 ( Receiver<65536, 3> ( acumulator2, queue, 3, 2 ) );
	std::thread publisher1 ( Publisher<65536, 3> ( 1, 10000001, 1, queue) );
	publisher1.join();
	int64_t index = 0;
	while ( true ) {
		try {
			index = queue.allocate(3);
			break;
		} catch ( isdl::queue_full f ) {
		}
	}
	for ( int i = 0; i< 3; ++i ) {
		queue [ index+i ] = -1;
	}
	queue.commit( index, 3);
	receiver1.join();
	receiver2.join();
	receiver3.join();
	ASSERT_EQUAL ( acumulator+acumulator1+acumulator2, 10000001L*(10000000L/2), "Make sure acumulated value is correct" );
	
}

TEST ( "One producer three consumers test", test5 )




/**
 * Multiple producers multiple consumers test with queue size power of two
 */
void test7 () {
	isdl::ringqueue < int, int64_t, 1048576, 3 > queue;
	int64_t acumulator = 0;
	int64_t acumulator1 = 0;
	int64_t acumulator2 = 0;
	std::thread receiver1 ( Receiver<1048576, 3> ( acumulator, queue, 3, 0 ) );
	std::thread receiver2 ( Receiver<1048576, 3> ( acumulator1, queue, 3, 1 ) );
	std::thread receiver3 ( Receiver<1048576, 3> ( acumulator2, queue, 3, 2 ) );
	std::thread publisher1 ( Publisher<1048576, 3> ( 1, 100000001, 2, queue) );
	std::thread publisher2 ( Publisher<1048576, 3> ( 2, 100000001, 2, queue) );
	publisher1.join();
	publisher2.join();
	int64_t index = 0;
	index = queue.allocate(3);
	for ( int i = 0; i< 3; ++i ) {
		queue [ index+i ] = -1;
	}
	queue.commit( index, 3);
	receiver1.join();
	receiver2.join();
	receiver3.join();
	ASSERT_EQUAL ( acumulator+acumulator1+acumulator2, 100000001L*(100000000L/2), "Make sure acumulated value is correct" );
}
	
TEST ( "Two producers three consumers test ring size power of two", test7 )



void test8 () {

	ASSERT_EQUAL ( isdl::get_power_of_two ( 65536, 0 ), 16, "Make sure power of two is calculated correctly" );
	ASSERT_EQUAL ( ( 1 << isdl::get_power_of_two ( 65536, 0 ) ) - 1, 65535, "Make sure the mask is calculated properly" );
	
}

TEST ( "Power of two calculation test", test8 )

