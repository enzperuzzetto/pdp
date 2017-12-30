
// end_point
// Copyright (c) 2012-2016 Damien Magoni

/*
#define WIN_DOWS
#ifdef WIN_DOWS
	#include <winsock2.h>
#else
	//#include <netdb.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
#endif
*/

#include "end_point.h"

namespace nemo
{

frame::frame()
{
	buffer.clear();
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
	std::vector<byte> bcast;
	bcast.assign(6, 0xFF);
	if ( address_bytes.size() != bcast.size() )
		return false;
	return ( address_bytes == bcast );
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

end_point::end_point()
{
	server = false;
	read_overlap = false;
	write_overlap = false;
	waiting_for_next_frame = false;
	total_bytes_to_write = 0;
	tcp_socket = 0;
	tcp_acc_socket = 0;
	udp_socket = 0;
	nemo_pt = 0;
	in_buf_size = default_buffer_size;
	out_buf_size = default_buffer_size;
	in_buf.resize(in_buf_size);
	in_buf.assign(in_buf_size, 0x00);
	out_buf.resize(out_buf_size);
	out_buf.assign(out_buf_size, 0x00);
	udp_remote_endpoint.address(boost::asio::ip::address::from_string("127.0.0.2"));
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
//	static const boost::regex e("^([a-zA-Z0-9]|[a-zA-Z0-9][a-zA-Z0-9\-]{0,61}[a-zA-Z0-9])(\.([a-zA-Z0-9]|[a-zA-Z0-9][a-zA-Z0-9\-]{0,61}[a-zA-Z0-9]))*$");
	static const boost::regex e("^([a-zA-Z0-9]|[a-zA-Z0-9][a-zA-Z0-9\\-]{0,61}[a-zA-Z0-9])(\\.([a-zA-Z0-9]|[a-zA-Z0-9][a-zA-Z0-9\\-]{0,61}[a-zA-Z0-9]))*$");
	return regex_match(s, e);
}

// when connected use (on socket):
// local_endpoint();
// remote_endpoint();

boost::asio::ip::udp::endpoint end_point::make_udp_local_endpoint() // never call more than once!
{
	std::string port_nb;
	if ( local_port_nb == "*" )
	{
		std::ostringstream oss;
		oss << nemo_pt->udp_base_source_port++;
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
		if ( nemo_pt->udp_resolver && ( is_fqdn(net_addr) || net_addr == "*" ) )
		{
			if ( net_addr == "*" )
			{
				net_addr = boost::asio::ip::host_name(); // returns computer name on windows!
				if ( nemo_pt->debug )
					nemo_pt->output << "host_name() returned: " << net_addr << "\n";
				net_addr = "localhost";
			}
			boost::asio::ip::udp::resolver::query query(net_addr, port_nb); // why protocol dependent?
			boost::asio::ip::udp::resolver::iterator endpoint_iterator = nemo_pt->udp_resolver->resolve(query);
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
		nemo_pt->error << "\nexception in make_udp_endpoint: " <<  e.what() << "\n";
		throw;
	}
}

boost::asio::ip::tcp::endpoint end_point::make_tcp_local_endpoint() // never call more than once!
{
	std::string port_nb;
	if ( local_port_nb == "*" )
	{
		std::ostringstream oss;
		oss << nemo_pt->tcp_base_source_port++;
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
		if ( nemo_pt->tcp_resolver && ( is_fqdn(net_addr) || net_addr == "*" ) )
		{
			if ( net_addr == "*" )
			{
				net_addr = boost::asio::ip::host_name(); // returns computer name on windows!
				if ( nemo_pt->debug )
					nemo_pt->output << "host_name() returned: " << net_addr << "\n";
				net_addr = "localhost";
			}
			boost::asio::ip::tcp::resolver::query query(net_addr, port_nb); // why protocol dependent?
			boost::asio::ip::tcp::resolver::iterator endpoint_iterator = nemo_pt->tcp_resolver->resolve(query);
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
		nemo_pt->error << "\nexception in make_tcp_endpoint: " <<  e.what() << "\n";
		throw;
	}
}

void end_point::print_ascii(std::vector<byte> v)
{
	std::string s;
	s.assign(v.begin(), v.end());
	if ( nemo_pt->debug )
		nemo_pt->output << "\n> handle r/w: " << s << " (wrote " << static_cast<dword>(v.size()) << " bytes of data including newline)\n% ";
}

void end_point::print_hexa(std::vector<byte> v)
{
	std::ostringstream os;
	os.flags(std::ios::right|std::ios::hex);
	for ( dword i = 0; i < v.size(); i++ )
	{
		dword j = v[i];
		if ( !(i % 16) )
			os << "\n" << std::setw(4) << std::setfill('0') << i << "  ";
		else if ( !(i % 8) )
			os << " ";
		else
			;
		os << std::setw(2) << std::setfill('0') << j << " ";
	}
	os << "\n\n"; 
	os.flags(std::ios::right|std::ios::dec);
	if ( nemo_pt->debug )
		nemo_pt->output << os.str();
}

// conn <ep> <proto> {<laddr>|*} {<lport>|*} <raddr> <rport>
void end_point::conn()
{
	if ( tran_proto == "tcp" )
	{
		try
		{
			tcp_socket = new boost::asio::ip::tcp::socket(*nemo_pt->io_serv);
			if ( medeb::active ) medeb::newm(tcp_socket, sizeof(boost::asio::ip::tcp::socket),__FILE__, __LINE__);
			tcp_socket->open(boost::asio::ip::tcp::v4()); // protocol dependent!
			if ( local_net_addr != "*" || local_port_nb != "*" ) // if * * -> no bind
				tcp_socket->bind(make_tcp_local_endpoint());
			tcp_socket->connect(make_tcp_remote_endpoint());
			tcp_socket->set_option(boost::asio::socket_base::keep_alive(true));
			tcp_socket->set_option(boost::asio::socket_base::linger(false, 0));
			//tcp_socket->set_option(boost::asio::socket_base::receive_buffer_size(max_frame_size));
			//tcp_socket->set_option(boost::asio::socket_base::send_buffer_size(max_frame_size));
			tcp_socket->set_option(boost::asio::ip::tcp::no_delay(true));
			boost::system::error_code ec;
			read_loop(ec, true);
		}
		catch (std::exception & e)
		{
			nemo_pt->error << "\nexception in conn: " <<  e.what() << "\n";
			throw;
		}
	}
	else if ( tran_proto == "udp" )
	{
		try
		{
			udp_socket = new boost::asio::ip::udp::socket(*nemo_pt->io_serv);
			if ( medeb::active ) medeb::newm(udp_socket, sizeof(boost::asio::ip::udp::socket),__FILE__, __LINE__);
			udp_socket->open(boost::asio::ip::udp::v4()); // protocol dependent!
			if ( local_net_addr != "*" || local_port_nb != "*" )
				udp_socket->bind(make_udp_local_endpoint());
			udp_socket->connect(make_udp_remote_endpoint());
			//udp_socket->set_option(boost::asio::socket_base::receive_buffer_size(max_frame_size));
			//udp_socket->set_option(boost::asio::socket_base::send_buffer_size(max_frame_size));
			boost::system::error_code ec;
			read_loop(ec, true);
		}
		catch (std::exception & e)
		{
			nemo_pt->error << "\nexception in conn: " <<  e.what() << "\n";
		}
	}
	else
	{
		nemo_pt->error << "\nerror in conn: unknown transport protocol\n";
	}
}

void end_point::handle_connect(const boost::system::error_code & ec) // only useful for async_connect
{
	if ( !ec ) // connect succeeded
	{
		boost::system::error_code ec;
		read_loop(ec, true);
	}
	else
	{
		nemo_pt->error << "\nerror in handle connect: " << ec.value() << " = " << ec.message() << "\n";
	}
}

// serv {raw|qemu} <ep> <proto> {<laddr>|*} {<lport>|*} {<raddr>|?} {<rport>|?}
void end_point::serv()
{
	server = true;
	if ( tran_proto == "tcp" )
	{
		try
		{
			tcp_acc_socket = new boost::asio::ip::tcp::acceptor(*nemo_pt->io_serv);
			if ( medeb::active ) medeb::newm(tcp_acc_socket, sizeof(boost::asio::ip::tcp::acceptor),__FILE__, __LINE__);
			tcp_acc_socket->open(boost::asio::ip::tcp::v4()); // protocol dependent!
			tcp_acc_socket->bind(make_tcp_local_endpoint());
			tcp_acc_socket->listen(1);
			tcp_acc_socket->set_option(boost::asio::socket_base::reuse_address(true));
			tcp_socket = new boost::asio::ip::tcp::socket(*nemo_pt->io_serv);
			if ( medeb::active ) medeb::newm(tcp_socket, sizeof(boost::asio::ip::tcp::socket),__FILE__, __LINE__);
			tcp_acc_socket->async_accept(*tcp_socket, boost::bind(&end_point::handle_accept, this, boost::asio::placeholders::error));
		}
		catch (std::exception & e)
		{
			nemo_pt->error << "\nexception in serv: " <<  e.what() << "\n";
			throw;
		}
	}
	else if ( tran_proto == "udp" )
	{
		try
		{
			udp_socket = new boost::asio::ip::udp::socket(*nemo_pt->io_serv);
			if ( medeb::active ) medeb::newm(udp_socket, sizeof(boost::asio::ip::udp::socket),__FILE__, __LINE__);
			udp_socket->open(boost::asio::ip::udp::v4()); // protocol dependent!
			udp_socket->bind(make_udp_local_endpoint());
			if ( remote_net_addr != "?" && remote_port_nb != "?" )
				udp_socket->connect(make_udp_remote_endpoint()); // establish a filter for any other endpoint
			//udp_socket->set_option(boost::asio::socket_base::receive_buffer_size(max_frame_size));
			//udp_socket->set_option(boost::asio::socket_base::send_buffer_size(max_frame_size));
			boost::system::error_code ec;
			read_loop(ec, true);
		}
		catch (std::exception & e)
		{
			nemo_pt->error << "\nexception in serv: " <<  e.what() << "\n";
		}
	}
	else
	{
		nemo_pt->error << "\nerror in serv: unknown transport protocol\n";
	}
}

void end_point::handle_accept(const boost::system::error_code & ec)
{
	if ( !ec ) // accept succeeded
	{
		if ( remote_net_addr != "?" && tcp_socket->remote_endpoint().address().to_string() != remote_net_addr )
		{
			nemo_pt->warning << "\nwarning in handle_accept: client address is different from expected\n";
			// shall we close the socket? not for the moment
		}
		if	( remote_port_nb != "?" && tcp_socket->remote_endpoint().port() != atoi(remote_port_nb.c_str()) )
		{
			nemo_pt->warning << "\nwarning in handle_accept: client port is different from expected\n";
			// shall we close the socket? not for the moment
		}
		tcp_socket->set_option(boost::asio::socket_base::keep_alive(true));
		tcp_socket->set_option(boost::asio::socket_base::linger(true, 0)); // 30
		//tcp_socket->set_option(boost::asio::socket_base::receive_buffer_size(max_frame_size));
		//tcp_socket->set_option(boost::asio::socket_base::send_buffer_size(max_frame_size));
		tcp_socket->set_option(boost::asio::ip::tcp::no_delay(true));
		boost::system::error_code ec;
		read_loop(ec, true);
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
			if ( server && !tcp_acc_socket ) // async calls processed
			{
				kill();
				return;
			}
			if ( !tcp_socket && !udp_socket )
			{
				kill();
				return;
			}
			; // end_point already deleted by a disc in main thread
		}
		else if ( ec == boost::asio::error::make_error_code(boost::asio::error::connection_refused) ) // could happen?
		{
			nemo_pt->error << " : remote host unavailable\n"; // remote closed
		}
		else
		{
			nemo_pt->error << "error in handle accept: " << ec.value() << " = " << ec.message() << "\n";
		}
	}
}

/*
void linux_depleteSendBuffer(int fd) 
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
}
*/

void end_point::kill()
{
	nemo_pt->end_points.erase(id);
	//nemo_pt->binding_table.erase(id);
	if ( medeb::active ) medeb::delm(this);
	delete this;
}

void end_point::disc() // sync, because if async, ep may be deleted before disc occurs
{
	if ( tran_proto == "tcp" ) // terminate cli and serv (unlike remote disc)
	{
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
					;//nemo_pt->error << "\nexception in tcp soc disc: " << e.what() << "\n";
				}
				tcp_socket->close();
				if ( medeb::active ) medeb::delm(tcp_socket);
				delete tcp_socket;
				tcp_socket = 0;
			}
			if ( tcp_acc_socket )
			{
				tcp_acc_socket->cancel();
				tcp_acc_socket->close();
				if ( medeb::active ) medeb::delm(tcp_acc_socket);
				delete tcp_acc_socket;
				tcp_acc_socket = 0;
			}
		}
		catch (std::exception & e)
		{
			nemo_pt->error << "\nexception in tcp disc: " << e.what() << "\n";
		}
	}
	else if ( tran_proto == "udp" )
	{
		try
		{
			if ( udp_socket )
			{
				udp_socket->close();
				if ( medeb::active ) medeb::delm(udp_socket);
				delete udp_socket;
				udp_socket = 0;
			}
		}
		catch (std::exception & e)
		{
			nemo_pt->error << "\nexception in udp disc: " << e.what() << "\n";
		}
	}
	else
	{
		nemo_pt->error << "\nerror in disc: unknown transport protocol\n";
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
			if ( ibs < default_buffer_size || ibs > max_buffer_size )
				in_buf_size = default_buffer_size;
			else
				in_buf_size = ibs;
		}
		in_buf.resize(in_buf_size); // in_buf should not be currently used
		in_buf.assign(in_buf_size, 0x00);
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
			if ( obs < default_buffer_size || obs > max_buffer_size )
				out_buf_size = default_buffer_size;
			else
				out_buf_size = obs;
		}
		out_buf.resize(out_buf_size); // out_buf should not be currently used
		out_buf.assign(out_buf_size, 0x00);
	}
	else
	{
		std::cout << "\n> unknown arg (try help)\n";
	}
}

