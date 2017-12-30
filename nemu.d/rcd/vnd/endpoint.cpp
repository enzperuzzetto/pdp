
// endpoint
// Copyright (C) 2012-2016 Damien Magoni

#include "vnd.hpp"

namespace vnd
{
	end_point::end_point()
	{
		server = false;
		connected = false;
		read_overlap = false;
		write_overlap = false;
		waiting_for_next_frame = false;
		total_bytes_to_write = 0;
		tcp_socket = 0;
		tcp_acc_socket = 0;
		udp_socket = 0;
		ssl_socket = 0;
		ssl_ctx = 0;
#ifdef VLIN
		stm_socket = 0;
		stm_acc_socket = 0;
		dgm_socket = 0;
		dgm_remote_endpoint.path(""); // unused
		tap_psd = 0;
		native_tap_desc = -1;
		tap_desc = -1;
		vde_data_socket = 0;
		native_vde_data_desc = -1;
		vde_data_desc = -1;
		vde_ctl_socket = 0;
		native_vde_ctl_desc = -1;
		vde_ctl_desc = -1;
		vde_con = 0;
#endif
		vde_switch = "";
		vde_port = "0";
		vde_group = "";
		vde_mode = "0";
		bound_if = 0;
		vnd_pt = 0;
		in_buf_size = default_buffer_size;
		out_buf_size = default_buffer_size;
		in_buf.resize(in_buf_size);
		in_buf.assign(in_buf_size, 0x00);
		out_buf.resize(out_buf_size);
		out_buf.assign(out_buf_size, 0x00);
		udp_remote_endpoint.address(boost::asio::ip::address::from_string("127.0.0.2")); // unused
		udp_remote_endpoint.port(4); // unassigned
	}

	end_point::~end_point()
	{
	}

	dword end_point::n_to_h_l(dword netl)
	{
		return 0;//ntohl(netl);
	}

	dword end_point::h_to_n_l(dword hostl)
	{
		return 0;//htonl(hostl);
	}

	void end_point::dword_to_bytes(dword d, byte b[]) // d is MSB first for network format
	{
		for ( int i = 0; i < 4; i++ )
		{
			dword dw = d >> ((3 - i) * 8);
			byte by = static_cast<byte>(dw);
			b[i] = by;
			//b[i] = static_cast<byte>((d >> ((3 - i) * 8)) && 0x000000FF);
		}
	}

	dword end_point::bytes_to_dword(byte b[]) // b is MSB first for network format
	{
		dword d = 0;
		for ( int i = 0; i < 4; i++ )
		{
			d |= (b[i] << ((3 - i) * 8));
		}
		return d;
	}

	//ip_address_regex = "^(([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])\.){3}([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])$";
	//hostname_regex = "^(([a-zA-Z]|[a-zA-Z][a-zA-Z0-9\-]*[a-zA-Z0-9])\.)*([A-Za-z]|[A-Za-z][A-Za-z0-9\-]*[A-Za-z0-9])$";
	//hostname_regex_sized = "^([a-zA-Z0-9]|[a-zA-Z0-9][a-zA-Z0-9\-]{0,61}[a-zA-Z0-9])(\.([a-zA-Z0-9]|[a-zA-Z0-9][a-zA-Z0-9\-]{0,61}[a-zA-Z0-9]))*$";

	bool end_point::is_fqdn(std::string & s)
	{
		static const boost::regex e("^([a-zA-Z0-9]|[a-zA-Z0-9][a-zA-Z0-9\\-]{0,61}[a-zA-Z0-9])(\\.([a-zA-Z0-9]|[a-zA-Z0-9][a-zA-Z0-9\\-]{0,61}[a-zA-Z0-9]))*$");
		return regex_match(s, e);
	}

	// when connected on socket use: local_endpoint() / remote_endpoint()

	boost::asio::ip::udp::endpoint end_point::make_udp_local_endpoint() // never call more than once!
	{
		std::string port_nb;
		if ( local_port_nb == "*" )
		{
			std::ostringstream oss;
			oss << vnd_pt->udp_base_source_port++;
			port_nb = oss.str();
			local_port_nb = port_nb;
		}
		else
			port_nb = local_port_nb;
		return make_udp_endpoint(local_net_addr, port_nb);
	}

	boost::asio::ip::udp::endpoint end_point::make_udp_remote_endpoint()
	{
		return make_udp_endpoint(remote_net_addr, remote_port_nb);
	}

	boost::asio::ip::udp::endpoint end_point::make_udp_endpoint(std::string net_addr, std::string port_nb)
	{
		try
		{
			boost::asio::ip::udp::resolver::iterator endpoint_iterator;
			if ( vnd_pt->udp_resolver && ( is_fqdn(net_addr) || net_addr == "*" ) )
			{
				if ( net_addr == "*" )
				{
					net_addr = boost::asio::ip::host_name(); // returns computer name on windows!
					if ( vnd_pt->debug )
						vnd_pt->output << "> host_name() returned: " << net_addr << "\n";
					net_addr = "localhost";
				}
				boost::asio::ip::udp::resolver::query query(net_addr, port_nb); // why protocol dependent?
				boost::asio::ip::udp::resolver::iterator endpoint_iterator = vnd_pt->udp_resolver->resolve(query);
				// no async_resolve, only use first answer
				// *endpoint_iterator++
				return *endpoint_iterator;
			}
			else
			{
				boost::asio::ip::udp::endpoint endpoint;
				endpoint.address(boost::asio::ip::address::from_string(net_addr.c_str()));
				endpoint.port(atoi(port_nb.c_str()));
				return endpoint;
			}
		}
		catch (std::exception & e)
		{
			vnd_pt->error << "exception> in make_udp_endpoint: " <<  e.what() << "\n";
			//throw;
			boost::asio::ip::udp::endpoint endpoint;
			endpoint.address(boost::asio::ip::address::from_string("0.0.0.0"));
			endpoint.port(atoi("0"));
			return endpoint;
		}
	}

	boost::asio::ip::tcp::endpoint end_point::make_tcp_local_endpoint() // never call more than once!
	{
		std::string port_nb;
		if ( local_port_nb == "*" )
		{
			std::ostringstream oss;
			oss << vnd_pt->tcp_base_source_port++;
			port_nb = oss.str();
			local_port_nb = port_nb;
		}
		else
			port_nb = local_port_nb;
		return make_tcp_endpoint(local_net_addr, port_nb);
	}

	boost::asio::ip::tcp::endpoint end_point::make_tcp_remote_endpoint()
	{
		return make_tcp_endpoint(remote_net_addr, remote_port_nb);
	}

