
// nemo
// Copyright (c) 2012-2016 Damien Magoni

#include "nemo.h"

namespace nemo
{
	void map_type::generate(std::vector<std::string> & v)
	{
		if ( v.size() != 7 )
			return;
		map_setting = map_flat;//strtoul(v[2].c_str(), 0, 10);
		x_size = strtoul(v[3].c_str(), 0, 10);
		if ( x_size <= 0 )
			x_size = 1;
		y_size = strtoul(v[4].c_str(), 0, 10);
		if ( y_size <= 0 )
			y_size = 1;
		z_size = strtoul(v[5].c_str(), 0, 10);
		if ( z_size <= 0 )
			z_size = 1;
		space_step_in_m = strtod(v[6].c_str(), 0);
	}

	void map_type::load(std::vector<std::string> & v)
	{
		nemo_ptr->output << "not implemented yet\n";
	}

	void map_type::save(std::vector<std::string> & v)
	{
		nemo_ptr->output << "not implemented yet\n";
	}

	//mob gen <duration> <nb_of_nodes> <nb_of_events> <max_speed> <max_accel>
	void mobility_scenario_type::generate(std::vector<std::string> & v)
	{
		if ( v.size() != 7 )
			return;
		mobility_events.clear();
		double secs = strtod(v[2].c_str(), 0);
		nrt_scheduler.scenario_duration_in_s = fpsec(secs);
		long long ns = static_cast<long long>(secs * 1E9);
		nrt_scheduler.scenario_duration = nsec(ns);
		nrt_scheduler.start_time = boost::chrono::high_resolution_clock::now(); // virtual epoch for nRT, never change this afterwards!
		// generate v3 nodes and initial mobility events
		int nb = strtoul(v[3].c_str(), 0, 10); // number of nodes
		for ( int i = 1; i <= nb; ++i )
		{
			mobile_device_type * m = new mobile_device_type;
			m->id = i;
			m->wic = wic_cisco_aironet;
			m->tx_power = default_tx_power;
			m->margin = default_margin;
			m->carrier_freq = default_carrier_freq;
			m->position.x = prost::random_l(0, nemo_ptr->map.x_size);
			m->position.y = prost::random_l(0, nemo_ptr->map.y_size);
			m->position.z = 0;//prost::random_l(0, nemo_ptr->map.z_size);
			m->velocity.x = prost::random_l(0, strtoul(v[5].c_str(), 0, 10)); // vitesse de debut : oui // de fin, min, max ?
			m->velocity.y = prost::random_l(0, strtoul(v[5].c_str(), 0, 10));
			m->velocity.z = 0;
			m->acceleration.x = prost::random_l(0, strtoul(v[6].c_str(), 0, 10));
			m->acceleration.y = prost::random_l(0, strtoul(v[6].c_str(), 0, 10));
			m->acceleration.z = 0;
			m->last_time = nrt_scheduler.start_time;
			m->delta_time = nsec(0);
			nemo_ptr->mobile_devices[i] = m;
			// should we create initial me? yes
			mobility_event_type * me = new mobility_event_type;
			me->delta_time = m->delta_time;
			me->time = m->last_time;
			me->device_id = m->id;
			me->velocity = m->velocity;
			me->acceleration = m->acceleration;
			mobility_events.insert(std::pair<nsec, mobility_event_type *>(me->delta_time, me));
		}
		// generate v4 mobility events
		int nb2 = strtoul(v[4].c_str(), 0, 10); // number of events
		for ( int i = 1; i <= nb2; ++i )
		{
			mobility_event_type * me = new mobility_event_type;
			long secdu = nrt_scheduler.scenario_duration_in_s.count(); // losing floating part
			long long secdt = static_cast<long long>(prost::random_l(1, secdu - 1) * 1E9); // second granularity!! should it be configurable?
			me->delta_time = nsec(secdt); // pb here if using unsigned
			me->time = me->delta_time + nrt_scheduler.start_time;
			me->device_id = prost::random_l(1, nb);
			me->velocity.x = prost::random_l(0, strtoul(v[5].c_str(), 0, 10)); // vitesse de debut de fin min max ?
			me->velocity.y = prost::random_l(0, strtoul(v[5].c_str(), 0, 10));
			me->velocity.z = 0;
			me->acceleration.x = prost::random_l(0, strtoul(v[6].c_str(), 0, 10)); // accel de debut : oui // de fin, min, max ?
			me->acceleration.y = prost::random_l(0, strtoul(v[6].c_str(), 0, 10));
			me->acceleration.z = 0;
			mobility_events.insert(std::pair<nsec, mobility_event_type *>(me->delta_time, me));
		}
	}

	double mobility_scenario_type::distance(position_type p1, position_type p2)
	{
		double sx = std::pow(static_cast<double>(p2.x - p1.x), 2);
		double sy = std::pow(static_cast<double>(p2.y - p1.y), 2);
		double sz = std::pow(static_cast<double>(p2.z - p1.z), 2);
		double s = sx + sy + sz;
		if ( s < 0 )
		{
			nemo_ptr->error << "can not compute the sqrt of a negative number\n";
			s = 0;
		}
		return std::sqrt(s);
	}

	// mob node {<id>|all} {txp <dBm>|margin <dBm>|wic {aironet|xjack|wavelan}}
	void mobility_scenario_type::node(std::vector<std::string> & v)
	{
		while ( v.size() > 3 && v.size() % 2 == 1 )
		{
			std::map<dword, mobile_device_type *>::iterator mdit;
			mdit = nemo_ptr->mobile_devices.begin(); 
			while ( mdit != nemo_ptr->mobile_devices.end() )
			{
				mobile_device_type * m = mdit->second;
				if ( v[2] != "all" && strtoul(v[2].c_str(), 0, 10) != m->id )
				{
					mdit++;
					continue;
				}
				m->set(v[v.size() - 2], v[v.size() - 1]);
				mdit++;
			}
			v.pop_back();
			v.pop_back();
		}
	}

	void mobile_device_type::set(std::string par, std::string val)
	{
		if ( par == "txp" )
			tx_power = strtod(val.c_str(), 0);
		if ( par == "margin" )
			margin = strtod(val.c_str(), 0);
		if ( par == "cfreq" )
			carrier_freq = strtod(val.c_str(), 0);
		if ( par == "wic" )
		{
			if ( val == "aironet" ) // 802.11g
			{
				wic = wic_cisco_aironet;
			}
			else if ( val == "xjack" ) // 802.11g
			{
				wic = wic_3com_xjack;
			}
			else if ( val == "wavelan" ) // 802.11
			{
				wic = wic_lucent_wavelan;
			}
			else
			{
				wic = wic_cisco_aironet;
			}
		}
	}

	double mobility_scenario_type::bitrate(double receive_level_dBm, wic_type wt)
	{
		if ( wt == wic_cisco_aironet ) // 802.11g
		{
			if ( receive_level_dBm >= -71 )
				return 54000000;
			else if ( receive_level_dBm >= -75 )
				return 48000000;
			else if ( receive_level_dBm >= -80 )
				return 36000000;
			else if ( receive_level_dBm >= -84 )
				return 24000000;
			else if ( receive_level_dBm >= -86 )
				return 18000000;
			else if ( receive_level_dBm >= -86 )
				return 12000000;
			else if ( receive_level_dBm >= -86 )
				return 9000000;
			else if ( receive_level_dBm >= -86 )
				return 6000000;
			else
				return 0;
		}
		else if ( wt == wic_3com_xjack )
		{
			if ( receive_level_dBm >= -70 )
				return 54000000;
			else if ( receive_level_dBm >= -74 )
				return 48000000;
			else if ( receive_level_dBm >= -80 )
				return 36000000;
			else if ( receive_level_dBm >= -83 )
				return 24000000;
			else if ( receive_level_dBm >= -87 )
				return 18000000;
			else if ( receive_level_dBm >= -88 )
				return 12000000;
			else if ( receive_level_dBm >= -89 )
				return 9000000;
			else if ( receive_level_dBm >= -90 )
				return 6000000;
			else
				return 0;
		}
		else if ( wt == wic_lucent_wavelan )
		{
			if ( receive_level_dBm >= -69.61949627 ) // true for txp = 20dBm and margin = 10 dBm
				return 2000000;
			else
				return 0;
		}
		else
		{
			return 0;
		}
	}

