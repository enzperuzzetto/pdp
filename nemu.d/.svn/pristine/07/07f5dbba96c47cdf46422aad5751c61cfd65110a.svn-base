
// iface
// Copyright (C) 2012-2016 Damien Magoni

#include "vnd.hpp"

namespace vnd
{
	frame::frame()
	{
	}

	frame::~frame()
	{
	}

	std::string address::print() const
	{
		std::ostringstream os;
		os.flags(std::ios::right|std::ios::hex);
		for ( dword i = 0; i < address_bytes.size(); i++ )
		{
			dword j = address_bytes[i];
			os << std::setw(2) << std::setfill('0') << j;
			if ( i < address_bytes.size() - 1 )
				os << ":";
		}
		os.flags(std::ios::right|std::ios::dec);
		return os.str();
	}

	bool address::is_broadcast()
	{
		std::vector<byte> bcsta;
		bcsta.assign(6, 0xFF);
		if ( address_bytes.size() != bcsta.size() )
			return false;
		return ( address_bytes == bcsta );
	}

	bool address::is_null()
	{
		std::vector<byte> nulla;
		nulla.assign(6, 0x00);
		if ( address_bytes.size() != nulla.size() )
			return false;
		return ( address_bytes == nulla );
	}

	bool address::operator==(const address & rh) const
	{
		return rh.is_equal(*this); // 2nd/double dispatch
	}

	bool address::operator!=(const address & rh) const
	{
		return !rh.is_equal(*this); // 2nd/double dispatch
	}

	bool address::operator<(const address & rh) const
	{
		return rh.is_inferior(*this); // 2nd/double dispatch
	}

	bool address::is_equal(const address & rh) const // lexicographic order
	{
		if ( address_bytes.size() != rh.address_bytes.size() )
			return false;
		dword i;
		for ( i = 0; i < address_bytes.size(); i++ )
		{
			if ( address_bytes[i] != rh.address_bytes[i] )
				return false;
		}
		return true;
	}

	bool address::is_inferior(const address & rh) const // lexicographic order
	{
		dword i;
		for ( i = 0; i < rh.address_bytes.size(); i++ )
		{
			if ( i >= address_bytes.size() )
				break;
			if ( address_bytes[i] < rh.address_bytes[i] )
				return true;
			else if ( address_bytes[i] > rh.address_bytes[i] )
				return false;
			else
				continue;
		}
		if ( address_bytes.size() < rh.address_bytes.size() )
			return true;
		else
			return false;
	}

	hold_queue::hold_queue(boost::asio::io_service * io_serv)
	{
		up = true;
		input = true;
		vli = 0;
		bw = default_bw; // bits per s
		dl = 0; // delay in s
		dv = 0; // delay variation in %
		ber = 0; // bit error rate
		queue_size = default_queue_size;
		buffer_size = default_buffer_size;
		burst = queue_size * buffer_size; // bytes
		excess_burst = burst;
		buffer_queue.clear();
		line_pt = 0;
		activity_duration = fpsec(0);
		epoch_time = boost::chrono::high_resolution_clock::now();
		unix_epoch_time = boost::chrono::system_clock::to_time_t(boost::chrono::system_clock::now()); // = std::time(NULL)
		last_unstat_time = epoch_time;
		last_frame_time = epoch_time;
		timer = new high_resolution_timer_type(*io_serv);
		timer->expires_at(epoch_time);
		timer_running = false;
		bytes_counter = 0;
		frames_counter = 0;
		lost_bytes_counter = 0;
		lost_frames_counter = 0;
	}

	hold_queue::~hold_queue()
	{
		if ( timer )
		{
			// endpt may not exist anymore! so the handler will segfault
			if ( line_pt->bound_ep ) // does this work?
			{
				dword asyn_ops_cancelled = 0;
				try
				{
					asyn_ops_cancelled = timer->cancel();
				}
				catch ( std::exception & e )
				{
					line_pt->vnd_pt->error << "\n" << print_id() << " exception: " <<  e.what() << "\n";
				}
				if ( asyn_ops_cancelled )
				{
					line_pt->vnd_pt->warning << "\n" << print_id() << " expires_at/cancel has cancelled " << asyn_ops_cancelled << " operations\n"; 
				}
			}
			delete timer;
			timer = 0;
		}
		std::map<time_type, frame *>::iterator it = buffer_queue.begin();
		while ( it != buffer_queue.end() )
		{
			line_pt->vnd_pt->destroy_frame(it->second);
			it++;
		}
	}

	std::string hold_queue::print_id()
	{
		std::string s = line_pt->id + "-";
		if ( input )
			return s + "in";
		return s+ "out";
	}

