
// end_point
// Copyright (C) 2012 Damien Magoni. All rights reserved.

#if !defined(END_POINT_HPP)
#define END_POINT_HPP

#include <boost/thread/thread.hpp>
#include <boost/chrono.hpp>
#include <boost/bind.hpp>
#include <boost/regex.hpp>
#include <boost/asio.hpp>

#include "mt19937.h"
#include "prost.h"
#include "medeb.h"
#include "mesap.h"
#include "nemo.h"

typedef boost::chrono::nanoseconds nsec;
typedef boost::chrono::microseconds usec;
typedef boost::chrono::milliseconds msec;
typedef boost::chrono::seconds sec;
typedef boost::chrono::duration<double> fpsec;
typedef boost::chrono::high_resolution_clock::time_point time_type;
typedef boost::asio::basic_waitable_timer<boost::chrono::high_resolution_clock> high_resolution_timer_type;
typedef boost::chrono::process_user_cpu_clock::time_point rel_time_type;

const dword default_buffer_size = 16384UL; // bytes, should never be < max frame size = 1518
const dword max_buffer_size     = 65536UL; // bytes
const dword default_bw          = 100000000UL; // 100M bits per second
const dword max_bw              = 1000000000UL; // 1G bits per second
const dword nb_of_buffers       = 200;
const dword default_burst       = default_buffer_size * nb_of_buffers; // bytes
const dword max_burst           = max_buffer_size * nb_of_buffers; // bytes
const double max_dl             = 10.0; // s

namespace nemo
{
	class address
	{
	public:
		std::vector<byte> address_bytes;

		bool is_broadcast();
		std::string print() const;
		bool operator==(const address & rh) const;
		bool operator!=(const address & rh) const;
		bool operator<(const address & rh) const;
		bool is_equal(const address & rh) const;
		bool is_inferior(const address & rh) const;
	};

	class frame
	{
	public:
		time_type arr_ts; // arrival (at the endpoint) timestamp
		time_type queue_ts_key; // pushed in queue timestamp
		rel_time_type tt1, tt2; // for bitrate throughput stats
		word size;
		word ref_count;
		std::vector<byte> buffer;
		address src, dst;
		std::string input_interface;

		frame();
		~frame();
	};

	class network_mobilizer;

	class end_point
	{
	public:
		bool server;
		std::string id;
		std::string tran_proto, remote_net_addr, remote_port_nb, local_net_addr, local_port_nb;
		network_mobilizer * nemo_pt;
		dword in_buf_size, out_buf_size;
		std::vector<byte> in_buf, out_buf; // buffers for async r/w calls
		byte in_lbuf[4], out_lbuf[4];
		dword total_bytes_to_write, total_bytes_to_read;
		time_type current_frame_arrival;//, current_frame_departure;
		//rel_time_type current_frame_arrival;
		address last_known_address; // for QEMU/dynamips
		dword next_frame_size; // for TCP
		bool waiting_for_next_frame; // for TCP
		bool read_overlap, write_overlap; // sanity check
		boost::asio::ip::tcp::socket * tcp_socket; // client or service socket
		boost::asio::ip::tcp::acceptor * tcp_acc_socket; // server, only 1 connection authorized at any time
		boost::asio::ip::udp::socket * udp_socket;
		boost::asio::ip::udp::endpoint udp_remote_endpoint; // stores the remote address of the last received packet

		end_point();
		~end_point();
		dword n_to_h_l(dword netl);
		dword h_to_n_l(dword hostl);
		dword bytes_to_dword(byte b[]);
		void dword_to_bytes(dword d, byte b[]);
		bool is_fqdn(std::string & s);
		boost::asio::ip::tcp::endpoint make_tcp_local_endpoint();
		boost::asio::ip::tcp::endpoint make_tcp_remote_endpoint();
		boost::asio::ip::tcp::endpoint make_tcp_endpoint(std::string net_addr, std::string port_nb);
		boost::asio::ip::udp::endpoint make_udp_local_endpoint();
		boost::asio::ip::udp::endpoint make_udp_remote_endpoint();
		boost::asio::ip::udp::endpoint make_udp_endpoint(std::string net_addr, std::string port_nb);
		void print_ascii(std::vector<byte> v);
		void print_hexa(std::vector<byte> v);
		void conn();
		void handle_connect(const boost::system::error_code & error);
		void serv();
		void handle_accept(const boost::system::error_code & ec);
		void kill();
		void disc();
		void set(std::string k, std::string v);
		void read_loop(const boost::system::error_code & ec, bool short_circuit);
		void write_loop(const boost::system::error_code & ec, frame * f);
		void msg(std::string msg);
		void manage_async_error(const boost::system::error_code & ec, std::string s);
		void handle_read(const boost::system::error_code & ec, std::size_t bytes_transferred);
		void handle_write(const boost::system::error_code & ec, std::size_t bytes_transferred);
		//void handle_send(const boost::system::error_code & ec, std::size_t bytes_transferred);
		//void handle_receive(const boost::system::error_code & ec, std::size_t bytes_transferred);
	};
}

#endif