	void mobility_scenario_type::load_nemo(std::string & fn)
	{
		std::ifstream f(fn.c_str());
		if ( !f )
		{
			nemo_ptr->error << "failed to open [nemo] type mobility scenario file " << fn << "\n";
			return;
		}
		mobility_events.clear();
		nrt_scheduler.start_time = boost::chrono::high_resolution_clock::now(); // virtual epoch for nRT
		nrt_scheduler.scenario_duration = nsec(0); // we don't know yet
		while ( !f.eof() )
		{
			std::string l;
			std::getline(f, l); // add error mgmt
			if ( !l.find("#") || !l.size() || !l.find("exit") )//== std::string::npos ) // exit must be at beginning of line!
				continue;
			if ( nemo_ptr->debug )
				nemo_ptr->output << "> reading: " << l << "\n";
			std::istringstream iss(l);
			std::vector<std::string> tokens;
			while ( iss.good() )
			{
				std::string s;
				iss >> s;
				tokens.push_back(s);
			}
			if ( tokens.size() == 2 && tokens[1] == "EOS" )
			{
				double secs = strtod(tokens[0].c_str(), 0);
				nrt_scheduler.scenario_duration_in_s = fpsec(secs);
				long long ns = static_cast<long long>(secs * 1E9);
				nrt_scheduler.scenario_duration = nsec(ns);
			}
			else if ( tokens.size() == 11 && strtod(tokens[0].c_str(), 0) == 0 ) // watch out its a double!
			{
				//# time_s mn_id pos_x pos_y pos_z vel_x vel_y vel_z acc_x acc_y acc_z
				//    0      1    123   456    0     2     6     0     1     0     0
				// should we tsst for double entries of same node? maybe
				mobile_device_type * m = new mobile_device_type;
				m->delta_time = nsec(0);
				m->last_time = nrt_scheduler.start_time;
				m->id = strtoul(tokens[1].c_str(), 0, 10);
				m->wic = wic_cisco_aironet;
				m->tx_power = default_tx_power;
				m->margin = default_margin;
				m->carrier_freq = default_carrier_freq;
				m->position.x = strtod(tokens[2].c_str(), 0);
				m->position.y = strtod(tokens[3].c_str(), 0);
				m->position.z = strtod(tokens[4].c_str(), 0);
				m->velocity.x = strtod(tokens[5].c_str(), 0);
				m->velocity.y = strtod(tokens[6].c_str(), 0);
				m->velocity.z = strtod(tokens[7].c_str(), 0);
				m->acceleration.x = strtod(tokens[8].c_str(), 0);
				m->acceleration.y = strtod(tokens[9].c_str(), 0);
				m->acceleration.z =  strtod(tokens[10].c_str(), 0);
				nemo_ptr->mobile_devices[m->id] = m; 
				// should we create initial me? yes
				mobility_event_type * me = new mobility_event_type;
				me->delta_time = m->delta_time;
				me->time = m->last_time;
				me->device_id = m->id;
				me->velocity = m->velocity;
				me->acceleration = m->acceleration;
				mobility_events.insert(std::pair<nsec, mobility_event_type *>(me->delta_time, me));
			}
			else if ( tokens.size() == 8 )
			{
				//# time_s mn_id vel_x vel_y vel_z acc_x acc_y acc_z
				//   3.45    10    4     6     0     1     0     0
				mobility_event_type * me = new mobility_event_type;
				me->delta_time = nsec(static_cast<long long>(1E9 * strtod(tokens[0].c_str(), 0)));
				me->time = me->delta_time + nrt_scheduler.start_time;
				me->device_id = strtoul(tokens[1].c_str(), 0, 10);
				me->velocity.x = strtod(tokens[2].c_str(), 0);
				me->velocity.y = strtod(tokens[3].c_str(), 0);
				me->velocity.z = strtod(tokens[4].c_str(), 0);
				me->acceleration.x = strtod(tokens[5].c_str(), 0);
				me->acceleration.y = strtod(tokens[6].c_str(), 0);
				me->acceleration.z = strtod(tokens[7].c_str(), 0);
				nemo_ptr->mobility_scenario.mobility_events.insert(std::pair<nsec, mobility_event_type *>(me->delta_time, me));
			}
			else
				nemo_ptr->output << "> could not process line, skipping it\n";
		}
		f.close();
		if ( nrt_scheduler.scenario_duration == nsec(0) )
		{
			nemo_ptr->output << "> could not find EOS time, using last event time\n";
			nrt_scheduler.scenario_duration = mobility_events.rbegin()->first;
			nrt_scheduler.scenario_duration_in_s = fpsec(static_cast<double>(nrt_scheduler.scenario_duration.count()) / 1E9);
		}
	}

/*
#	Steady-state random_l Waypoint Model
#	numNodes   =     50
#	maxX       =   1000.00
#	maxY       =   1000.00
#	endTime    =   5000.00
#	speedMean  =   10.0000
#	speedDelta =    9.0000
#	pauseMean  =     20.00
#	pauseDelta =     20.00
#	output     =      N

# output format is NS2
#	Initial positions:
$node_(0) set X_ 258.270521359214
$node_(0) set Y_ 102.679585225733
$node_(0) set Z_ 0.000000000000
$ns_ at 0.000000000000 "$node_(0) setdest 266.306154088260 50.521009625178 0.000000000000"
$node_(1) set X_ 584.254405823678
$node_(1) set Y_ 800.471645783187
$node_(1) set Z_ 0.000000000000
$ns_ at 0.000000000000 "$node_(1) setdest 530.295056072201 957.574445734534 7.159503693794"
$node_(2) set X_ 555.169843720718
$node_(2) set Y_ 749.766645413628
$node_(2) set Z_ 0.000000000000
$ns_ at 0.000000000000 "$node_(2) setdest 523.781282139840 791.876172084304 1.206961050981"
...

#	Movements:
# start_time node_id setdest final_x final_y speed
$ns_ at 2.285848421030 "$node_(29) setdest 305.833337039609 718.973127994208 0.000000000000"
$ns_ at 2.588910350452 "$node_(8) setdest 887.838492583408 996.248911133152 0.000000000000"
$ns_ at 5.613500611207 "$node_(45) setdest 607.727486457549 671.039295229613 0.000000000000"
$ns_ at 6.143589350314 "$node_(29) setdest 313.968211558633 20.583679909158 13.094838077247"
$ns_ at 6.237715639613 "$node_(13) setdest 689.162453026121 838.848721626610 0.000000000000"
$ns_ at 6.797565901677 "$node_(11) setdest 604.574759772315 255.062498271029 0.000000000000"
$ns_ at 7.047415435719 "$node_(47) setdest 311.538312263572 118.912923205138 14.663742882043"
$ns_ at 7.346616714985 "$node_(38) setdest 810.975851403072 588.811216684436 0.000000000000"
...
*/

	dword mobility_scenario_type::get_node_id(std::string t)
	{
		dword pos = t.find("$node_(");
		if ( pos == std::string::npos )
		{
			return -1; //error
		}
		else
		{
			std::string s = t.substr(pos + 7);
			dword d = atoi(s.c_str());
			return d;
		}
	}

