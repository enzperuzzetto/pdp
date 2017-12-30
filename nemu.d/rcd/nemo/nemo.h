
// nemo
// Copyright (c) 2012-2016 Damien Magoni

#if !defined(NEMO_H)
#define NEMO_H

#include "sched.h"
#include "end_point.h"

namespace nemo
{
	const std::string copyright = "Copyright (c) 2012-2016 Damien Magoni";
	const std::string version   = "Version 0.2.4";

	//typedef long long unit;
	typedef double unit;

	class position_type
	{
	public:
		unit x, y, z;
	};
	class velocity_type
	{
	public:
		unit x, y, z;
	};
	class acceleration_type
	{
	public:
		unit x, y, z;
	};
	class jerk_type
	{
	public:
		unit x, y, z;
	};

	class voxel_type
	{
	public:
		unit x, y, z;
		byte attenuation;
	};

/*
	file formats for maps

	# setting = flat
	# name x_size y_size z_size space_step
	# mapflat xs ys zs ss

	# setting = surface
	# name x_size y_size z_size space_step
	# mapsurf xs ys zs ss
	#
	# x1 y1 z1 a1
	# x2 y2 z2 a2
	# etc

	# setting = surface
	# name x_size y_size z_size space_step
	# mapspace xs ys zs ss
	# a1(0,0,0)
	# a2(0,0,1)
	# etc
*/

	typedef byte map_setting_type;
	const map_setting_type map_flat = 1; // z = 0, attn = 0
	const map_setting_type map_surface = 2; // only one point z1 in z for each (x, y), attn = a(x, y) < 0 for z <= z1 
	const map_setting_type map_space = 3; // one or more points zi in z for each (x, y), attn = a(x, y, z) <= 0

	class network_mobilizer;

	class map_type
	{
	public:
		network_mobilizer * nemo_ptr;
		map_setting_type map_setting;
		unit x_size, y_size, z_size;
		double space_step_in_m; // 1 = 1 meter
		typedef std::vector<unit> line;
		typedef std::vector<line> plane;
		std::vector<plane> space;

		void generate(std::vector<std::string> & v);
		void load(std::vector<std::string> & v);
		void save(std::vector<std::string> & v);
	};

	typedef byte wic_type;
	const wic_type wic_cisco_aironet = 1;
	const wic_type wic_3com_xjack = 2;
	const wic_type wic_lucent_wavelan = 3;

	const double default_tx_power = 20.0;
	const double default_margin = 10.0;
	const double default_carrier_freq = 2.4897E9;
	
	class mobile_device_type
	{
	public:
		dword id;
		wic_type wic;
		double tx_power; // in dBm
		double margin; // in dBm
		double carrier_freq; // in Hz

		position_type position;
		velocity_type velocity;
		acceleration_type acceleration;
		//jerk_type jerk;
		nsec delta_time; // relative to start time
		time_type last_time; // = start_time + delta_time
		
		// for ns2 import
		nsec ns_curr_time, ns_dest_time;
		position_type ns_curr_position, ns_dest_position;
		velocity_type ns_curr_velocity;
		acceleration_type ns_curr_acceleration;

		void set(std::string par, std::string val);
	};

	class mobility_event_type : public sched::acted
	{
	public:
		nsec delta_time; // relative to start time
		time_type time; // = start_time + delta_time
		dword device_id;
		velocity_type velocity;
		acceleration_type acceleration;
		//jerk_type jerk;
		//nsec duration; // or use another event to stop/modify current event? yes
	};

	typedef byte connectivity_status_type;
	const connectivity_status_type connectivity_status_start = 1; 
	const connectivity_status_type connectivity_status_update = 2; 
	const connectivity_status_type connectivity_status_stop = 3; 

	class connectivity_event_type : public sched::acted
	{
	public:
		nsec delta_time; // relative to start time
		time_type time; // = start_time + delta_time
		dword emitting_device_id;
		dword receiving_device_id;
		connectivity_status_type connectivity_status;
		double bw, dly, dlv, ber, dist, attn;

		std::string print_status();
		void set_status(std::string s);
	};