void end_point::msg(std::string s)
{
	total_bytes_to_write = s.size();
	if ( tran_proto == "tcp" )
	{
		if ( write_overlap )
			nemo_pt->error << "async_write overlap\n";
		write_overlap = true;
		boost::asio::async_write(*tcp_socket, boost::asio::buffer(s.data(), s.size()), // watch out for s lifetime!
			boost::bind(&end_point::handle_write, this, boost::asio::placeholders::error, 
				boost::asio::placeholders::bytes_transferred));
	}
	else if ( tran_proto == "udp" )
	{
		if ( server && !udp_remote_endpoint.port() )
		{
			nemo_pt->error << "\nerror in msg : unknown remote endpoint\n";
		}
		else
		{
			if ( write_overlap )
				nemo_pt->error << "async_send(_to) overlap\n";
			write_overlap = true;
			// connected udp
			udp_socket->async_send(boost::asio::buffer(s.data(), s.size()),
				boost::bind(&end_point::handle_write, this,	boost::asio::placeholders::error, 
					boost::asio::placeholders::bytes_transferred));
			// unconnected udp
/*			udp_socket->async_send_to(boost::asio::buffer(s.data(), s.size()), get_udp_remote_endpoint(),
				boost::bind(&end_point::handle_write, this,	boost::asio::placeholders::error, 
					boost::asio::placeholders::bytes_transferred));
*/		}
	}
	else
	{
		nemo_pt->error << "\nerror in msg : unknown transport protocol\n";
	}
}

