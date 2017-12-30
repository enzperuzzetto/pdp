
// virtual network device (vnd)
// Copyright (C) 2012-2016 Damien Magoni

#include "vnd.hpp"
#ifdef VLIN
#include <readline/readline.h>
#include <readline/history.h>
#endif

namespace vnd
{
	network_device::network_device(std::map<std::string, std::string> arg) : 
		error(arg["-d"] + arg["-n"] + "_error.log", mesap::console|mesap::logging, false),
		warning(arg["-d"] + arg["-n"] + "_warning.log", mesap::console|mesap::logging, false),
		output(arg["-d"] + arg["-n"] + "_output.log", mesap::console, false)
	{
		args = arg;
		name = arg["-n"];
		port_seed = static_cast<word>(atol(arg["-s"].c_str()));
		device = device_hub;
		debug = false;
		tcp_base_source_port = port_seed * 100;
		udp_base_source_port = port_seed * 100;
		tcp_resolver = 0;
		udp_resolver = 0;
		stran = 0;
		promiscuous_mode = false;
		address_id.address_bytes.assign(6, 0x00);
		ap_address.address_bytes.assign(6, 0x00); // !!!
	}

	network_device::~network_device()
	{
		if ( tcp_resolver )
			delete tcp_resolver;
		if ( udp_resolver )
			delete udp_resolver;
		if ( stran )
			delete stran;
	}

	frame * network_device::create_frame()
	{
		frame * f = new frame;
		if ( medeb::active ) medeb::newm(f, sizeof(frame),__FILE__, __LINE__);
		f->arr_ts = boost::chrono::high_resolution_clock::now();
		f->ref_count = 1; // the first frame
/*
		time_type arr_ts; // arrival (at the endpoint) timestamp
		time_type queue_ts_key; // pushed in queue timestamp
*/
		return f;
	}

	frame * network_device::copy_frame(frame * f)
	{
		frame * nf = new frame;
		if ( medeb::active ) medeb::newm(nf, sizeof(frame),__FILE__, __LINE__);
		*nf = *f; // copy
		nf->ref_count = 1; // the first copied frame
		return nf;
	}

	void network_device::destroy_frame(frame * f)
	{
		f->ref_count--;
		if ( f->ref_count < 1 )
		{
			if ( medeb::active ) medeb::delm(f);
			delete f;
			f = 0;
		}
	}

	void network_device::set(std::string k, std::string v)
	{
		if ( k == "mode" )
		{
			if ( v == "link" )
			{
				device = device_link;
			}
			else if ( v == "hub" )
			{
				device = device_hub;
			}
			else if ( v == "switch" )
			{
				device = device_switch;
			}
			else if ( v == "access-point" )
			{
				device = device_access_point;
				byte b4 = static_cast<byte>(prost::random(0, 255));
				byte b5 = static_cast<byte>(prost::random(0, 255));
				byte b6 = static_cast<byte>(prost::random(0, 255));
				std::vector<byte> maca;
				maca.push_back(0xAC);
				maca.push_back(0xCE);
				maca.push_back(0x55);
				maca.push_back(b4);
				maca.push_back(b5);
				maca.push_back(b6);
				address_id.address_bytes = maca;
				ap_address.address_bytes = maca; // same as above
			}
			else if ( v == "mob-infra" )
			{
				device = device_mobile_infra;
				std::vector<byte> maca;
				maca.assign(6, 0x00);
				address_id.address_bytes = maca; // MAC @ will be obtained from QEMU outgoing frames
				ap_address.address_bytes = maca; // we do not know any AP yet
			}
			else if ( v == "mob-adhoc" )
			{
				device = device_mobile_adhoc;
				std::vector<byte> maca;
				maca.assign(6, 0x00);
				address_id.address_bytes = maca; // MAC @ will be obtained from QEMU outgoing frames
				ap_address.address_bytes = maca; // we will never know any AP and leave this to 0
			}
			else
				device = device_hub;
		}
		if ( k == "debug" )
		{
			if ( v == "on" )
			{
				debug = true;
			}
			else
			{
				debug = false;
			}
		}
/*		if ( k == "hexa" )
		{
			if ( v == "on" )
			{
				hexa = true;
			}
			else
			{
				hexa = false;
			}
		}
*/	}

	void network_device::forward_link(frame * f)
	{
		itlt = linking_table.begin();
		retset = linking_table.equal_range(f->input_interface);
		int fc = 0;
		for ( itlt = retset.first; itlt != retset.second; ++itlt )
		{
			if ( itlt->second != f->input_interface ) // loop check
			{
				fc++;
				f->ref_count = fc;
				io_serv->post(boost::bind(&hold_queue::push, line_interfaces[itlt->second]->output_queue, f)); // avoid blocking calls
				//line_interfaces[itt->second]->output_queue->push(f);
			}
			else
				warning << "> loop detected in linking table\n";
		}
		if ( !fc )
		{
			destroy_frame(f); // if post is never called do we have to destroy the frame here? yes
		}
	}

	void network_device::forward_hub(frame * f, interface_type ifty)
	{
		int fc = 0;
		itli = line_interfaces.begin();
		while ( itli != line_interfaces.end() )
		{
			if ( ifty == itli->second->interface_family ) // interface type check
			{
				if ( f->vli == itli->second->output_queue->vli ) // VLAN check
				{
					if ( f->input_interface != itli->first ) // loop check
					{
						fc++;
						f->ref_count = fc;
						io_serv->post(boost::bind(&hold_queue::push, itli->second->output_queue, f));
						//itl->second->output_queue->push(f);
					}
				}
			}
			itli++;
		}
		if ( !fc )
		{
			destroy_frame(f); // if post is never called do we have to destroy the frame here? yes
		}
	}
	
	void network_device::forward_switch(frame * f, interface_type ifty)
	{
		if ( !f->dst.is_broadcast() && forwarding_table.size() )
		{
			itft = forwarding_table.find(f->dst);
			if (  itft != forwarding_table.end() ) // MAC dst found
			{
				if ( ifty == line_interfaces[itft->second]->interface_family ) // interface type check
				{
					if ( f->vli == line_interfaces[itft->second]->output_queue->vli ) // VLAN check
					{
						if ( f->input_interface != itft->second ) // loop check
						{
							io_serv->post(boost::bind(&hold_queue::push, line_interfaces[itft->second]->output_queue, f));
							//line_interfaces[itf->second]->output_queue->push(f);
						}
						else
						{
							warning << "> loop detected in forwarding table\n";
							destroy_frame(f);
						}
					}
					else
					{
						//warning << "> output port does not belong to the same VLAN\n";
						destroy_frame(f);
					}
				}
				else
				{
					destroy_frame(f);
				}
				return;
			}
		}
		forward_hub(f, ifty);
	}

	void network_device::forward_access_point(frame * f)
	{
		if ( line_interfaces[f->input_interface]->interface_family == interface_ral ) // coming from wireless net
		{
			if ( !(!f->from_ds && f->to_ds) )
			{
				error << "wireless frame in AP has wrong DS flags\n";
				destroy_frame(f);
				return;
			}
			if ( !f->bss.is_null() && f->bss != address_id ) // frame not for me, but what if AP not known yet?
			{
				warning << "this frame must go through a different AP\n";
				destroy_frame(f);
				return;
			}
		}
		else if ( line_interfaces[f->input_interface]->interface_family == interface_nic ) // coming from wired net
		{
			;
		}
		else
		{
			error << "wrong interface type for this frame type and this device type\n";
			destroy_frame(f);
			return;
		}
		if ( !f->dst.is_broadcast() && forwarding_table.size() )
		{
			itft = forwarding_table.find(f->dst);
			if ( itft != forwarding_table.end() ) // known dst
			{
				if ( line_interfaces[itft->second]->interface_family == interface_ral ) // dst on wireless net, will be bcast'd to all rals although it's useless except for promiscuous
				{
					line_interfaces[f->input_interface]->input_queue->postprocess(f);
					forward_hub(f, interface_ral);
				}
				else if ( line_interfaces[itft->second]->interface_family == interface_nic ) // dst on wired net
				{
					forward_switch(f, interface_nic);
				}
				else
				{
					error << "wrong interface type for this device type\n";
				}
				return;
			}
		}
		forward_hub(f, interface_nic); // unknown dst
		frame * nf = copy_frame(f);
		line_interfaces[nf->input_interface]->input_queue->postprocess(nf); // will this work?? probably not, lets make a copy first 
		forward_hub(nf, interface_ral); // unknown dst
	}