	class mobility_scenario_type : public sched::actor
	{
	public:
		network_mobilizer * nemo_ptr;
		sched::nrt_scheduler_type nrt_scheduler;
		std::multimap<nsec, mobility_event_type *> mobility_events;
		std::map<dword, mobile_device_type *>::iterator mdit, mdit2;
		typedef std::map<dword, connectivity_event_type *> reachable_mobile_type; // list storing all devices able to receive from (connected to) a given device with latest ce
		reachable_mobile_type::iterator rmtit;
		std::map<dword, reachable_mobile_type> reachable_mobiles; // stores the connectivity list of all devices
		std::map<dword, reachable_mobile_type>::iterator rmit;

		void generate(std::vector<std::string> & v);
		void load(std::vector<std::string> & v);
		void load_nemo(std::string & fn);
		void load_ns2(std::string & fn);
		dword get_node_id(std::string t);
		void calculate_velocity(mobile_device_type * m, double final_x, double final_y, double modv, nsec st);
		void save(std::vector<std::string> & v);
		void save_nemo(std::string & fn);
		void node(std::vector<std::string> & v);
		void process(std::vector<std::string> & v); // create connectivity events through nrts
		void execute(sched::action_type * a);
		double distance(position_type p1, position_type p2);
		double bitrate(double rx_level_in_dBm, wic_type wt);
		connectivity_event_type * are_connected(dword md1, dword md2);
		void update_mobile_device(mobile_device_type * md, nsec delta_time);
	};

	class connectivity_scenario_type : public sched::actor
	{
	public:
		network_mobilizer * nemo_ptr;
		sched::rt_scheduler_type rt_scheduler;
		std::multimap<nsec, connectivity_event_type *> connectivity_events;

		void generate(std::vector<std::string> & v);
		void load(std::vector<std::string> & v);
		void save(std::vector<std::string> & v);
		void process(); // send orders to vnd through rts
		void execute(const boost::system::error_code & ec, sched::action_type * a);
	};

	class end_point;
	class frame;

	class network_mobilizer
	{
	public:
		std::string name;
		std::map<std::string, std::string> args;
		boost::asio::io_service * io_serv;
		boost::asio::io_service::strand * stran;
		bool debug;
		map_type map;
		std::map<dword, mobile_device_type *> mobile_devices;
		mobility_scenario_type mobility_scenario;
		connectivity_scenario_type connectivity_scenario;
		mesap::messenger error, warning, output;
		std::map<std::string, end_point *> end_points;
		std::map<std::string, end_point *>::iterator itep;
		word port_seed, tcp_base_source_port, udp_base_source_port;
		boost::asio::ip::tcp::resolver * tcp_resolver;
		boost::asio::ip::udp::resolver * udp_resolver;

		network_mobilizer(std::map<std::string, std::string> arg);
		~network_mobilizer();
		void create_frame(frame * f);
		void destroy_frame(frame * f);
		void process();
		void conn_endpoint(std::vector<std::string> & v);
		void serv_endpoint(std::vector<std::string> & v);
		void disc_endpoint(std::vector<std::string> & v);
		void msg(std::vector<std::string> & v);
		void show(std::vector<std::string> & v);
		void list(std::ostream & os, std::string arg1);
		void command(std::string s);
		void load(std::string s);
		void help();
		//void run();
		void prompt();
	};
}

#endif

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

void DmpAgent::set_range()
{
	double power = transmission_power - margin_ - rx_thr_;
	// power to distance formulae
	transmission_range = pow(10, 0.05 * (power - 20 * log10(4 * acos(-1) / 300000000) - 20 * log10(2.4897 * 1000000000)));
	// TO COMMENT LATER
	//transmission_range = range_max_;
}
void DmpAgent::set_power()
{
	energy_used = energy_used + pow(10, transmission_power/10) * (Scheduler::instance().clock() - transmission_power_change_time) / 1000;
	transmission_power_change_time = Scheduler::instance().clock();
	// distance to power formulae
	double attenuation = 20 * log10(4 * acos(-1) / 300000000) + 20 * log10(2.4897*1000000000) + 20 * log10(transmission_range);
	transmission_power = attenuation + margin_ + rx_thr_;
	if ( transmission_power > tx_max_ )
       		transmission_power = tx_max_;
}
*/