	void mobility_scenario_type::calculate_velocity(mobile_device_type * m, double final_x, double final_y, double modv, nsec st)
	{
		fpsec fps = fpsec(st);
		double ts = fps.count();
		double dx, dy, curr_x, curr_y;
		if ( !ts )
		{
			curr_x = static_cast<double>(m->position.x);
			curr_y = static_cast<double>(m->position.y);
			dx = final_x - static_cast<double>(m->position.x);
			dy = final_y - static_cast<double>(m->position.y);
		}
		else if ( st < m->ns_dest_time ) // mobility event starts *before* the end of the previous mobility event
		{
			// compute how much distance has been covered and current position
			fpsec dt = fpsec(st - m->ns_curr_time);
			double dtime_s = dt.count();
			curr_x = static_cast<double>(m->ns_curr_position.x) + m->ns_curr_velocity.x * dtime_s;
			curr_y = static_cast<double>(m->ns_curr_position.y) + m->ns_curr_velocity.y * dtime_s;
			dx = final_x - curr_x;
			dy = final_y - curr_y;
		}
		else // mobility event starts *after* the end of the previous mobility event
		{
			curr_x = static_cast<double>(m->ns_dest_position.x); // ns_dest_position should be equal to position
			curr_y = static_cast<double>(m->ns_dest_position.y);
			dx = final_x - static_cast<double>(m->ns_dest_position.x);
			dy = final_y - static_cast<double>(m->ns_dest_position.y);
		}
		double dist = sqrt(dx * dx + dy * dy);
		// calculate new current velocity
		if ( modv < 1.0e-9 ) // v = 0
		{
			m->ns_curr_velocity.x = 0;
			m->ns_curr_velocity.y = 0;
			m->ns_curr_velocity.z = 0;
		}
		else // v > 0
		{
			double alpha = 0;
			if ( fabs(dx) < 1.0e-9 )
			{
				if ( dy > 0 )
					alpha = 3.14159265 / 2;
				else
					alpha = -3.14159265 / 2;
			}
			else
			{
				alpha = atan2(dy, dx);
			}
			m->ns_curr_velocity.x = modv * cos(alpha);
			m->ns_curr_velocity.y = modv * sin(alpha);
			m->ns_curr_velocity.z = 0;
		}
		m->ns_curr_acceleration.x = 0;
		m->ns_curr_acceleration.y = 0;
		m->ns_curr_acceleration.z = 0;
		m->ns_curr_time = st;
		m->ns_dest_time = nsec(static_cast<long long>(1E9 * (dist / modv))) + st;
		if ( !ts )
		{
			m->ns_curr_position = m->position;
			m->velocity = m->ns_curr_velocity;
			m->acceleration = m->ns_curr_acceleration;
		}
		else if ( st < m->ns_dest_time )
		{
			m->ns_curr_position.x = curr_x; // the dest has not been reached
			m->ns_curr_position.y = curr_y; // the dest has not been reached
		}
		else
		{
			m->ns_curr_position = m->ns_dest_position; // the dest has been reached
		}
		m->ns_dest_position.x = final_x;
		m->ns_dest_position.y = final_y;
		m->ns_dest_position.z = 0;
	}
	
	void mobility_scenario_type::load_ns2(std::string & fn)
	{
		std::ifstream f(fn.c_str());
		if ( !f )
		{
			nemo_ptr->error << "failed to open [ns2] type mobility scenario file " << fn << "\n";
			return;
		}
		mobility_events.clear();
		nrt_scheduler.start_time = boost::chrono::high_resolution_clock::now(); // virtual epoch for nRT
		nrt_scheduler.scenario_duration = nsec(0); // we don't know yet
		int c = 0;
		while ( !f.eof() )
		{
			std::string l;
			std::getline(f, l); // add error mgmt
			if ( !l.find("#") || !l.size() ) // # must be at the beginning of the line!
				continue;
			if ( 0 )//nemo_ptr->debug )
				nemo_ptr->output << "> reading: " << l << "\n";
			c++;
			if ( c && !(c % 1000) )
				nemo_ptr->output << ".";
			std::istringstream iss(l);
			std::vector<std::string> tokens;
			while ( iss.good() )
			{
				std::string s;
				iss >> s;
				tokens.push_back(s);
			}
			if ( tokens.size() == 4 && !tokens[0].find("$node_(") && tokens[1] == "set" ) // compute initial position, and never change it!
			{
				dword nid = get_node_id(tokens[0]);
				if ( nemo_ptr->mobile_devices.find(nid) == nemo_ptr->mobile_devices.end() )
				{
					mobile_device_type * m = new mobile_device_type;
					m->id = nid;
					m->wic = wic_cisco_aironet;
					m->tx_power = default_tx_power;
					m->margin = default_margin;
					m->carrier_freq = default_carrier_freq;
					m->delta_time = nsec(0);
					m->last_time = nrt_scheduler.start_time;
					m->position.x = 0;
					m->position.y = 0;
					m->position.z = 0;
					m->velocity.x = 0;
					m->velocity.y = 0;
					m->velocity.z = 0;
					m->acceleration.x = 0;
					m->acceleration.y = 0;
					m->acceleration.z = 0;
					nemo_ptr->mobile_devices[m->id] = m; 
				}
				mobile_device_type * m = nemo_ptr->mobile_devices[nid];
				if ( tokens[2] == "X_" )
					m->position.x = strtod(tokens[3].c_str(), 0); // should use double for units? yes
				if ( tokens[2] == "Y_" )
					m->position.y = strtod(tokens[3].c_str(), 0);
				if ( tokens[2] == "Z_" )
					m->position.z = strtod(tokens[3].c_str(), 0);
			}
			else if ( tokens.size() == 8 && !tokens[2].find("0.0") && tokens[0] == "$ns_" && tokens[4] == "setdest" )//!= std::string::npos ) // compute initial velocity, and never change it!
			{
				dword nid = get_node_id(tokens[3]);
				if ( nemo_ptr->mobile_devices.find(nid) == nemo_ptr->mobile_devices.end() )
				{
					nemo_ptr->output << "error: undefined mobile node, skipping line\n"; // should not happen otherwise x and y may be undefined
					continue;
				}
				mobile_device_type * m = nemo_ptr->mobile_devices[nid];
				//#       start_time     node_id    setdest final_x          final_y          speed
				//$ns_ at 0.000000000000 "$node_(1) setdest 530.295056072201 957.574445734534 7.159503693794"
				double ts = strtod(tokens[2].c_str(), 0);
				if ( ts )
				{
					nemo_ptr->output << "error: time should be 0, skipping line\n";
					continue;
				}
				nsec st = nsec(static_cast<long long>(1E9 * ts));
				double final_x = strtod(tokens[5].c_str(), 0);
				double final_y = strtod(tokens[6].c_str(), 0);
				double modv = strtod(tokens[7].c_str(), 0);
				calculate_velocity(m, final_x, final_y, modv, st);
				// generate event for initial velocity? yes
				// should we create initial me? yes
				mobility_event_type * me = new mobility_event_type;
				me->delta_time = nsec(0);//m->delta_time;
				me->time = nrt_scheduler.start_time;
				me->device_id = m->id;
				me->velocity = m->velocity;
				me->acceleration = m->acceleration;
				mobility_events.insert(std::pair<nsec, mobility_event_type *>(me->delta_time, me));
			}
			else if ( tokens.size() == 8 && tokens[0] == "$ns_" && tokens[4] == "setdest" ) // compute current velocity and corresponding event
			{
				dword nid = get_node_id(tokens[3]);
				if ( nemo_ptr->mobile_devices.find(nid) == nemo_ptr->mobile_devices.end() )
				{
					nemo_ptr->output << "error: undefined mobile node\n"; // should not happen otherwise x and y may be undefined
					continue;
				}
				mobile_device_type * m = nemo_ptr->mobile_devices[nid];
				//#       start_time     node_id     setdest final_x          final_y         speed
				//$ns_ at 6.143589350314 "$node_(29) setdest 313.968211558633 20.583679909158 13.094838077247"
				double ts = strtod(tokens[2].c_str(), 0);
				nsec st = nsec(static_cast<long long>(1E9 * ts));
				double final_x = strtod(tokens[5].c_str(), 0);
				double final_y = strtod(tokens[6].c_str(), 0);
				double modv = strtod(tokens[7].c_str(), 0);
				calculate_velocity(m, final_x, final_y, modv, st);
				if ( st > m->ns_dest_time ) // event happens after the end of the previous event, thus we first add a stopping order before the next event
				{
					mobility_event_type * me = new mobility_event_type;
					me->delta_time = m->ns_dest_time;
					me->time = me->delta_time + nrt_scheduler.start_time;
					me->device_id = nid;
					me->velocity.x = 0;
					me->velocity.y = 0;
					me->velocity.z = 0;
					me->acceleration.x = 0;
					me->acceleration.y = 0;
					me->acceleration.z = 0;
					mobility_events.insert(std::pair<nsec, mobility_event_type *>(me->delta_time, me));
				}
				// create the new event
				mobility_event_type * me = new mobility_event_type;
				me->delta_time = st;
				me->time = me->delta_time + nrt_scheduler.start_time;
				me->device_id = nid;
				me->velocity = m->ns_curr_velocity;
				me->acceleration = m->ns_curr_acceleration;
				mobility_events.insert(std::pair<nsec, mobility_event_type *>(me->delta_time, me));
			}
			else
			{
				nemo_ptr->output << "unable to parse this line of the ns2 tcl file, skipping it\n";
			}
		}
		nemo_ptr->output << "\n";
		f.close();
		// heuristic to find the total duration of the ns2 simulation, pick latest event
		mobility_event_type * lme = mobility_events.rbegin()->second;
		mobile_device_type * lm = nemo_ptr->mobile_devices.find(lme->device_id)->second;
		nsec tsd = lm->ns_dest_time;
		nrt_scheduler.scenario_duration_in_s = fpsec(static_cast<double>(tsd.count()) / 1E9);
		nrt_scheduler.scenario_duration = nsec(tsd);
	}