	void network_device::forward_mobile(frame * f)
	{
		if ( line_interfaces[f->input_interface]->interface_family == interface_ral ) // coming from wireless net
		{
			if ( device == device_mobile_infra )
			{
				if ( !(f->from_ds && !f->to_ds) )
				{
					error << "wireless frame in mobile infra has wrong DS flags\n";
					destroy_frame(f);
					return;
				}
			}
			if ( device == device_mobile_adhoc )
			{
				if ( f->from_ds || f->to_ds )
				{
					error << "wireless frame in mobile adhoc has wrong DS flags\n";
					destroy_frame(f);
					return;
				}
			}
			forward_hub(f, interface_wic);
		}
		else if ( line_interfaces[f->input_interface]->interface_family == interface_wic ) // coming from mobile device
		{
			line_interfaces[f->input_interface]->input_queue->postprocess(f);
			forward_hub(f, interface_ral);
		}
		else
		{
			error << "wrong interface type for this frame type and this device type\n";
			destroy_frame(f);
			return;
		}
	}

	void network_device::forward(/*const boost::system::error_code & ec,*/ frame * f)
	{
		/*if ( ec ) // currently unused
		{
			error << "\nerror in forward (timer issue if called by async_wait): " << ec.value() << " = " << ec.message() << "\n";
			destroy_frame(f);
			return;
		}*/
		if ( device == device_link )
			forward_link(f);
		else if ( device == device_hub )
			forward_hub(f, interface_nic);
		else if ( device == device_switch )
			forward_switch(f, interface_nic);
		else if ( device == device_access_point )
			forward_access_point(f);
		else if ( device == device_mobile_infra )
			forward_mobile(f);
		else if ( device == device_mobile_adhoc )
			forward_mobile(f);
		else
		{
			output << "> unknown device emulation\n";
		}
	}

	void network_device::load(std::string s)
	{
		if ( s.empty() )
			return;
		std::ifstream f(s.c_str());
	    if ( !f.is_open() )// ( !f )//f.bad() ) // doesn't work
		{
			//perror(("error while opening file " + s).c_str());
			error << "failed to open configuration file " << s << "\n";
			return;
		}
		std::string l;
		while ( std::getline(f, l) )//!f.eof() ) // could miss errors
		{
			if ( !l.find("#") || !l.size() || !l.find("exit") )//== std::string::npos ) // exit must be at beginning of line!
				continue;
			if ( debug )
				output << "> executing: " << l << "...\n";
			stran->post(boost::bind(&network_device::command, this, l)); // shall we post the command? is the post order preserved?
		}
		if ( f.bad() )
		{
			//perror(("error while reading file " + s).c_str());
			error << "error while reading the configuration file " << s << "\n";
		}
		f.close();
	}

	void network_device::prompt()
	{
		output << name << "% ";
	}

	void network_device::run()
	{
		try
		{
			tcp_resolver = new boost::asio::ip::tcp::resolver(*io_serv);
			udp_resolver = new boost::asio::ip::udp::resolver(*io_serv);
		}
		catch (std::exception & e)
		{
			warning << "> exception in run : " << e.what() << "\n% ";
		}
		std::auto_ptr<boost::asio::io_service::work> work(new boost::asio::io_service::work(*io_serv)); // work = new...?
		boost::thread t(boost::bind(&boost::asio::io_service::run, io_serv));
		if ( args.find("-f") != args.end() && args["-f"] != "" )
			load(args["-f"]);
#ifdef VLIN
		::rl_bind_key('\t', ::rl_complete); // Configure readline to auto-complete paths when the tab key is hit
#endif
		while ( true )
		{
			stran->post(boost::bind(&network_device::prompt, this));
			std::string cmd;
#ifdef VLIN
			//char shell_prompt[128];
			//snprintf(shell_prompt, sizeof(shell_prompt), "%s:%s $ ", getenv("USER"), getcwd(NULL, 1024)); 
			char * input = ::readline("");//shell_prompt);
			if ( !input ) // Check for EOF
				break;
			if ( input && *input )
				::add_history(input);
			cmd.assign(input);
			free(input);
#else
			std::getline(std::cin, cmd); // blocking call / main thread
#endif
			//if ( debug ) output << "> you typed: " << cmd << "\n";//std::endl;
			if ( cmd.find("exit") != std::string::npos )
			{
				break;
			}
			else if ( cmd.find("load") != std::string::npos )
			{
				if ( cmd.size() > 5 )
					load(cmd.substr(cmd.find("load") + 5));
			}
			else
			{
				stran->post(boost::bind(&network_device::command, this, cmd));
				// shall we post the command? is the ordering preserved? if only one thread is used, it should be
			}
		}
		output << "> vnd exiting...\n";
		std::map<std::string, end_point *>::iterator itm = end_points.begin();
		while ( itm != end_points.end() )
		{
			stran->post(boost::bind(&end_point::disc, itm->second)); // stran may not be necessary
			itm++;
		}
		work.reset(); // finish async tasks
		t.join();
		while ( line_interfaces.size() )
		{
			line_interfaces.begin()->second->kill();
		}
	}

	void network_device::add_interface(std::vector<std::string> & argus)
	{
		if ( argus.size() < 2 )
		{
			output << "> missing args\n";
			return;
		}
		if ( line_interfaces.find(argus[1]) == line_interfaces.end() )
		{
			line_interface * li = new line_interface(io_serv);
			if ( medeb::active ) medeb::newm(li, sizeof(line_interface),__FILE__, __LINE__);
			li->vnd_pt = this;
			li->id = argus[1];
			line_interfaces[li->id] = li;
			if ( argus.size() == 2 )
			{
				li->interface_family = interface_nic;
				li->protocol = protocol_ethernet;
				output << "> interface type set to nic, proto set to eth\n";
			}
			if ( argus.size() > 2 )
			{
				if ( argus[2] == "nic" ) // should only be created on a fixed node
				{
					li->interface_family = interface_nic;
					li->protocol = protocol_ethernet;
				}
				else if ( argus[2] == "wic" ) // should only be created to attach a mobile node
				{
					li->interface_family = interface_wic;
					li->protocol = protocol_ethernet;
				}
				else if ( argus[2] == "ral" )
				{
					li->interface_family = interface_ral;
					// what if the mode is changed after the interface creation? arg!
					li->protocol = protocol_pseudo_802_11;
				}
				else
				{
					li->interface_family = interface_nic;
					li->protocol = protocol_ethernet;
					output << "> unknown interface type\n";
					output << "> interface type set to nic, proto set to eth\n";
				}
				if ( argus.size() > 3 )
				{
					if ( argus[3] == "raw" )
						li->protocol = protocol_none;
					else if ( argus[3] == "eth" )
						li->protocol = protocol_ethernet;
					else if ( argus[3] == "p11" )
						li->protocol = protocol_pseudo_802_11;
					else
						;
				}
			}
			output << "> interface " << li->id << " added\n";
		}
		else
		{
			output << "> interface already exists\n";
		}
	}

	void network_device::remv_interface(std::vector<std::string> & argus)
	{
		if ( argus.size() < 2 )
			return;
		if ( argus[1] != "all" && line_interfaces.find(argus[1]) == line_interfaces.end() )
		{
			output << "> interface does not exist\n";
			return;
		}
		std::map<std::string, line_interface *>::iterator it = line_interfaces.begin();
		while ( it != line_interfaces.end() )
		{
			if ( argus[1] != "all" && argus[1] != it->first )
			{
				it++;
				continue;
			}
			output << "> interface " << it->first << " removed\n";
			it->second->kill(); // invalidates "it"!
			it = line_interfaces.begin();
			//it++;
		}
	}

