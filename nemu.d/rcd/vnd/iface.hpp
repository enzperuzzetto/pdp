
// iface
// Copyright (C) 2012-2016 Damien Magoni

#ifndef IFACE_HPP
#define IFACE_HPP

#include "endpoint.hpp"

namespace vnd
{
	const byte max_vli              = 128; // 1 -> 128
	const qword default_bw          = 10000000000ULL; // 10G bits per second
	const qword max_bw              = 100000000000ULL; // 100G bits per second
	const dword default_queue_size  = 256; // number of frames/packets (unitless)
	const dword max_queue_size      = 16384;
	//const dword default_burst       = default_buffer_size * default_queue_size; // bytes
	//const dword max_burst           = max_buffer_size * max_queue_size; // bytes
	const double max_dl             = 100.0; // s
	const double max_dv             = 100.0; // %

	class frame;
	class line_interface;

	class hold_queue
	{
	public:
		high_resolution_timer_type * timer;
		bool timer_running;
		fpsec activity_duration;
		//std::queue<time_type> frames_tss; // frames timestamps
		time_type epoch_time, last_unstat_time, last_frame_time;
		std::time_t unix_epoch_time;
		bool up, input; // is an input queue
		byte vli; // VLAN ID
		qword bw; // bits per s
		double dl; // delay in s
		double dv; // delay variation in % (aka jitter)
		double ber; // bit error rate in 0->1 probability
		dword queue_size; // number of packets (unitless)
		dword buffer_size; // currently set equal to endpoint buffer size but need not be
		dword burst, excess_burst;
		dword frames_counter;
		dword bytes_counter;
		dword lost_frames_counter;
		dword lost_bytes_counter;
		std::map<time_type, frame *> buffer_queue;
		line_interface * line_pt;

		hold_queue(boost::asio::io_service * io_serv);
		~hold_queue();
		dword byte_count();
		bool compute_ber(frame * f);
		void discard_frame(frame * f);
		void fast_read_loop();
        void handle_wait(const boost::system::error_code & ec, frame * f);
        void wait(frame * f);
        void preprocess(frame * f);
		void push(frame * f);
		void postprocess(frame * f);
		void schedule();
		void set(std::string k, std::string v);
		void set_buffer(dword bs);
		void compute_stats(std::map<std::string, double> * s);
		void reset_stats();
		std::string print_id();
	};

	class end_point;
	class network_device;
	class hold_queue;

	typedef byte interface_type;
	const interface_type interface_nic = 1; // wired network interface card (Ethernet)
	const interface_type interface_wic = 2; // wireless interface card (802.11)
	const interface_type interface_ral = 3; // radio link to wireless neighbor

	typedef byte protocol_type;
	const protocol_type protocol_none = 1; // raw data, no interpretation
	const protocol_type protocol_ethernet = 2; // Ethernet II / DIX
	const protocol_type protocol_pseudo_802_11 = 3; // pseudo 802.11
	const protocol_type protocol_802_11 = 4; // IEEE 802.11
	const protocol_type protocol_ip_v4 = 5; // IPv4

	typedef byte trace_format_type;
	const trace_format_type trace_format_text = 1; // readable text format
	const trace_format_type trace_format_hexa = 2; // readable hexa format
	const trace_format_type trace_format_pcap = 3; // wireshark pcap format

	class line_interface
	{
	public:
		std::string id;
		address ha; // hardware address
		interface_type interface_family;
		protocol_type protocol;
		address last_known_address; // for L2/L3
		bool trace_flag; // console
		trace_format_type trace_format;
		bool trace_file_flag;
		trace_format_type trace_file_format;
		std::ofstream trace_file;
		end_point * bound_ep; // bijection for now
		network_device * vnd_pt;
		hold_queue * input_queue;
		hold_queue * output_queue;

		line_interface(boost::asio::io_service * io_serv);
		~line_interface();
		void trace(frame * f, std::size_t bytes_transferred);
		void kill();
	};
}

#endif
