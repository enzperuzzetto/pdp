
// sched (schedulers)
// Copyright (c) 2012-2016 Damien Magoni

#if !defined(SCHED_H)
#define SCHED_H

//#include <boost/chrono/chrono.hpp>
#include <boost/thread/thread.hpp>
#include <boost/chrono.hpp>
#include <boost/bind.hpp>
#include <boost/regex.hpp>
#include <boost/asio.hpp>

#include "mt19937.h"
#include "prost.h"
#include "medeb.h"
#include "mesap.h"
#include "util.h"

typedef boost::chrono::nanoseconds nsec;
typedef boost::chrono::microseconds usec;
typedef boost::chrono::milliseconds msec;
typedef boost::chrono::seconds sec;
typedef boost::chrono::duration<double> fpsec;
typedef boost::chrono::high_resolution_clock::time_point time_type;
typedef boost::asio::basic_waitable_timer<boost::chrono::high_resolution_clock> timer_type;

namespace sched
{
	class action_type;
	class actor
	{
	public:
		virtual void execute(action_type * a);
		virtual void execute(const boost::system::error_code & ec, action_type * a);
		virtual ~actor();
	};

	class acted
	{
	public:
		virtual ~acted();
	};

	typedef int action_status_type;
	const action_status_type queued = 1;
	const action_status_type started = 2;
	const action_status_type finished = 3;
	const action_status_type cancelled = 4;

	typedef dword action_id_type;

	class action_type
	{
	public:
		action_status_type status;
		action_id_type id;
		time_type start_time, stop_time, call_time; // when the handler is called
		nsec duration;
		fpsec duration_in_s;
		std::string description; // what does the action
		std::string handler; // what to do at the call time (end) of the action
		actor * doer; // the one who does the action (subject)
		acted * doed; // the one upon whom the action is done (object)
	};

	class nrt_scheduler_type
	{
	public:
		bool running;
		time_type start_time, current_time;
		nsec scenario_duration;
		fpsec scenario_duration_in_s;
		nsec time_step_in_ns; // granularity, sampling interval, etc
		action_id_type action_id_counter;
		std::multimap<time_type, action_type *> actions;
		std::multimap<time_type, action_type *>::iterator ait;
		mesap::messenger * error, * warning, * output;

		nrt_scheduler_type();
		~nrt_scheduler_type();
		void reset();
		void run();
		void pause();
		void resume();
		void halt();
		action_id_type schedule(action_type * a);	// sched later event
		bool cancel(action_id_type id);	// cancel event
	};

	class rt_scheduler_type
	{
	public:
		boost::asio::io_service * io_serv;
		boost::asio::io_service::strand * stran;
		bool running;
		time_type start_time, current_time;
		nsec scenario_duration;
		fpsec scenario_duration_in_s;
		action_id_type action_id_counter;
		std::multimap<time_type, action_type *> actions;

		timer_type * timer;
		bool timer_running;
		time_type epoch_time, last_action_time, action_call_time;
		mesap::messenger * error, * warning, * output;

		rt_scheduler_type();
		~rt_scheduler_type();
		void run();
		void reset();
		void start();
		void pause();
		void resume();
		void stop();
		action_id_type schedule(action_type * a); // sched later event
		void schedule_next(); // sched later event
		//bool cancel(action_id_type id);	// cancel event
	};
}

#endif