	void network_device::up_interface(std::vector<std::string> & argus)
	{
		if ( argus.size() == 1 )
			argus.push_back("all");
		if ( argus[1] != "all" && line_interfaces.find(argus[1]) == line_interfaces.end() )
		{
			output << "> interface does not exist\n";
			return;
		}
		if ( argus.size() == 2 )
			argus.push_back("all");
		if ( argus[2] != "in" && argus[2] != "out" && argus[2] != "all" )
		{
			output << "> missing {in|out|all}\n";
			return;
		}
		std::map<std::string, line_interface *>::iterator it = line_interfaces.begin();
		while ( it != line_interfaces.end() )
		{
			if ( argus[1] != "all" && argus[1] != it->first )
			{
				it++;
				continue;
			}
			if ( argus[2] == "in" || argus[2] == "all" )
			{
				//io_serv->post(boost::bind(&hold_queue::set, line_interfaces[argus[1]]->input_queue, "up", ""));
				line_interfaces[argus[1]]->input_queue->set("up", "");
				output << "> interface " << argus[1] << " input queue up\n";
			}
			if ( argus[2] == "out" || argus[2] == "all" )
			{
				//io_serv->post(boost::bind(&hold_queue::set, line_interfaces[argus[1]]->output_queue, "up", ""));
				line_interfaces[argus[1]]->output_queue->set("up", "");
				output << "> interface " << argus[1] << " output queue up\n";
			}
			it++;
		}
	}

	void network_device::down_interface(std::vector<std::string> & argus)
	{
		if ( argus.size() == 1 )
			argus.push_back("all");
		if ( argus[1] != "all" && line_interfaces.find(argus[1]) == line_interfaces.end() )
		{
			output << "> interface does not exist\n";
			return;
		}
		if ( argus.size() == 2 )
			argus.push_back("all");
		if ( argus[2] != "in" && argus[2] != "out" && argus[2] != "all" )
		{
			output << "> missing {in|out|all}\n";
			return;
		}
		std::map<std::string, line_interface *>::iterator it = line_interfaces.begin();
		while ( it != line_interfaces.end() )
		{
			if ( argus[1] != "all" && argus[1] != it->first )
			{
				it++;
				continue;
			}
			if ( argus[2] == "in" || argus[2] == "all" )
			{
				//io_serv->post(boost::bind(&hold_queue::set, line_interfaces[argus[1]]->input_queue, "down", ""));
				line_interfaces[argus[1]]->input_queue->set("down", "");
				output << "> interface " << argus[1] << " input queue down\n";
			}
			if ( argus[2] == "out" || argus[2] == "all" )
			{
				//io_serv->post(boost::bind(&hold_queue::set, line_interfaces[argus[1]]->output_queue, "down", ""));
				line_interfaces[argus[1]]->output_queue->set("down", "");
				output << "> interface " << argus[1] << " output queue down\n";
			}
			it++;
		}
	}

	void network_device::set_interface(std::vector<std::string> & argus)
	{
		if ( argus[1] != "all" && line_interfaces.find(argus[1]) == line_interfaces.end() )
		{
			output << "> interface does not exist\n";
			return;
		}
		while ( argus.size() > 4 && argus.size() % 2 == 1 )
		{
			std::map<std::string, line_interface *>::iterator it = line_interfaces.begin();
			while ( it != line_interfaces.end() )
			{
				if ( argus[1] != "all" && argus[1] != it->first )
				{
					it++;
					continue;
				}
				if ( argus[2] == "in" || argus[2] == "all" )
				{
					it->second->input_queue->set(argus[3], argus[4]);
				}
				if ( argus[2] == "out" || argus[2] == "all" )
				{
					it->second->output_queue->set(argus[3], argus[4]);
				}
				if ( argus[2] != "in" && argus[2] != "out" && argus[2] != "all" )
				{
					output << "> missing {in|out|all}\n";
					return;
				}
				it++;
			}
			argus.pop_back();
			argus.pop_back();
		}
	}

	void network_device::unset_interface(std::vector<std::string> & argus)
	{
		if ( argus[1] != "all" && line_interfaces.find(argus[1]) == line_interfaces.end() )
		{
			output << "> interface does not exist\n";
			return;
		}
		while ( argus.size() > 3 )
		{
			std::map<std::string, line_interface *>::iterator it = line_interfaces.begin();
			while ( it != line_interfaces.end() )
			{
				if ( argus[1] != "all" && argus[1] != it->first )
				{
					it++;
					continue;
				}
				if ( argus[2] == "in" || argus[2] == "all" )
				{
					it->second->input_queue->set(argus[3], "u");
				}
				if ( argus[2] == "out" || argus[2] == "all" )
				{
					it->second->output_queue->set(argus[3], "u");
				}
				if ( argus[2] != "in" && argus[2] != "out" && argus[2] != "all" )
				{
					output << "> missing {in|out|all}\n";
					return;
				}
				it++;
			}
			argus.pop_back(); // no values for keys
		}
	}

	void network_device::tie_interfaces(std::vector<std::string> & argus) // if1 -> if2 unidirectional
	{
		if ( argus.size() != 3 )
		{
			output << "> wrong nb of args\n";
			return;
		}
		if ( line_interfaces.find(argus[1]) == line_interfaces.end() )
		{
			output << "> interface " << argus[1] << " does not exist\n";
			return;
		}
		if ( line_interfaces.find(argus[2]) == line_interfaces.end() )
		{
			output << "> interface " << argus[2] << " does not exist\n";
			return;
		}
		std::multimap<std::string, std::string>::iterator it = linking_table.lower_bound(argus[1]);
		while ( it != linking_table.upper_bound(argus[1]) )
		{
			if ( it->second == argus[2] ) // link already exists
			{
				output << "> tie already exists\n";
				return;
			}
			it++;
		}
		linking_table.insert(std::pair<std::string, std::string>(argus[1], argus[2]));
		output << "> tied " << argus[1] << " to " << argus[2] << "\n";
		return;
	}

	void network_device::untie_interfaces(std::vector<std::string> & argus)
	{
		if ( argus.size() != 3 )
		{
			output << "> wrong nb of args\n";
			return;
		}
		if ( line_interfaces.find(argus[1]) == line_interfaces.end() )
		{
			output << "> interface " << argus[1] << " does not exist\n";
			return;
		}
		if ( line_interfaces.find(argus[2]) == line_interfaces.end() )
		{
			output << "> interface " << argus[2] << " does not exist\n";
			return;
		}
		std::multimap<std::string, std::string>::iterator it = linking_table.lower_bound(argus[1]);
		while ( it != linking_table.upper_bound(argus[1]) )
		{
			if ( it->second == argus[2] ) // link exists
			{
				linking_table.erase(it);
				output << "> untied " << argus[1] << " from " << argus[2] << "\n";
				return;
			}
			it++;
		}
		output << "> tie from " << argus[1] << " to " << argus[2] << " does not exist\n";
		return;
	}

	encapsulation_type network_device::get_encapsulation(std::string mode)
	{
		if ( mode == "len" )
			return encapsulation_length;
		return encapsulation_none;
	}

	std::string network_device::print_encapsulation(encapsulation_type mode)
	{
		if ( mode == encapsulation_length )
			return "len";
		return "raw";
	}

	// incn <ep> {raw|len} {tcp|udp} {<laddr>|*} {<lport>|*} <raddr> <rport>
	void network_device::in_conn_endpoint(std::vector<std::string> & argus)
	{
		if ( end_points.find(argus[1]) == end_points.end() )
		{
			end_point * ep = 0;
			try
			{
				ep = new end_point;
				if ( medeb::active ) medeb::newm(ep, sizeof(end_point),__FILE__, __LINE__);
				ep->vnd_pt = this;
				ep->id = argus[1];
				ep->encapsulation = get_encapsulation(argus[2]);
				ep->tran_proto = argus[3];
				ep->local_net_addr = argus[4];
				ep->local_port_nb = argus[5];
				ep->remote_net_addr = argus[6];
				ep->remote_port_nb = argus[7];
				end_points[ep->id] = ep;
				ep->conn();
				output << "> endpt " << ep->id << " connected\n";
			}
			catch (std::exception & e)
			{
				error << "> exception in command: " <<  e.what() << "\n";
				if ( ep ) 
				{
					end_points.erase(ep->id);
					if ( medeb::active ) medeb::delm(ep);
					delete ep;
				}
			}
		}
		else
		{
			output << "> endpt already exists\n";
		}
	}