	dword hold_queue::byte_count()
	{
		dword count = 0;
		std::map<time_type, frame *>::iterator it = buffer_queue.begin();
		while ( it != buffer_queue.end() )
		{
			count += it->second->buffer.size();
			it++;
		}
		return count;
	}

	bool hold_queue::compute_ber(frame * f)
	{
		double bit_length = f->buffer.size() * 8;
		double proba_frame_ok = pow(1.0 - ber, bit_length);
		//std::cout << "prob="<<proba_frame_ok<<"\n";
		double proba = 100 * proba_frame_ok;
		bool ok = prost::roll(proba);
		//line_pt->vnd_pt->warning << "warn: ber="<<ber<<" proba="<<proba<<" ok="<<ok<<"\n";
		return ok;
	}

	void hold_queue::discard_frame(frame * f)
	{
		line_pt->vnd_pt->destroy_frame(f);
		fast_read_loop();
	}

	void hold_queue::fast_read_loop()
	{
		if ( line_pt->bound_ep ) // not needed? needed in case of inj or disc
			line_pt->vnd_pt->io_serv->post(boost::bind(&end_point::reread_loop, line_pt->bound_ep)); // concur access
			//line_pt->bound_ep->read_loop(ec, true);
		else
			line_pt->vnd_pt->warning << "warning> can not call reread_loop on undefined endpoint\n";
	}

	void hold_queue::handle_wait(const boost::system::error_code & ec, frame * f)
	{
		timer_running = false;
		if ( ec ) // used by async_wait
		{
			if ( ec == boost::asio::error::make_error_code(boost::asio::error::operation_aborted) )
				line_pt->vnd_pt->warning << "warning> [handle_wait] the timer was cancelled (because of a call to expire/cancel): " << ec.value() << " = " << ec.message() << "\n";
			else
				line_pt->vnd_pt->error << "error> [handle_wait] (timer pb from async_wait): " << ec.value() << " = " << ec.message() << "\n";
			line_pt->vnd_pt->destroy_frame(f); // ???
			return;
		}
		if ( input )
		{
			if ( line_pt->trace_flag || line_pt->trace_file_flag )
				line_pt->trace(f, 0);
			line_pt->vnd_pt->io_serv->post(boost::bind(&network_device::forward, line_pt->vnd_pt/*, boost::asio::placeholders::error*/, f));
			//boost::system::error_code ec;
			//line_pt->vnd_pt->forward(ec, f);
			schedule();
/*			if ( line_pt->bound_ep )
				line_pt->vnd_pt->io_serv->post(boost::bind(&end_point::reread_loop, line_pt->bound_ep));
				//line_pt->vnd_pt->io_serv->post(boost::bind(&end_point::read_loop, line_pt->bound_ep, boost::asio::placeholders::error, false)); // issues with placeholders::error and previous arg != this
			else
				line_pt->vnd_pt->warning << "warning> can not call read_loop on undefined endpoint\n";
*/		}
		else
		{
			if ( line_pt->bound_ep )
				line_pt->bound_ep->rewrite_loop(f);
				//line_pt->vnd_pt->io_serv->post(boost::bind(&end_point::rewrite_loop, line_pt->bound_ep, f));
				//line_pt->vnd_pt->io_serv->post(boost::bind(&end_point::write_loop, line_pt->bound_ep, boost::asio::placeholders::error, f));
			else
				line_pt->vnd_pt->warning << "warning> can not call write_loop on undefined endpoint\n";
		}
	}

