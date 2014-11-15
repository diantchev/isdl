// PlatformUnitTest.cpp : Defines the entry point for the console application.
//

#include <thread>
#include <gtest/gtest.h>
#include <core/LocklessQueue.h>




namespace {
	

	template < typename T > class Consumer {
	T *_queue;
	unsigned long long& _elementCount;
	unsigned long long& _elementSum;
	public:
		Consumer ( T *queue, unsigned long long &elementCount, unsigned long long &elementSum ) : _queue ( queue ),
		_elementCount ( elementCount ), _elementSum ( elementSum ) {}
		void run ( ) {
			unsigned long long value;
			_elementCount = 0;
			_elementSum = 0;
			do {
				while ( !_queue->dequeue ( value ) ) ; //std::this_thread::yield();
				++_elementCount;
				_elementSum += value;
			
			} while ( value );
		}
	};

	template < typename T > class Producer {
	T *_queue;
	unsigned long long _elementCount;
	unsigned long long& _elementSum;
	public:
		Producer ( T *queue, unsigned long long elementCount, unsigned long long& elementSum ) :
			_queue ( queue ), _elementCount ( elementCount ),_elementSum ( elementSum ) {}
		void run ( ) {
			_elementSum = 0;
			for ( unsigned long long i = 1; i< _elementCount; ++i ) {
				while ( !_queue->enqueue ( i ) ) ; // std::this_thread::yield();
				_elementSum += i;
			
			} 
		}
	};

	TEST ( LocklessQueue, SingleThreadedConsumerSingleThreadedProducer ) {

		isdl::LocklessQueue < unsigned long long, 100,isdl::SingleThreadedModel, isdl::SingleThreadedModel > *queue = 
			new isdl::LocklessQueue < unsigned long long, 100, isdl::SingleThreadedModel, isdl::SingleThreadedModel > ();
		unsigned long long consumerCount, consumerSum, producerSum;
		unsigned long long producerCount = 1000000; 
		Consumer<isdl::LocklessQueue < unsigned long long, 100, isdl::SingleThreadedModel, isdl::SingleThreadedModel> > consumer
		(queue, consumerCount, consumerSum );
		Producer<isdl::LocklessQueue < unsigned long long, 100, isdl::SingleThreadedModel, isdl::SingleThreadedModel> > producer
		(queue, producerCount, producerSum );
		
		std::thread consumerThread ( &Consumer< isdl::LocklessQueue < unsigned long long , 100, isdl::SingleThreadedModel, isdl::SingleThreadedModel > >::run, &consumer ) ;
		std::thread producerThread ( &Producer< isdl::LocklessQueue < unsigned long long , 100, isdl::SingleThreadedModel, isdl::SingleThreadedModel > >::run, &producer ) ;
		
		producerThread.join();
		queue->enqueue ( 0 );
		consumerThread.join();
		delete queue;

		ASSERT_EQ ( consumerCount, producerCount );
		ASSERT_EQ ( consumerSum, producerSum );
	}