	// insr <ep> {raw|len} {tcp|udp} {<laddr>|*} {<lport>|*} {<raddr>|?} {<rport>|?}
	void network_device::in_serv_endpoint(std::vector<std::string> & argus)
	{
		if ( end_points.find(argus[1]) == end_points.end() )
		{
			end_point * ep = 0;
			try
			{
				ep = new end_point;
				if ( medeb::active ) medeb::newm(ep, sizeof(end_point),__FILE__, __LINE__);
				ep->vnd_pt = this;
				ep->server = true;
				ep->id = argus[1];
				ep->encapsulation = get_encapsulation(argus[2]);
				ep->tran_proto = argus[3];
				ep->local_net_addr = argus[4];
				ep->local_port_nb = argus[5];
				ep->remote_net_addr = argus[6];
				ep->remote_port_nb = argus[7];
				end_points[ep->id] = ep;
				ep->serv();
				output << "> endpt " << ep->id << " listening\n";
			}
			catch (std::exception & e)
			{
				error << "> exception in command: " <<  e.what() << "\n";
				if ( ep ) 
				{
					end_points.erase(ep->id);
					if ( medeb::active ) medeb::delm(ep);
					delete ep;
				}
			}
		}
		else
		{
			output << "> endpt already exists\n";
		}
	}

	// uncn <ep> {raw|len} {stm|dgm} <socket_pathname>
	void network_device::un_conn_endpoint(std::vector<std::string> & argus)
	{
		if ( end_points.find(argus[1]) == end_points.end() )
		{
			end_point * ep = 0;
			try
			{
				ep = new end_point;
				if ( medeb::active ) medeb::newm(ep, sizeof(end_point),__FILE__, __LINE__);
				ep->vnd_pt = this;
				ep->id = argus[1];
				ep->encapsulation = get_encapsulation(argus[2]);
				ep->tran_proto = argus[3];
				ep->path_name = argus[4];
				end_points[ep->id] = ep;
				ep->conn();
				output << "> endpt " << ep->id << " connected\n";
			}
			catch (std::exception & e)
			{
				error << "> exception in command: " <<  e.what() << "\n";
				if ( ep ) 
				{
					end_points.erase(ep->id);
					if ( medeb::active ) medeb::delm(ep);
					delete ep;
				}
			}
		}
		else
		{
			output << "> endpt already exists\n";
		}
	}

	// unsr <ep> {raw|len} {stm|dgm} <socket_pathname>
	void network_device::un_serv_endpoint(std::vector<std::string> & argus)
	{
		if ( end_points.find(argus[1]) == end_points.end() )
		{
			end_point * ep = 0;
			try
			{
				ep = new end_point;
				if ( medeb::active ) medeb::newm(ep, sizeof(end_point),__FILE__, __LINE__);
				ep->vnd_pt = this;
				ep->server = true;
				ep->id = argus[1];
				ep->encapsulation = get_encapsulation(argus[2]);
				ep->tran_proto = argus[3];
				ep->path_name = argus[4];
				end_points[ep->id] = ep;
				ep->serv();
				output << "> endpt " << ep->id << " listening\n";
			}
			catch (std::exception & e)
			{
				error << "> exception in command: " <<  e.what() << "\n";
				if ( ep ) 
				{
					end_points.erase(ep->id);
					if ( medeb::active ) medeb::delm(ep);
					delete ep;
				}
			}
		}
		else
		{
			output << "> endpt already exists\n";
		}
	}

	void network_device::disc_endpoint(std::vector<std::string> & argus)
	{
		if ( argus.size() < 2 )
			return;
		if ( argus[1] != "all" && end_points.find(argus[1]) == end_points.end() )
		{
			output << "> endpoint does not exist\n";
			return;
		}
		std::map<std::string, end_point *>::iterator it = end_points.begin();
		while ( it != end_points.end() )
		{
			if ( argus[1] != "all" && argus[1] != it->first )
			{
				it++;
				continue;
			}
			it->second->disc(); // already posted by command
			//io_serv->post(boost::bind(&end_point::disc, end_points[argus[1]]));
			//stran->post(boost::bind(&end_point::disc, it->second)); // stran may not be necessary
			output << "\n> endpoint " << it->first << " ordered to close\n"; // should be ok as erase is done in callback
			it++;
		}
	}

	// tap <ep> <name> <user>
	void network_device::tap_endpoint(std::vector<std::string> & argus)
	{
		if ( end_points.find(argus[1]) == end_points.end() )
		{
			end_point * ep = 0;
			try
			{
				ep = new end_point;
				if ( medeb::active ) medeb::newm(ep, sizeof(end_point),__FILE__, __LINE__);
				ep->vnd_pt = this;
				ep->encapsulation = encapsulation_none;
				ep->id = argus[1];
				ep->tap_name = argus[2];
				//ep->tap_user = argus[3];
				ep->tran_proto = "tap";
				end_points[ep->id] = ep;
				ep->tap();
				output << "> endpt " << ep->id << " connected\n";
			}
			catch (...)//std::exception & e)
			{
				error << "> exception in tap_endpoint: \n";// <<  e.what() << "\n";
				if ( ep ) 
				{
					end_points.erase(ep->id);
					if ( medeb::active ) medeb::delm(ep);
					delete ep;
				}
			}
		}
		else
		{
			output << "> endpt already exists\n";
		}
	}

	// vde <ep> <switch> <port> <group> <mode>
	void network_device::vde_endpoint(std::vector<std::string> & argus)
	{
		if ( end_points.find(argus[1]) == end_points.end() )
		{
			end_point * ep = 0;
			try
			{
				ep = new end_point;
				if ( medeb::active ) medeb::newm(ep, sizeof(end_point),__FILE__, __LINE__);
				ep->vnd_pt = this;
				ep->encapsulation = encapsulation_none;
				ep->id = argus[1];
				ep->vde_switch = argus[2];
				if ( argus.size() > 3 )
					ep->vde_port = argus[3];
				if ( argus.size() > 4 )
					ep->vde_group = argus[4];
				if ( argus.size() > 5 )
					ep->vde_mode = argus[5];
				ep->tran_proto = "vde";
				end_points[ep->id] = ep;
				ep->vde();
				output << "> endpt " << ep->id << " connected\n";
			}
			catch (std::exception & e)
			{
				error << "> exception in vde_endpoint: " <<  e.what() << "\n";
				if ( ep ) 
				{
					end_points.erase(ep->id);
					if ( medeb::active ) medeb::delm(ep);
					delete ep;
				}
			}
		}
		else
		{
			output << "> endpt already exists\n";
		}
	}

	void network_device::par_endpoint(std::vector<std::string> & argus)
	{
		if ( argus[1] != "all" && end_points.find(argus[1]) == end_points.end() )
		{
			output << "> endpoint does not exist\n";
			return;
		}
		std::map<std::string, end_point *>::iterator it = end_points.begin();
		while ( it != end_points.end() )
		{
			if ( argus[1] != "all" && argus[1] != it->first )
			{
				it++;
				continue;
			}
			while ( argus.size() > 4 )
			{
				if ( argus[2] == "in" || argus[2] == "all" )
				{
					//io_serv->post(boost::bind(&end_point::set, end_points[argus[1]], argus[3]+"in", argus[4]));
					end_points[argus[1]]->set(argus[3]+"in", argus[4]);
				}
				if ( argus[2] == "out" || argus[2] == "all" )
				{
					//io_serv->post(boost::bind(&end_point::set, end_points[argus[1]], argus[3]+"out", argus[4]));
					end_points[argus[1]]->set(argus[3]+"out", argus[4]);
				}
				if ( argus[2] != "in" && argus[2] != "out" && argus[2] != "all" )
				{
					output << "> missing {in|out|all}\n";
					return;
				}
				argus.pop_back();
				argus.pop_back();
			}
			it++;
		}
	}

	void network_device::unpar_endpoint(std::vector<std::string> & argus)
	{
		if ( argus[1] != "all" && end_points.find(argus[1]) == end_points.end() )
		{
			output << "> endpoint does not exist\n";
			return;
		}
		std::map<std::string, end_point *>::iterator it = end_points.begin();
		while ( it != end_points.end() )
		{
			if ( argus[1] != "all" && argus[1] != it->first )
			{
				it++;
				continue;
			}
			while ( argus.size() > 3 )
			{
				if ( argus[2] == "in" || argus[2] == "all" )
				{
					//io_serv->post(boost::bind(&end_point::set, end_points[argus[1]], argus[3]+"in", "u"));
					end_points[argus[1]]->set(argus[3]+"in", "u");
				}
				if ( argus[2] == "out" || argus[2] == "all" )
				{
					//io_serv->post(boost::bind(&end_point::set, end_points[argus[1]], argus[3]+"out", "u"));
					end_points[argus[1]]->set(argus[3]+"out", "u");
				}
				if ( argus[2] != "in" && argus[2] != "out" && argus[2] != "all" )
				{
					output << "> missing {in|out|all}\n";
					return;
				}
				argus.pop_back(); // no values for keys
			}
			it++;
		}
	}

