
#include <thread>
#include <iostream>
#include "LocklessQueue.h"




isdl::LocklessQueue < unsigned long long, 568, isdl::SingleThreadedModel, isdl::SingleThreadedModel > queue;




class Producer {
	isdl::LocklessQueue < unsigned long long, 568, isdl::SingleThreadedModel, isdl::SingleThreadedModel>& _queue;
	const unsigned long long _maxNumber;
public:
	Producer ( isdl::LocklessQueue < unsigned long long, 568, isdl::SingleThreadedModel, isdl::SingleThreadedModel>& queue, 
unsigned long long maxNumber ) : _queue ( queue ),
	_maxNumber ( maxNumber ) {}
	void operator() () {
		std::cout << "Producer " << std::endl;
		for ( unsigned long long i = 1; i<=_maxNumber; ++i ) {
			while ( !_queue.enqueue  ( i ) );
		}
	}
	unsigned long long getCount () {
		return (_maxNumber+1) * ( _maxNumber >> 1 );
	}
};

class Consumer {
	isdl::LocklessQueue < unsigned long long, 568, isdl::SingleThreadedModel, isdl::SingleThreadedModel>& _queue;
	unsigned long long& _count;
public:
	Consumer ( isdl::LocklessQueue < unsigned long long, 568, isdl::SingleThreadedModel, isdl::SingleThreadedModel>& queue, unsigned long long& count ) :
 _queue ( queue ), _count (count) {}
	void operator () () {
		unsigned long long value;
		std::cout << "Consumer " << std::endl;
		do {
			while ( !_queue.dequeue ( value ) );
			_count += value;
		} while ( value );
	
	}
	
	unsigned long long getCount ( ) { return _count; }
};

int main ( int argc, char *argv[] ) {
	
	unsigned long long count = 0;
	std::cout << "Main procedure " << std::endl;
	Producer producer ( queue, 1000000 );
	std::cout << "Producer constructed " << std::endl;
	Consumer consumer ( queue, count );
	std::cout << "Consumer constructed " << std::endl;
	//std::thread producerThread ( producer );
	std::thread producerThread ( Producer ( queue, 1000000 ) );
	std::cout << "Producer thread started " << std::endl;
	//std::thread consumerThread ( consumer );
	std::thread consumerThread ( Consumer ( queue, count ) );
	std::cout << "Consumer thread started " << std::endl;


	producerThread.join ();

	queue.enqueue ( 0 );

	consumerThread.join ();

	std::cout << "Producer count: " << producer.getCount() << " Consumer count: " << count << std::endl;

	return 0;

}