	TEST ( LocklessQueue, SingleThreadedConsumerMultiThreadedProducer ) {

		isdl::LocklessQueue < unsigned long long, 100,isdl::MultiThreadedModel, isdl::SingleThreadedModel > *queue = 
			new isdl::LocklessQueue < unsigned long long, 100, isdl::MultiThreadedModel, isdl::SingleThreadedModel > ();
		unsigned long long consumerCount, consumerSum, producerSum1, producerSum2, producerSum3;
		unsigned long long producerCount = 1000000; 
		Consumer<isdl::LocklessQueue < unsigned long long, 100, isdl::MultiThreadedModel, isdl::SingleThreadedModel> > consumer
		(queue, consumerCount, consumerSum );
		Producer<isdl::LocklessQueue < unsigned long long, 100, isdl::MultiThreadedModel, isdl::SingleThreadedModel> > producer1
		(queue, producerCount, producerSum1 );
		Producer<isdl::LocklessQueue < unsigned long long, 100, isdl::MultiThreadedModel, isdl::SingleThreadedModel> > producer2
		(queue, producerCount, producerSum2 );
		Producer<isdl::LocklessQueue < unsigned long long, 100, isdl::MultiThreadedModel, isdl::SingleThreadedModel> > producer3
		(queue, producerCount, producerSum3 );
		
		std::thread consumerThread ( &Consumer< isdl::LocklessQueue < unsigned long long , 100, isdl::MultiThreadedModel, isdl::SingleThreadedModel > >::run, &consumer ) ;
		std::thread producerThread1 ( &Producer< isdl::LocklessQueue < unsigned long long , 100, isdl::MultiThreadedModel, isdl::SingleThreadedModel > >::run, &producer1 ) ;
		std::thread producerThread2 ( &Producer< isdl::LocklessQueue < unsigned long long , 100, isdl::MultiThreadedModel, isdl::SingleThreadedModel > >::run, &producer2 ) ;
		std::thread producerThread3 ( &Producer< isdl::LocklessQueue < unsigned long long , 100, isdl::MultiThreadedModel, isdl::SingleThreadedModel > >::run, &producer3 ) ;
		
		producerThread1.join();
		producerThread2.join();
		producerThread3.join();
		queue->enqueue ( 0 );
		consumerThread.join();
		ASSERT_EQ ( consumerCount, 3*producerCount  - 2);
		ASSERT_EQ ( consumerSum, producerSum1+producerSum2+producerSum2 );
	}

	TEST ( LocklessQueue, MultiThreadedConsumerSingleThreadedProducer ) {

		isdl::LocklessQueue < unsigned long long, 100,isdl::SingleThreadedModel, isdl::MultiThreadedModel > *queue = 
			new isdl::LocklessQueue < unsigned long long, 100, isdl::SingleThreadedModel, isdl::MultiThreadedModel > ();
		unsigned long long consumerCount1,consumerCount2, consumerCount3, consumerSum1, consumerSum2, consumerSum3, producerSum;
		unsigned long long producerCount = 1000000; 
		Consumer<isdl::LocklessQueue < unsigned long long, 100, isdl::SingleThreadedModel, isdl::MultiThreadedModel> > consumer1
		(queue, consumerCount1, consumerSum1 );
		Consumer<isdl::LocklessQueue < unsigned long long, 100, isdl::SingleThreadedModel, isdl::MultiThreadedModel> > consumer2
		(queue, consumerCount2, consumerSum2 );
		Consumer<isdl::LocklessQueue < unsigned long long, 100, isdl::SingleThreadedModel, isdl::MultiThreadedModel> > consumer3
		(queue, consumerCount3, consumerSum3 );
		Producer<isdl::LocklessQueue < unsigned long long, 100, isdl::SingleThreadedModel, isdl::MultiThreadedModel> > producer
		(queue, producerCount, producerSum );
		
		std::thread consumerThread1 ( &Consumer< isdl::LocklessQueue < unsigned long long , 100, isdl::SingleThreadedModel, isdl::MultiThreadedModel > >::run, &consumer1 ) ;
		std::thread consumerThread2 ( &Consumer< isdl::LocklessQueue < unsigned long long , 100, isdl::SingleThreadedModel, isdl::MultiThreadedModel > >::run, &consumer2 ) ;
		std::thread consumerThread3 ( &Consumer< isdl::LocklessQueue < unsigned long long , 100, isdl::SingleThreadedModel, isdl::MultiThreadedModel > >::run, &consumer3 ) ;
		std::thread producerThread ( &Producer< isdl::LocklessQueue < unsigned long long , 100, isdl::SingleThreadedModel, isdl::MultiThreadedModel > >::run, &producer ) ;
		
		producerThread.join();
		queue->enqueue ( 0 );
		queue->enqueue ( 0 );
		queue->enqueue ( 0 );
		consumerThread1.join();
		consumerThread2.join();
		consumerThread3.join();
		ASSERT_EQ ( consumerCount1+consumerCount2+consumerCount3, producerCount + 2);
		ASSERT_EQ ( consumerSum1+consumerSum2+consumerSum3, producerSum );
	}

