///Definition of classes that allow implementation of observer pattern

#include "LocklessQueue.h"
#include "evinterfaces.h"
#include <impl/events.h>


namespace isdl {




///@brief EventDispatcher class provides an interface for associating a waitable object with an action object
class EventDispatcher {

	/// Size of the operation queue for adding and removing events
	static const size_t EventQueueSize = 100; 

	struct OperationMessage {
		enum Operation {
			ADD_EVENT,
			REMOVE_EVENT
		};
		Operation _operation;
		std::shared_ptr < Action > _action;
		Waitable *_event;
		OperationMessage ( Operation operation, Waitable *event, std::shared_ptr < Action > action ) :
			_operation ( operation ), _action ( action ), _event ( event ) {}
	};

	LocklessQueue < OperationMessage, EventsQueueSize, MultiThreadedModel, SingleThreadedModel > _eventQueue;

	void operation () {

		OperationMessage message;

		while ( _eventQueue.dequeue ( message ) ) {
			
			switch ( message._operation ) {
				case OperationMessage::ADD_EVENT:

					break;
				case OperationMessage::REMOVE_EVENT:

					break;
			}

		}	
	}

	shared_ptr < EventDispathcerImpl > _impl;

	//// Event for the control operations
	shared_ptr < Event > _controlEvent;

	//// Action for the control operations
	shared_ptr < Action > _controlAction;
	
	EventDispatcher ( const EventDispatcher& ) = delete;
	
	EventDispatcher& operator = ( const EventDispatcher& ) = delete;


public:

	///@brief constructor
	EventDispathcer ( ) : _impl ( EventDispatcherImpl::instance( ) ) {
		/// Create an event for handling adding and removing control events
	}

	///@brief associates event with a function 
	///@param event is a pointer to platform dependent waitable object when event is set
	///	the specified action will be called in the context of the thread blocked on the run 
	///	method
	///@param[in] fn is a function pointer or functional object to be executed when event occurs
       	///@param[in] args... is varadic argument providing parameters for the action function	
	template < typename Function, typename... Args > void addEvent ( Event *event, Function&& fn, Args&&... args ) {


		_eventQueue.enqueue ( OperationMessage ( OperationMessage::ADD_EVENT, event, 
				std::shared_ptr < Action * > ( new ActionAdapter < decltype ( std::bind ( function, args... ) ) > ( function , args...)),
				result->second->second ) ); 

		_impl->controlEvent().set();
	}

	///@brief removes the action associated with the specified event
	///@param[in] event is pointer to waitable object to be removed from the list of tracked events
	void removeEvent ( Event* event ) {
		_eventQueue.enqueue ( OperationMessage ( OperationMessage::REMOVE_EVENT, event, nullptr ) );
		_impl->controlEvent().set();
	}

	///@brief method will block the calling thread all the associated actions will be called in the context of blocked thread
	inline void run () {
		_imp->run ();
	}

};


}