	void hold_queue::wait(frame * f)
	{
		if ( timer_running )
		{
			if ( input )
			{
				line_pt->vnd_pt->warning << "warning> wait called while timer_running is true\n";
				line_pt->vnd_pt->warning << "warning> shall we call read_loop? no\n";
			}
	/*		else
			{
				line_pt->vnd_pt->warning << "> schedule called while timer_running is true\n";
				line_pt->vnd_pt->warning << "we'll come back\n"; // happens often
			}
	*/		return; // do nothing
		}
		dword bits = f->buffer.size() * 8;
		double fp_ns_per_bit = static_cast<double>(1.0E9) / static_cast<double>(bw);
		double fp_ns_per_frame = static_cast<double>(bits) * fp_ns_per_bit;
		if ( dl > 0 )
		{
			if ( dv > 0 )
			{
				double sign = 1;
				if ( prost::roll(50) )
					sign = -1;
				dl *= (1 + sign * prost::random_dbl(0, dv) / 100);
			}
			fp_ns_per_frame += (dl * 1.0E9);
		}
		qword ns_wt = static_cast<qword>(fp_ns_per_frame);
		nsec wait_time = nsec(ns_wt);
		time_type expiration_time;
		if ( input )
		{
			f->arr_ts += wait_time;
			expiration_time = f->arr_ts;
		}
		else
		{
			expiration_time = f->arr_ts + wait_time;
		}
		//if ( last_frame_time == epoch_time ) // no previous frame received
		{
			//last_frame_time = f->arr_ts;
		}
		if ( expiration_time > boost::chrono::high_resolution_clock::now() ) // wait
		{
			std::size_t asyn_ops_cancelled = 0;
			try
			{
				asyn_ops_cancelled = timer->expires_at(expiration_time);
			}
			catch ( std::exception & e )
			{
				line_pt->vnd_pt->error << "[in/output queue->schedule] expires_at exception: " <<  e.what() << "\n";
			}
			if ( asyn_ops_cancelled ) // there were pending ops, impossible! we managed to cancel the timer, start new asynchronous wait
			{
				line_pt->vnd_pt->warning << "[in/output queue->schedule] expires_at has cancelled " << static_cast<dword>(asyn_ops_cancelled) << " operations\n";
			}
			timer_running = true;
			timer->async_wait(boost::bind(&hold_queue::handle_wait, this, boost::asio::placeholders::error, f));
		}
		else // act immediately
		{
			boost::system::error_code ec;
			handle_wait(ec, f);
		}
	}