	boost::asio::ip::tcp::endpoint end_point::make_tcp_endpoint(std::string net_addr, std::string port_nb)
	{
		try
		{
			boost::asio::ip::tcp::resolver::iterator endpoint_iterator;
			if ( vnd_pt->tcp_resolver && ( is_fqdn(net_addr) || net_addr == "*" ) )
			{
				if ( net_addr == "*" )
				{
					net_addr = boost::asio::ip::host_name(); // returns computer name on windows!
					if ( vnd_pt->debug )
						vnd_pt->output << "> host_name() returned: " << net_addr << "\n";
					net_addr = "localhost";
				}
				boost::asio::ip::tcp::resolver::query query(net_addr, port_nb); // why protocol dependent?
				boost::asio::ip::tcp::resolver::iterator endpoint_iterator = vnd_pt->tcp_resolver->resolve(query);
				// no async_resolve, only use first answer
				// *endpoint_iterator++
				return *endpoint_iterator;
			}
			else if ( net_addr == "*" ) // use INADDR_ANY
			{
				return boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), static_cast<word>(atoi(port_nb.c_str())));
			}
			else
			{
				return boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(net_addr.c_str()), atoi(port_nb.c_str()));
			}
		}
		catch (std::exception & e)
		{
			vnd_pt->error << "exception> in make_tcp_endpoint: " <<  e.what() << "\n";
			//throw;
			return boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string("0.0.0.0"), atoi("0"));
		}
	}

	// incn <ep> {raw|len} {tcp|udp} {<laddr>|*} {<lport>|*} <raddr> <rport>
	void end_point::conn()
	{
		try
		{
			if ( tran_proto == "stm" )
			{
#ifdef VLIN
				stm_socket = new boost::asio::local::stream_protocol::socket(*vnd_pt->io_serv);
				if ( medeb::active ) medeb::newm(stm_socket, sizeof(boost::asio::local::stream_protocol::socket),__FILE__, __LINE__);
				stm_socket->open();//boost::asio::local::stream_protocol::endpoint(remote_path_name).protocol());
				stm_socket->connect(boost::asio::local::stream_protocol::endpoint(remote_path_name));
				//tcp_socket->set_option(boost::asio::socket_base::keep_alive(true));
				//tcp_socket->set_option(boost::asio::socket_base::linger(false, 0));
				//tcp_socket->set_option(boost::asio::socket_base::receive_buffer_size(in_buf_size));
				//tcp_socket->set_option(boost::asio::socket_base::send_buffer_size(out_buf_size));
				connected = true;
				boost::system::error_code ec;
				read_loop(ec, true);
#endif
			}
			else if ( tran_proto == "dgm" )
			{
#ifdef VLIN
				std::fstream f(local_path_name.c_str(), std::ios_base::trunc|std::ios_base::binary);
				f.close(); // creates the file but why: for unlink to succeed?
				::unlink(local_path_name.c_str()); // dgm socket client side unique pathname
				dgm_socket = new boost::asio::local::datagram_protocol::socket(*vnd_pt->io_serv);
				if ( medeb::active ) medeb::newm(udp_socket, sizeof(boost::asio::local::datagram_protocol::socket),__FILE__, __LINE__);
				dgm_socket->open();
				dgm_socket->bind(boost::asio::local::datagram_protocol::endpoint(local_path_name));
				dgm_remote_endpoint = boost::asio::local::datagram_protocol::endpoint(remote_path_name);
				dgm_socket->connect(dgm_remote_endpoint); // does not bind the path_name, shall we use this dgm_ep?
				//udp_socket->set_option(boost::asio::socket_base::receive_buffer_size(in_buf_size));
				//udp_socket->set_option(boost::asio::socket_base::send_buffer_size(out_buf_size));
				boost::system::error_code ec;
				read_loop(ec, true);
#endif
			}
			else if ( tran_proto == "tcp" )
			{
				tcp_socket = new boost::asio::ip::tcp::socket(*vnd_pt->io_serv);
				if ( medeb::active ) medeb::newm(tcp_socket, sizeof(boost::asio::ip::tcp::socket),__FILE__, __LINE__);
				tcp_socket->open(boost::asio::ip::tcp::v4()); // protocol dependent!
				if ( local_net_addr != "*" || local_port_nb != "*" ) // if * * -> no bind
					tcp_socket->bind(make_tcp_local_endpoint());
				tcp_socket->connect(make_tcp_remote_endpoint());
				tcp_socket->set_option(boost::asio::socket_base::keep_alive(true));
				tcp_socket->set_option(boost::asio::socket_base::linger(false, 0));
				//tcp_socket->set_option(boost::asio::socket_base::receive_buffer_size(in_buf_size));
				//tcp_socket->set_option(boost::asio::socket_base::send_buffer_size(out_buf_size));
				tcp_socket->set_option(boost::asio::ip::tcp::no_delay(true));
				connected = true;
				boost::system::error_code ec;
				read_loop(ec, true);
			}
			else if ( tran_proto == "udp" )
			{
				udp_socket = new boost::asio::ip::udp::socket(*vnd_pt->io_serv);
				if ( medeb::active ) medeb::newm(udp_socket, sizeof(boost::asio::ip::udp::socket),__FILE__, __LINE__);
				udp_socket->open(boost::asio::ip::udp::v4()); // protocol dependent!
				if ( local_net_addr != "*" || local_port_nb != "*" )
					udp_socket->bind(make_udp_local_endpoint());
				udp_socket->connect(make_udp_remote_endpoint());
				//udp_socket->set_option(boost::asio::socket_base::receive_buffer_size(in_buf_size));
				//udp_socket->set_option(boost::asio::socket_base::send_buffer_size(out_buf_size));
				boost::system::error_code ec;
				read_loop(ec, true);
			}
			else if ( tran_proto == "ssl" )
			{
				ssl_ctx = new boost::asio::ssl::context(boost::asio::ssl::context::sslv23);
				ssl_ctx->load_verify_file("ca.pem");
				ssl_socket = new boost::asio::ssl::stream<boost::asio::ip::tcp::socket>(*vnd_pt->io_serv, *ssl_ctx);
				if ( medeb::active ) medeb::newm(ssl_socket, sizeof(boost::asio::ssl::stream<boost::asio::ip::tcp::socket>),__FILE__, __LINE__);

				ssl_socket->set_verify_mode(boost::asio::ssl::verify_peer);
				ssl_socket->set_verify_callback(boost::bind(&end_point::verify_certificate, this, _1, _2));

				boost::asio::ip::tcp::resolver::query qu(remote_net_addr.c_str(), remote_port_nb.c_str());
				boost::asio::ip::tcp::resolver::iterator it = vnd_pt->tcp_resolver->resolve(qu);
				//boost::asio::connect(ssl_socket->lowest_layer(), it);//make_tcp_remote_endpoint(),
				//	boost::bind(&end_point::handle_connect, this, boost::asio::placeholders::error));
				boost::asio::async_connect(ssl_socket->lowest_layer(), it, //make_tcp_remote_endpoint(),
					boost::bind(&end_point::handle_connect, this, boost::asio::placeholders::error));
			}
			else
			{
				vnd_pt->error << "error> in conn: unknown transport protocol\n";
			}
		}
		catch (std::exception & e)
		{
			vnd_pt->error << "exception> in conn: " <<  e.what() << "\n";
			//throw;
		}
	}

	void end_point::handle_connect(const boost::system::error_code & ec) // only useful for async_connect
	{
		if ( !ec ) // connect succeeded
		{
			if ( tran_proto == "tcp" || tran_proto == "stm" )
			{
				boost::system::error_code ec;
				read_loop(ec, true);
			}
			else if ( tran_proto == "ssl" )
			{
				ssl_socket->async_handshake(boost::asio::ssl::stream_base::client,
					boost::bind(&end_point::handle_handshake, this, boost::asio::placeholders::error));
			}
			else
			{
				vnd_pt->error << "error> in serv: unknown transport protocol\n";
			}
		}
		else
		{
			vnd_pt->error << "error> in handle connect: " << ec.value() << " = " << ec.message() << "\n";
		}
	}

	void end_point::handle_handshake(const boost::system::error_code & ec)
	{
		if ( !ec )
		{
			connected = true;
			boost::system::error_code erc;
			read_loop(erc, true);
/*			std::cout << "Enter message: ";
			std::cin.getline(request_, max_length);
			size_t request_length = strlen(request_);
			boost::asio::async_write(socket_, boost::asio::buffer(request_, request_length),
				boost::bind(&client::handle_write, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
*/		}
		else
		{
			vnd_pt->error << "error> in handle handshake: " << ec.value() << " = " << ec.message() << "\n";
		}
	}

	// insr <ep> {raw|len} {tcp|udp} {<laddr>|*} {<lport>|*} {<raddr>|?} {<rport>|?}
	void end_point::serv()
	{
		server = true;
		try
		{
			if ( tran_proto == "stm" )
			{
#ifdef VLIN
				std::fstream f(local_path_name.c_str(), std::ios_base::trunc|std::ios_base::binary);
				f.close(); // creates the file but why: for unlink to succeed?
				::unlink(local_path_name.c_str());
				stm_acc_socket = new boost::asio::local::stream_protocol::acceptor(*vnd_pt->io_serv);
				if ( medeb::active ) medeb::newm(stm_acc_socket, sizeof(boost::asio::local::stream_protocol::acceptor),__FILE__, __LINE__);
				stm_acc_socket->open();//boost::asio::local::stream_protocol::endpoint(local_path_name).protocol());
				stm_acc_socket->bind(boost::asio::local::stream_protocol::endpoint(local_path_name));
				stm_acc_socket->listen(1);
				//stm_acc_socket->set_option(boost::asio::socket_base::reuse_address(true));
				stm_socket = new boost::asio::local::stream_protocol::socket(*vnd_pt->io_serv);
				if ( medeb::active ) medeb::newm(stm_socket, sizeof(boost::asio::local::stream_protocol::socket),__FILE__, __LINE__);
				stm_acc_socket->async_accept(*stm_socket, boost::bind(&end_point::handle_accept, this, boost::asio::placeholders::error));
#endif
			}
			else if ( tran_proto == "dgm" )
			{
#ifdef VLIN
				std::fstream f(local_path_name.c_str(), std::ios_base::trunc|std::ios_base::binary);
				f.close(); // creates the file but why: for unlink to succeed?
				::unlink(local_path_name.c_str());
				dgm_socket = new boost::asio::local::datagram_protocol::socket(*vnd_pt->io_serv);
				if ( medeb::active ) medeb::newm(dgm_socket, sizeof(boost::asio::local::datagram_protocol::socket),__FILE__, __LINE__);
				dgm_socket->open();//boost::asio::local::datagram_protocol::endpoint(local_path_name).protocol());
				dgm_socket->bind(boost::asio::local::datagram_protocol::endpoint(local_path_name));
				dgm_remote_endpoint = boost::asio::local::datagram_protocol::endpoint(remote_path_name);
				//dgm_socket->set_option(boost::asio::socket_base::receive_buffer_size(in_buf_size));
				//dgm_socket->set_option(boost::asio::socket_base::send_buffer_size(out_buf_size));
				boost::system::error_code ec;
				read_loop(ec, true);
#endif
			}
			else if ( tran_proto == "tcp" )
			{
				tcp_acc_socket = new boost::asio::ip::tcp::acceptor(*vnd_pt->io_serv);
				if ( medeb::active ) medeb::newm(tcp_acc_socket, sizeof(boost::asio::ip::tcp::acceptor),__FILE__, __LINE__);
				tcp_acc_socket->open(boost::asio::ip::tcp::v4()); // protocol dependent!
				tcp_acc_socket->bind(make_tcp_local_endpoint());
				tcp_acc_socket->listen(1);
				tcp_acc_socket->set_option(boost::asio::socket_base::reuse_address(true));
				tcp_socket = new boost::asio::ip::tcp::socket(*vnd_pt->io_serv);
				if ( medeb::active ) medeb::newm(tcp_socket, sizeof(boost::asio::ip::tcp::socket),__FILE__, __LINE__);
				tcp_acc_socket->async_accept(*tcp_socket, boost::bind(&end_point::handle_accept, this, boost::asio::placeholders::error));
			}
			else if ( tran_proto == "udp" )
			{
				udp_socket = new boost::asio::ip::udp::socket(*vnd_pt->io_serv);
				if ( medeb::active ) medeb::newm(udp_socket, sizeof(boost::asio::ip::udp::socket),__FILE__, __LINE__);
				udp_socket->open(boost::asio::ip::udp::v4()); // protocol dependent!
				udp_socket->bind(make_udp_local_endpoint());
				if ( remote_net_addr != "?" && remote_port_nb != "?" )
				{
					udp_socket->connect(make_udp_remote_endpoint()); // establish a filter for any other endpoint
					udp_remote_endpoint.address(boost::asio::ip::address::from_string(remote_net_addr.c_str()));
					udp_remote_endpoint.port(atoi(remote_port_nb.c_str()));
				}
				//udp_socket->set_option(boost::asio::socket_base::receive_buffer_size(in_buf_size));
				//udp_socket->set_option(boost::asio::socket_base::send_buffer_size(out_buf_size));
				boost::system::error_code ec;
				read_loop(ec, true);
			}
			else if ( tran_proto == "ssl" )
			{
				tcp_acc_socket = new boost::asio::ip::tcp::acceptor(*vnd_pt->io_serv);
				if ( medeb::active ) medeb::newm(tcp_acc_socket, sizeof(boost::asio::ip::tcp::acceptor),__FILE__, __LINE__);
				tcp_acc_socket->open(boost::asio::ip::tcp::v4()); // protocol dependent!
				tcp_acc_socket->bind(make_tcp_local_endpoint());
				tcp_acc_socket->listen(1);
				tcp_acc_socket->set_option(boost::asio::socket_base::reuse_address(true));

				ssl_ctx = new boost::asio::ssl::context(boost::asio::ssl::context::sslv23);
				ssl_ctx->set_options(boost::asio::ssl::context::default_workarounds
					| boost::asio::ssl::context::no_sslv2 | boost::asio::ssl::context::single_dh_use);
				ssl_ctx->set_password_callback(boost::bind(&end_point::get_password, this));
				ssl_ctx->use_certificate_chain_file("server.pem");
				ssl_ctx->use_private_key_file("server.pem", boost::asio::ssl::context::pem);
				ssl_ctx->use_tmp_dh_file("dh512.pem");

				ssl_socket = new boost::asio::ssl::stream<boost::asio::ip::tcp::socket>(*vnd_pt->io_serv, *ssl_ctx);
				if ( medeb::active ) medeb::newm(ssl_socket, sizeof(boost::asio::ssl::stream<boost::asio::ip::tcp::socket>),__FILE__, __LINE__);
				tcp_acc_socket->async_accept(ssl_socket->lowest_layer(), boost::bind(&end_point::handle_accept, this, boost::asio::placeholders::error));
			}
			else
			{
				vnd_pt->error << "error> in serv: unknown transport protocol\n";
			}
		}
		catch (std::exception & e)
		{
			vnd_pt->error << "exception> in serv: " <<  e.what() << "\n";
			//throw;
		}
	}

	void end_point::reserv()
	{
		try
		{
			if ( tran_proto == "tcp" ) // return server to listening state
			{
				connected = false;
				tcp_socket->shutdown(boost::asio::ip::tcp::socket::shutdown_both);
				tcp_socket->close();
				if ( medeb::active ) medeb::delm(tcp_socket);
				delete tcp_socket;
				tcp_socket = new boost::asio::ip::tcp::socket(*vnd_pt->io_serv);
				if ( medeb::active ) medeb::newm(tcp_socket, sizeof(boost::asio::ip::tcp::socket),__FILE__, __LINE__);
				tcp_acc_socket->async_accept(*tcp_socket, boost::bind(&end_point::handle_accept, this, boost::asio::placeholders::error));
				if ( bound_if )
					vnd_pt->forwarding_table.erase(bound_if->last_known_address);
			}
			else if ( tran_proto == "stm" )
			{
#ifdef VLIN
				connected = false;
				stm_socket->shutdown(boost::asio::local::stream_protocol::socket::shutdown_both);
				//stm_socket->cancel(); // use this?
				stm_socket->close();
				if ( medeb::active ) medeb::delm(stm_socket);
				delete stm_socket;
				stm_socket = new boost::asio::local::stream_protocol::socket(*vnd_pt->io_serv);
				if ( medeb::active ) medeb::newm(stm_socket, sizeof(boost::asio::local::stream_protocol::socket),__FILE__, __LINE__);
				stm_acc_socket->async_accept(*stm_socket, boost::bind(&end_point::handle_accept, this, boost::asio::placeholders::error));
				if ( bound_if )
					vnd_pt->forwarding_table.erase(bound_if->last_known_address);
#endif
			}
			else
				;
		}
		catch (std::exception & e)
		{
			vnd_pt->error << "exception> in reserv (when returning to listening state): " << e.what() << "\n";
		}
	}

	void end_point::handle_accept(const boost::system::error_code & ec)
	{
		if ( !ec ) // accept succeeded
		{
			if ( tran_proto == "stm" )
			{
#ifdef VLIN
				if ( vnd_pt->debug )
				{
					vnd_pt->output << "\nstm_socket->remote_endpoint().path() = " << stm_socket->remote_endpoint().path() << "\n";
					vnd_pt->output << "remote_path_name internally defined = " << remote_path_name << "\n";
				}
				//if ( /*remote_path_name != "?" &&*/ stm_socket->remote_endpoint().path() != remote_path_name )
				{
					//vnd_pt->warning << "warning> in handle_accept: client pathname is different from expected\n"; // it is always different it seems
					// shall we close the socket? not for the moment
				}
				//tcp_socket->set_option(boost::asio::socket_base::keep_alive(true));
				//tcp_socket->set_option(boost::asio::socket_base::linger(true, 0)); // 30
				//tcp_socket->set_option(boost::asio::socket_base::receive_buffer_size(in_buf_size));
				//tcp_socket->set_option(boost::asio::socket_base::send_buffer_size(out_buf_size));
				connected = true;
				boost::system::error_code ec;
				read_loop(ec, true);
#endif
			}
			else if ( tran_proto == "tcp" )
			{
				if ( remote_net_addr != "?" && tcp_socket->remote_endpoint().address().to_string() != remote_net_addr )
				{
					vnd_pt->warning << "warning> in handle_accept: client address is different from expected\n";
					// shall we close the socket? not for the moment
				}
				if	( remote_port_nb != "?" && tcp_socket->remote_endpoint().port() != atoi(remote_port_nb.c_str()) )
				{
					vnd_pt->warning << "warning> in handle_accept: client port is different from expected\n";
					// shall we close the socket? not for the moment
				}
				tcp_socket->set_option(boost::asio::socket_base::keep_alive(true));
				tcp_socket->set_option(boost::asio::socket_base::linger(true, 0)); // 30
				//tcp_socket->set_option(boost::asio::socket_base::receive_buffer_size(in_buf_size));
				//tcp_socket->set_option(boost::asio::socket_base::send_buffer_size(out_buf_size));
				tcp_socket->set_option(boost::asio::ip::tcp::no_delay(true));
				connected = true;
				boost::system::error_code ec;
				read_loop(ec, true);
			}
			else if ( tran_proto == "ssl" )
			{
				ssl_socket->async_handshake(boost::asio::ssl::stream_base::server,
					boost::bind(&end_point::handle_handshake, this, boost::asio::placeholders::error));			
			}
			else
			{
				vnd_pt->error << "error> in serv: unknown transport protocol\n";
			}
		}
		else // accept failed
		{
			//errc::connection_aborted = ECONNABORTED // boost system error_code
			//#define ECONNABORTED 9906 // boost cerrno, 
			//#define ERROR_CONNECTION_ABORTED         1236L // winerror
			//#define WSAECONNABORTED                  10053L // winerror
			if ( ec.value() == 1236 
				|| ec == boost::asio::error::make_error_code(boost::asio::error::connection_aborted) // tcp local
				|| ec == boost::asio::error::make_error_code(boost::asio::error::operation_aborted) ) // could happen?
			{
				// correct and useful? must deallocate acceptor socket, etc!
				if ( ( server && !tcp_acc_socket ) || ( !tcp_socket && !udp_socket ) ) // async calls processed
				{
					kill();
					return;
				}
#ifdef VLIN
				if ( ( server && !stm_acc_socket ) || ( !stm_socket && !dgm_socket ) ) // async calls processed
				{
					kill();
					return;
				}
#endif
				// end_point already deleted by a disc in main thread
			}
			else if ( ec == boost::asio::error::make_error_code(boost::asio::error::connection_refused) ) // remote closed
			{
				vnd_pt->error << "error> in handle accept: remote host unavailable, " << ec.value() << " = " << ec.message() << "\n";
			}
			else
			{
				vnd_pt->error << "error> in handle accept: " << ec.value() << " = " << ec.message() << "\n";
			}
		}
	}

	/*void linux_depleteSendBuffer(int fd) 
	{
		int lastOutstanding=-1;
		for(;;) 
		{
			int outstanding;
			ioctl(fd, SIOCOUTQ, &outstanding);
			if(outstanding != lastOutstanding) 
				printf("Outstanding: %d\n", outstanding);
			lastOutstanding = outstanding;
			if(!outstanding)
				break;
			usleep(1000);
		}
	}*/

	std::string end_point::get_password()
	{
		return std::string("test");
	}

	bool end_point::verify_certificate(bool preverified, boost::asio::ssl::verify_context& ctx)
	{
		// The verify callback can be used to check whether the certificate that is
		// being presented is valid for the peer. For example, RFC 2818 describes
		// the steps involved in doing this for HTTPS. Consult the OpenSSL
		// documentation for more details. Note that the callback is called once
		// for each certificate in the certificate chain, starting from the root
		// certificate authority.

		// In this example we will simply print the certificate's subject name.
		char subject_name[256];
		X509* cert = X509_STORE_CTX_get_current_cert(ctx.native_handle());
		X509_NAME_oneline(X509_get_subject_name(cert), subject_name, 256);
		vnd_pt->output << "Verifying " << subject_name << "\n";

		return preverified;
	}

	// tap <ep> <ifname>
	void end_point::tap()
	{
#ifdef VLIN
		try
		{
			tap_desc = ::open("/dev/net/tun", O_RDWR);
			if ( tap_desc < 0 )
			{
				::perror("open(/dev/net/tun)");
				throw "open(/dev/net/tun)";
			}
			struct ifreq ifr;
			::memset(&ifr, 0, sizeof(ifr));
			ifr.ifr_flags = IFF_TAP | IFF_NO_PI;
			::strncpy(ifr.ifr_name, tap_name.c_str(), IFNAMSIZ);
			int err = ::ioctl(tap_desc, TUNSETIFF, (void *)&ifr);
			if ( err < 0 )
			{
				::perror("ioctl(TUNSETIFF)");
				::close(tap_desc);
				throw "ioctl(TUNSETIFF)";
			}
			//::strcpy(dev, ifr.ifr_name); // useful? yes if the OS selects the dev
			native_tap_desc = tap_desc;
			tap_psd = new boost::asio::posix::stream_descriptor(*vnd_pt->io_serv, native_tap_desc);
			if ( medeb::active ) medeb::newm(tap_psd, sizeof(boost::asio::posix::stream_descriptor),__FILE__, __LINE__);
			if ( !tap_psd->is_open() )
			{
				throw "tap_psd is not open\n";
			}
			if ( !tap_psd->native_non_blocking() ) // may be incorrect if set directly on the native desc
			{
				//vnd_pt->output << "tap descr in native blocking mode\n";
				tap_psd->native_non_blocking(true);
			}
			if ( !tap_psd->non_blocking() ) // may be incorrect if set directly on the native desc
			{
				//vnd_pt->output << "tap descr in blocking mode\n";
				tap_psd->non_blocking(true);
			}
			boost::system::error_code ec;
			read_loop(ec, true);
		}
		catch (...)//std::exception & e)
		{
			if ( tap_psd )
			{
				tap_psd->close();
				delete tap_psd;
				tap_psd = 0;
			}
			//vnd_pt->error << "exception> in tap: " <<  e.what() << "\n";
			throw;
		}
#endif
	}

	// vde <ep> <switch> <port> <group> <mode>
	void end_point::vde()
	{
#ifdef VLIN
		try
		{
			std::string cn = "VND:" + vnd_pt->name + ":" + id;
/*			if ( bound_if ) // bind is always after ep creation so this is useless
				cn += bound_if->id + ":" + id;
			else
				cn += "u:" + id;
*/			struct vde_open_args voa;
			int vport = atoi(vde_port.c_str()); // 0=any port
			if ( vport >= 0 )
				voa.port = vport;
			else
				voa.port = 0;
			char * csg = new char[256];
			strncpy(csg, vde_group.c_str(), 256);
			//if ( vde_group == "" )
				voa.group = NULL; // = no group change
			//else
			//	voa.group = csg;
			int vmode = atoi(vde_mode.c_str()); // 0 = standard socket mode? mode_t
			//if ( vmode >= 0 )
			//	voa.mode = vmode; // beware as we need an octal number
			//else
				voa.mode = 0700; // or 0600?
			char * css = new char[256];
			strncpy(css, vde_switch.c_str(), 256);
			char * csc = new char[256];
			strncpy(csc, cn.c_str(), 256);
			std::cout << "vde name = " << css << ", descr = " << csc << std::endl;
			vde_con = ::vde_open(css, csc, &voa);
			delete [] csg; // risky?
			delete [] css;
			delete [] csc;
			if ( !vde_con )
			{
				throw vnd_exception("vde_open() failed");
			}

			vde_data_desc = ::vde_datafd(vde_con);
			if ( vde_data_desc < 0 )
			{
				throw vnd_exception("vde_datafd() failed");
			}
			native_vde_data_desc = vde_data_desc;
			boost::asio::local::datagram_protocol::endpoint dpe;
			vde_data_socket = new boost::asio::local::datagram_protocol::socket(*vnd_pt->io_serv, dpe.protocol(), native_vde_data_desc);
			if ( medeb::active ) medeb::newm(vde_data_socket, sizeof(boost::asio::local::datagram_protocol::socket),__FILE__, __LINE__);
			if ( !vde_data_socket->is_open() )
			{
				throw vnd_exception("vde_data_socket is not open");
			}
			if ( !vde_data_socket->native_non_blocking() ) // may be incorrect if set directly on the native desc
			{
				//vnd_pt->output << "vde descr in native blocking mode\n";
				vde_data_socket->native_non_blocking(true);
			}
			if ( !vde_data_socket->non_blocking() ) // may be incorrect if set directly on the native desc
			{
				//vnd_pt->output << "vde descr in blocking mode\n";
				vde_data_socket->non_blocking(true);
			}
			//vde_socket->set_option(boost::asio::socket_base::receive_buffer_size(in_buf_size));
			//vde_socket->set_option(boost::asio::socket_base::send_buffer_size(out_buf_size));

			vde_ctl_desc = ::vde_ctlfd(vde_con);
			if ( vde_ctl_desc < 0 )
			{
				throw vnd_exception("vde_ctlfd() failed");
			}
			native_vde_ctl_desc = vde_ctl_desc;
			boost::asio::local::datagram_protocol::endpoint dpec;
			vde_ctl_socket = new boost::asio::local::datagram_protocol::socket(*vnd_pt->io_serv, dpec.protocol(), native_vde_ctl_desc);
			if ( medeb::active ) medeb::newm(vde_ctl_socket, sizeof(boost::asio::local::datagram_protocol::socket),__FILE__, __LINE__);
			if ( !vde_ctl_socket->is_open() )
			{
				throw vnd_exception("vde_ctl_socket is not open");
			}

			boost::system::error_code ec;
			read_loop(ec, true);
		}
		catch (std::exception & e)
		{
			if ( vde_con )
			{
				if ( ::vde_close(vde_con) )
					vnd_pt->error << "vde_close failed\n";//throw;
				vde_con = 0;
			}
			vde_data_desc = -1;
			native_vde_data_desc = -1;
			if ( vde_data_socket )
			{
				delete vde_data_socket;
				vde_data_socket = 0;
			}
			vde_ctl_desc = -1;
			native_vde_ctl_desc = -1;
			if ( vde_ctl_socket )
			{
				delete vde_ctl_socket;
				vde_ctl_socket = 0;
			}
			::perror("perror: ");
			vnd_pt->error << "> exception in vde: " <<  e.what() << "\n";
			throw;
		}
#endif
	}

	void end_point::kill()
	{
		if ( bound_if )
		{
			bound_if->bound_ep = 0;
			vnd_pt->forwarding_table.erase(bound_if->last_known_address);
		}
		vnd_pt->end_points.erase(id);
		//vnd_pt->binding_table.erase(id);
		if ( medeb::active ) medeb::delm(this);
		delete this; // end_point commits suicide
	}

	void end_point::disc() // sync, because if async, ep may be deleted before disc occurs
	{
		if ( tran_proto == "stm" ) // terminate cli and serv (unlike remote disc)
		{
#ifdef VLIN
			connected = false;
			try
			{
				if ( stm_socket )
				{
					stm_socket->shutdown(boost::asio::local::stream_protocol::socket::shutdown_both);
					//stm_socket->cancel();
					stm_socket->close();
					if ( medeb::active ) medeb::delm(stm_socket);
					delete stm_socket;
					stm_socket = 0;
				}
				if ( stm_acc_socket )
				{
					stm_acc_socket->cancel();
					stm_acc_socket->close();
					if ( medeb::active ) medeb::delm(stm_acc_socket);
					delete stm_acc_socket;
					stm_acc_socket = 0;
				}
				if ( server )
					std::remove(local_path_name.c_str());
			}
			catch (std::exception & e)
			{
				vnd_pt->error << "exception> in stm disc: " << e.what() << "\n";
			}
#endif
		}
		else if ( tran_proto == "dgm" )
		{
#ifdef VLIN
			try
			{
				if ( dgm_socket )
				{
					dgm_socket->cancel();
					dgm_socket->close();
					if ( medeb::active ) medeb::delm(dgm_socket);
					delete dgm_socket;
					dgm_socket = 0;
				}
				std::remove(local_path_name.c_str());
			}
			catch (std::exception & e)
			{
				vnd_pt->error << "exception> in dgm disc: " << e.what() << "\n";
			}
#endif
		}
		else if ( tran_proto == "tcp" ) // terminate cli and serv (unlike remote disc)
		{
			connected = false;
			try
			{
				if ( tcp_socket )
				{
					try
					{
						tcp_socket->remote_endpoint(); // test if alive
						tcp_socket->shutdown(boost::asio::ip::tcp::socket::shutdown_both);
						// deplete send buffer -> see above
						// ensure all pending data has been read! -> read returns 0
					}
					catch (std::exception & e)
					{
						std::string cause(e.what());
						;//vnd_pt->error << "\nexception in tcp soc disc: " << e.what() << "\n";
					}
					//tcp_socket->cancel();
					tcp_socket->close();
					if ( medeb::active ) medeb::delm(tcp_socket);
					delete tcp_socket;
					tcp_socket = 0;
				}
				if ( tcp_acc_socket )
				{
					//tcp_acc_socket->cancel();
					tcp_acc_socket->close();
					if ( medeb::active ) medeb::delm(tcp_acc_socket);
					delete tcp_acc_socket;
					tcp_acc_socket = 0;
				}
			}
			catch (std::exception & e)
			{
				vnd_pt->error << "exception> in tcp disc: " << e.what() << "\n";
			}
		}
		else if ( tran_proto == "udp" )
		{			
			try
			{
				if ( udp_socket )
				{
					//udp_socket->cancel();
					udp_socket->close();
					if ( medeb::active ) medeb::delm(udp_socket);
					delete udp_socket;
					udp_socket = 0;
				}
			}
			catch (std::exception & e)
			{
				vnd_pt->error << "exception> in udp disc: " << e.what() << "\n";
			}
		}
		else if ( tran_proto == "ssl" )
		{
			connected = false;
			try
			{
				if ( ssl_socket )
				{
					try
					{
						ssl_socket->lowest_layer().remote_endpoint(); // test if alive
						ssl_socket->lowest_layer().shutdown(boost::asio::ip::tcp::socket::shutdown_both);
						// deplete send buffer -> see above
						// ensure all pending data has been read! -> read returns 0
					}
					catch (std::exception & e)
					{
						std::string cause(e.what());
						;//vnd_pt->error << "\nexception in tcp soc disc: " << e.what() << "\n";
					}
					//ssl_socket->lowest_layer().cancel();
					ssl_socket->lowest_layer().close();
					if ( medeb::active ) medeb::delm(ssl_socket);
					delete ssl_socket;
					ssl_socket = 0;
				}
				if ( tcp_acc_socket )
				{
					//tcp_acc_socket->cancel();
					tcp_acc_socket->close();
					if ( medeb::active ) medeb::delm(tcp_acc_socket);
					delete tcp_acc_socket;
					tcp_acc_socket = 0;
				}
			}
			catch (std::exception & e)
			{
				vnd_pt->error << "exception> in ssl disc: " << e.what() << "\n";
			}
		}
		else if ( tran_proto == "tap" )
		{
#ifdef VLIN
			try
			{
				if ( tap_psd )
				{
					//tap_psd->cancel();
					native_tap_desc = tap_psd->release();
					tap_psd->close();
					if ( medeb::active ) medeb::delm(tap_psd);
					delete tap_psd;
					tap_psd = 0;
				}
				native_tap_desc = -1;
				if ( tap_desc >= 0 )
					::close(tap_desc); // usefull
				tap_desc = -1;
			}
			catch (std::exception & e)
			{
				vnd_pt->error << "exception> in tap disc: " << e.what() << "\n";
			}
#endif
		}
		else if ( tran_proto == "vde" )
		{
#ifdef VLIN
			try
			{
				if ( vde_data_socket )
				{
					vde_data_socket->shutdown(boost::asio::local::datagram_protocol::socket::shutdown_both);
					vde_data_socket->close();
					if ( medeb::active ) medeb::delm(vde_data_socket);
					delete vde_data_socket;
					vde_data_socket = 0;
				}
				native_vde_data_desc = -1;
				vde_data_desc = -1;
				if ( vde_ctl_socket )
				{
					vde_ctl_socket->shutdown(boost::asio::local::datagram_protocol::socket::shutdown_both);
					vde_ctl_socket->close();
					if ( medeb::active ) medeb::delm(vde_ctl_socket);
					delete vde_ctl_socket;
					vde_ctl_socket = 0;
				}
				native_vde_ctl_desc = -1;
				vde_ctl_desc = -1;
				if ( vde_con )
				{
					//if ( ::vde_close(vde_con) < 0 ) // useless as already done by above calls
					//	vnd_pt->error << "vde_close failed\n";//throw;
					vde_con = 0;
				}
			}
			catch (std::exception & e)
			{
				vnd_pt->error << "exception> in vde disc: " << e.what() << "\n";
			}
#endif
		}
		else
		{
			vnd_pt->error << "error> in disc: unknown transport protocol\n";

		}
	}

	void end_point::set(std::string k, std::string v)
	{
		if ( k == "bsin" )
		{
			if ( v == "u" )
			{
				in_buf_size = default_buffer_size;
			}
			else
			{
				dword ibs = strtoul(v.c_str(), 0, 10);
				if ( ibs < default_buffer_size )
					in_buf_size = default_buffer_size;
				else if ( ibs > max_buffer_size )
					in_buf_size = max_buffer_size;				
				else
					in_buf_size = ibs;
			}
			in_buf.resize(in_buf_size); // in_buf should not be currently used!!
			in_buf.assign(in_buf_size, 0x00);
			if ( bound_if )
			{
				bound_if->input_queue->set_buffer(in_buf_size);
			}
		}
		else if ( k == "bsout" )
		{
			if ( v == "u" )
			{
				out_buf_size = default_buffer_size;
			}
			else
			{
				dword obs = strtoul(v.c_str(), 0, 10);
				if ( obs < default_buffer_size )
					out_buf_size = default_buffer_size;
				else if ( obs > max_buffer_size )
					out_buf_size = max_buffer_size;				
				else
					out_buf_size = obs;
			}
			out_buf.resize(out_buf_size); // out_buf should not be currently used!!
			out_buf.assign(out_buf_size, 0x00);
			if ( bound_if )
			{
				bound_if->output_queue->set_buffer(out_buf_size);
			}
		}
		else
		{
			std::cout << "\n> unknown arg (try help)\n";
		}
	}

	void end_point::msg(std::string s)
	{
		frame * f = vnd_pt->create_frame();
		f->buffer.resize(s.size());
		std::copy(s.begin(), s.begin() + s.size(), f->buffer.begin());
		boost::system::error_code ec;
		write_loop(ec, f);
	}

	void end_point::write_loop(const boost::system::error_code & ec, frame * f)
	{
/*		if ( write_overlap ) // should we do this? same as in read_loop?
		{
			return;
		}*/
/*		if ( bound_if )
		{
			bound_if->output_queue->timer_running = false;
		}
*/		if ( ec ) // used by async_wait
		{
			if ( ec == boost::asio::error::make_error_code(boost::asio::error::operation_aborted) )
				vnd_pt->warning << "warning> [write_loop] the timer was cancelled (because of a call to expire/cancel): " << ec.value() << " = " << ec.message() << "\n";
			else
				vnd_pt->error << "error> [write_loop] (timer pb from async_wait): " << ec.value() << " = " << ec.message() << "\n";
			vnd_pt->destroy_frame(f);
			return;
		}
		/* if ( bound_if )
		{
			// solution 2: diverges at high bitrates because it does not consider socket time
			fpsec fps = (boost::chrono::high_resolution_clock::now() - f->queue_ts_key);
			bound_if->output_queue->activity_duration += fps;
		} */	
		if ( tran_proto == "tcp" || tran_proto == "stm" )
		{
			if ( tran_proto == "tcp" && !tcp_socket )
				return;
#ifdef VLIN
			if ( tran_proto == "stm" && !stm_socket )
				return;
#endif
			if ( !connected )
				return;
			//out_buf.assign(out_buf_size, 0x00);
			if ( encapsulation == encapsulation_length )
			{
				total_bytes_to_write = f->buffer.size() + 4;
				dword hl = f->buffer.size();
				dword_to_bytes(/*h_to_n_l*/(hl), out_lbuf);
				std::copy(out_lbuf, out_lbuf + 4, out_buf.begin());
				std::copy(f->buffer.begin(), f->buffer.end(), out_buf.begin() + 4);
			}
			else // no encaps
			{
				total_bytes_to_write = f->buffer.size();
				std::copy(f->buffer.begin(), f->buffer.end(), out_buf.begin());
			}
			if ( write_overlap )
				vnd_pt->error << "error> async_write overlap\n";
			write_overlap = true;
			if ( tran_proto == "tcp" )
			{
				boost::asio::async_write(*tcp_socket, boost::asio::buffer(out_buf, total_bytes_to_write),
					boost::bind(&end_point::handle_write, this, boost::asio::placeholders::error, 
						boost::asio::placeholders::bytes_transferred));
			}
			else //if ( tran_proto == "stm" )
			{
#ifdef VLIN
				boost::asio::async_write(*stm_socket, boost::asio::buffer(out_buf, total_bytes_to_write),
					boost::bind(&end_point::handle_write, this, boost::asio::placeholders::error, 
						boost::asio::placeholders::bytes_transferred));
#endif
			}
		}
		else if ( tran_proto == "udp" || tran_proto == "dgm" )
		{
			if ( tran_proto == "udp" && !udp_socket )
				return;
#ifdef VLIN
			if ( tran_proto == "dgm" && !dgm_socket )
				return;
#endif
			if ( tran_proto == "udp" && server && udp_remote_endpoint.port() == 4 )
			{
				vnd_pt->error << "error> in async write data : unknown remote endpoint\n";
			}
#ifdef VLIN
			if ( tran_proto == "dgm" && server && dgm_remote_endpoint.path() != remote_path_name )
			{
				vnd_pt->error << "error> in async write data : unknown remote endpoint\n";
			}
#endif
			else
			{
				total_bytes_to_write = f->buffer.size();
				if ( write_overlap )
					vnd_pt->error << "error> async_send(_to) overlap\n";
				write_overlap = true;
				//out_buf.assign(out_buf_size, 0x00);
				std::copy(f->buffer.begin(), f->buffer.end(), out_buf.begin());
				if ( tran_proto == "udp" )
				{
					// connected udp
					udp_socket->async_send(boost::asio::buffer(out_buf, f->buffer.size()), 
						boost::bind(&end_point::handle_write, this, boost::asio::placeholders::error, 
							boost::asio::placeholders::bytes_transferred));
					// unconnected udp
					//udp_socket->async_send_to(boost::asio::buffer(out_buf, f->buffer.size()), get_udp_remote_endpoint(),
					//	boost::bind(&end_point::handle_write, this, boost::asio::placeholders::error, 
					//		boost::asio::placeholders::bytes_transferred));		
				}
				else //if ( tran_proto == "dgm" )
				{
#ifdef VLIN
					// connected dgm, seems to work when dgm is a client but not when dgm is a server
					//dgm_socket->async_send(boost::asio::buffer(out_buf, f->buffer.size()), 
					//	boost::bind(&end_point::handle_write, this, boost::asio::placeholders::error, 
					//		boost::asio::placeholders::bytes_transferred));
					// unconnected dgm
					dgm_socket->async_send_to(boost::asio::buffer(out_buf, f->buffer.size()), dgm_remote_endpoint,
						boost::bind(&end_point::handle_write, this, boost::asio::placeholders::error, 
							boost::asio::placeholders::bytes_transferred));		
#endif
				}
			}
		}
		else if ( tran_proto == "tap" )
		{
#ifdef VLIN
			if ( !tap_psd )
				return;
			total_bytes_to_write = f->buffer.size();
			out_buf.assign(out_buf_size, 0x00);
			std::copy(f->buffer.begin(), f->buffer.end(), out_buf.begin());
			if ( write_overlap )
				vnd_pt->error << "error> async_write overlap\n";
			write_overlap = true;
			boost::asio::async_write(*tap_psd, boost::asio::buffer(out_buf, f->buffer.size()),
				boost::bind(&end_point::handle_write, this, boost::asio::placeholders::error, 
					boost::asio::placeholders::bytes_transferred));
#endif
		}
		else if ( tran_proto == "vde" )
		{
#ifdef VLIN
			if ( !vde_data_socket )
				return;
			total_bytes_to_write = f->buffer.size();
			out_buf.assign(out_buf_size, 0x00);
			std::copy(f->buffer.begin(), f->buffer.end(), out_buf.begin());
			if ( write_overlap )
				vnd_pt->error << "error> async_write overlap\n";
			write_overlap = true;
			/*boost::asio::async_write*/vde_data_socket->async_send(/* *vde_socket, */boost::asio::null_buffers(), //boost::asio::buffer(out_buf, f->buffer.size()),
				boost::bind(&end_point::handle_write, this, boost::asio::placeholders::error, 
					boost::asio::placeholders::bytes_transferred));
#endif
		}
		else if ( tran_proto == "ssl" )
		{
			if ( !ssl_socket )
				return;
			if ( !connected )
				return;
			//out_buf.assign(out_buf_size, 0x00);
			if ( encapsulation == encapsulation_length )
			{
				total_bytes_to_write = f->buffer.size() + 4;
				dword hl = f->buffer.size();
				dword_to_bytes(/*h_to_n_l*/(hl), out_lbuf);
				out_buf.assign(out_buf_size, 0x00);
				std::copy(out_lbuf, out_lbuf + 4, out_buf.begin());
				std::copy(f->buffer.begin(), f->buffer.end(), out_buf.begin() + 4);
			}
			else
			{
				total_bytes_to_write = f->buffer.size();
				std::copy(f->buffer.begin(), f->buffer.end(), out_buf.begin());
			}
			if ( write_overlap )
				vnd_pt->error << "error> async_write overlap\n";
			write_overlap = true;
			boost::asio::async_write(*ssl_socket, boost::asio::buffer(out_buf, total_bytes_to_write),
				boost::bind(&end_point::handle_write, this, boost::asio::placeholders::error, 
					boost::asio::placeholders::bytes_transferred));
		}
		else
		{
			vnd_pt->error << "error> in msg : unknown transport protocol\n";
		}
		vnd_pt->destroy_frame(f); // as the frame is now in the endpt output buffer
	}

	void end_point::read_loop(const boost::system::error_code & ec, bool short_circuit)
	{
		if ( read_overlap ) // a previous read_loop call has not finished
		{
			vnd_pt->warning << "warning> read_loop overlap\n";
			return;
		}
/*		if ( bound_if )
		{
			bound_if->input_queue->timer_running = false;
		}
*/		if ( ec ) // used by async_wait
		{
			if ( ec == boost::asio::error::make_error_code(boost::asio::error::operation_aborted) )
				vnd_pt->warning << "warning> [read_loop] the timer was cancelled (because of a call to expire/cancel): " << ec.value() << " = " << ec.message() << "\n";
			else
				vnd_pt->error << "error> [read_loop] (timer pb from async_wait): " << ec.value() << " = " << ec.message() << "\n";
			return;
		}
		/* if ( bound_if )
		{
			if ( !short_circuit )
			{
				// solution 2: diverges at high bitrates because it does not consider socket time
				// solution 2: we do not have the frame, may the current_frame_arrival be erroneous?
				//fpsec fps = (boost::chrono::high_resolution_clock::now() - current_frame_arrival);
				fpsec fps = (boost::chrono::high_resolution_clock::now() - bound_if->input_queue->frames_tss.front());
				bound_if->input_queue->frames_tss.pop();
				bound_if->input_queue->activity_duration += fps;
			}
		} */
		if ( tran_proto == "tcp" || tran_proto == "stm" )
		{
			if ( tran_proto == "tcp" && !tcp_socket )
				return;
#ifdef VLIN
			if ( tran_proto == "stm" && !stm_socket )
				return;
#endif
			if ( !connected )
				return;
			if ( encapsulation == encapsulation_none )
			{
				if ( read_overlap )
					vnd_pt->error << "error> async_read_some overlap\n";
				//in_buf.assign(in_buf_size, 0x00);
				if ( tran_proto == "tcp" )
				{
					tcp_socket->async_read_some(boost::asio::buffer(in_buf), 
						boost::bind(&end_point::handle_read, this, boost::asio::placeholders::error, 
							boost::asio::placeholders::bytes_transferred));
				}
				else //if ( tran_proto == "stm" )
				{
#ifdef VLIN
					stm_socket->async_read_some(boost::asio::buffer(in_buf), 
						boost::bind(&end_point::handle_read, this, boost::asio::placeholders::error, 
							boost::asio::placeholders::bytes_transferred));
#endif
				}
			}
			else if ( encapsulation == encapsulation_length )
			{
				if ( !waiting_for_next_frame )
				{
					if ( read_overlap )
						vnd_pt->error << "error> async_read overlap\n";
					//in_buf.assign(in_buf_size, 0x00);
					if ( tran_proto == "tcp" )
					{
						boost::asio::async_read(*tcp_socket, boost::asio::buffer(in_buf, 4), 
							boost::bind(&end_point::handle_read, this, boost::asio::placeholders::error, 
								boost::asio::placeholders::bytes_transferred));
					}
					else //if ( tran_proto == "stm" )
					{
#ifdef VLIN
						boost::asio::async_read(*stm_socket, boost::asio::buffer(in_buf, 4), 
							boost::bind(&end_point::handle_read, this, boost::asio::placeholders::error, 
								boost::asio::placeholders::bytes_transferred));
#endif
					}
				}
				else // waiting for next frame
				{
					if ( read_overlap )
						vnd_pt->error << "error> async_read overlap\n";
					in_buf.assign(in_buf_size, 0x00);
					if ( tran_proto == "tcp" )
					{
						boost::asio::async_read(*tcp_socket, boost::asio::buffer(in_buf, next_frame_size), 
							boost::bind(&end_point::handle_read, this, boost::asio::placeholders::error, 
								boost::asio::placeholders::bytes_transferred));
					}
					else //if ( tran_proto == "stm" )
					{
#ifdef VLIN
						boost::asio::async_read(*stm_socket, boost::asio::buffer(in_buf, next_frame_size), 
							boost::bind(&end_point::handle_read, this, boost::asio::placeholders::error, 
								boost::asio::placeholders::bytes_transferred));
#endif
					}
				}
			}
			else
			{
				vnd_pt->error << "error> unknown encapsulation\n";
			}
			read_overlap = true;
		}
		else if ( tran_proto == "udp" || tran_proto == "dgm" )
		{
			if ( tran_proto == "udp" && !udp_socket )
				return;
#ifdef VLIN
			if ( tran_proto == "dgm" && !dgm_socket )
				return;
#endif
			if ( read_overlap )
				vnd_pt->error << "error> async_receive_from overlap\n";
			//in_buf.assign(in_buf_size, 0x00);
			if ( tran_proto == "udp" )
			{
				// replace by receive_from? only works for an unconnected socket? it seems not
				udp_socket->async_receive_from(boost::asio::buffer(in_buf),	udp_remote_endpoint,
					boost::bind(&end_point::handle_read, this, boost::asio::placeholders::error, 
						boost::asio::placeholders::bytes_transferred));
			}
			else //if ( tran_proto == "dgm" )
			{
#ifdef VLIN
				// receive is replaced by receive_from for the dgm server to work properly
				dgm_socket->async_receive_from(boost::asio::buffer(in_buf), dgm_remote_endpoint,	
					boost::bind(&end_point::handle_read, this, boost::asio::placeholders::error, 
						boost::asio::placeholders::bytes_transferred));
#endif
			}
			read_overlap = true;
		}
		else if ( tran_proto == "tap" )
		{
#ifdef VLIN
			if ( !tap_psd )
				return;
			if ( read_overlap )
				vnd_pt->error << "error> async_read_some overlap\n";
			in_buf.assign(in_buf_size, 0x00);
			tap_psd->async_read_some(boost::asio::buffer(in_buf), 
				boost::bind(&end_point::handle_read, this, boost::asio::placeholders::error, 
					boost::asio::placeholders::bytes_transferred));
			read_overlap = true;
#endif
		}
		else if ( tran_proto == "vde" )
		{
#ifdef VLIN
			if ( vde_data_socket )
			{
				if ( read_overlap )
					vnd_pt->error << "error> async_read_some overlap\n";
				in_buf.assign(in_buf_size, 0x00);
				vde_data_socket->async_receive(boost::asio::null_buffers(),//boost::asio::buffer(in_buf), // assuming the socket is connected
					boost::bind(&end_point::handle_read, this, boost::asio::placeholders::error, 
						boost::asio::placeholders::bytes_transferred));
				read_overlap = true;
			}
			if ( vde_ctl_socket )
			{
				vde_ctl_socket->async_receive(boost::asio::null_buffers(),//boost::asio::buffer(in_buf), // assuming the socket is connected
					boost::bind(&end_point::handle_vde_ctl, this, boost::asio::placeholders::error, 
						boost::asio::placeholders::bytes_transferred));
			}
#endif
		}
		else if ( tran_proto == "ssl" )
		{
			if ( !ssl_socket )
				return;
			if ( !connected )
				return;
			if ( encapsulation == encapsulation_none )
			{
				if ( read_overlap )
					vnd_pt->error << "error> async_read_some overlap\n";
				in_buf.assign(in_buf_size, 0x00);
				ssl_socket->async_read_some(boost::asio::buffer(in_buf), 
					boost::bind(&end_point::handle_read, this, boost::asio::placeholders::error, 
						boost::asio::placeholders::bytes_transferred));
				read_overlap = true;
			}
			else if ( encapsulation == encapsulation_length )
			{
				if ( !waiting_for_next_frame )
				{
					if ( read_overlap )
						vnd_pt->error << "error> async_read overlap\n";
					in_buf.assign(in_buf_size, 0x00);
					boost::asio::async_read(*ssl_socket, boost::asio::buffer(in_buf, 4), 
						boost::bind(&end_point::handle_read, this, boost::asio::placeholders::error, 
							boost::asio::placeholders::bytes_transferred));
					read_overlap = true;
				}
				else
				{
					if ( read_overlap )
						vnd_pt->error << "error> async_read overlap\n";
					in_buf.assign(in_buf_size, 0x00);
					boost::asio::async_read(*ssl_socket, boost::asio::buffer(in_buf, next_frame_size), 
						boost::bind(&end_point::handle_read, this, boost::asio::placeholders::error, 
							boost::asio::placeholders::bytes_transferred));
					read_overlap = true;
				}
			}
			else
			{
				vnd_pt->error << "error> unknown encapsulation\n";
			}
		}
		else
		{
			vnd_pt->error << "error> in read_loop : unknown transport protocol\n";
		}
	}

	void end_point::manage_rw_error(const boost::system::error_code & ec, std::string s)
	{
		//#define ECONNABORTED 9906 // boost cerrno, 
		//errc::connection_aborted = ECONNABORTED // boost system error_code
		//#define ERROR_CONNECTION_ABORTED         1236L // winerror
		//#define WSAECONNABORTED                  10053L // winerror
		if ( ec.value() == 1236 
		|| ec == boost::asio::error::make_error_code(boost::asio::error::connection_aborted) // tcp local
		|| ec == boost::asio::error::make_error_code(boost::asio::error::operation_aborted) ) // udp local, also valid for async interupts?
		{
/*
			if ( ( server && !tcp_acc_socket ) || ( !tcp_socket && !udp_socket ) ) // async calls processed
#ifdef VLIN
			if ( !vde_data_socket || !vde_ctl_socket )
#endif
*/			{
				if ( s == "vde_ctl" )
				{
					return;
				}
/*				if ( tran_proto == "vde" && !vde_disc )
				{
					vde_disc++;
					return;
				}
*/				kill(); // works if called only once!
				return;
			}
			//else
			//	vnd_pt->error << "the endpoint has no valid handle, should not happen\n";
			// end_point already disc'ed by main thread
		}
		else if ( ec == boost::asio::error::make_error_code(boost::asio::error::connection_reset) // tcp remote has reset connection
			|| ec == boost::asio::error::make_error_code(boost::asio::error::eof) // tcp remote has closed connection normally
			|| ec == boost::asio::error::make_error_code(boost::asio::error::broken_pipe) ) // happens after receiving eof
		{
			if ( server && (tran_proto == "tcp" || tran_proto == "stm") ) // return server to listening state
			{
				reserv();
			}
			else // cli tcp/stm (or cli/serv udp?)
			{
				disc();
			}
		}
		else if ( ec == boost::asio::error::make_error_code(boost::asio::error::connection_refused) ) // udp remote, could be tcp?
		{
			if ( tran_proto == "udp" || tran_proto == "dgm" )
			{
				vnd_pt->warning << "warning> in handle " << s << " of " << id << ": " << ec.value() << " = " << ec.message() << "\n";
				//	<< " (connection refused / udp remote unavailable)\n"; // remote closed
				if ( s == "read" )
				{
					reread_loop();
				}
				//disc();
			}
			/* else if ( tran_proto == "stm" ) // If a call to connect for a Unix domain stream socket finds that the listening socket's queue is full (Section 4.5), ECONNREFUSED is returned immediately.
			{
				vnd_pt->warning << "warning> stm server listening queue is full\n";				
			} */
			else
			{
				vnd_pt->error << "error> in handle " << s << ": value = " << ec.value() << ", message = " << ec.message() << "\n";
				vnd_pt->error << "error> closing endpoint to avoid pbs\n";
				disc();
			}
		}
		else if ( ec.value() == 22 && tran_proto == "tap" && s == "write" )
		{
			vnd_pt->warning << "warning> tap must be written a minimum of 14 bytes\n";
		}
		else
		{
			vnd_pt->error 
				//<< "[" << static_cast<unsigned long long>(boost::this_thread::get_id())"]"
				<< "error> in handle " << s << ": value = " << ec.value() << ", message = " << ec.message() << "\n";
			if ( s == "write" )
			{
				vnd_pt->error << "error> send(_to)/write should have written " << total_bytes_to_write << "\n";
			}
			else
			{
				vnd_pt->error << "error> closing endpoint to avoid pbs\n";
				disc();
			}
		}
	}

	// MAC | TAG | L/T | LLC | SNAP |   DATA  | CRC
	//  12 | (4) |  2  |  3  |   5  | 38-1500 |  4
	// min frame length = 64
	// max frame length = 1518 + 4 if tag
	// min data length:
	// DIX = 46 (2)
	// 802.2 = 43 (5)
	// SNAP = 38 (10)

	/* dynamips
	 * very simple protocol to send packets over tcp
	 * 32 bits in network format - size of packet, then packet itself and so on.
	 
	static ssize_t netio_tcp_send(netio_inet_desc_t *nid,void *pkt,size_t pkt_len)
	{
	   u_long l = htonl(pkt_len);
	   if (write(nid->fd,&l,sizeof(l)) == -1)
		  return(-1);
	   return(write(nid->fd,pkt,pkt_len));
	}

	static ssize_t netio_tcp_recv(netio_inet_desc_t *nid,void *pkt,size_t max_len)
	{
	   u_long l;
	   if (read(nid->fd,&l,sizeof(l)) != sizeof(l))
		  return(-1);
	   if (ntohl(l) > max_len)
		  return(-1);
	   return(read(nid->fd,pkt,ntohl(l)));
	}
	*/
	// in udp, one frame = one udp packet

	bool end_point::is_active()
	{
		if ( tcp_socket || udp_socket || ssl_socket 
#ifdef VLIN
		|| stm_socket || dgm_socket || tap_psd || vde_data_socket //|| vde_ctl_socket
#endif
		)
		{
			return true;
		}
		return false;
	}

	void end_point::rewrite_loop(frame * f)
	{
		if ( is_active() )
		{
			boost::system::error_code ec;
			write_loop(ec, f);
		}
		else
		{
			vnd_pt->destroy_frame(f);
		}
	}

	void end_point::reread_loop()
	{
		if ( is_active() )
		{
			boost::system::error_code ec;
			read_loop(ec, true); // we can short circuit
		}
	}

	void end_point::handle_read(const boost::system::error_code & ec, std::size_t bytes_transferred)
	{
		read_overlap = false;
		if ( ec ) // read failed
		{
			manage_rw_error(ec, "read");
			return;
		}
		if ( tran_proto == "vde" ) // let's read now!
		{
#ifdef VLIN
			// use available on socket? no
			char * vb = new char[in_buf_size];
			long bt = 0;
			bt = ::vde_recv(vde_con, vb, in_buf_size, 0); // should be fast and return immediately
			if ( bt < 0 )
			{
				vnd_pt->error << "vde_recv failed\n";
				return;
				// do something!
			}
			else
			{
				bytes_transferred = (dword)bt;
				//vnd_pt->output << "vde_recv brought " << bytes_transferred << " bytes\n";
				std::copy(vb, vb + bytes_transferred, in_buf.begin()); // implicit conversion from char to byte?
			}
			delete [] vb;
#endif
		}
		if ( !bytes_transferred )
		{
			vnd_pt->warning << "warning> read succeeded with 0 bytes transferred\n"; // is it an error? no (for now)
			reread_loop();
			return;
		}
		// with async_read_some it may be possible to have a short read with bytes transferred < input buffer size passed to read
		if ( encapsulation == encapsulation_length )
		{
			if ( !waiting_for_next_frame )
			{
				if ( bytes_transferred != 4 ) // may not work if length is stuck to the frame, async_read should read just the good nb
				{
					vnd_pt->error << "error> can not get the next frame's length: disconnecting\n";
					disc();
					return;
				}
				std::copy(in_buf.begin(), in_buf.begin() + 4, in_lbuf);
				next_frame_size = /*n_to_h_l*/(bytes_to_dword(in_lbuf));
				if ( next_frame_size < 16/*64*/ || next_frame_size > 1518 ) // Ethernet standard is 64, but should we count FCS here? yes
				{
					vnd_pt->error << "error> frame length has wrong value: disconnecting\n";
					disc();
					return;
				}
				waiting_for_next_frame = true;
				reread_loop();
				return;
			}
			else // check expected frame size, expand to other tran than tcp?
			{
				waiting_for_next_frame = false;
				if ( bytes_transferred != next_frame_size )
				{
					vnd_pt->error << "error> bytes received do not match frame length: disconnecting\n";
					disc();
					return;
				}
			}
		}
		if ( tran_proto == "udp" ) // manage udp remote endpoint check
		{
			if ( remote_port_nb == "?" && udp_remote_endpoint.port() != 4 ) // first packet from undefined remote
			{
				remote_net_addr = udp_remote_endpoint.address().to_string();
				std::ostringstream oss;
				oss << udp_remote_endpoint.port();
				remote_port_nb = oss.str();
				// shall we connect the server to this client? yes (for the moment)
				udp_socket->connect(make_udp_remote_endpoint());
				// should i manage errors? i guess
			}
			if ( false )//udp_remote_endpoint != make_udp_remote_endpoint() ) // shall we check each packet? no
			{
				vnd_pt->warning << "warning> in handle_read: remote udp address or port is different from expected\n";
				// given the previous code, this should never happen
				// shall we close the socket? not for the moment
			}
		}
        frame * f = vnd_pt->create_frame(); // ts will be the real received time
		f->buffer.resize(bytes_transferred); // should we use a fixed size for efficiency?
		std::copy(in_buf.begin(), in_buf.begin() + bytes_transferred, f->buffer.begin());
		//in_buf.assign(in_buf_size, 0x00);
		/* if ( bound_if )
		{
			bound_if->input_queue->frames_tss.push(f->arr_ts); // should not overlap because async_read does not
		} */
		//f->tt1 = boost::chrono::process_user_cpu_clock::now();
		current_frame_arrival = f->arr_ts;
		// no more tracing at endpoint
        if ( bound_if ) // push in delay buffer //inqueue
		{
			vnd_pt->io_serv->post(boost::bind(&hold_queue::preprocess, bound_if->input_queue, f)); // concur access needed?
		}
		else
		{
			vnd_pt->destroy_frame(f);
			vnd_pt->warning << "warning> ep not bound to line interface (in), dropping frame\n";
		}
		reread_loop();
	}

	void end_point::handle_write(const boost::system::error_code & ec, std::size_t bytes_transferred)
	{
		write_overlap = false;
		if ( ec ) // write failed
		{
			manage_rw_error(ec, "write");// of " + bytes_transferred + " bytes");
			return;
		}
		/* if ( bound_if )
		{
			// solution 3: including socket time and assuming ordered posts! still, it does not work
			fpsec fps = (boost::chrono::high_resolution_clock::now() - bound_if->output_queue->frames_tss.front());
			bound_if->output_queue->frames_tss.pop();
			bound_if->output_queue->activity_duration += fps;
		} */		
		if ( tran_proto == "vde" ) // let's write now!
		{
#ifdef VLIN
			// use available on socket? no
			byte * vb = new byte[total_bytes_to_write];//in_buf_size];
			std::copy(out_buf.begin(), out_buf.begin() + total_bytes_to_write, vb);
			std::cout << "bytes_transferred before vde_send (should be 0)  = " << bytes_transferred << std::endl;
			long bt = 0;
			//do {
			bt = ::vde_send(vde_con, reinterpret_cast<const char *>(vb), total_bytes_to_write, 0); // should be fast and return immediately
			//} while ( bt < 0 && errno == EINTR ); // in qemu but why??
			if ( bt < 0 )
			{
				std::cout << "error in vde_send which returned " << bt << std::endl;
				bytes_transferred = 0;
			}
			else
			{
				bytes_transferred = (dword)bt; // conversion signed int : ssize_t -> unsigned int : size_t
			}
			std::cout << "bytes_transferred after vde_send = " << bytes_transferred << std::endl;
			delete [] vb;
#endif
		}
		if ( !bytes_transferred )
		{
			vnd_pt->warning << "warning> send/write succeeded with 0 bytes transferred\n";
		}
		if ( bytes_transferred != total_bytes_to_write )
		{
			vnd_pt->warning << "warning> send/write succeeded with " << (bytes_transferred) << " bytes transferred != " << total_bytes_to_write << " total bytes to write/transfer\n";
			// async_write: this should cause an ec error and thus should not happen
			// async_send(_to)/write_some: this should not cause an ec error and thus may happen
		}
		// no more tracing after a successful transmission, must be done before write even before async_wait
		// is there something to write in the interface output queue?
		if ( bound_if )
		{
			bound_if->output_queue->schedule();
			//vnd_pt->io_serv->post(boost::bind(&hold_queue::schedule, bound_if->output_queue));
		}
	}

	void end_point::handle_vde_ctl(const boost::system::error_code & ec, std::size_t bytes_transferred)
	{
		if ( !ec ) // read succeeded
		{
			char * vb = new char[in_buf_size];
			long bt = 0;
#ifdef VLIN
			bt = ::recv(native_vde_ctl_desc, vb, in_buf_size, 0); // should be fast and return immediately
#endif
			if ( bt < 0 )
			{
				//vnd_pt->error << "vde_ctl recv failed\n";
			}
			else
			{
				bytes_transferred = (dword)bt;
				if ( !bytes_transferred )
				{
					//vnd_pt->output << "vde_ctl read succeeded with 0 bytes transferred\n"; // is it an error? no (for now)
				}
				else
				{
					//vnd_pt->output << "vde_ctl recv " << bytes_transferred << " bytes with content = " << vb << "\n"; // should be EOF
				}
			}
			//vnd_pt->output << "vde connection was stopped by remote switch -> disc and kill ep\n";
			delete [] vb;
			disc();
		}
		else
		{
			manage_rw_error(ec, "vde_ctl");
		}
	}
}