void end_point::write_loop(const boost::system::error_code & ec, frame * f)
{
	if ( ec ) // used by async_wait
	{
		if ( ec == boost::asio::error::operation_aborted )
			nemo_pt->warning << "\n> [write_loop] the timer was cancelled (because of a call to expire or cancel): " << ec.value() << " = " << ec.message() << "\n";
		else
			nemo_pt->error << "\n> error in write_loop (timer pb from async_wait): " << ec.value() << " = " << ec.message() << "\n";
		nemo_pt->destroy_frame(f);
		return;
	}
	if ( tran_proto == "tcp" )
	{
		total_bytes_to_write = f->buffer.size();
		out_buf.assign(out_buf_size, 0x00);
		std::copy(f->buffer.begin(), f->buffer.end(), out_buf.begin());
		if ( write_overlap )
			nemo_pt->error << "async_write overlap\n";
		write_overlap = true;
		boost::asio::async_write(*tcp_socket, boost::asio::buffer(out_buf, f->buffer.size()),
			boost::bind(&end_point::handle_write, this, boost::asio::placeholders::error, 
				boost::asio::placeholders::bytes_transferred));
	}
	else if ( tran_proto == "udp" )
	{
		if ( server && udp_remote_endpoint.port() == 4 )
		{
			nemo_pt->error << "\nerror in async write data : unknown remote endpoint\n";
		}
		else
		{
			total_bytes_to_write = f->buffer.size();
			if ( write_overlap )
				nemo_pt->error << "async_send(_to) overlap\n";
			write_overlap = true;
			out_buf.assign(out_buf_size, 0x00);
			std::copy(f->buffer.begin(), f->buffer.end(), out_buf.begin());
			// connected udp
			udp_socket->async_send(boost::asio::buffer(out_buf, f->buffer.size()), 
				boost::bind(&end_point::handle_write, this, boost::asio::placeholders::error, 
					boost::asio::placeholders::bytes_transferred));
			// unconnected udp
/*			udp_socket->async_send_to(boost::asio::buffer(out_buf, f->buffer.size()), get_udp_remote_endpoint(),
				boost::bind(&end_point::handle_write, this, boost::asio::placeholders::error, 
					boost::asio::placeholders::bytes_transferred));
*/		}
	}
	else
	{
		nemo_pt->error << "\nerror in msg : unknown transport protocol\n";
	}
	nemo_pt->destroy_frame(f);
}