	// called by handle_read/input or inj/input
	void hold_queue::preprocess(frame * f)
	{
		f->input_interface = line_pt->id;
		f->vli = vli;
		if ( line_pt->protocol == protocol_none )
		{
			push(f);
			return;
		}
		if ( line_pt->interface_family == interface_ral && line_pt->protocol == protocol_pseudo_802_11 )
		{
			if ( f->buffer.size() < 21/*64*/ || f->buffer.size() > 2304 )
			{
				//if ( line_pt->vnd_pt->debug ) 
					line_pt->vnd_pt->warning << "\n> [" << print_id() << "] bad length frame, dropping frame\n";
				discard_frame(f);		
				return;
			}
			if ( f->buffer[0] == 0x00 ) // IBSS
			{
				f->to_ds = false;
				f->from_ds = false;
			}
			else if ( f->buffer[0] == 0x01 ) // toDS
			{
				f->to_ds = true;
				f->from_ds = false;
			}
			else if ( f->buffer[0] == 0x02 ) // fromDS
			{
				f->to_ds = false;
				f->from_ds = true;
			}
			else if ( f->buffer[0] == 0x03 ) // WDS
			{
				f->to_ds = true;
				f->from_ds = true;
			}
			else
			{
				//if ( line_pt->vnd_pt->debug ) 
					line_pt->vnd_pt->warning << "\n> [" << print_id() << "] bad to/from DS field in pseudo header, dropping frame\n";
				discard_frame(f);
				return;
			}
			if ( !f->to_ds && !f->from_ds ) // from ad hoc mobile node
			{
				if ( line_pt->vnd_pt->device != device_mobile_adhoc )
				{
					line_pt->vnd_pt->error << "> we should be in an adhoc mobile device\n";
				}
				f->dst.address_bytes.assign(f->buffer.begin() + 1, f->buffer.begin() + 7);
				f->src.address_bytes.assign(f->buffer.begin() + 7, f->buffer.begin() + 13);
				f->bss.address_bytes.assign(f->buffer.begin() + 13, f->buffer.begin() + 19);
			}
			else if ( f->to_ds && !f->from_ds ) // from infra mobile node
			{
				if ( line_pt->vnd_pt->device != device_access_point )
				{
					line_pt->vnd_pt->error << "> we should be in an AP device\n";
				}
				f->bss.address_bytes.assign(f->buffer.begin() + 1, f->buffer.begin() + 7);
				f->src.address_bytes.assign(f->buffer.begin() + 7, f->buffer.begin() + 13);
				f->dst.address_bytes.assign(f->buffer.begin() + 13, f->buffer.begin() + 19);
			}
			else if ( !f->to_ds && f->from_ds ) // from AP
			{
				if ( line_pt->vnd_pt->device != device_mobile_infra )
				{
					line_pt->vnd_pt->error << "> we should be in an infra mobile device\n";
				}
				f->dst.address_bytes.assign(f->buffer.begin() + 1, f->buffer.begin() + 7);
				f->bss.address_bytes.assign(f->buffer.begin() + 7, f->buffer.begin() + 13);
				f->src.address_bytes.assign(f->buffer.begin() + 13, f->buffer.begin() + 19);
				line_pt->vnd_pt->ap_address = f->bss; // last AP @ is always stored as associated AP
			}
			else // if ( f->to_ds && f->from_ds ) // WDS bridge
			{
				if ( line_pt->vnd_pt->device != device_access_point )
				{
					line_pt->vnd_pt->error << "> we should be in an AP/WDS bridge device\n";
				}
				f->dst.address_bytes.assign(f->buffer.begin() + 1, f->buffer.begin() + 7); // !!!
				f->bss.address_bytes.assign(f->buffer.begin() + 7, f->buffer.begin() + 13);
				f->dst.address_bytes.assign(f->buffer.begin() + 13, f->buffer.begin() + 19);
				f->src.address_bytes.assign(f->buffer.begin() + 19, f->buffer.begin() + 25);
			}
			line_pt->last_known_address = f->src; // last known address of remote neighbor
			// decapsulate and rebuild an eth frame, shall we avoid this step? not for the moment but could be good for efficiency 
			int ns = f->buffer.size() - 7; // false if coming from WDS!
			std::vector<byte> nf(ns, 0x00);
			std::copy(f->dst.address_bytes.begin(), f->dst.address_bytes.end(), nf.begin());
			std::copy(f->src.address_bytes.begin(), f->src.address_bytes.end(), nf.begin() + 6);
			std::copy(f->buffer.begin() + 19, f->buffer.end(), nf.begin() + 12); // false if WDS!
			f->buffer = nf; // copy back in f!
		}
		if ( line_pt->interface_family == interface_nic && line_pt->protocol == protocol_ethernet )
		{
			if ( f->buffer.size() < 14/*64*/ || f->buffer.size() > 1518 )
			{
				//if ( line_pt->vnd_pt->debug ) 
					line_pt->vnd_pt->warning << "\n> [" << print_id() << "] bad length frame, dropping frame\n";
				discard_frame(f);
				return;
			}
			f->dst.address_bytes.assign(f->buffer.begin(), f->buffer.begin() + 6); // dont care about bit/byte order? no
			f->src.address_bytes.assign(f->buffer.begin() + 6, f->buffer.begin() + 12);
			line_pt->last_known_address = f->src; // used for hub, switch and ap devices
		}
		if ( line_pt->interface_family == interface_wic ) // in mobile device, get QEMU MAC @ and store it
		{
			if ( line_pt->vnd_pt->device != device_mobile_adhoc && line_pt->vnd_pt->device != device_mobile_infra )
			{
				line_pt->vnd_pt->error << "> we should be in a mobile device\n";
			}
			if ( line_pt->protocol == protocol_ethernet )
			{
				if ( f->buffer.size() < 14/*64*/ || f->buffer.size() > 1518 )
				{
					//if ( line_pt->vnd_pt->debug ) 
						line_pt->vnd_pt->warning << "\n> [" << print_id() << "] bad length frame, dropping frame\n";
					discard_frame(f);
					return;
				}
				f->dst.address_bytes.assign(f->buffer.begin(), f->buffer.begin() + 6); // dont care about bit/byte order? no
				f->src.address_bytes.assign(f->buffer.begin() + 6, f->buffer.begin() + 12);
				line_pt->vnd_pt->address_id = f->src; // in mobile device, get QEMU MAC @ and store it
			}
		}
		if ( line_pt->vnd_pt->device == device_switch || line_pt->vnd_pt->device == device_access_point )
		{
			if ( f->src.is_null() )
			{
				//if ( line_pt->vnd_pt->debug ) 
					line_pt->vnd_pt->warning << "\n> [" << print_id() << "] undefined src @, bad length frame?, dropping frame\n";
				discard_frame(f);
				return;
			}
			// store src addr of each frame in forwarding table! should be optimized
			//if ( prost::roll(10) ) //vnd_pt->forwarding_table.size() )
			{
				std::map<address, std::string>::iterator itf = line_pt->vnd_pt->forwarding_table.find(f->src);
				if ( itf != line_pt->vnd_pt->forwarding_table.end() ) // @ already exists
				{
					if ( itf->second != line_pt->id ) // not same interface, machine has changed
					{
						line_pt->vnd_pt->forwarding_table.erase(itf); // because c++ std forbids map update
						line_pt->vnd_pt->forwarding_table[f->src] = line_pt->id; // store mapping
					}
				}
				else
					line_pt->vnd_pt->forwarding_table[f->src] = line_pt->id; // store mapping
			}
		}
/*		if ( ( line_pt->vnd_pt->device == device_mobile_infra || line_pt->vnd_pt->device == device_mobile_adhoc )
			&& ( !line_pt->vnd_pt->promiscuous_mode ) 
			&& ( line_pt->interface_family == interface_ral )
			&& ( !f->dst.is_broadcast() )
			&& ( !line_pt->vnd_pt->address_id.is_null() )
			&& ( f->dst != line_pt->vnd_pt->address_id ) )
*/		if ( false )
		{
			line_pt->vnd_pt->warning << "this frame is not destined for this mobile device, dropping it\n";
			discard_frame(f);
			return;
		}
		push(f);
	}