	void network_device::bind_endpoint(std::vector<std::string> & argus)
	{
		if ( end_points.find(argus[1]) != end_points.end() )
		{
			if ( line_interfaces.find(argus[2]) != line_interfaces.end() )
			{
				//binding_table[argus[1]] = argus[2];
				end_points[argus[1]]->bound_if = line_interfaces[argus[2]]; // bijection
				line_interfaces[argus[2]]->bound_ep = end_points[argus[1]];
				output << "> bound " << argus[1] << " to " << argus[2] << "\n";
			}
			else
			{
				output << "> intf does not exist\n";
			}
		}
		else
		{
			output << "> endpt does not exist\n";
		}
	}

	void network_device::unbind_endpoint(std::vector<std::string> & argus)
	{
		if ( end_points.find(argus[1]) != end_points.end() )
		{
			if ( line_interfaces.find(argus[2]) != line_interfaces.end() )
			{
				//binding_table.erase(argus[1]);
				end_points[argus[1]]->bound_if = 0;
				line_interfaces[argus[2]]->bound_ep = 0;
				output << "> unbound " << argus[1] << " from " << argus[2] << "\n";
			}
			else
			{
				output << "> intf does not exist\n";
			}
		}
		else
		{
			output << "> endpt does not exist\n";
		}
	}

	std::string network_device::cli_data_to_string(std::vector<std::string> & argus)
	{
		std::string message;
		if ( argus[0] == "txt" )
		{
			for ( dword i = 1; i < argus.size(); i++ )
			{
				message += argus[i];
				if ( i < argus.size() - 1 )
					message += " ";
				else
					message += "\n";
			}
		}
		else if ( argus[0] == "hex" )
		{
			for ( dword i = 1; i < argus.size(); i++ )
			{
				if ( argus[i].size() % 2 )
				{
					output << "> each hex word must have an even size, dropping this word\n";
					continue;
				}
				for ( dword j = 0; j < argus[i].size() - 1; j = j + 2 )
				{
					int msb = toupper(argus[i][j]);
					int lsb = toupper(argus[i][j+1]);
					if ( isdigit(msb) )
						msb -= 0x30;
					else if ( isalpha(msb) )
					{
						if ( 0x40 < msb && msb < 0x47 ) // A to F
							msb = (msb - 0x40) + 0x09; // = -x37
						else
						{
							output << "> non hex character in word, dropping this char\n";
							continue;
						}
					}
					else
					{
						output << "> non hex character in word, dropping this char\n";
						continue;
					}
					if ( isdigit(lsb) )
						lsb -= 0x30;
					else if ( isalpha(lsb) )
					{
						if ( 0x40 < lsb && lsb < 0x47 ) // A to F
							lsb = (lsb - 0x40) + 0x09; // = -x37
						else
						{
							output << "> non hex character in word, dropping this char\n";
							continue;
						}
					}
					else
					{
						output << "> non hex character in word, dropping this char\n";
						continue;
					}
					char c = (char)((msb << 4) | lsb);
					message.push_back(c);
				}
			}
		}
		else
		{
			output << "> unknown data mode (try txt or hex)\n";
			message = "";
		}
		return message;
	}
	
	// snd <ep> {txt|hex} <data>
	void network_device::snd(std::vector<std::string> & argus)
	{
		if ( end_points.find(argus[1]) != end_points.end() )
		{
			std::vector<std::string> argus2;
			argus2.assign(argus.size() - 2, "");
			std::copy(argus.begin() + 2, argus.end(), argus2.begin());
			std::string message = cli_data_to_string(argus2);
			if ( message != "" )
			{
				//io_serv->post(boost::bind(&end_point::msg, end_points[argus[1]], message)); // message by copy
				end_points[argus[1]]->msg(message);
			}
		}
		else
		{
			output << "> endpt does not exist\n";
		}
	}

	// inj {<if>|all} {in|out|all} {txt|hex} <data>
	void network_device::inj(std::vector<std::string> & argus)
	{
		if ( argus.size() < 5 )
		{
			output << "> not enough args\n";
			return;
		}
		std::vector<std::string> argus2;
		argus2.assign(argus.size() - 3, "");
		std::copy(argus.begin() + 3, argus.end(), argus2.begin());
		std::string message = cli_data_to_string(argus2);
		if ( message == "" )
		{
			output << "> no data to inject\n";
			return;
		}
		if ( argus[1] != "all" && line_interfaces.find(argus[1]) == line_interfaces.end() )
		{
			output << "> interface does not exist\n";
			return;
		}
		if ( argus[2] != "in" && argus[2] != "out" && argus[2] != "all" )
		{
			output << "> missing {in|out|all}\n";
			return;
		}
		std::map<std::string, line_interface *>::iterator it = line_interfaces.begin();
		while ( it != line_interfaces.end() )
		{
			if ( argus[1] != "all" && argus[1] != it->first )
			{
				it++;
				continue;
			}
			if ( argus[2] == "in" || argus[2] == "all" )
			{
				frame * f = create_frame();
				f->input_interface = it->first;
				f->vli = it->second->input_queue->vli;
				f->buffer.resize(message.size());
				std::copy(message.begin(), message.end(), f->buffer.begin());
				// cant call preprocess because it calls push, cant this be changed? 
				//io_serv->post(boost::bind(&hold_queue::preprocess, it->second->input_queue, fbuf));
				boost::system::error_code ec;
				//line_pt->vnd_pt->forward(ec, f); // as forward is done before inq wait, thus we dont use the inq wait: shall we do this?
				io_serv->post(boost::bind(&network_device::forward, this, /*ec,*/ f));
			}
			if ( argus[2] == "out" || argus[2] == "all" )
			{
				frame * f = create_frame();
				f->input_interface = ""; // undefined
				f->vli = it->second->output_queue->vli;
				f->buffer.resize(message.size());
				std::copy(message.begin(), message.end(), f->buffer.begin());
				io_serv->post(boost::bind(&hold_queue::push, it->second->output_queue, f));
			}
			it++;
		}
	}

	void network_device::format_stat(std::ostream & os, double d)
	{
		double fp = 0, ip = 0;
		fp = modf(d, &ip);
		dword dwip = static_cast<dword>(ip);
		os << std::right << std::setw(8) << std::noshowpoint << std::noshowpos;
		if ( d < 0 )
		{
			os << std::fixed << "ERROR" << "|";
			return;
		}
		if ( d == 0 )
		{
			os << std::fixed << "0" << "|";
			return;
		}
		if ( dwip < 1 )
		{
			os << std::scientific << std::setprecision(1) << d << "|";
			//os << std::fixed << "0" << "|";
		}
		else if ( dwip < 10UL ) // 9.999999
		{
			os << std::fixed << std::setprecision(6) << d << "|";
		}
		else if ( dwip < 100UL ) // 99.99999
		{
			os << std::fixed << std::setprecision(5) << d << "|";
		}
		else if ( dwip < 1000UL ) // 999.9999
		{
			os << std::fixed << std::setprecision(4) << d << "|";
		}
		else if ( dwip < 10000UL ) // 9999.999
		{
			os << std::fixed << std::setprecision(3) << d << "|";
		}
		else if ( dwip < 100000UL ) // 99999.99
		{
			os << std::fixed << std::setprecision(2) << d << "|";
		}
		else if ( dwip < 1000000UL ) // 999999.9
		{
			os << std::fixed << std::setprecision(1) << d << "|";
		}
		else if ( dwip < 100000000UL && !fp ) // 99999999
		{
			os << std::fixed << dwip << "|";
		}
		else
		{
			os << std::scientific << std::setprecision(1) << d << "|";
		}
	}

