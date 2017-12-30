
// virtual network device (vnd)
// Copyright (C) 2012-2016 Damien Magoni

#ifndef VND_HPP
#define VND_HPP

#include "iface.hpp"

namespace vnd
{
	const std::string copyright = "vnd (c) 2012-2016 dm";
	const std::string version   = "version 0.5.6";

	typedef byte device_type;
	const device_type device_link = 1; // use link table
	const device_type device_hub = 2; // use bcast and collisions
	const device_type device_switch = 3; // use forwarding table
	const device_type device_access_point = 4; // wireless hub/wired bridge
	const device_type device_mobile_infra = 5; // mobile/roaming wireless station in infrastructure mode (BSS/ESS)
	const device_type device_mobile_adhoc = 6; // mobile/roaming wireless station in ad hoc mode (IBSS/MANet)

	class end_point;
	class line_interface;
	class address;

	class network_device
	{
	public:
		std::string name;
		std::map<std::string, std::string> args;
		mesap::messenger error, warning, output;
		device_type device;
		bool debug;
/*		bool trace_flag; // console
		trace_format_type trace_format;
		bool trace_file_flag;
		trace_format_type trace_file_format;
		std::ofstream trace_file;
*/
		word port_seed, tcp_base_source_port, udp_base_source_port;
		address address_id; // own @ of mobile node or AP
		address ap_address; // if mobile node, this stores current known AP @
		bool promiscuous_mode; // make sense in mobile mode!

		boost::asio::io_service * io_serv;
		boost::asio::io_service::strand * stran;
		boost::asio::ip::tcp::resolver * tcp_resolver;
		boost::asio::ip::udp::resolver * udp_resolver;
		std::map<std::string, end_point *> end_points;
		std::map<std::string, end_point *>::iterator itep;
		std::map<std::string, line_interface *> line_interfaces;
		std::map<std::string, line_interface *>::iterator itli;
		std::multimap<std::string, std::string> linking_table; // if1 -> if2, unidirectional/simplex!
		std::multimap<std::string, std::string>::iterator itlt;
		std::pair<std::multimap<std::string, std::string>::iterator, std::multimap<std::string, std::string>::iterator> retset;
		std::map<address, std::string> forwarding_table; // dst_addr, out_if
		std::map<address, std::string>::iterator itft;

		//std::map<std::string, std::string> binding_table; // ep <-> if, bidir/jection for now

		network_device(std::map<std::string, std::string> arg);
		~network_device();
		frame * create_frame();
		frame * copy_frame(frame * f);
		void destroy_frame(frame * f);
		void add_interface(std::vector<std::string> & v);
		void remv_interface(std::vector<std::string> & v);
		void up_interface(std::vector<std::string> & v);
		void down_interface(std::vector<std::string> & v);
		void set_interface(std::vector<std::string> & v);
		void unset_interface(std::vector<std::string> & v);
		void tie_interfaces(std::vector<std::string> & v);
		void untie_interfaces(std::vector<std::string> & v);
		void stat_interface(std::vector<std::string> & v);
		void unstat_interface(std::vector<std::string> & v);
		void trace_interface(std::vector<std::string> & v);
		void untrace_interface(std::vector<std::string> & v);
		void in_conn_endpoint(std::vector<std::string> & v);
		void in_serv_endpoint(std::vector<std::string> & v);
		void un_conn_endpoint(std::vector<std::string> & v);
		void un_serv_endpoint(std::vector<std::string> & v);
		void tap_endpoint(std::vector<std::string> & v);
		void vde_endpoint(std::vector<std::string> & v);
		void disc_endpoint(std::vector<std::string> & v);
		void par_endpoint(std::vector<std::string> & v);
		void unpar_endpoint(std::vector<std::string> & v);
		void bind_endpoint(std::vector<std::string> & v);
		void unbind_endpoint(std::vector<std::string> & v);
		encapsulation_type get_encapsulation(std::string mode);
		std::string print_encapsulation(encapsulation_type mode);
		std::string cli_data_to_string(std::vector<std::string> & argus);
		std::string print_interface_type(line_interface * li);
		std::string print_interface_protocol(line_interface * li);
		void snd(std::vector<std::string> & v);
		void inj(std::vector<std::string> & v);
		void show(std::vector<std::string> & v);
		void dump(std::vector<std::string> & v);
		void clear(std::vector<std::string> & v);
		void set(std::string k, std::string v);
		void list(std::ostream & os, std::string v);
		void stat(std::ostream & os, std::vector<std::string> & v);
		void format_stat(std::ostream & os, double d);
		void command(std::string s);
		void load(std::string s);
		void help();
		void run();
		void prompt();
		void forward(/*const boost::system::error_code & ec,*/ frame * f);
		void forward_link(frame * f);
		void forward_hub(frame * f, interface_type ifty);
		void forward_switch(frame * f, interface_type ifty);
		void forward_access_point(frame * f);
		void forward_mobile(frame * f);
	};

	class vnd_exception : public std::exception
	{
	public :
		std::string message;
		
		vnd_exception() throw();
		virtual ~vnd_exception() throw();
		vnd_exception(std::string s) throw();
		virtual const char * what() const throw();
	};
}

#endif
