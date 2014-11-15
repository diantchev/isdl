/// Queue template definitions
#pragma once
#include <atomic>

namespace isdl {


///@brief Policy class for single threaded model
class SingleThreadedModel {
};


///@brief policy class for multi threaded model
class MultiThreadedModel {
};


///@brief static converter from size_t to bit shift
template < size_t SIZE > struct SizeBitShift {

	enum { value = SizeBitShift < ( SIZE >> 1 ) >::value, index = SizeBitShift < ( SIZE >> 1 ) >::index+1 };
};


template < > struct SizeBitShift < 0 > {
	enum { value = 0, index = 0 };
};

/// Policy classes to define different behaviour



template < size_t RING_MASK, size_t SHIFT_INDEX, typename T, typename ThreadingModel > class QueueIndex {
};


template < size_t RING_MASK, size_t SHIFT_INDEX, typename T > class QueueIndex < RING_MASK, SHIFT_INDEX, T, SingleThreadedModel> {
	T _index;
	std::atomic < T > _size;
public:
	QueueIndex ( T initialSize ) : _index ( 0 ), _size ( initialSize ) {}
	inline bool incrementIndex ( T& index ) {
		if ( ! _size  )
			return false;
		--_size;
		index = _index & RING_MASK;
		++_index;
	 	return true; 
	}

	inline T commitIndex () { return 1; }
	
	inline void updateIndex ( T increment ) {
		_size += increment;
	}
	
	
};


template < size_t RING_MASK, size_t SHIFT_INDEX, typename T > class QueueIndex < RING_MASK, SHIFT_INDEX, T, MultiThreadedModel > {

	std::atomic < T > _runningIndex;			///< Running index 
	std::atomic < T > _committedIndex;			///< CommittedIndex

public:
	QueueIndex ( T initialSize ) :
		 _runningIndex ( initialSize ), _committedIndex ( 0 ) {}

	inline bool incrementIndex ( T& index ) {
		T newIndex;
		do {
			/// Check if we have enough elements to write in the queue
			index = _runningIndex;
			if ( ! ( index & RING_MASK ) )
				return false;
			/// Increment head index and decrement write size in one shot
			newIndex = index + RING_MASK;
			
		} while ( ! _runningIndex.compare_exchange_weak ( index, newIndex ) );
		index = (index>>SHIFT_INDEX)&RING_MASK;
		return true;
	}


	inline T commitIndex () {
		/// Commit the write
		T committedIndex, index, commitSize, newIndex;

		++_committedIndex;
		/// Check if we can update tail to signal arival of more elements
		do {
			///Synchronize head atomic with committed head
			do {
				index = _runningIndex;
				committedIndex = _committedIndex;
			}while ( index != _runningIndex );
			commitSize = committedIndex & RING_MASK;
			if ( commitSize + ( committedIndex >> SHIFT_INDEX ) < index >> SHIFT_INDEX )
				return 0;
			newIndex = committedIndex + ( commitSize << SHIFT_INDEX ) - commitSize; 
		} while ( !_committedIndex.compare_exchange_weak ( committedIndex, newIndex ) );
		return commitSize;
		

	}
	
	inline void updateIndex ( T count ) {
		_runningIndex += count;
	}
};



template < typename T, size_t MAX_SIZE, typename ProducerModel, typename ConsumerModel > class LocklessQueue {

	static const size_t SHIFT_INDEX = SizeBitShift < MAX_SIZE >::index;
	static const size_t ARRAY_SIZE = 1<<SHIFT_INDEX;
	static const size_t RING_MASK = ARRAY_SIZE - 1;
	T _items [ ARRAY_SIZE ];
	QueueIndex < RING_MASK, SHIFT_INDEX, unsigned long long, ProducerModel > _head; 
	QueueIndex < RING_MASK, SHIFT_INDEX, unsigned long long, ConsumerModel > _tail; 

public:
	LocklessQueue ( ) : _head ( MAX_SIZE ), _tail( 0 ) { }



	bool enqueue ( const T& item ) {
		unsigned long long index;
		if ( ! _head.incrementIndex( index ) )
			return false;
		_items[index] = item;
		index = _head.commitIndex ();
		_tail.updateIndex ( index );
		return true;

	}

	bool dequeue ( T& item ) {
		unsigned long long index;
		if ( ! _tail.incrementIndex( index ) )
			return false;
		item = _items[index];
		index = _tail.commitIndex ();
		_head.updateIndex ( index );
		return true;
	}

};

}