	void network_device::stat(std::ostream & os, std::vector<std::string> & argus)
	{
		std::map<std::string, double> qst;
		if ( argus.size() == 1 )
		{
			argus.push_back("all");
			argus.push_back("all");
		}
		if ( argus.size() == 2 )
		{
			argus.push_back("all");
		}
		if ( argus.size() > 3 )
		{
			output << "wrong args\n";
			return;
		}
		os << "\nStatistics of interfaces\n";
		os << "interf|frames  |bytes   |frames/s|bytes/s |lost Fs |lost Bs |lost Fps|lost Bps|\n";
		os << "------|--------|--------|--------|--------|--------|--------|--------|--------|\n";
		std::map<std::string, line_interface *>::iterator it = line_interfaces.begin();
		while ( it != line_interfaces.end() )
		{
			if ( argus[1] != "all" && argus[1] != it->first )
			{
				it++;
				continue;
			}
			if ( argus[2] == "in" || argus[2] == "all" )
			{
				os << std::setw(6) << std::left << (it->first.substr(0, 3) + "-in") << "|";
				qst.clear();
				line_interfaces[it->first]->input_queue->compute_stats(&qst);
				format_stat(os, qst["F"]);
				format_stat(os, qst["B"]);
				format_stat(os, qst["Fps"]);
				format_stat(os, qst["Bps"]);
				format_stat(os, qst["lF"]);
				format_stat(os, qst["lB"]);
				format_stat(os, qst["lFps"]);
				format_stat(os, qst["lBps"]);
				os << "\n";
			}
			if ( argus[2] == "out" || argus[2] == "all" )
			{
				os << std::setw(6) << std::left << (it->first.substr(0, 2) + "-out") << "|";
				qst.clear();
				line_interfaces[it->first]->output_queue->compute_stats(&qst);
				format_stat(os, qst["F"]);
				format_stat(os, qst["B"]);
				format_stat(os, qst["Fps"]);
				format_stat(os, qst["Bps"]);
				format_stat(os, qst["lF"]);
				format_stat(os, qst["lB"]);
				format_stat(os, qst["lFps"]);
				format_stat(os, qst["lBps"]);
				os << "\n";
			}
			if ( argus[1] != "all" )
			{
				//os << "Stats computed over " << std::fixed << std::setprecision(0) << qst["d"] / 60 << " minute(s) and " << qst["d"] % 60 <<" second(s)\n";
				//os << "Queue started since " << std::fixed << std::setprecision(0) << qst["D"] / 60 << " minute(s) and " << qst["D"] % 60 <<" second(s)\n";
				os << "Stats computed over " << static_cast<long>(qst["d"]) / 60 << " minute(s) and " << static_cast<long>(qst["d"]) % 60 <<" second(s)\n";
				os << "Queue started since " << static_cast<long>(qst["D"]) / 60 << " minute(s) and " << static_cast<long>(qst["D"]) % 60 <<" second(s)\n";
			}
			it++;
		}
		os << "------|--------|--------|--------|--------|--------|--------|--------|--------|\n";
		os << "\n";
	}

	std::string network_device::print_interface_type(line_interface * li)
	{
		if ( li->interface_family == interface_nic )
			return "nic";
		else if ( li->interface_family == interface_wic )
			return "wic";
		else if ( li->interface_family == interface_ral )
			return "ral";
		else
			return " ? ";
	}

	std::string network_device::print_interface_protocol(line_interface * li)
	{
		if ( li->protocol == protocol_ethernet )
			return "eth";
		else if ( li->protocol == protocol_pseudo_802_11 )
			return "p11";
		else if ( li->protocol == protocol_none )
			return "raw";
		else
			return " ? ";
	}

	void network_device::list(std::ostream & os, std::string arg1)
	{
		if ( arg1 == "if" )
		{
			os << "\nInterfaces\n";
			os << "interf|typ|pro|endpnt|que|up |vli|bandwdth|delay   |jitter %|BER     |queue sz|\n";
			os << "------|---|---|------|---|---|---|--------|--------|--------|--------|--------|\n";
			std::map<std::string, line_interface *>::iterator it = line_interfaces.begin();
			while ( it != line_interfaces.end() )
			{
				// input queue
				os << std::setw(6) << std::left << it->first.substr(0, 6) << "|" 
					<< print_interface_type(it->second) << "|" << print_interface_protocol(it->second) << "|";
				if ( it->second->bound_ep )
					os << std::setw(6) << std::left << it->second->bound_ep->id.substr(0, 6);
				else
					os << std::setw(6) << std::left << "no ep ";
				if ( it->second->input_queue->up )
					os << "| in|yes|";
				else
					os << "| in| no|";
				os << std::setw(3) << std::right << std::fixed << static_cast<int>(it->second->input_queue->vli) << "|";
				format_stat(os, static_cast<double>(it->second->input_queue->bw));
				format_stat(os, it->second->input_queue->dl);
				format_stat(os, it->second->input_queue->dv);
				format_stat(os, it->second->input_queue->ber);
				format_stat(os, it->second->input_queue->queue_size);
				os << "\n";
				// output queue
				os << "      |   |   |      ";
				if ( it->second->output_queue->up )
					os << "|out|yes|";
				else
					os << "|out| no|";
				os << std::setw(3) << std::right << std::fixed << static_cast<int>(it->second->output_queue->vli) << "|";
				format_stat(os, static_cast<double>(it->second->output_queue->bw));
				format_stat(os, it->second->output_queue->dl);
				format_stat(os, it->second->output_queue->dv);
				format_stat(os, it->second->output_queue->ber);
				format_stat(os, it->second->output_queue->queue_size);
				os << "\n";
				it++;
			}
			os << "------|---|---|------|---|---|---|--------|--------|--------|--------|--------|\n";
			os << "\n";
		}
		else if ( arg1 == "ep" )
		{
			os << "\nEndpoints\n";
			os << "endpoint  |ecp|interface|type|prot|laddress       |lport|raddress       |rport|\n";
			os << "----------|---|---------|----|----|---------------|-----|---------------|-----|\n";
			std::map<std::string, end_point *>::iterator it = end_points.begin();
			while ( it != end_points.end() )
			{
				os << std::setw(10) << std::left    
					<< it->first.substr(0, 10) << "|" << print_encapsulation(it->second->encapsulation) << "|";
				if ( it->second->bound_if )
					os << std::setw(9) << std::left << it->second->bound_if->id.substr(0, 9) << "|";
				else
					os << std::setw(9) << std::left << "unbound" << "|";
				if ( it->second->server )
					os << "serv|";
				else
					os << "clie|";
				if ( it->second->tran_proto == "tap" )
				{
					os << " tap|";
					os << std::setw(15) << std::right << it->second->tap_name.substr(0, 15) << "|";
					os << std::setw(5) << std::right << it->second->tap_desc << "|"; 
					os << std::setw(15) << std::right << "n/a" << "|"; 
					os << std::setw(5) << std::right << "n/a" << "|";
				}
				if ( it->second->tran_proto == "vde" )
				{
					os << " vde|";
					os << std::setw(15) << std::right << it->second->vde_switch.substr(0, 15) << "|";
					os << std::setw(5) << std::right << it->second->vde_port << "|"; 
					os << std::setw(15) << std::right << "n/a" << "|"; 
					os << std::setw(5) << std::right << "n/a" << "|";
				}
				if ( it->second->tran_proto == "tcp" )
				{
					os << " tcp|";
					try
					{
						os << std::setw(15) << std::right << it->second->tcp_socket->local_endpoint().address().to_string() << "|";
						os << std::setw(5) << std::right << it->second->tcp_socket->local_endpoint().port() << "|"; 
						os << std::setw(15) << std::right << it->second->tcp_socket->remote_endpoint().address().to_string() << "|"; 
						os << std::setw(5) << std::right << it->second->tcp_socket->remote_endpoint().port() << "|";
					}
					catch (std::exception & e) // listening
					{
						std::string cause(e.what());
						os << std::setw(15) << std::right << (it->second->local_net_addr /*+ "!"*/) << "|";
						os << std::setw(5) << std::right << (it->second->local_port_nb + "") << "|";
						os << std::setw(15) << std::right << "n/a" << "|"; 
						os << std::setw(5) << std::right << "n/a" << "|";
					}
				}
				if ( it->second->tran_proto == "stm" )
				{
#ifdef VLIN
					os << " stm|";
					try
					{
						// we'll solve this later
/*						os << std::setw(15) << std::right << it->second->stm_socket->local_endpoint().path() << "|";
						os << std::setw(5) << std::right << it->second->stm_socket->local_endpoint().protocol() << "|"; 
						os << std::setw(15) << std::right << it->second->stm_socket->remote_endpoint().path() << "|"; 
						os << std::setw(5) << std::right << it->second->stm_socket->remote_endpoint().protocol() << "|";
*/					
						os << std::setw(15) << std::right << (it->second->path_name + "!").substr(0, 15) << "|";
						os << std::setw(5) << std::right << (it->second->local_port_nb + "?!").substr(0, 15) << "|";
						os << std::setw(15) << std::right << "n/a" << "|"; 
						os << std::setw(5) << std::right << "n/a" << "|";
					}
					catch (std::exception & e) // listening?
					{
						os << std::setw(15) << std::right << (it->second->path_name + "!").substr(0, 15) << "|";
						os << std::setw(5) << std::right << (it->second->local_port_nb + "?!").substr(0, 15) << "|";
						os << std::setw(15) << std::right << "n/a" << "|"; 
						os << std::setw(5) << std::right << "n/a" << "|";
					}
#endif
				}
				if ( it->second->tran_proto == "udp" )
				{
					os << " udp|";
					if ( it->second->server )
					{
						try
						{
							os << std::setw(15) << std::right << it->second->udp_socket->local_endpoint().address().to_string() << "|";
							os << std::setw(5) << std::right << it->second->udp_socket->local_endpoint().port() << "|"; 
							os << std::setw(15) << std::right << it->second->udp_socket->remote_endpoint().address().to_string() << "|"; 
							os << std::setw(5) << std::right << it->second->udp_socket->remote_endpoint().port() << "|";
						}
						catch (std::exception & e) // unknown remote
						{
							std::string cause(e.what());
							if ( it->second->udp_remote_endpoint.address().to_string() != "127.0.0.2" )
							{
								os << std::setw(15) << std::right << it->second->udp_remote_endpoint.address().to_string() << "|"; 
								os << std::setw(5) << std::right << it->second->udp_remote_endpoint.port() << "|";
							}
							else
							{
								os << std::setw(15) << std::right << "n/a" << "|"; 
								os << std::setw(5) << std::right << "n/a" << "|";
							}
						}
					}
					else // udp client
					{
						try
						{
							os << std::setw(15) << std::right << it->second->udp_socket->local_endpoint().address().to_string() << "|";
							os << std::setw(5) << std::right << it->second->udp_socket->local_endpoint().port() << "|"; 
							os << std::setw(15) << std::right << it->second->udp_socket->remote_endpoint().address().to_string() << "|"; 
							os << std::setw(5) << std::right << it->second->udp_socket->remote_endpoint().port() << "|";
						}
						catch (std::exception & e) // unknown local?
						{
							std::string cause(e.what());
							os << std::setw(15) << std::right << (it->second->local_net_addr + "!") << "|";
							os << std::setw(5) << std::right << (it->second->local_port_nb + "") << "|";
							os << std::setw(15) << std::right << (it->second->remote_net_addr + "!") << "|";
							os << std::setw(5) << std::right << (it->second->remote_port_nb + "") << "|";
						}
					}
				}
				if ( it->second->tran_proto == "dgm" )
				{
#ifdef VLIN
					os << " dgm|";
					os << std::setw(15) << std::right << (it->second->path_name + "!").substr(0, 15) << "|";
					os << std::setw(5) << std::right << (it->second->local_port_nb + "?!").substr(0, 15) << "|";
					os << std::setw(15) << std::right << "n/a" << "|"; 
					os << std::setw(5) << std::right << "n/a" << "|";
#endif
				}
				os << "\n";
				it++;
			}
			os << "----------|---|---------|----|----|---------------|-----|---------------|-----|\n";
			os << "\n";
		}
		else if ( arg1 == "lk" )
		{
			os << "\nLink table\n";
			os << "First interface    --> Second interface  |\n";
			os << "--------------------|--------------------|\n";
			std::multimap<std::string, std::string>::iterator it = linking_table.begin();
			while ( it != linking_table.end() )
			{
				os << std::setw(20) << std::left << it->first.substr(0, 20) << "|" 
					<< std::setw(20) << std::left << it->second.substr(0, 20) << "|\n";
				it++;
			}
			os << "--------------------|--------------------|\n";
			os << "\n";
		}
		else if ( arg1 == "fw" )
		{
			os << "\nForwarding table\n";
			os << "Hardware address |Output interface    |\n";
			os << "-----------------|--------------------|\n";
			std::map<address, std::string>::iterator it = forwarding_table.begin();
			while ( it != forwarding_table.end() )
			{
				os << std::setw(17) << std::left << (it->first).print().substr(0, 17) << "|" 
					<< std::setw(20) << std::left << it->second.substr(0, 20) << "|\n";
				it++;
			}
			os << "-----------------|--------------------|\n";
			os << "\n";
		}
		else //if ( arg1 == "op" )
		{
			os << "\nModes\n";
			os << "-------------------------------------\n";
			os << "device name: " << name << "\n";
			if ( device == device_link )
				os << "mode link\n";
			if ( device == device_hub )
				os << "mode hub\n";
			if ( device == device_switch )
				os << "mode switch\n";
			if ( device == device_access_point )
				os << "mode access point\n";
			if ( device == device_mobile_infra )
				os << "mode mobile infrastructure\n";
			if ( device == device_mobile_adhoc )
				os << "mode mobile adhoc\n";
			if ( debug )
				os << "debug on\n";
			else
				os << "debug off\n";
			os << "-------------------------------------\n";
		}
	}