void end_point::read_loop(const boost::system::error_code & ec, bool short_circuit)
{
	if ( ec ) // used by async_wait
	{
		if ( ec == boost::asio::error::operation_aborted )
			nemo_pt->warning << "\n> [read_loop] the timer was cancelled (because of a call to expire or cancel): " << ec.value() << " = " << ec.message() << "\n";
		else
			nemo_pt->error << "\n> error in read_loop (timer pb from async_wait): " << ec.value() << " = " << ec.message() << "\n";
		return;
	}
	if ( tran_proto == "tcp" )
	{
		if ( read_overlap )
			nemo_pt->error << "async_read_some overlap\n";
		in_buf.assign(in_buf_size, 0x00);
		tcp_socket->async_read_some(boost::asio::buffer(in_buf), 
			boost::bind(&end_point::handle_read, this, boost::asio::placeholders::error, 
				boost::asio::placeholders::bytes_transferred));
		read_overlap = true;
	}
	else if ( tran_proto == "udp" )
	{
		if ( read_overlap )
			nemo_pt->error << "async_receive_from overlap\n";
		in_buf.assign(in_buf_size, 0x00);
		udp_socket->async_receive_from(boost::asio::buffer(in_buf),	udp_remote_endpoint,
			boost::bind(&end_point::handle_read, this, boost::asio::placeholders::error, 
				boost::asio::placeholders::bytes_transferred));
		read_overlap = true;
	}
	else
	{
		nemo_pt->error << "\nerror in read_loop : unknown transport protocol\n";
	}
}

