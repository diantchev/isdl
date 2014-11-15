
namespace isdl {

///@brief abstract action interface 
class Action {
public:
	virtual void execute () = 0;
	virtual ~Action () {}
};



struct Event {
	virtual set () = 0;
	virtual reset () = 0;
};



///@brief implementation specific implementation of the interface
struct EventDispatcherImpl {

	static EventDispatcherImpl* instance( );
	virtual void addEvent ( Event *event, Action *action ) = 0;
	virtual void removeAction ( Event& event ) = 0;
	virtual void run() = 0;
	virtual ~EventDispatcherImpl () {} ;
};
	

} /// end isdl