	void network_device::stat_interface(std::vector<std::string> & argus)
	{
		stat(std::cout, argus);
	}

	void network_device::unstat_interface(std::vector<std::string> & argus)
	{
		if ( argus.size() < 3 )
		{
			output << "> missing args\n";
			return;
			//argus.push_back("all");
			//argus.push_back("all");
		}
		if ( argus[1] != "all" && line_interfaces.find(argus[1]) == line_interfaces.end() )
		{
			output << "> interface does not exist\n";
			return;
		}
		std::map<std::string, line_interface *>::iterator it = line_interfaces.begin();
		while ( it != line_interfaces.end() )
		{
			if ( argus[1] != "all" && argus[1] != it->first )
			{
				it++;
				continue;
			}
			if ( argus[2] == "in" || argus[2] == "all" )
			{
				line_interfaces[it->first]->input_queue->reset_stats();
			}
			if ( argus[2] == "out" || argus[2] == "all" )
			{
				line_interfaces[it->first]->output_queue->reset_stats();
			}
			if ( argus[2] != "in" && argus[2] != "out" && argus[2] != "all" )
			{
				output << "> missing {in|out|all}\n";
				return;
			}
			it++;
		}
	}

	void network_device::show(std::vector<std::string> & argus)
	{
		if ( argus.size() == 2 )
		{
			if ( argus[1] == "if" || argus[1] == "ep" || argus[1] == "lk" || argus[1] == "fw" || argus[1] == "op" )
			{
				list(std::cout, argus[1]);
				return;
			}
		}
		output << "> wrong or missing args\n";
	}

	void network_device::dump(std::vector<std::string> & argus)
	{
		if ( argus.size() == 3 )
		{
			std::ofstream ofs(argus[2].c_str());
			if ( ofs.bad() )
			{
				error << "failed to open dump file " << argus[2] << "\n";
				return;
			}
			if ( argus[1] == "if" || argus[1] == "ep" || argus[1] == "lk" || argus[1] == "fw" )
			{
				ofs << name << "\n---------\n";
				list(ofs, argus[1]);
				ofs.close();
				return;
			}
			if ( argus[1] == "st" )
			{
				argus.pop_back();
				argus.pop_back();
				ofs << name << "\n---------\n";
				stat(ofs, argus);
				ofs.close();
				return;
			}
			ofs.close();
		}
		output << "> wrong or missing args\n";
	}

	void network_device::clear(std::vector<std::string> & argus)
	{
		if ( argus.size() == 2 )
		{
			if ( argus[1] == "lk" || argus[1] == "fw" )
			{
				if ( argus[1] == "lk" )
					linking_table.clear();
				else if ( argus[1] == "fw" )
					forwarding_table.clear();
				else
					output << "> arg should be lk or fw\n";
				return;
			}
		}
		output << "> wrong or missing args\n";
	}