	void mobility_scenario_type::load(std::vector<std::string> & v)
	{
		if ( v[2].empty() )
		{
			std::cout << "undefined mobility model file format\n";
			return;
		}
		if ( v[2] == "nemo" )
		{
			load_nemo(v[3]);
		}
		else if ( v[2] == "ns2" )
		{
			load_ns2(v[3]);
		}
		else
			std::cout << "unknown mobility model file format\n";
	}

	std::string print_nsec(nsec ns)
	{
		double d = static_cast<double>(ns.count()) / 1E9;
		std::ostringstream o;
		o /*<< std::right << std::setw(13)*/ << std::showpoint << std::noshowpos;
		o << std::fixed << std::setprecision(6) << d;
		return o.str();
	}

	// mob save {nemo|ns2} <filename>
	void mobility_scenario_type::save(std::vector<std::string> & v)
	{
		if ( v[2].empty() )
		{
			std::cout << "undefined mobility model file format\n";
		}
		else if ( v[2] == "nemo" )
		{
			save_nemo(v[3]);
		}
		else if ( v[2] == "ns2" )
		{
			std::cout << "saving to ns2 format not implemented yet\n";
			//save_ns2(v);
		}
		else
			std::cout << "unknown mobility model file format\n";
	}

	void mobility_scenario_type::save_nemo(std::string & fn)
	{
		std::ofstream f(fn.c_str());
		// dump nodes and initial events
		f << "# time_s mn_id pos_x pos_y pos_z vel_x vel_y vel_z acc_x acc_y acc_z\n";
		std::map<dword, mobile_device_type *>::iterator mdit;
		mdit = nemo_ptr->mobile_devices.begin(); 
		while ( mdit != nemo_ptr->mobile_devices.end() )
		{
			mobile_device_type * m = mdit->second;
			f << 0 << " "
				<< m->id << " "
				<< m->position.x << " "
				<< m->position.y << " "
				<< m->position.z << " "
				<< m->velocity.x << " "
				<< m->velocity.y << " "
				<< m->velocity.z << " "
				<< m->acceleration.x << " "
				<< m->acceleration.y << " "
				<< m->acceleration.z << "\n";
			mdit++;
		}
		// dump random mobility events
		f << "\n# time_s mn_id vel_x vel_y vel_z acc_x acc_y acc_z\n";
		std::multimap<nsec, mobility_event_type *>::iterator meit = mobility_events.begin();
		while ( meit != mobility_events.end() )
		{
			mobility_event_type * me = meit->second;
			f 
				<< print_nsec(me->delta_time) << " "
//				<< fpsec(me->delta_time).count() << " "
				<< me->device_id << " "
				<< me->velocity.x << " "
				<< me->velocity.y << " "
				<< me->velocity.z << " "
				<< me->acceleration.x << " "
				<< me->acceleration.y << " "
				<< me->acceleration.z << "\n";
			meit++;
		}
		// write End Of Simulation (EOS) event
		f << print_nsec(nrt_scheduler.scenario_duration) << " EOS\n";
		f.close();
	}

	void mobility_scenario_type::update_mobile_device(mobile_device_type * md, nsec delta_time)
	{
		long long ldt = delta_time.count(); // in nsec
		double dt = static_cast<double>(ldt) / 1.0E9; // in sec
		// we use mirroring/bumping instead of wrapping around (with mod %)
		unit tmp_x = md->position.x + md->velocity.x * (dt) + 0.5 * md->acceleration.x * pow((dt), 2);
		if ( tmp_x > nemo_ptr->map.x_size )
			tmp_x = 2 * nemo_ptr->map.x_size - tmp_x; // it should not go above 2 * x_max!
		if ( tmp_x < 0 )
			tmp_x = -tmp_x;
		md->position.x = tmp_x;
		unit tmp_y = md->position.y + md->velocity.y * (dt) + 0.5 * md->acceleration.y * pow((dt), 2);
		if ( tmp_y > nemo_ptr->map.y_size )
			tmp_y = 2 * nemo_ptr->map.y_size - tmp_y; // it should not go above 2 * y_max!
		if ( tmp_y < 0 )
			tmp_y = -tmp_y;
		md->position.y = tmp_y;
		unit tmp_z = md->position.z + md->velocity.z * (dt) + 0.5 * md->acceleration.z * pow((dt), 2);
		if ( tmp_z > nemo_ptr->map.z_size )
			tmp_z = 2 * nemo_ptr->map.z_size - tmp_z; // it should not go above 2 * z_max!
		if ( tmp_z < 0 )
			tmp_z = -tmp_z;
		md->position.z = tmp_z;
		md->velocity.x = md->velocity.x + md->acceleration.x * (dt);
		md->velocity.y = md->velocity.y + md->acceleration.y * (dt);
		md->velocity.z = md->velocity.z + md->acceleration.z * (dt);
	}

	connectivity_event_type * mobility_scenario_type::are_connected(dword md1, dword md2) // md1 can/could reach md2!
	{
		rmit = reachable_mobiles.find(md1); // find emitter
		if ( rmit != reachable_mobiles.end() )
		{
			rmtit = rmit->second.find(md2); // find receiver
			if ( rmtit != rmit->second.end() )
			{
				return rmtit->second; // return last ce
			}
			return 0;
		}
		return 0;
	}

