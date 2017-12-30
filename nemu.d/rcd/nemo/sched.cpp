
// sched (schedulers)
// Copyright (c) 2012-2016 Damien Magoni

#include "sched.h"

namespace sched
{
	actor::~actor()
	{
	}

	void actor::execute(action_type * a)
	{
		;// do nothing in base class a priori
	}

	void actor::execute(const boost::system::error_code & ec, sched::action_type * a)
	{
		;
	}

/*	nemo::mobility_event_type * acted::met_ptr()
	{
		return dynamic_cast<nemo::mobility_event_type *>(this); 
	}
*/	
	acted::~acted()
	{
	}

	nrt_scheduler_type::nrt_scheduler_type()//std::map<std::string, std::string> arg) :
//		*error("nrts_error.log", mesap::console|mesap::logging, true),
//		*warning("nrts_warning.log", mesap::console|mesap::logging, false),
//		*output("nrts_info.log", mesap::console, false)
	{
	}

	nrt_scheduler_type::~nrt_scheduler_type()
	{
	}

	void nrt_scheduler_type::reset()
	{
		running = false;
		action_id_counter = 1;
		actions.clear();
	}
/*
	void nrt_scheduler_type::run()
	{
		running = true;
		while ( running && actions.size() )
		{
			// start action(s) if necessary
			ait = actions.begin();
			// is the following useful?
			while ( ait != actions.end() ) // uses cpu! -> can do diff?
			{
				if ( ait->second->start_time <= current_time )
				{
					ait->second->status = started;
				}
				ait++;
			}
			ait = actions.begin();
			action_type * current_action = ait->second;
			if ( current_action->call_time < current_time )
			{
				*error << "this action should have been executed before now\n";
				cancel(current_action->id);
			}
			else if ( current_action->call_time == current_time ) // do it
			{
				//current_action->status = started;
				current_action->doer->execute(current_action);
				if ( current_action->status == finished )
				{
					if ( medeb::active ) medeb::delm(current_action);
					delete current_action;
					actions.erase(ait);
				}
				else
					*error << "this action has not properly finished\n";
			}
			else //if ( current_action->call_time > clock )
				current_time = current_action->call_time;
		}
		if ( actions.empty() )
		{
			halt();
		}
	}
*/
	// halt is no longer here, but it maybe useful?
	void nrt_scheduler_type::run()
	{
		while ( actions.size() )
		{
			// start action(s) if necessary
			ait = actions.begin();
			action_type * current_action = ait->second;
			if ( current_action->call_time < current_time )
			{
				*error << "this action should have been executed before now\n";
				cancel(current_action->id);
			}
			else if ( current_action->call_time == current_time ) // do it
			{
				//current_action->status = started;
				current_action->doer->execute(current_action);
				if ( current_action->status == finished )
				{
					if ( medeb::active ) medeb::delm(current_action);
					delete current_action;
					actions.erase(ait);
				}
				else
					*error << "this action has not properly finished\n";
			}
			else //if ( current_action->call_time > clock )
				current_time = current_action->call_time;
		}
	}

	void nrt_scheduler_type::resume()
	{
		running = true;
	}

	void nrt_scheduler_type::halt()
	{
		running = false;
	}

	action_id_type nrt_scheduler_type::schedule(action_type * a)
	{
		//std::cout << "scheduling action at " << std::fixed << std::setprecision(3) << a->stop_time << " s\n";
		if ( a->start_time < current_time || a->call_time < current_time )
			*error << "can not schedule back in time\n";
		if ( a->duration < fpsec(0) )
			*error << "can not schedule an action with a negative duration\n";
		if ( !a->doer )
			*error << "can not schedule an action without an actor\n";
		//if ( a->start_time + a->duration != a->stop_time )
			//mp->err << "time vector is not correct\n"; // may happen if duration is shorter than expected
		a->status = queued;
		a->id = action_id_counter;
		action_id_counter++;
		actions.insert(std::pair<time_type, action_type *>(a->call_time, a));
		return a->id;
	}