void end_point::manage_async_error(const boost::system::error_code & ec, std::string s)
{
	//#define ECONNABORTED 9906 // boost cerrno, 
	//errc::connection_aborted = ECONNABORTED // boost system error_code
	//#define ERROR_CONNECTION_ABORTED         1236L // winerror
	//#define WSAECONNABORTED                  10053L // winerror
	if ( ec.value() == 1236 
		|| ec == boost::asio::error::make_error_code(boost::asio::error::connection_aborted) // tcp local
		|| ec == boost::asio::error::make_error_code(boost::asio::error::operation_aborted) ) // udp local
	{
		if ( server && !tcp_acc_socket ) // async calls processed
		{
			kill();
			return;
		}
		if ( !tcp_socket && !udp_socket )
		{
			kill();
			return;
		}
		// end_point already deleted by main thread
	}
	else if ( ec == boost::asio::error::make_error_code(boost::asio::error::connection_reset) // tcp remote
		|| ec == boost::asio::error::make_error_code(boost::asio::error::eof) // tcp remote closed connection normaly
		|| ec == boost::asio::error::make_error_code(boost::asio::error::broken_pipe) ) // after receiving eof
	{
		if ( server && tran_proto == "tcp" ) // return server to listening state
		{
			try
			{
				tcp_socket->shutdown(boost::asio::ip::tcp::socket::shutdown_both);
				tcp_socket->close();
				if ( medeb::active ) medeb::delm(tcp_socket);
				delete tcp_socket;
				tcp_socket = new boost::asio::ip::tcp::socket(*nemo_pt->io_serv);
				if ( medeb::active ) medeb::newm(tcp_socket, sizeof(boost::asio::ip::tcp::socket),__FILE__, __LINE__);
				tcp_acc_socket->async_accept(*tcp_socket, boost::bind(&end_point::handle_accept, this, boost::asio::placeholders::error));
			}
			catch (std::exception & e)
			{
				nemo_pt->error << "\nexception in manage async error (return to listening state): " << e.what() << "\n";
			}
		}
		else // cli tcp (or cli/serv udp?)
		{
			disc();
		}
	}
	else if ( ec == boost::asio::error::make_error_code(boost::asio::error::connection_refused) ) // udp remote, could be tcp?
	{
		nemo_pt->error << s << " : udp remote unavailable\n"; // remote closed
	}
	else if ( s == "send(_to)/write" )
	{
		nemo_pt->error << "send(_to)/write should have written " << total_bytes_to_write << "\n";
	}
	else
	{
		std::cout << "\n[" << boost::this_thread::get_id() << "] error in handle " << s << ": " << ec.value() << " = " << ec.message() << "\n";
		nemo_pt->error << "closing endpoint to avoid pbs\n";
		disc();
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

void end_point::handle_read(const boost::system::error_code & ec, std::size_t bytes_transferred)
{
	read_overlap = false;
	if ( !ec ) // read succeeded
	{
		if ( !bytes_transferred )
		{
			nemo_pt->warning << "\nread succeeded with 0 bytes transferred\n"; // is it an error? no (for now)
			boost::system::error_code ec;
			if ( tcp_socket || udp_socket )
				read_loop(ec, true); // we can short circuit
			return;
		}
		// with async_read_some it may be possible to have a short read with bytes transferred < input buffer size passed to read
		// get a buffer or a frame
		{
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
					nemo_pt->warning << "\nwarning in handle_read: remote udp address or port is different from expected\n";
					// given the previous code, this should never happen
					// shall we close the socket? not for the moment
				}
			}
			frame * fbuf = new frame;
			if ( medeb::active ) medeb::newm(fbuf, sizeof(frame),__FILE__, __LINE__);
			fbuf->buffer.resize(bytes_transferred);
			std::copy(in_buf.begin(), in_buf.begin() + bytes_transferred, fbuf->buffer.begin());
			in_buf.assign(in_buf_size, 0x00);
			fbuf->size = bytes_transferred;
			fbuf->arr_ts = boost::chrono::high_resolution_clock::now();
			current_frame_arrival = fbuf->arr_ts;
			fbuf->ref_count = 0;
			if ( nemo_pt->debug ) // print in console
			{
				//nemo_pt->output << "\n[" << id << "->" << "console" << "]\n";
				//print_ascii(fbuf->buffer);
			}
			// execute remote order from nemu
			std::string l;
			l.assign(fbuf->buffer.begin(), fbuf->buffer.end()); // l contains \n
			if ( !l.find("#") || !l.size() || !l.find("exit") )//== std::string::npos ) // exit must be at beginning of line!
				;
			else
			{
				if ( nemo_pt->debug )
					nemo_pt->output << "> executing remote order: " << l;// << "...\n";
				nemo_pt->stran->post(boost::bind(&network_mobilizer::command, nemo_pt, l));
			}
			// destroy frame, we do not do forwarding
			{
				nemo_pt->destroy_frame(fbuf);
				boost::system::error_code ec;
				if ( tcp_socket || udp_socket )
					read_loop(ec, true); // we can short circuit
				//return;
			}
		}
	}
	else
	{
		manage_async_error(ec, "read");
	}
}