	void mobility_scenario_type::execute(sched::action_type * a)
	{
		if ( a->handler == "measurement_timer_expired" )
		{
			nemo_ptr->output << "nRT: doing measurement at simulated time " 
				<< print_nsec(a->call_time - nrt_scheduler.start_time) << "\n";
			// update individual data for each mobile
			mdit = nemo_ptr->mobile_devices.begin();
			while ( mdit != nemo_ptr->mobile_devices.end() )
			{
				mobile_device_type * md = mdit->second;
				nsec delta_time = a->call_time - md->last_time;
				update_mobile_device(md, delta_time);
				mdit++;
			}
			// check all connectivity/reachability
			// WARNING id1 of mdit1 may be higher than id2 of mdit2, as all virtual links are unidirectional
			mdit = nemo_ptr->mobile_devices.begin();
			while ( mdit != nemo_ptr->mobile_devices.end() )
			{
				if ( 0 )//nemo_ptr->debug )
					nemo_ptr->output << "check all nodes reachable by node " << mdit->first << "\n";
				mdit2 = nemo_ptr->mobile_devices.begin();
				while ( mdit2 != nemo_ptr->mobile_devices.end() )
				{
					if ( 0 )//nemo_ptr->debug )
						nemo_ptr->output << "check reachable node " << mdit2->first << "\n";
					if ( mdit == mdit2 )
					{
						mdit2++;
						continue;
					}
					// compute distance
					double tx_distance = distance(mdit->second->position, mdit2->second->position);
					if ( tx_distance == 0 )
						tx_distance = 1E-9;
					// distance to power attn formulae (two ray ground model)
					double attenuation = 20 * std::log10(4 * std::acos(-1.0) / 3E8) 
						+ 20 * std::log10(mdit->second->carrier_freq) + 20 * std::log10(tx_distance);
					double rx_threshold = mdit->second->tx_power - mdit->second->margin - attenuation;
					double bw = bitrate(rx_threshold, mdit2->second->wic);
					connectivity_event_type * ce = are_connected(mdit->first, mdit2->first); // can md1 already reach md2?
					if ( ce ) // pair currently connected
					{
						if ( bw )
						{
							if ( bw != ce->bw ) // update/modify
							{
								connectivity_event_type * nce = new connectivity_event_type;
								nce->connectivity_status = connectivity_status_update;
								nce->emitting_device_id = mdit->first;
								nce->receiving_device_id = mdit2->first;
								nce->time = a->call_time;
								nce->delta_time = a->call_time - nrt_scheduler.start_time;
								nce->bw = bw;
								nce->dly = 0; // fn of dist?
								nce->dlv = 0; // fn of ?
								nce->ber = 0; // fn of dist?
								nce->attn = attenuation;
								nce->dist = tx_distance;
								nemo_ptr->connectivity_scenario.connectivity_events.insert(std::pair<nsec, connectivity_event_type *>(nce->delta_time, nce));
								
								reachable_mobiles[mdit->first].erase(mdit2->first);
								if ( reachable_mobiles[mdit->first].empty() )
									reachable_mobiles.erase(mdit->first); // necessary?
								reachable_mobiles[mdit->first][mdit2->first] = nce; // must erase to stores latest ce (map behavior)
							}
							else // connectivity remains the same
								;
						}
						else // if bw = 0, disconnect
						{
							ce = new connectivity_event_type;
							ce->connectivity_status = connectivity_status_stop;
							ce->emitting_device_id = mdit->first;
							ce->receiving_device_id = mdit2->first;
							ce->delta_time = a->call_time - nrt_scheduler.start_time;
							ce->time = a->call_time;
							ce->bw = bw;
							ce->dly = 0; // fn of dist?
							ce->dlv = 0; // fn of ?
							ce->ber = 0; // fn of dist?
							ce->attn = attenuation;
							ce->dist = tx_distance;
							nemo_ptr->connectivity_scenario.connectivity_events.insert(std::pair<nsec, connectivity_event_type *>(ce->delta_time, ce));
							
							reachable_mobiles[mdit->first].erase(mdit2->first);
							if ( reachable_mobiles[mdit->first].empty() )
								reachable_mobiles.erase(mdit->first);
						}
					}
					else // pair not currently connected
					{
						if ( bw ) // connect with bw
						{
							ce = new connectivity_event_type;
							ce->connectivity_status = connectivity_status_start;
							ce->emitting_device_id = mdit->first;
							ce->receiving_device_id = mdit2->first;
							ce->delta_time = a->call_time - nrt_scheduler.start_time;
							ce->time = a->call_time;
							ce->bw = bw;
							ce->dly = 0; // fn of dist?
							ce->dlv = 0; // fn of ?
							ce->ber = 0; // fn of dist?
							ce->attn = attenuation;
							ce->dist = tx_distance;
							nemo_ptr->connectivity_scenario.connectivity_events.insert(std::pair<nsec, connectivity_event_type *>(ce->delta_time, ce));
							reachable_mobiles[mdit->first][mdit2->first] = ce; // stores latest ce
						}
						else // out of range
							;
					}
					mdit2++;
				}
				mdit++;
			}
			a->status = sched::finished;
		}
		else if ( a->handler == "mobility_event" )
		{
			// a mobility event occurs at t1 between previous measurement at t0 and future measurement at t2
			// should we compute it? better precision if sampling rate is low
			// update mobile node
			//mobility_event_type * me = a->doed->met_ptr();
			mobility_event_type * me = dynamic_cast<mobility_event_type *>(a->doed);
			mobile_device_type * md = nemo_ptr->mobile_devices[me->device_id];
			// compute new position at t1 for old params from t0 to t1
			nsec delta_time = me->time - md->last_time; // t1 - t0
			update_mobile_device(md, delta_time);
			// update params but dont compute from t1 to t2, it will be done at t2
			md->last_time = me->time;
			md->velocity = me->velocity;
			md->acceleration = me->acceleration;
			a->status = sched::finished;
		}
		else
		{
			nemo_ptr->error << "no handler for this action in mobility_scenario_type\n";
		}
	}

	void mobility_scenario_type::process(std::vector<std::string> & v)
	{
		if ( v.size() < 3 )
			return;
		nrt_scheduler.reset();
		double time_step_in_s = strtod(v[2].c_str(), 0);//1.0;
		dword sampling_nb = prost::round_to_l(nrt_scheduler.scenario_duration_in_s.count() / time_step_in_s);
		// schedule measurements
		for ( long i = 0; i <= sampling_nb; i++ )
		{
			sched::action_type * a = new sched::action_type;
			if ( medeb::active ) medeb::newm(a, sizeof(sched::action_type),__FILE__, __LINE__);
			a->description = "scheduled step for measuring network state";
			a->handler = "measurement_timer_expired";
			long long nsecd = i * time_step_in_s * 1E9;
			a->call_time = nrt_scheduler.start_time + nsec(nsecd);
			a->duration = nsec(0);
			a->start_time = a->call_time;
			a->stop_time = a->start_time + a->duration;
			//a->start_time = i * sampling_period;
			//a->duration = sampling_period; // XXX BIG PB : set stop = next start? or stop = next start - 1???
			a->doer = this;
			a->doed = 0;
			nrt_scheduler.schedule(a);
		}
		// enroll all mobility events
		std::multimap<nsec, mobility_event_type *>::iterator meit = mobility_events.begin();
		while ( meit != mobility_events.end() )
		{
			mobility_event_type * me = meit->second;
			sched::action_type * a = new sched::action_type;
			if ( medeb::active ) medeb::newm(a, sizeof(sched::action_type),__FILE__, __LINE__);
			a->description = "scheduled mobility event";
			a->handler = "mobility_event";
			a->call_time = me->time; // here is why time must be properly set!! and not only delta_time
			a->duration = nsec(0);
			a->start_time = a->call_time;
			a->stop_time = a->start_time + a->duration;
			a->doer = this;
			a->doed = me;
			nrt_scheduler.schedule(a);
			meit++;
		}
		nrt_scheduler.run(); // step by step
		// connectivity_events is ready to be parsed by rt
	}

	std::string connectivity_event_type::print_status()
	{
		if ( connectivity_status == connectivity_status_start )
			return "start";
		else if ( connectivity_status == connectivity_status_update )
			return "update";
		else
			return "stop";
	}

	void connectivity_event_type::set_status(std::string s)
	{
		if ( s == "start" )
			connectivity_status = connectivity_status_start;
		else if ( s == "update" )
			connectivity_status = connectivity_status_update;
		else
			connectivity_status = connectivity_status_stop;
	}

	// cnn gen <duration> <nb_of_nodes> <nb_of_events> <max_bw>
	void connectivity_scenario_type::generate(std::vector<std::string> & v) // mainly used for testing
	{
		connectivity_events.clear();
		double sd = strtod(v[2].c_str(), 0);
		nemo_ptr->mobility_scenario.nrt_scheduler.scenario_duration_in_s = fpsec(sd);
		rt_scheduler.scenario_duration_in_s = fpsec(sd);
		long long ns = static_cast<long long>(sd * 1E9);
		rt_scheduler.scenario_duration = nsec(ns);

		long nodes = strtol(v[3].c_str(), 0, 10);
		long events = strtol(v[4].c_str(), 0, 10);
		long mx_bw = strtol(v[5].c_str(), 0, 10);
		std::multimap<nsec, connectivity_event_type *>::iterator ceit = connectivity_events.begin();
		for ( long i = 0; i < events; i++ )
		{
			connectivity_event_type * ce = new connectivity_event_type;
			long time_s = prost::random_l(0, strtol(v[2].c_str(), 0, 10)); // granularity = second
			ce->delta_time = nsec(static_cast<long long>(1E9 * time_s));
			ce->emitting_device_id = prost::random_l(1, nodes);
			long mid2 = 0;
			while ( true )
			{
				mid2 = prost::random_l(1, nodes); // may take a lot of time!
				if ( ce->emitting_device_id != mid2 )
					break;
			}
			ce->receiving_device_id = mid2;
			ce->set_status("start"); // how to set a proper value! (store previous states?)
			ce->bw = prost::random_l(mx_bw / 10, mx_bw);
			ce->dly = 0; // fn of dist?
			ce->dlv = 0; // fn of ?
			ce->ber = 0; // fn of dist?
			ce->attn = 0;
			ce->dist = -1;
			connectivity_events.insert(std::pair<nsec, connectivity_event_type *>(ce->delta_time, ce));
		}
	}