	// {trace|untrace} {<if>|all} {txt|hex|pcap} {console|<dump_file>}
	void network_device::trace_interface(std::vector<std::string> & argus)
	{
		if ( argus.size() != 4 )
		{
			output << "> wrong nb of args\n";
			return;
		}
		if ( argus[1] != "all" && line_interfaces.find(argus[1]) == line_interfaces.end() )
		{
			output << "> interface does not exist\n";
			return;
		}
		std::map<std::string, line_interface *>::iterator it = line_interfaces.begin();
		while ( it != line_interfaces.end() )
		{
			if ( argus[1] != "all" && argus[1] != it->first )
			{
				it++;
				continue;
			}
			if ( argus[3] == "console" )
			{
				if ( argus[0] == "trace" )
					it->second->trace_flag = true;
				else // if ( argus[1] == "untrace" )
					it->second->trace_flag = false;
				if ( argus[2] == "hex" )
					it->second->trace_format = trace_format_hexa;
				else
					it->second->trace_format = trace_format_text;
			}
			else // if ( argus[3] != "console" )
			{
				if ( argus[0] == "trace" ) // if already opened file, it is closed and opened with new name
				{
					if ( it->second->trace_file.is_open() ) // in case previous trace was not closed
						it->second->trace_file.close();
					it->second->trace_file_flag = true;
					if ( argus[2] == "pcap" )
						it->second->trace_file_format = trace_format_pcap;
					else if ( argus[2] == "hex" )
						it->second->trace_file_format = trace_format_hexa;
					else
						it->second->trace_file_format = trace_format_text;
					if ( it->second->trace_file_format == trace_format_pcap )
					{
						it->second->trace_file.open(argus[3].c_str(), std::ios_base::out|std::ios_base::trunc|std::ios_base::binary);
						std::vector<byte> global_header; // 24 bytes global header
						util::dword_to_bytes(0xa1b2c3d4ul, global_header); // guint32 magic_number
						util::dword_to_bytes(0x00020004ul, global_header); // guint16 version_major + guint16 version_minor
						util::dword_to_bytes(0x00000000ul, global_header); // gint32  thiszone; // GMT to local correction
						util::dword_to_bytes(0x00000000ul, global_header); // guint32 sigfigs; // accuracy of timestamps
						util::dword_to_bytes(0x00010000ul, global_header); // guint32 snaplen; // max length of captured packets, in octets: 65k here
						util::dword_to_bytes(0x00000001ul, global_header); // guint32 network; // data link type: 1 = Ethernet
						it->second->trace_file.write(util::print_ascii(global_header).c_str(), global_header.size()); // pb with unsigned?				
					}
					else
					{
						it->second->trace_file.open(argus[3].c_str(), std::ios_base::out|std::ios_base::trunc);
					}
				}
				else // if ( argus[0] == "untrace" ) // if filename is different the correct file will be closed anyway
				{
					it->second->trace_file_flag = false;
					if ( it->second->trace_file.is_open() )
						it->second->trace_file.close();
				}
			}
			it++;
		}
	}

	void network_device::help()
	{
		output 
			<< copyright << "\n"
			<< version << "\n"
			<< "  add <if> {nic|wic|ral} {raw|eth|p11}\n"
			<< "  rem {<if>|all}\n"
			<< "  {up|down} {<if>|all} {in|out|all}\n"
			<< "  {set|unset} {<if>|all} {in|out|all} \n"
			<< "    {vli <id>|bw <bps>|dl <s>|dv <%>|ber <bep>|qs <queue_size_pkts>}\n"
			<< "  {stat|unstat} {<if>|all} {in|out|all}\n" 
			<< "  {trace|untrace} {<if>|all} {txt|hex|pcap} {console|<dump_file>}\n"
			<< "  {tie|untie} <if1> <if2>\n"
			<< "  incn <ep> {raw|len} {tcp|udp} {<laddr>|*} {<lport>|*} <raddr> <rport>\n"
			<< "  insr <ep> {raw|len} {tcp|udp} {<laddr>|*} {<lport>|*} {<raddr>|?} {<rport>|?}\n"
			<< "  uncn <ep> {raw|len} {stm|dgm} <socket_pathname>\n"
			<< "  unsr <ep> {raw|len} {stm|dgm} <socket_pathname>\n"
			<< "  tap <ep> <tap_ifname>\n"
			<< "  vde <ep> <switch_pathname> <port> <group> <mode>\n"
			<< "  disc {<ep>|all}\n"
			<< "  {par|unpar} {<ep>|all} {in|out|all} bs <buffer_size_bytes>\n"
			<< "  {bind|unbind} <ep> <if>\n"
			<< "  snd <ep> {txt|hex} <data>\n"
			<< "  inj {<if>|all} {in|out|all} {txt|hex} <data>\n"
			<< "  show {if|ep|lk|fw|op}\n"
			<< "  clear {lk|fw}\n"
			<< "  load <conf_file>\n"
			<< "  dump {if|ep|lk|fw|st} <dump_file>\n"
			<< "  mode {link|hub|switch|access-point|mob-infra|mob-adhoc}\n"
			<< "  debug {on|off}\n"
			<< "  name <device_name>\n"
			<< "  exit\n";
	}

	void network_device::command(std::string s)
	{
		std::istringstream iss(s);
		std::vector<std::string> argus;
		while ( iss.good() )
		{
			std::string s;
			iss >> s;
			argus.push_back(s);
		}
		std::string c = argus[0];
		if ( c == "" )
		{
			if ( debug )
				;//output << "> missing command\n";
			else
				;
		}
		else if ( c == "add" )
		{
			add_interface(argus);
		}
		else if ( c == "rem" )
		{
			remv_interface(argus);
		}
		else if ( c == "up" )
		{
			up_interface(argus);
		}
		else if ( c == "down" )
		{
			down_interface(argus);
		}
		else if ( c == "set" )
		{
			set_interface(argus);
		}
		else if ( c == "unset" )
		{
			unset_interface(argus);
		}
		else if ( c == "tie" )
		{
			tie_interfaces(argus);
		}
		else if ( c == "untie" )
		{
			untie_interfaces(argus);
		}
		else if ( c == "stat" )
		{
			stat_interface(argus);
		}
		else if ( c == "unstat" )
		{
			unstat_interface(argus);
		}
		else if ( c == "trace" )
		{
			trace_interface(argus);
		}
		else if ( c == "untrace" )
		{
			trace_interface(argus); // hack, too lazy to write another function
		}
		else if ( c == "incn" )
		{
			in_conn_endpoint(argus);
		}
		else if ( c == "insr" )
		{
			in_serv_endpoint(argus);
		}
		else if ( c == "uncn" )
		{
#ifdef VLIN
			un_conn_endpoint(argus);
#else
			output << "> local socket unsupported on this system\n";
#endif
		}
		else if ( c == "unsr" )
		{
#ifdef VLIN
			un_serv_endpoint(argus);
#else
			output << "> local socket unsupported on this system\n";
#endif
		}
		else if ( c == "tap" )
		{
#ifdef VLIN
			tap_endpoint(argus);
#else
			output << "> tap unsupported on this system\n";
#endif
		}
		else if ( c == "vde" )
		{
#ifdef VLIN
			vde_endpoint(argus);
#else
			output << "> VDE unsupported on this system\n";
#endif
		}
		else if ( c == "disc" )
		{
			disc_endpoint(argus);
		}
		else if ( c == "bind" )
		{
			bind_endpoint(argus);
		}
		else if ( c == "unbind" )
		{
			unbind_endpoint(argus);
		}
		else if ( c == "par" )
		{
			par_endpoint(argus);
		}
		else if ( c == "unpar" )
		{
			unpar_endpoint(argus);
		}
		else if ( c == "snd" )
		{
			snd(argus);
		}
		else if ( c == "inj" )
		{
			inj(argus);
		}
		else if ( c == "show" )
		{
			show(argus);
		}
		else if ( c == "dump" )
		{
			dump(argus);
		}
		else if ( c == "clear" )
		{
			clear(argus);
		}
		else if ( c == "mode" || c == "debug" )//|| c == "hexa" )
		{
			if ( argus.size() == 2 )
				set(c, argus[1]);
		}
		else if ( c == "name" )
		{
			name = argus[1];
		}
		else if ( c == "help" )
		{
			help();
		}
		else
			output << "> unknown command: " << c << ", (see help for syntax)\n";
	}

	vnd_exception::vnd_exception() throw()
	{
		message = "";
	}

	vnd_exception::vnd_exception(std::string s) throw()
	{
		message = s;
	}

	vnd_exception::~vnd_exception() throw()
	{
	}

	const char * vnd_exception::what() const throw()
	{
		return message.c_str();
	}
}
