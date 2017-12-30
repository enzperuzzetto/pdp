
// endpoint
// Copyright (C) 2012-2016 Damien Magoni

#ifndef ENDPOINT_HPP
#define ENDPOINT_HPP

#include <boost/thread/thread.hpp>
#include <boost/chrono.hpp>
#include <boost/bind.hpp>
#include <boost/regex.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include "mt19937.hpp"
#include "prost.hpp"
#include "medeb.hpp"
#include "mesap.hpp"
#ifdef VLIN
extern "C" 
{
	#include <linux/if_tun.h>
	#include <sys/socket.h>
	#include <libvdeplug.h>
}
#endif

typedef boost::chrono::nanoseconds nsec;
typedef boost::chrono::microseconds usec;
typedef boost::chrono::milliseconds msec;
typedef boost::chrono::seconds sec;
typedef boost::chrono::duration<double> fpsec;
typedef boost::chrono::high_resolution_clock::time_point time_type;
typedef boost::asio::basic_waitable_timer<boost::chrono::high_resolution_clock> high_resolution_timer_type;
typedef boost::chrono::process_user_cpu_clock::time_point rel_time_type;

namespace vnd
{
	const dword default_buffer_size =  2048UL; // bytes, should never be < max frame size = 1518
	const dword max_buffer_size     = 16384UL; //65536UL; // bytes

	class address
	{
	public:
		std::vector<byte> address_bytes;

		bool is_broadcast();
		bool is_null();
		std::string print() const;
		bool operator==(const address & rh) const;
		bool operator!=(const address & rh) const;
		bool operator<(const address & rh) const;
		bool is_equal(const address & rh) const;
		bool is_inferior(const address & rh) const;
	};

	// function toDS fromDS
	// IBSS     0    0
	// to AP    1    0
	// from AP  0    1
	// WDS br   1    1

    // warning: a frame object is shared by all its output copies!!!
	class frame
	{
	public:
		// meta information
		time_type arr_ts; // arrival (at the endpoint) timestamp
		time_type queue_ts_key; // pushed in queue timestamp
		rel_time_type tt1, tt2; // for bitrate throughput stats
		std::string input_interface;
		word ref_count; // number of identical frames ("copies")
		byte vli; // VLAN ID tag (only used in device between in and out)
		// frame itself
		std::vector<byte> buffer;
		// extracted fields
		address src, dst, bss; // AP @
		bool to_ds;
		bool from_ds;

		frame();
		~frame();
	};

	class line_interface;
	class network_device;

	typedef byte encapsulation_type;
	const encapsulation_type encapsulation_none = 1; // raw data
	const encapsulation_type encapsulation_length = 2; // 4 bytes (in network byte order) containing payload length, same as QEMU/Dynamips

	class end_point
	{
	public:
		network_device * vnd_pt;
		line_interface * bound_if;
		std::vector<byte> in_buf, out_buf, msg_buf; // buffers for async r/w calls
		dword in_buf_size, out_buf_size;
		dword total_bytes_to_write, total_bytes_to_read; // diff from buf_sizes
		bool read_overlap, write_overlap; // sanity check
		time_type current_frame_arrival;//, current_frame_departure;
		encapsulation_type encapsulation; // for TCP/length encaps
		byte in_lbuf[4], out_lbuf[4]; // for TCP/length encaps
		bool waiting_for_next_frame; // for TCP/length encaps
		dword next_frame_size; // for TCP/length encaps
		bool connected;
		bool server;
		std::string id;
		std::string tran_proto;
		std::string remote_net_addr, remote_port_nb, local_net_addr, local_port_nb;
		std::string path_name, remote_path_name, local_path_name;
		std::string tap_name;
		int tap_desc;
		std::string vde_switch, vde_port, vde_group, vde_mode;
		int vde_data_desc, vde_ctl_desc;
		boost::asio::ip::tcp::socket * tcp_socket; // client or service socket
		boost::asio::ip::tcp::acceptor * tcp_acc_socket; // server, only 1 connection authorized at any time
		boost::asio::ip::udp::socket * udp_socket;
		boost::asio::ip::udp::endpoint udp_remote_endpoint; // stores the remote address of the last received packet
		boost::asio::ssl::stream<boost::asio::ip::tcp::socket> * ssl_socket;
		boost::asio::ssl::context * ssl_ctx;
#ifdef VLIN
		boost::asio::local::stream_protocol::socket * stm_socket;
		boost::asio::local::stream_protocol::acceptor * stm_acc_socket;
		boost::asio::local::datagram_protocol::socket * dgm_socket;
		boost::asio::local::datagram_protocol::endpoint dgm_remote_endpoint; // stores the remote address of the last received packet
		boost::asio::posix::stream_descriptor * tap_psd;
		boost::asio::posix::stream_descriptor::native_handle_type native_tap_desc;
		boost::asio::local::datagram_protocol::socket * vde_data_socket;
		boost::asio::local::datagram_protocol::socket::native_handle_type native_vde_data_desc;
		boost::asio::local::datagram_protocol::socket * vde_ctl_socket;
		boost::asio::local::datagram_protocol::socket::native_handle_type native_vde_ctl_desc;
		VDECONN * vde_con;
#endif

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
		void conn();
		void handle_connect(const boost::system::error_code & ec);
		void handle_handshake(const boost::system::error_code & ec);
		void serv();
		void reserv();
		void handle_accept(const boost::system::error_code & ec);
		bool verify_certificate(bool preverified, boost::asio::ssl::verify_context& ctx);
		std::string get_password();
		void tap();
		void vde();
		void kill();
		void disc();
		void set(std::string k, std::string v);
		void read_loop(const boost::system::error_code & ec, bool short_circuit);
		void write_loop(const boost::system::error_code & ec, frame * f);
		void msg(std::string msg);
		void reread_loop();
		void rewrite_loop(frame * f);
		bool is_active();
		void manage_rw_error(const boost::system::error_code & ec, std::string s);
		void handle_read(const boost::system::error_code & ec, std::size_t bytes_transferred);
		void handle_write(const boost::system::error_code & ec, std::size_t bytes_transferred);
		void handle_vde_ctl(const boost::system::error_code & ec, std::size_t bytes_transferred);
		//void handle_send(const boost::system::error_code & ec, std::size_t bytes_transferred);
		//void handle_receive(const boost::system::error_code & ec, std::size_t bytes_transferred);
	};
}

#endif