	// cnn load file.cnn
	void connectivity_scenario_type::load(std::vector<std::string> & v)
	{
		if ( v.size() != 3 )
			return;
		if ( v[2].empty() )
			return;
		std::ifstream f(v[2].c_str());
		if ( !f )
		{
			nemo_ptr->error << "failed to open connectivity scenario file " << v[2] << "\n";
			return;
		}
		connectivity_events.clear();
		//nrt_scheduler.start_time = boost::chrono::high_resolution_clock::now(); // virtual epoch for nRT
		//nrt_scheduler.scenario_duration = nsec(0); // we don't know yet
		rt_scheduler.scenario_duration = nsec(0); // we don't know yet
		while ( !f.eof() )
		{
			std::string l;
			std::getline(f, l); // add error mgmt
			if ( !l.find("#") || !l.size() )
				continue;
			if ( nemo_ptr->debug )
				nemo_ptr->output << "> reading: " << l << "\n";
			std::istringstream iss(l);
			std::vector<std::string> tokens;
			while ( iss.good() )
			{
				std::string s;
				iss >> s;
				tokens.push_back(s);
			}
			if ( tokens.size() == 2 && tokens[1] == "EOS" )
			{
				double secs = strtod(tokens[0].c_str(), 0);
				//nrt_scheduler.scenario_duration_in_s = fpsec(secs);
				rt_scheduler.scenario_duration_in_s = fpsec(secs);
				long long ns = static_cast<long long>(secs * 1E9);
				//nrt_scheduler.scenario_duration = nsec(ns);
				rt_scheduler.scenario_duration = nsec(ns);
			}
			else if ( tokens.size() == 10 )
			{
				connectivity_event_type * ce = new connectivity_event_type;
				ce->delta_time = nsec(static_cast<long long>(1E9 * strtod(tokens[0].c_str() , 0)));
				ce->emitting_device_id = strtoul(tokens[1].c_str(), 0, 10);
				ce->receiving_device_id = strtoul(tokens[2].c_str(), 0, 10);
				ce->set_status(tokens[3]);
				ce->bw = strtoul(tokens[4].c_str(), 0, 10);
				ce->dly = strtod(tokens[5].c_str(), 0);
				ce->dlv = strtod(tokens[6].c_str(), 0);
				ce->ber = strtod(tokens[7].c_str(), 0);
				ce->attn = strtod(tokens[8].c_str(), 0);
				ce->dist = strtod(tokens[9].c_str(), 0);
				connectivity_events.insert(std::pair<nsec, connectivity_event_type *>(ce->delta_time, ce));
			}
			else
				nemo_ptr->output << "> could not process line, skipping it\n";
		}
		f.close();
		if ( rt_scheduler.scenario_duration == nsec(0) )
		{
			nemo_ptr->output << "> could not find EOS time, using last event time\n";
			rt_scheduler.scenario_duration = nemo_ptr->mobility_scenario.mobility_events.rbegin()->first;
			rt_scheduler.scenario_duration_in_s = fpsec(static_cast<double>(rt_scheduler.scenario_duration.count()) / 1E9);
		}
	}

	void connectivity_scenario_type::save(std::vector<std::string> & v)
	{
		if ( v.size() != 3 )
			return;
		if ( v[2].empty() )
			return;
		std::ofstream f(v[2].c_str());
		if ( !f )
		{
			nemo_ptr->error << "failed to create connectivity scenario file " << v[2] << "\n";
			return;
		}
		// dump connectivity events
		f << "# time_s 1st_mn_id 2nd_mn_id status bandwidth_bps delay_s delay_var_% ber_proba attenuation_dBm distance_m\n";
		std::multimap<nsec, connectivity_event_type *>::iterator ceit = connectivity_events.begin();
		while ( ceit != connectivity_events.end() )
		{
			connectivity_event_type * ce = ceit->second;
			f
//				<< print_nsec(ceit->first) << " "
				<< print_nsec(ce->delta_time) << " " 
//				<< fpsec(ce->delta_time).count() << " " 
				<< ce->emitting_device_id << " "
				<< ce->receiving_device_id << " "
				<< ce->print_status() << " "
				<< static_cast<dword>(ce->bw) << " "
				<< ce->dly << " "
				<< ce->dlv << " "
				<< ce->ber << " "
				<< ce->attn << " "
				<< ce->dist << "\n";
			ceit++;
		}
		// dump EOS
		f << print_nsec(nemo_ptr->mobility_scenario.nrt_scheduler.scenario_duration) << " EOS\n"; // pb avec le temps!!
		f.close();
	}

	void connectivity_scenario_type::execute(const boost::system::error_code & ec, sched::action_type * a)
	{
		rt_scheduler.timer_running = false;
		if ( ec ) // useless if sync
		{
			if ( ec == boost::asio::error::operation_aborted ) // could happen?
				nemo_ptr->error << "\nerror in execute (timer cancelled from async_wait): " << ec.value() << " = " << ec.message() << "\n";
			else
				nemo_ptr->error << "\nerror in execute (timer issue from async_wait): " << ec.value() << " = " << ec.message() << "\n";
			return;
		}
		if ( a->handler == "connectivity_event" )
		{
			// send the order to nemu at the proper time
			connectivity_event_type * ce = dynamic_cast<connectivity_event_type *>(a->doed);
			std::ostringstream oss;
			oss 
				//<< "[" << a->call_time.time_since_epoch() << "] " 
				<< "[" << print_nsec(ce->delta_time) << "] " 
				<< ce->emitting_device_id << " "
				<< ce->receiving_device_id << " "
				<< ce->print_status() << " "
				<< static_cast<dword>(ce->bw) << " "
				<< ce->dly << " "
				<< ce->dlv << " "
				<< ce->ber << " "
				<< ce->dist << "\n";
			oss.flush();
			std::string com = oss.str();
			nemo_ptr->itep = nemo_ptr->end_points.begin();
			while ( nemo_ptr->itep != nemo_ptr->end_points.end() )
			{
				nemo_ptr->itep->second->msg(com);
				nemo_ptr->itep++;
			}
			if ( nemo_ptr->debug )
				nemo_ptr->output << com << "\n";
			if ( medeb::active ) medeb::delm(a);
			delete a;
			rt_scheduler.schedule_next();
		}
		else if ( a->handler == "EOS_event" )
		{
			// send the order to nemu at the proper time
			//connectivity_event_type * ce = dynamic_cast<connectivity_event_type *>(a->doed);
			std::ostringstream oss;
			oss << "[" << print_nsec(rt_scheduler.scenario_duration) << "] EOS\n";
			oss.flush();
			std::string com = oss.str();
			nemo_ptr->itep = nemo_ptr->end_points.begin();
			while ( nemo_ptr->itep != nemo_ptr->end_points.end() )
			{
				nemo_ptr->itep->second->msg(com);
				nemo_ptr->itep++;
			}
			if ( nemo_ptr->debug )
				nemo_ptr->output << com << "\n";
			if ( medeb::active ) medeb::delm(a);
			delete a;
			rt_scheduler.schedule_next();
		}
		else
		{
			std::cout << "no handler for this action in connectivity_scenario_type\n";
		}
	}

	void connectivity_scenario_type::process()
	{
		if ( rt_scheduler.scenario_duration != nsec(0) )
			;
		else if ( nemo_ptr->mobility_scenario.nrt_scheduler.scenario_duration != nsec(0) )
			rt_scheduler.scenario_duration = nemo_ptr->mobility_scenario.nrt_scheduler.scenario_duration;
		else
			nemo_ptr->output << "scenario duration is undefined\n";
		rt_scheduler.reset();
		std::multimap<nsec, connectivity_event_type *>::iterator ceit = connectivity_events.begin();
		while ( ceit != connectivity_events.end() )
		{
			connectivity_event_type * ce = ceit->second;
			sched::action_type * a = new sched::action_type;
			if ( medeb::active ) medeb::newm(a, sizeof(sched::action_type),__FILE__, __LINE__);
			a->description = "scheduled connectivity event";
			a->handler = "connectivity_event";
			a->call_time = ceit->first + rt_scheduler.start_time;
			a->duration = nsec(0);
			a->start_time = a->call_time;
			a->stop_time = a->start_time + a->duration; 
			a->doer = this;
			a->doed = ce;
			rt_scheduler.schedule(a);
			ceit++;
		}
		// schedule EOS
		sched::action_type * a = new sched::action_type;
		if ( medeb::active ) medeb::newm(a, sizeof(sched::action_type),__FILE__, __LINE__);
		a->description = "scheduled EOS event";
		a->handler = "EOS_event";
		a->call_time = rt_scheduler.scenario_duration + rt_scheduler.start_time;
		a->duration = nsec(0);
		a->start_time = a->call_time;
		a->stop_time = a->start_time + a->duration; 
		a->doer = this;
		a->doed = 0;
		rt_scheduler.schedule(a);
		rt_scheduler.run(); // leap by leap with RT CLI
	}