	TEST ( LocklessQueue, MultiThreadedConsumerMultiThreadedProducer ) {

		isdl::LocklessQueue < unsigned long long, 100,isdl::MultiThreadedModel, isdl::MultiThreadedModel > *queue = 
			new isdl::LocklessQueue < unsigned long long, 100, isdl::MultiThreadedModel, isdl::MultiThreadedModel > ();
		unsigned long long consumerCount1,consumerCount2, consumerCount3, consumerSum1, consumerSum2, consumerSum3, producerSum1,
			producerSum2, producerSum3;
		unsigned long long producerCount = 1000000; 
		Consumer<isdl::LocklessQueue < unsigned long long, 100, isdl::MultiThreadedModel, isdl::MultiThreadedModel> > consumer1
		(queue, consumerCount1, consumerSum1 );
		Consumer<isdl::LocklessQueue < unsigned long long, 100, isdl::MultiThreadedModel, isdl::MultiThreadedModel> > consumer2
		(queue, consumerCount2, consumerSum2 );
		Consumer<isdl::LocklessQueue < unsigned long long, 100, isdl::MultiThreadedModel, isdl::MultiThreadedModel> > consumer3
		(queue, consumerCount3, consumerSum3 );
		Producer<isdl::LocklessQueue < unsigned long long, 100, isdl::MultiThreadedModel, isdl::MultiThreadedModel> > producer1
		(queue, producerCount, producerSum1 );
		Producer<isdl::LocklessQueue < unsigned long long, 100, isdl::MultiThreadedModel, isdl::MultiThreadedModel> > producer2
		(queue, producerCount, producerSum2 );
		Producer<isdl::LocklessQueue < unsigned long long, 100, isdl::MultiThreadedModel, isdl::MultiThreadedModel> > producer3
		(queue, producerCount, producerSum3 );
		
		std::thread consumerThread1 ( &Consumer< isdl::LocklessQueue < unsigned long long , 100, isdl::MultiThreadedModel, isdl::MultiThreadedModel > >::run, &consumer1 ) ;
		std::thread consumerThread2 ( &Consumer< isdl::LocklessQueue < unsigned long long , 100, isdl::MultiThreadedModel, isdl::MultiThreadedModel > >::run, &consumer2 ) ;
		std::thread consumerThread3 ( &Consumer< isdl::LocklessQueue < unsigned long long , 100, isdl::MultiThreadedModel, isdl::MultiThreadedModel > >::run, &consumer3 ) ;
		std::thread producerThread1 ( &Producer< isdl::LocklessQueue < unsigned long long , 100, isdl::MultiThreadedModel, isdl::MultiThreadedModel > >::run, &producer1 ) ;
		std::thread producerThread2 ( &Producer< isdl::LocklessQueue < unsigned long long , 100, isdl::MultiThreadedModel, isdl::MultiThreadedModel > >::run, &producer2 ) ;
		std::thread producerThread3 ( &Producer< isdl::LocklessQueue < unsigned long long , 100, isdl::MultiThreadedModel, isdl::MultiThreadedModel > >::run, &producer3 ) ;
		
		producerThread1.join();
		producerThread2.join();
		producerThread3.join();
		queue->enqueue ( 0 );
		queue->enqueue ( 0 );
		queue->enqueue ( 0 );
		consumerThread1.join();
		consumerThread2.join();
		consumerThread3.join();
		ASSERT_EQ ( consumerCount1+consumerCount2+consumerCount3, 3*producerCount );
		ASSERT_EQ ( consumerSum1+consumerSum2+consumerSum3, producerSum1+producerSum2+producerSum3 );
	}
}

int main ( int argc, char *argv[] ) {
	::testing::InitGoogleTest ( &argc, argv );
	return RUN_ALL_TESTS ();
}

