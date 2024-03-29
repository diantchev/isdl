/**
 * Implementation of methods of the logger framework
 */

#include <logger>
#include <disruptor>
#include <cstring>
#include <unordered_map>
#include <thread>
#include <chrono>
#include <iomanip>
#include <iostream>


namespace isdl {


const char *_trace = "TRACE";
const char *_debug = "DEBUG";
const char *_info = "INFO";
const char *_warning = "WARN";
const char *_error = "ERROR";


_log_level::operator const char *() {
	switch ( _v ) {
	case log_level::trace:
		return _trace;
	case log_level::debug:
		return _debug;
	case log_level::info:
		return _info;
	case log_level::warning:
		return _warning;
	case log_level::error:
		return _error;
	}
	return _info;
}


constexpr size_t LOG_QUEUE_SIZE_POWER_OF_TWO = 17;
constexpr size_t LOG_BUFFER_SIZE = 100;
constexpr const char  *DEFAULT_TIME_FORMAT = "%Y-%b-%d-%H:%M:%S.";

constexpr log_level default_log_level = log_level::info;



/**
 *@brief Captures logging information
 */
struct log_event {
	log_level _level;
        log_back  *_back;
        const char *_file_name;
        int _src_line_number; 
        timestamp _timestamp;
        bool _end_of_batch; 
        seq_t _prev_batch_seq;
        char _msg[LOG_BUFFER_SIZE];
        size_t _msg_len;

};

/**
 *@brief Disruptor wait strategy 
 */
struct WaitStrategy {
	void wait () {
		std::this_thread::yield();
	}

	void notify () {
	}
};




static class basic_logback : public log_back {
public:
	virtual void add ( log_level __v, const char *__f, int __l, timestamp __t,  const char *__m,
		size_t __s, bool __b, bool __e ) {
		if ( __b ) {
			time_t local_time = std::chrono::system_clock::to_time_t ( __t );
			std::cout << std::put_time ( std::localtime ( &local_time ), 
				DEFAULT_TIME_FORMAT ) << 
			std::chrono::duration_cast<std::chrono::milliseconds>(__t.time_since_epoch() ).count() % 1000 <<" : ";
		}
		std::cout.write ( __m, __s );
		if ( __e ) {
			std::cout << std::endl;
		}
		
	}
} default_logback;

class logger : public basic_logger  {
	friend class default_logger_factory;
public:
	logger ( log_back *lback, log_level lvl ) : basic_logger ( lback, lvl ) {}
	logger ( ) : basic_logger ( &default_logback, log_level::info ) {}
};

static logger default_logger;

/**
 * Map with the names of all the registered log backs
 */
static std::unordered_map < std::string, logger > _log_backs;






disruptor < log_event, int64_t, WaitStrategy> _disruptor ( 2 << LOG_QUEUE_SIZE_POWER_OF_TWO );

struct log_handler {

	void process_event ( int64_t seq, log_event& ev ) {

		int64_t prev_seq = ev._prev_batch_seq;

		if ( prev_seq != seq ) {

			process_event ( prev_seq, _disruptor[prev_seq] );
			ev._back->add ( ev._level, ev._file_name, ev._src_line_number,
			ev._timestamp, ev._msg, ev._msg_len, false, ev._end_of_batch );

		} else {

			ev._back->add ( ev._level, ev._file_name, ev._src_line_number,
			ev._timestamp, ev._msg, ev._msg_len, true, ev._end_of_batch );
		}
	}
	