	network_mobilizer::network_mobilizer(std::map<std::string, std::string> arg) :
		error(arg["-d"] + "nemo_"+arg["-p"]+"_error.log", mesap::console|mesap::logging, false),
		warning(arg["-d"] + "nemo_"+arg["-p"]+"_warning.log", mesap::console|mesap::logging, false),
		output(arg["-d"] + "nemo_"+arg["-p"]+"_output.log", mesap::console, false)
	{
		args = arg;

		port_seed = static_cast<word>(atol(arg["-p"].c_str()));
		//debug = false;
		debug = true;
		tcp_base_source_port = port_seed * 100;
		udp_base_source_port = port_seed * 100;
		tcp_resolver = 0;
		udp_resolver = 0;
		stran = 0;
		map.nemo_ptr = this;
		mobility_scenario.nemo_ptr = this;
		mobility_scenario.nrt_scheduler.error = &error;
		mobility_scenario.nrt_scheduler.warning = &warning;
		mobility_scenario.nrt_scheduler.output = &output;
		mobility_scenario.nrt_scheduler.scenario_duration = nsec(0);
		connectivity_scenario.nemo_ptr = this;
		connectivity_scenario.rt_scheduler.error = &error;
		connectivity_scenario.rt_scheduler.warning = &warning;
		connectivity_scenario.rt_scheduler.output = &output;
		connectivity_scenario.rt_scheduler.scenario_duration = nsec(0);
	}

	network_mobilizer::~network_mobilizer()
	{
		if ( tcp_resolver )
			delete tcp_resolver;
		if ( udp_resolver )
			delete udp_resolver;
		if ( stran )
			delete stran;
	}

	void network_mobilizer::create_frame(frame * f)
	{
	}

	void network_mobilizer::destroy_frame(frame * f)
	{
		f->ref_count--;
		if ( f->ref_count < 1 )
		{
			if ( medeb::active ) medeb::delm(f);
			delete f;
			f = 0;
		}
	}

	void network_mobilizer::load(std::string s)
	{
		if ( s.empty() )
			return;
		std::ifstream f(s.c_str());
		if ( !f )
		{
			error << "failed to open configuration file " << s << "\n";
			return;
		}
		while ( !f.eof() )
		{
			std::string l;
			std::getline(f, l); // add error mgmt
			if ( !l.find("#") || !l.size() || !l.find("exit") )//== std::string::npos ) // # and "exit" must be at beginning of line!
				continue;
			if ( debug )
				output << "> executing: " << l << "...\n";
			stran->post(boost::bind(&network_mobilizer::command, this, l)); // shall we post the command? is the post order preserved?
		}
		f.close();
	}

	void network_mobilizer::prompt()
	{
		output << "nemo% ";
	}