void end_point::handle_write(const boost::system::error_code & ec, std::size_t bytes_transferred)
{
	write_overlap = false;
	if ( !ec ) // write succeeded
	{
		if ( !bytes_transferred )
		{
			nemo_pt->warning << "send/write succeeded with 0 bytes transferred\n";
		}
		if ( bytes_transferred != total_bytes_to_write )
		{
			nemo_pt->warning << "send/write succeeded with " << static_cast<dword>(bytes_transferred) << " bytes transferred != " << total_bytes_to_write << " bytes to transfer\n";
			// async_write: this should cause an ec error and thus should not happen
			// async_send_to: this should not cause an ec error and thus may happen
		}
		if ( nemo_pt->debug ) // print in console, do it here? or in schedule? here
		{
/*			std::vector<byte> tbuffer;
			tbuffer.resize(bytes_transferred);
			std::copy(out_buf.begin(), out_buf.begin() + bytes_transferred, tbuffer.begin()); // out_buf should not have been overwritten by multiple async_send_to calls!
			out_buf.assign(out_buf_size, 0x00);
			nemo_pt->output << "[in switch, " << id << "->" << "console" << "]\n";
			print_ascii(tbuffer);
*/		}
	}
	else
	{
		manage_async_error(ec, "send(_to)/write");// of " + bytes_transferred + " bytes");
	}
}

}