	bool event ( int64_t seq, log_event& ev ) {
		if ( ! ev._end_of_batch ) 
			return false;
		process_event ( seq, ev );
		return true;
	}

} handler;
	



/**
 * @brief implementation of logger factory
 */
struct default_logger_factory : public logger_factory {
	default_logger_factory () {
		_disruptor.first ( handler );
		_disruptor.start ();
		 
	}
	virtual basic_logger& get_logger ( const char *back_name );
	virtual void add_logger ( const char *name, log_back *back, log_level level );

} _log_factory;

logger_factory *log_factory = &_log_factory;








/**
 *@brief Gets a logger with the specified name 
 *@param logger_name is the name for the logger 
 */
basic_logger& default_logger_factory::get_logger( const char *logger_name ) {

	/// Check if the we have a logger for that name already
	std::string log_key = std::string ( logger_name );
	auto itr = _log_backs.find ( log_key ) ;
	if ( itr == _log_backs.end() ) {
		/// Use the default logback if there is no logback registerd
		return default_logger;
	} else {
		return itr->second;
	} 
	
}

/**
 *@brief registers logger configuration with the factory
 *@param name is the configuration name
 *@param back is the a pointer to a backer for the loging
 *@param level is the log level for this configuration
 */
void default_logger_factory::add_logger ( const char *name, log_back *back, log_level level ) {
	_log_backs[std::string ( name )]._back = back;
	_log_backs[std::string ( name )]._level = level;
	 
}


void log_buffer::_init_ptrs () {
	_curr_seq = _disruptor.next();
	log_event& ev = _disruptor [_curr_seq];
	ev._end_of_batch = true;
	ev._prev_batch_seq = _curr_seq;
	ev._msg_len = 0;
	ev._back = _back;
	/// Initialize the inherited members
	_M_out_beg = ev._msg; 
	_M_out_cur = _M_out_beg;
	_M_out_end = _M_out_beg+LOG_BUFFER_SIZE;	
}

log_buffer::log_buffer ( log_level __v, log_back *__b, const char *__f, 
			int __l, timestamp __t ) : 
		_back { __b }  {
	_init_ptrs();
	/// Mark this entry as end of batch first
	/// And change it later on if we need a batch with more
	/// than one entry
	log_event& ev = _disruptor [_curr_seq];
	ev._level = __v;
	ev._back = _back;
	ev._file_name = __f;
	ev._src_line_number = __l;
	ev._timestamp = __t;
	_M_in_beg = nullptr;
	_M_in_cur = nullptr;
	_M_in_end = nullptr;
		
		
}

/**
 *@brief insert multiple character in the buffer
 */
std::streamsize log_buffer::xsputn ( const char_type *__s, std::streamsize __n ) {
	size_t cpy_len = __n;
	
	/// Set the message length based on the difference between the 
	/// beginning pointer and the curr pointer. This is the only relyable way
	/// of getting the length of the buffer
	_disruptor [ _curr_seq ]._msg_len = _M_out_cur - _M_out_beg;
	size_t remaining_size = LOG_BUFFER_SIZE - _disruptor[_curr_seq]._msg_len;
	/// Check if we have enough space to copy the whole buffer
	while ( cpy_len > remaining_size ) { /// Overflow the buffer 
		std::memcpy ( _M_out_cur, __s, remaining_size );
		__s += remaining_size;
		cpy_len -= remaining_size;
		_disruptor [_curr_seq ]._msg_len += remaining_size;
		_disruptor [_curr_seq ]._end_of_batch = false;
		_disruptor.publish( _curr_seq);
		auto seq = _curr_seq;
		/// _init_ptrs changes the value of _curr_seq with newelly allocated 
		/// sequence
		_init_ptrs ();
		_disruptor[_curr_seq]._prev_batch_seq = seq;
		remaining_size = LOG_BUFFER_SIZE;
		
	}
	/// Insert the rest of the data
	if ( cpy_len > 0 ) {
		std::memcpy ( _M_out_cur, __s, cpy_len );
		_M_out_cur += cpy_len;
		_disruptor [_curr_seq]._msg_len += cpy_len;
	} 

	return __n;




}

/**
 *@brief called when overflow occurs. Gets a new slot and caries on
 */
log_buffer::int_type log_buffer::overflow(log_buffer::int_type __c ) {
	_disruptor[_curr_seq]._msg_len = _M_out_cur - _M_out_beg;
	_disruptor[_curr_seq]._end_of_batch = false;
	_disruptor.publish ( _curr_seq, 1 );
	auto seq = _curr_seq;
	_init_ptrs ();
	*_M_out_cur++ = __c;
	return __c;
}

/**
 *@brief destructor
 * 	Makes sure that the last data is committed
 */
log_buffer::~log_buffer () {
	_disruptor[_curr_seq]._msg_len = _M_out_cur - _M_out_beg;
	_disruptor.publish ( _curr_seq );
}





}