	// called inside output/schedule before write_loop: wrong!!!
	// called inside forward_mobile before forward_hub
	void hold_queue::postprocess(frame * f)
	{
		int ns = f->buffer.size() + 7;
		std::vector<byte> nf(ns, 0x00);
		if ( line_pt->vnd_pt->device == device_mobile_adhoc )
		{
			f->bss = line_pt->vnd_pt->ap_address; // should be 0
			nf[0] = 0x00;
			std::copy(f->dst.address_bytes.begin(), f->dst.address_bytes.end(), nf.begin() + 1);
			std::copy(f->src.address_bytes.begin(), f->src.address_bytes.end(), nf.begin() + 7);
			std::copy(f->bss.address_bytes.begin(), f->bss.address_bytes.end(), nf.begin() + 13);
			std::copy(f->buffer.begin() + 12, f->buffer.end(), nf.begin() + 19);
		}
		if ( line_pt->vnd_pt->device == device_mobile_infra )
		{
			f->bss = line_pt->vnd_pt->ap_address;
			nf[0] = 0x01;
			std::copy(f->bss.address_bytes.begin(), f->bss.address_bytes.end(), nf.begin() + 1);
			std::copy(f->src.address_bytes.begin(), f->src.address_bytes.end(), nf.begin() + 7);
			std::copy(f->dst.address_bytes.begin(), f->dst.address_bytes.end(), nf.begin() + 13);
			std::copy(f->buffer.begin() + 12, f->buffer.end(), nf.begin() + 19);
		}
		if ( line_pt->vnd_pt->device == device_access_point )
		{
			f->bss = line_pt->vnd_pt->address_id;
			nf[0] = 0x02;
			std::copy(f->dst.address_bytes.begin(), f->dst.address_bytes.end(), nf.begin() + 1);
			std::copy(f->bss.address_bytes.begin(), f->bss.address_bytes.end(), nf.begin() + 7);
			std::copy(f->src.address_bytes.begin(), f->src.address_bytes.end(), nf.begin() + 13);
			std::copy(f->buffer.begin() + 12, f->buffer.end(), nf.begin() + 19);
		}
		f->buffer.resize(ns);
		std::copy(nf.begin(), nf.end(), f->buffer.begin());
	}

	void hold_queue::push(frame * f)
	{
		if ( !up )
		{
			//if ( line_pt->vnd_pt->debug ) 
				line_pt->vnd_pt->warning << "\n> [" << print_id() << "] is down, dropping frame\n";
			discard_frame(f);
			return;
		}
		if ( !compute_ber(f) )
		{
			lost_frames_counter++;
			lost_bytes_counter += f->buffer.size();
			//if ( line_pt->vnd_pt->debug ) 
				line_pt->vnd_pt->warning << "\n> [" << print_id() << "] bit error during transmission, dropping frame\n";
			discard_frame(f);
			return;
		}
		if ( byte_count() + f->buffer.size() <= excess_burst )
		{
			std::pair<std::map<time_type, frame *>::iterator, bool> ret; 
			if ( input )
			{
				ret = buffer_queue.insert(std::pair<time_type, frame *>(f->arr_ts, f));
				if ( !ret.second )
				{
					lost_frames_counter++;
					lost_bytes_counter += f->buffer.size();
					line_pt->vnd_pt->warning << "\n> [" << print_id() << "] insertion failed because of the same arrival timestamp, dropping frame\n";
					discard_frame(f);
					return;
				}
				else // we suppose ts > previous frame ts
				{
					frames_counter++;
					bytes_counter += f->buffer.size();
					f->queue_ts_key = f->arr_ts;
					schedule();
					//line_pt->vnd_pt->io_serv->post(boost::bind(&hold_queue::schedule, this)); // should the call be async/post'd?
				}
			}
			else // output
			{
				time_type t = boost::chrono::high_resolution_clock::now();
				ret = buffer_queue.insert(std::pair<time_type, frame *>(t, f));
				if ( !ret.second )
				{
					lost_frames_counter++;
					lost_bytes_counter += f->buffer.size();
					line_pt->vnd_pt->warning << "\n> [" << print_id() << "] insertion failed because of the same internal timestamp, dropping frame\n";
					discard_frame(f);
					return;
				}
				else
				{
					//f->tt2 = boost::chrono::process_user_cpu_clock::now();
					f->queue_ts_key = t; // each "copy" overwrites the previous queue_ts_key! (= arr_ts value for the first one)
					if ( !line_pt->bound_ep )//|| buffer_queue.size() == 1 )
					{
						// remove from insertion
						unsigned int /*std::size_type*/ elrem = buffer_queue.erase(t);
						if ( !elrem )
							line_pt->vnd_pt->error << "frame not erased from queue: impossible\n";
						// print output frame if you want
						lost_frames_counter++;
						lost_bytes_counter += f->buffer.size();
						if ( line_pt->vnd_pt->debug )
							line_pt->vnd_pt->warning << "\n> [" << print_id() << "] line interface not bound to any endpoint, dropping frame\n";
						discard_frame(f);
						return;
					}
					else
					{
						frames_counter++;
						bytes_counter += f->buffer.size();
						//frames_tss.push(t);
						schedule(); // should be called from endpoint? also!
						//line_pt->vnd_pt->io_serv->post(boost::bind(&hold_queue::schedule, this)); // should be async/post'd?
					}
				}
			}
		}
		else // overflow
		{
			lost_frames_counter++;
			lost_bytes_counter += f->buffer.size();
			//if ( line_pt->vnd_pt->debug ) 
				line_pt->vnd_pt->warning << "\n> [" << print_id() << "] queue overflow, dropping frame\n";
			discard_frame(f);
		}
	}

