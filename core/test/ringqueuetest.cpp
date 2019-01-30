#include "../include/ringqueue"
#include "../../unittest/include/unittest.h"
#include <iostream>
#include <thread>
#include <memory>



void test1 () {

	isdl::ringqueue < int, int64_t, 20 > queue;

	int64_t first_seq = queue.allocate(1);

	ASSERT_EQUAL ( queue.available_size(), 19, "Make sure available size is decremented after allocation" );

	ASSERT_EQUAL ( queue.committed_elements(), 0, "Commited elements are unaffected" );

	ASSERT_EQUAL ( queue.allocate_index(), 1, "Allocate index is incremented by 1");

	ASSERT_EQUAL ( queue.commit_index(), 0, "Commit index is untouched" );

	ASSERT_EQUAL ( queue.read_index(), 0, "Read index is the same" );

	queue [ first_seq ] = 5;

	queue.commit ( first_seq, 1 );

	ASSERT_EQUAL ( queue.available_size(), 19, " Allocate index doesn't change by commit operation" );

	ASSERT_EQUAL ( queue.committed_elements(), 1 , "Commited elements count is incremented" );

	ASSERT_EQUAL ( queue.allocate_index(), 1 , "Allocate index is the same");

	ASSERT_EQUAL ( queue.commit_index(), 1 , "Commit index is incremented");

	ASSERT_EQUAL ( queue.read_index(), 0, "Read index is unchanged" );


	queue.free ( first_seq, 1 );

	ASSERT_EQUAL ( queue.available_size(), 20, "Element is freed available size is back to normal" );

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

	isdl::ringqueue < int, int64_t, 20 > queue;

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
	
	ASSERT_EQUAL ( queue.available_size(), 12, "Make sure available size is decremented after allocation" );

	ASSERT_EQUAL ( queue.committed_elements(), 0, "Commited elements are unaffected" );

	ASSERT_EQUAL ( queue.allocate_index(), 8, "Allocate index is incremented by 1");

	ASSERT_EQUAL ( queue.commit_index(), 0, "Commit index is untouched" );

	ASSERT_EQUAL ( queue.read_index(), 0, "Read index is the same" );

};

TEST ( "Multi threaded allocation test", test2 )

template < size_t QueueSize > class Receiver {
	int64_t& _acumulator;
	isdl::ringqueue<int, int64_t, QueueSize>& _queue;
	int _number;
	int _index;
	

public:
	Receiver ( int64_t& acumulator, isdl::ringqueue<int, int64_t, QueueSize>& queue, int number_of_receivers, 
		int receiver_index ) :
		_acumulator ( acumulator ), _queue ( queue ), _number {number_of_receivers},
		_index {receiver_index} {}

	void operator () () {
		_acumulator = 0;
		int64_t last_index = 0;
		while ( true ) {
			int64_t value = 0;
			int64_t read_index = _queue.read_index();
			size_t committed = _queue.committed_elements();
			while ( last_index < read_index + committed ) {
			
				if ( last_index % _number == _index ) {
					value = _queue [ last_index ];
					if ( value < 0 ) break;
					_acumulator += value;
					_queue.free(last_index, 1);
				}
				++last_index;
			
			}
			if ( value < 0 ) break;
		}

	}

	
};


template < size_t QueueSize> class Publisher {
	int _start;
	int _end;
	int _increment;
	isdl::ringqueue<int, int64_t, QueueSize>& _queue;

public:
	Publisher ( int start, int end, int increment, isdl::ringqueue<int, int64_t, QueueSize>& queue ):
	_start{start}, _end{end}, _increment {increment}, _queue { queue }
	 {}

	void operator () () {
		int64_t index = 0;
		for ( int i = _start; i < _end; i += _increment ) {
			/// Wait for slots to become available
			while ( _queue.available_size() == 0);
			while ( true ) {
				try {
					index = _queue.allocate(1);
					_queue [index] = i;
					_queue.commit(index,1);
					break;
				} catch ( isdl::queue_full f) {
				}
			}
		}
	}
};

void test4 () {
	isdl::ringqueue < int, int64_t, 70000 > queue;
	int64_t acumulator = 0;
	std::thread receiver ( Receiver < 70000 > ( acumulator, queue, 1, 0 ) );
	std::thread publisher ( Publisher < 70000 > ( 1, 10000001, 1, queue) );
	publisher.join();
	int64_t index = queue.allocate(1);
	queue [ index ] = -1;
	queue.commit( index, 1);
	receiver.join();
	ASSERT_EQUAL ( acumulator, 10000001L*(10000000L/2), "Make sure acumulated value is correct" );
	
}

TEST ( "Single Producer Single Consumer Test", test4 )
/**
 * Multiple producers multiple consumers test
 */
void test5 () {
	isdl::ringqueue < int, int64_t, 70000 > queue;
	int64_t acumulator = 0;
	int64_t acumulator1 = 0;
	int64_t acumulator2 = 0;
	std::thread receiver1 ( Receiver<70000> ( acumulator, queue, 3, 0 ) );
	std::thread receiver2 ( Receiver<70000> ( acumulator1, queue, 3, 1 ) );
	std::thread receiver3 ( Receiver<70000> ( acumulator2, queue, 3, 2 ) );
	std::thread publisher1 ( Publisher<70000> ( 1, 10000001, 2, queue) );
	std::thread publisher2 ( Publisher<70000> ( 2, 10000001, 2, queue) );
	publisher1.join();
	publisher2.join();
	int64_t index = queue.allocate(3);
	for ( int i = 0; i< 3; ++i ) {
		queue [ index+i ] = -1;
		queue.commit( index+i, 1);
	}
	receiver1.join();
	receiver2.join();
	receiver3.join();
	ASSERT_EQUAL ( acumulator+acumulator1+acumulator2, 10000001L*(10000000L/2), "Make sure acumulated value is correct" );
	
}

TEST ( "Two producers three consumers test", test5 )