	bool nrt_scheduler_type::cancel(action_id_type id)
	{
		ait = actions.begin();
		while ( ait != actions.end() )
		{
			if ( ait->second->id == id )
			{
				if ( ait->second->status == queued || ait->second->status == started )
				{
					if ( medeb::active ) medeb::delm(ait->second);
					delete ait->second;
					actions.erase(ait);
					return true;
				}
				else
				{
					*error << "can not cancel a finished/cancelled action\n";
					return false;
				}
			}
			ait++;
		}
		return false; // nothing to cancel
	}

	rt_scheduler_type::rt_scheduler_type()// :
//		*error("rts_error.log", mesap::console|mesap::logging, true),
//		*warning("rts_warning.log", mesap::console|mesap::logging, false),
//		*output("rts_info.log", mesap::console, false)
	{
		timer = 0;
		timer_running = false;
	}

	rt_scheduler_type::~rt_scheduler_type()
	{
		if ( timer )
		{
			//timer->cancel(); // will call timer handler
			delete timer;
			timer = 0;
		}
	}

	void rt_scheduler_type::reset()
	{
		running = false;
		action_id_counter = 1;
		actions.clear();
		double start_delay = 0; // may be needed if there is a lot of connectivity events to schedule
		start_time = boost::chrono::high_resolution_clock::now() + boost::chrono::duration_cast<nsec>(fpsec(start_delay)); // RT start time!!!
		last_action_time = start_time;
	}

	action_id_type rt_scheduler_type::schedule(action_type * a)
	{
		//std::cout << "scheduling action at " << std::fixed << std::setprecision(3) << a->stop_time << " s\n";
		//if ( a->start_time < current_time || a->call_time < current_time )
		//	*error << "can not schedule back in time\n";
		if ( a->duration < fpsec(0) )
			*error << "can not schedule an action with a negative duration\n";
		if ( !a->doer )
			*error << "can not schedule an action without an actor\n";
		//if ( a->start_time + a->duration != a->stop_time )
			//mp->err << "time vector is not correct\n"; // may happen if duration is shorter than expected
		a->status = queued;
		a->id = action_id_counter;
		action_id_counter++;
		actions.insert(std::pair<time_type, action_type *>(a->call_time, a));
		return a->id;
	}

	void rt_scheduler_type::schedule_next()
	{
		if ( actions.empty() )
			return;
		action_type * a = actions.begin()->second;
		actions.erase(actions.begin());
		//nsec wait_time = boost::chrono::nanoseconds(100);
		//if ( last_action_time == start_time )
		//	last_action_time = f->arr_ts;// ? //boost::chrono::high_resolution_clock::now();
		//time_type action_call_time = last_action_time + wait_time;
		timer_running = true;
		std::size_t handlers_cancelled = 0;
		try
		{
			time_type ct = a->call_time;
			handlers_cancelled = timer->expires_at(ct);//a->call_time);//action_call_time);
		}
		catch ( std::exception & e )
		{
			*error << "\nrtscheduler exception: " <<  e.what() << "\n";
		}
		if ( handlers_cancelled )
		{
			*warning << "rts->expires_at has cancelled " << static_cast<unsigned long>(handlers_cancelled) << " handlers\n"; 
		}
		timer->async_wait(boost::bind(&actor::execute, a->doer, boost::asio::placeholders::error, a));
		//last_action_time = action_call_time;
	}

	void rt_scheduler_type::start()
	{
	}

	void rt_scheduler_type::pause()
	{
	}

	void rt_scheduler_type::resume()
	{
	}

	void rt_scheduler_type::stop()
	{
	}

	void rt_scheduler_type::run()
	{
		running = true;
		timer = new timer_type(*io_serv);
		//std::auto_ptr<boost::asio::io_service::work> work(new boost::asio::io_service::work(*io_serv)); // work = new...?
		//boost::thread t(boost::bind(&boost::asio::io_service::run, io_serv));
		//  main thread
		schedule_next();
		//while ( running && actions.size() ) // XXX useless?
		{
			//??????????????????????????????
			///command(); // blocking call, used to query and interrupt the rts
			//if ( exit_now )
			{
		//		running = false;
			}
		}
		//work.reset(); // finish async tasks
		//t.join();
	}
}