	void hold_queue::schedule()
	{
		if ( timer_running ) // here to avoid calling async_wait on running timer while having already popped up a frame
		{
			return;
		}
		// pb if line_pt is invalid because line is already killed
		if ( line_pt->bound_ep )
		{
/*			if ( input && line_pt->bound_ep->read_overlap )
			{
				line_pt->vnd_pt->warning << "\n> schedule called while read_overlap is true\n";
				return;
			}
*/			if ( !input && line_pt->bound_ep->write_overlap )
			{
				//line_pt->vnd_pt->warning << "\n> schedule called while write_overlap is true\n"; // happens often
				return;
			}
		}
		if ( buffer_queue.size() ) // why not a while? because write_loop may/would be called several times before handle_write returns
		{
			frame * f = buffer_queue.begin()->second;
			buffer_queue.erase(buffer_queue.begin());
			if ( input )
			{
				wait(f);
				//boost::system::error_code ec;
				//line_pt->vnd_pt->forward(ec, f);
				// why forward before inq wait? to compensate from endpt async read? yes
				// now we forward at the end of the wait
				//line_pt->vnd_pt->io_serv->post(boost::bind(&vswitch::forward, line_pt->vnd_pt, ec, f)); // should be async/post'd?
			}
			else // output
			{
				if ( line_pt->bound_ep )
				{
					if ( line_pt->trace_flag || line_pt->trace_file_flag ) // frame traced before waiting the proper time to be written/sent
						line_pt->trace(f, 1);//bytes_transferred); // 1 to indicate output
					wait(f);
				}
				else
				{
					line_pt->vnd_pt->destroy_frame(f);
					line_pt->vnd_pt->warning << "\nline interface (out) not bound to ep, dropping frame\n";
				}
			}
		}
	}

	void hold_queue::reset_stats()
	{
		last_unstat_time = boost::chrono::high_resolution_clock::now();
		activity_duration = fpsec(0);
		frames_counter = 0;
		bytes_counter = 0;
		lost_frames_counter = 0;
		lost_bytes_counter = 0;
	}

	void hold_queue::compute_stats(std::map<std::string, double> * s)
	{
		nsec ns = boost::chrono::high_resolution_clock::now() - last_unstat_time; // solution -1: includes idle periods!
		double d = static_cast<double>(ns.count()) / 1E9;
		activity_duration = fpsec(d);
		double D = fpsec(boost::chrono::high_resolution_clock::now() - epoch_time).count(); // does conversion?
		(*s)["D"] = D;
		(*s)["d"] = d;
		(*s)["F"] = frames_counter;
		(*s)["B"] = bytes_counter;
		(*s)["Fps"] = frames_counter / d;
		(*s)["Bps"] = bytes_counter / d;
		(*s)["lF"] = lost_frames_counter;
		(*s)["lB"] = lost_bytes_counter;
		(*s)["lFps"] = lost_frames_counter / d;
		(*s)["lBps"] = lost_bytes_counter / d;
	}