	void network_mobilizer::process()
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
		std::auto_ptr<boost::asio::io_service::work> work(new boost::asio::io_service::work(*io_serv));
		boost::thread t(boost::bind(&boost::asio::io_service::run, io_serv));
		if ( args.find("-f") != args.end() && args["-f"] != "" )
			load(args["-f"]);
		while ( true )
		{
			stran->post(boost::bind(&network_mobilizer::prompt, this));
			std::string cmd;
			std::getline(std::cin, cmd); // blocking call / main thread
			//if ( debug ) output << "> you typed: " << cmd << std::endl;
			if ( !cmd.find("exit") ) //!= std::string::npos ) // must be at the beginning of the line
			{
				break;
			}
			else if ( !cmd.find("load") ) //!= std::string::npos ) // as above
			{
				if ( cmd.size() > 5 )
					load(cmd.substr(cmd.find("load") + 5));
			}
			else
			{
				stran->post(boost::bind(&network_mobilizer::command, this, cmd));
				// shall we post the command? is the ordering preserved? if only one thread is used, it should be
			}
		}
	/*	std::map<std::string, end_point *>::iterator itm = end_points.begin();
		while ( itm != end_points.end() )
		{
			stran->post(boost::bind(&end_point::disc, itm->second)); // stran may not be necessary
			itm++;
		}
	*/	work.reset(); // finish async tasks
		t.join();
	}

	// conn <ep> <proto> {<laddr>|*} {<lport>|*} <raddr> <rport>
	void network_mobilizer::conn_endpoint(std::vector<std::string> & argus)
	{
		if ( end_points.find(argus[1]) == end_points.end() )
		{
			end_point * ep = 0;
			try
			{
				ep = new end_point;
				if ( medeb::active ) medeb::newm(ep, sizeof(end_point),__FILE__, __LINE__);
				ep->nemo_pt = this;
				ep->id = argus[1];
				ep->tran_proto = argus[2];
				ep->local_net_addr = argus[3];
				ep->local_port_nb = argus[4];
				ep->remote_net_addr = argus[5];
				ep->remote_port_nb = argus[6];
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

	// serv {raw|qemu} <ep> <proto> {<laddr>|*} {<lport>|*} {<raddr>|?} {<rport>|?}
	void network_mobilizer::serv_endpoint(std::vector<std::string> & argus)
	{
		if ( end_points.find(argus[2]) == end_points.end() )
		{
			end_point * ep = 0;
			try
			{
				ep = new end_point;
				if ( medeb::active ) medeb::newm(ep, sizeof(end_point),__FILE__, __LINE__);
				ep->nemo_pt = this;
				ep->server = true;
				ep->id = argus[1];
				ep->tran_proto = argus[2];
				ep->local_net_addr = argus[3];
				ep->local_port_nb = argus[4];
				ep->remote_net_addr = argus[5];
				ep->remote_port_nb = argus[6];
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

	void network_mobilizer::disc_endpoint(std::vector<std::string> & argus)
	{
		if ( end_points.find(argus[1]) != end_points.end() )
		{
			end_points[argus[1]]->disc();
			output << "\n> endpt ordered to close\n";
		}
		else
		{
			output << "> endpt does not exist\n";
		}
	}

	void network_mobilizer::msg(std::vector<std::string> & argus)
	{
		if ( end_points.find(argus[1]) != end_points.end() )
		{
			std::string message;
			for ( dword i = 2; i < argus.size(); i++ )
			{
				message += argus[i];
				if ( i < argus.size() - 1 )
					message += " ";
				else
					message += "\n";
			}
			//io_serv->post(boost::bind(&end_point::msg, end_points[argus[1]], message)); // message by copy
			end_points[argus[1]]->msg(message);
		}
		else
		{
			output << "> endpt does not exist\n";
		}
	}

	void network_mobilizer::help()
	{
		std::cout 
			<< copyright << "\n"
			<< version << "\n"
			<< "  map gen <map_type> <x_size> <y_size> <z_size> <space_step>\n"
			<< "  map {load|save} <map_file>\n"
			<< "  mob gen <duration> <nb_of_nodes> <nb_of_events> <max_speed> <max_accel>\n"
			<< "  mob {load|save} {nemo|ns2} <mob_filename>\n"
			<< "  mob node {<id>|all} {wic {aironet|xjack|wavelan}|txp <dBm>|margin <dBm>|cfreq <Hz>}\n"
			<< "  mob proc <time_step>\n"
			<< "  cnn gen <duration> <nb_of_nodes> <nb_of_events> <max_bw>\n"
			<< "  cnn {load|save} <cnn_filename>\n"
			<< "  cnn {start|pause|resume|stop|reset}\n"
			<< "  show {map|mob|cnn|ep}\n"
			<< "  seed {t|<seed>}\n"
			<< "  conn <ep> <proto> {<laddr>|*} {<lport>|*} <raddr> <rport>\n"
//			<< "  serv {raw|qemu} <ep> <proto> {<laddr>|*} {<lport>|*} {<raddr>|?} {<rport>|?}\n"
			<< "  disc <ep>\n"
			<< "  msg <ep> <txt>\n"
			<< "  debug {on|off}\n"
			<< "  load <command_file>\n"
			<< "  exit\n";
	}

	void network_mobilizer::show(std::vector<std::string> &v)
	{
		if ( v.size() < 2 )
			output << "missing arg (see help)\n";
		else if ( v[1] == "map" )
		{
			; // print map
		}
		else if ( v[1] == "mob" )
		{
			; // print mob scenario
		}
		else if ( v[1] == "cnn" )
		{
			; // print cnn scenario
		}
		else if ( v[1] == "ep" )
		{
			list(std::cout, "ep");
		}
		else if ( v[1] == "op" )
		{
			list(std::cout, "op");
		}
		else
			output << "unknown arg (see help)\n";
	}

	void network_mobilizer::list(std::ostream & os, std::string arg1)
	{
		if ( arg1 == "ep" )
		{
			os << "\nEndpoints\n\n";
			os << "endpoint                |type|prot|laddress       |lport|raddress       |rport|\n";
			os << "------------------------|----|----|---------------|-----|---------------|-----|\n";
			std::map<std::string, end_point *>::iterator it = end_points.begin();
			while ( it != end_points.end() )
			{
				os << std::setw(24) << std::left 
					<< (it->first.substr(0, 23)) << "|";
				if ( it->second->server )
					os << "serv|";
				else
					os << "clie|";
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
						os << std::setw(15) << std::right << (it->second->local_net_addr /*+ "!"*/) << "|";
						os << std::setw(5) << std::right << (it->second->local_port_nb + "") << "|";
						os << std::setw(15) << std::right << "n/a" << "|"; 
						os << std::setw(5) << std::right << "n/a" << "|";
					}
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
							os << std::setw(15) << std::right << (it->second->local_net_addr + "!") << "|";
							os << std::setw(5) << std::right << (it->second->local_port_nb + "") << "|";
							os << std::setw(15) << std::right << (it->second->remote_net_addr + "!") << "|";
							os << std::setw(5) << std::right << (it->second->remote_port_nb + "") << "|";
						}
					}
				}
				os << "\n";
				it++;
			}
			os << "------------------------|----|----|---------------|-----|---------------|-----|\n";
			os << "\n";
		}
		else if ( arg1 == "op" )
		{
			if ( debug )
				os << "debug on\n";
			else
				os << "debug off\n";
/*			if ( trace )
				os << "trace on\n";
			else
				os << "trace off\n";
*/			os << "-------------------------------------\n";
		}
		else
			;
	}

	void network_mobilizer::command(std::string s)
	{
		std::istringstream iss(s);
		std::vector<std::string> argus;
		while ( iss.good() )
		{
			std::string s;
			iss >> s;
			argus.push_back(s);
		}
		if ( argus[0] == "" )
		{
			//if ( debug ) output << "> missing command\n";
		}
		else if ( argus[0] == "map" )
		{
			if ( argus[1] == "gen" )
				map.generate(argus);
			else if ( argus[1] == "load" )
				map.load(argus);
			else if ( argus[1] == "save" )
				map.save(argus);
			else
				;
		}
		else if ( argus[0] == "mob" )
		{
			if ( argus[1] == "gen" )
				mobility_scenario.generate(argus);
			else if ( argus[1] == "load" )
				mobility_scenario.load(argus);
			else if ( argus[1] == "save" )
				mobility_scenario.save(argus);
			else if ( argus[1] == "node" )
				mobility_scenario.node(argus);
			else if ( argus[1] == "proc" )
			{
				if ( mobility_scenario.mobility_events.size() )
				{
					output << "nRT: starting scenario...\n";
					mobility_scenario.process(argus);
				}
				else
					output << "no mobility scenario\n";
			}
			else
				;
		}
		else if ( argus[0] == "cnn" )
		{
			if ( argus[1] == "gen" )
				connectivity_scenario.generate(argus);
			else if ( argus[1] == "load" )
				connectivity_scenario.load(argus);
			else if ( argus[1] == "save" )
				connectivity_scenario.save(argus);
			else if ( argus[1] == "start" )
			{
				if ( connectivity_scenario.connectivity_events.size() )
				{
					output << "RT: starting scenario...\n";
					connectivity_scenario.process();
				}
				else
					output << "no connectivity scenario\n";
			}
			else if ( argus[1] == "pause" )
			{
				connectivity_scenario.rt_scheduler.pause();
			}
			else if ( argus[1] == "resume" )
			{
				connectivity_scenario.rt_scheduler.resume();
			}
			else if ( argus[1] == "stop" )
			{
				connectivity_scenario.rt_scheduler.stop();
			}
			else if ( argus[1] == "reset" )
			{
				connectivity_scenario.rt_scheduler.reset();
			}
		}
		else if ( argus[0] == "seed" )
		{
			if ( argus[1] == "t" )
				mt19937::init_genrand(static_cast<unsigned long>(time(0)));
			else
				mt19937::init_genrand(strtoul(argus[1].c_str(), 0, 10));
		}
		else if ( argus[0] == "conn" )
		{
			conn_endpoint(argus);
		}
		else if ( argus[0] == "serv" )
		{
			serv_endpoint(argus);
		}
		else if ( argus[0] == "disc" )
		{
			disc_endpoint(argus);
		}
		else if ( argus[0] == "msg" )
		{
			msg(argus);
		}
		else if ( argus[0] == "show" )
		{
			show(argus);
		}
		else if ( argus[0] == "debug" )
		{
			if ( argus.size() == 2 )
			{
				if ( argus[1] == "on" )
				{
					debug = true;
				}
				else
				{
					debug = false;
				}
			}
		}
		else if ( argus[0] == "help" )
		{
			help();
		}
		else
			std::cout << "> unknown command: " << argus[0] << ", (see help for syntax)\n";
	}
}

int main(int argc, char * argv[])
{
	std::map<std::string, std::string> argvs;
	argvs["-f"] = ""; // command filename
	argvs["-d"] = "./"; // working dir where logs are put
	argvs["-s"] = "t"; // seed using current time
	argvs["-c"] = ""; // batch conversion from ns2 to nemo
	if ( argc > 2 && argc % 2 == 1 )
	{
		for ( int i = 1; i < argc; i = i + 2 )
		{
			std::string k = std::string(argv[i]), v = std::string(argv[i+1]);
			if ( argvs.find(k) != argvs.end() )
				argvs.erase(k);
			argvs[k] = v; // should we check key duplicates? not done for now
		}
		std::cout 
			<< nemo::copyright << "\n"
			<< nemo::version << "\n\n"
			<< "waiting for commands...\n";
	}
	else
	{
		std::cout 
			<< nemo::copyright << "\n"
			<< nemo::version << "\n\n"
			<< "syntax: nemo -s {t|<seed>} -f <command_file> -d <log_dir>\n"
			<< "using default args...\n";
	}
	if ( argvs["-s"] == "t" )
		mt19937::init_genrand(static_cast<unsigned long>(time(0)));
	else
		mt19937::init_genrand(strtoul(argvs["-s"].c_str(), 0, 10));
	int ps = prost::random_l(6000, 60000);
	std::ostringstream oss;
	oss << ps;
	std::string sps = oss.str();
	argvs["-p"] = sps;

	// place batch conversion from ns2 to nemo here
	if ( argvs["-c"] != "" )
	{
		nemo::network_mobilizer n(argvs);
		std::string fn = argvs["-c"];
		n.mobility_scenario.load_ns2(fn);
		std::string nfn = fn.substr(0, fn.find_last_of('.')) + ".mob";
		n.mobility_scenario.save_nemo(nfn);
		return EXIT_SUCCESS;
	}

	if ( medeb::active ) medeb::init();
	boost::asio::io_service io_service;
	nemo::network_mobilizer n(argvs);
	n.io_serv = &io_service;
	n.stran = new boost::asio::io_service::strand(*n.io_serv);
	n.connectivity_scenario.rt_scheduler.io_serv = &io_service;
	n.process();
	if ( medeb::active ) medeb::dump();
	return EXIT_SUCCESS;
}