	void hold_queue::set(std::string k, std::string v)
	{
		if ( k == "up" )
		{
			up = true;
		}
		else if ( k == "down" )
		{
			up = false;
		}
		else if ( k == "vli" )
		{
			if ( v == "u" )
			{
				vli = 0;
			}
			else
			{
				byte svli = static_cast<byte>(strtoul(v.c_str(), 0, 10));
				if ( svli < 0 )
					vli = 0;
				else if ( svli < max_vli )
					vli = svli;
				else
					vli = max_vli;
			}
		}
		else if ( k == "bw" )
		{
			if ( v == "u" )
			{
				bw = default_bw;
			}
			else
			{
				//qword sbw = strtoull(v.c_str(), 0, 10);
				qword sbw = static_cast<qword>(strtod(v.c_str(), 0));
				if ( sbw <= 0 )
					bw = 1; // because we divide by bw
				else if ( sbw < max_bw )
					bw = sbw;
				else
					bw = max_bw;
			}
		}
		else if ( k == "dl" )
		{
			if ( v == "u" )
			{
				dl = 0;
			}
			else
			{
				double sdl = strtod(v.c_str(), 0);
				if ( sdl < 0 )
					dl = 0;
				else if ( sdl < max_dl )
					dl = sdl;
				else
					dl = max_dl;
			}
		}
		else if ( k == "dv" )
		{
			if ( v == "u" )
			{
				dv = 0;
			}
			else
			{
				double sdv = strtod(v.c_str(), 0);
				if ( sdv < 0 )
					dv = 0;
				else if ( sdv < max_dv )
					dv = sdv;
				else
					dv = max_dv;
			}
		}
		else if ( k == "ber" )
		{
			if ( v == "u" )
			{
				ber = 0;
			}
			else
			{
				double sber = strtod(v.c_str(), 0);
				if ( sber < 0 )
					ber = 0;
				else if ( sber < 1 )
					ber = sber;
				else
					ber = 1;
			}
		}
		else if ( k == "qs" )
		{
			if ( v == "u" )
			{
				queue_size = default_queue_size;
			}
			else
			{
				dword sqs = strtoul(v.c_str(), 0, 10);
				if ( sqs < 1 )
				{
					queue_size = 1;
				}
				else if ( sqs < max_queue_size )
				{
					queue_size = sqs;
				}
				else
				{
					queue_size = max_queue_size;
				}
			}
			burst = queue_size * buffer_size; // depends on endpoint in/out buffer size, should they be equal? yes for now
			excess_burst = burst;
		}
		else
		{
			std::cout << "\n> unknown arg (try help)\n";
		}
	}

	void hold_queue::set_buffer(dword bs) // if endpoint modifies in/out buffer size, burst is updated
	{
		buffer_size = bs;
		burst = queue_size * buffer_size;
		excess_burst = burst;
	}

	line_interface::line_interface(boost::asio::io_service * io_serv)
	{
		trace_flag = false;
		trace_format = trace_format_text;		
		trace_file_flag = false;
		trace_file_format = trace_format_text;
		bound_ep = 0;
		input_queue = new hold_queue(io_serv);
		input_queue->input = true;
		input_queue->line_pt = this;
		output_queue = new hold_queue(io_serv);
		output_queue->input = false;
		output_queue->line_pt = this;
	}

	line_interface::~line_interface()
	{
		delete input_queue;
		delete output_queue;
	}

	void line_interface::trace(frame * f, std::size_t bytes_transferred)
	{
		std::ostringstream preamble, postamble;
		std::vector<byte> buffer;
		hold_queue * hq = 0;
		if ( bytes_transferred == 0 ) // input
		{
			if ( bound_ep )
				preamble << "\n[" << util::timef() << ":" << bound_ep->id << "->" << id << "]\n";
			else
				preamble << "\n[" << util::timef() << ":" << "unbound ep" << "->" << id << "]\n";
			buffer = f->buffer;
			hq = input_queue;
		}
		else // output
		{
			if ( bound_ep )
				preamble << "\n[" << util::timef() << ":" << id << "->" << bound_ep->id << "]\n";
			else // is it possible? yes with the msg command
				preamble << "\n[" << util::timef() << ":" << id << "->" << "unbound ep" << "]\n";
			// frame is already destroyed here: not anymore!
			buffer = f->buffer;
			//buffer.resize(bytes_transferred);
			//std::copy(out_buf.begin(), out_buf.begin() + bytes_transferred, buffer.begin()); // out_buf should not have been overwritten by multiple async_send_to calls!
			//out_buf.assign(out_buf_size, 0x00); // necessary?
			hq = output_queue;
		}
		postamble << "\n[" << static_cast<dword>(buffer.size()) << " bytes w/ NL]\n";
		if ( trace_flag )
		{
			if ( trace_format == trace_format_hexa )
				vnd_pt->output << preamble.str() << util::print_hexa(buffer) << postamble.str() << "\n";
			else
				vnd_pt->output << preamble.str() << util::print_ascii(buffer) << postamble.str() << "\n";
		}
		if ( trace_file_flag )
		{
			if ( trace_file_format == trace_format_hexa )
				trace_file << preamble.str() << util::print_hexa(buffer) << postamble.str() << "\n";
			else if ( trace_file_format == trace_format_pcap )
			{
				//std::cout << "system_clock::now() = " << boost::chrono::system_clock::now() << std::endl;
				//std::cout << "system_clock::now().time_since_epoch() = " << (boost::chrono::system_clock::now().time_since_epoch()) << std::endl; // what is the epoch, we dont know
				usec us = boost::chrono::duration_cast<usec>(boost::chrono::high_resolution_clock::now() - hq->epoch_time);
				//std::cout << "us since queue start = " << us << std::endl;
				dword ts_sec = static_cast<dword>(hq->unix_epoch_time + (us.count() / 1000000LL)); // the date and time when this packet was captured. This value is in seconds since January 1, 1970 00:00:00 GMT; this is also known as a UN*X time_t. You can use the ANSI C time() function from time.h to get this value, but you might use a more optimized way to get this timestamp value. If this timestamp isn't based on GMT (UTC), use thiszone from the global header for adjustments.
				//std::cout << "ts_sec = " << ts_sec << std::endl;
				dword ts_usec = static_cast<dword>(us.count() % 1000000LL); // in regular pcap files, the microseconds when this packet was captured, as an offset to ts_sec. In nanosecond-resolution files, this is, instead, the nanoseconds when the packet was captured, as an offset to ts_sec /!\ Beware: this value shouldn't reach 1 second (in regular pcap files 1 000 000; in nanosecond-resolution files, 1 000 000 000); in this case ts_sec must be increased instead! 
				//std::cout << "ts_usec = " << ts_usec << std::endl;
				dword incl_len = buffer.size(); // number of octets of packet saved in file
				dword orig_len = buffer.size(); // actual length of packet
				std::vector<byte> header; // 16 bytes header
				util::dword_to_bytes(ts_sec, header); // guint32 ts_sec; // timestamp seconds
				util::dword_to_bytes(ts_usec, header); // guint32 ts_usec; // timestamp microseconds
				util::dword_to_bytes(incl_len, header); // guint32 incl_len; // number of octets of packet saved in file
				util::dword_to_bytes(orig_len, header); // guint32 orig_len; // actual length of packet
				trace_file.write(util::print_ascii(header).c_str(), header.size()); // pb with unsigned?				
				trace_file.write(util::print_ascii(buffer).c_str(), buffer.size()); // pb with unsigned?
			}
			else
				trace_file << preamble.str() << util::print_ascii(buffer) << postamble.str() << std::endl;
		}
	}

	void line_interface::kill()
	{
		if ( bound_ep )
		{
			bound_ep->bound_if = 0;
		}
		// remove from linking table
		vnd_pt->linking_table.erase(id); // first, unidirectional, all entries
		std::multimap<std::string, std::string>::iterator itt = vnd_pt->linking_table.begin();
		while ( itt != vnd_pt->linking_table.end() )
		{
			if ( itt->second == id )
			{
				vnd_pt->linking_table.erase(itt); // second, itt invalidated
				itt = vnd_pt->linking_table.begin();
				continue;
			}
			itt++;
		}
		// remove from forwarding table
		std::map<address, std::string>::iterator itf = vnd_pt->forwarding_table.begin();
		while ( itf != vnd_pt->forwarding_table.end() )
		{
			if ( itf->second == id )
			{
				vnd_pt->forwarding_table.erase(itf);
				break; // works if only one out if in table
			}
			itf++;
		}
		// remove from binding table
		if ( vnd_pt->line_interfaces[id]->bound_ep )
		{
			//binding_table.erase(line_interfaces[argus[1]]->bound_ep->id);
			vnd_pt->line_interfaces[id]->bound_ep->bound_if = 0; // same as above
		}
		// remove from line interfaces list
		vnd_pt->line_interfaces.erase(id);
		if ( medeb::active ) medeb::delm(this);
		delete this;
	}
}
