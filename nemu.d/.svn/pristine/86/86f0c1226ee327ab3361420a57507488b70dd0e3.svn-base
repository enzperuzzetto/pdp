
// virtual network device (vnd)
// Copyright (C) 2012-2016 Damien Magoni

#include "vnd.hpp"

int main(int argc, char * argv[])
{
	mt19937::init_genrand(static_cast<unsigned long>(time(0)));
	int ps = prost::random(100, 600); // x 100 < 65k
	std::ostringstream oss;
	oss << ps;
	std::string sps = oss.str();
	std::map<std::string, std::string> argvs;
	argvs["-n"] = "vnd" + sps;
	argvs["-f"] = "";
	argvs["-d"] = "./";
	argvs["-s"] = sps; // port_seed
	if ( argc > 2 && argc % 2 == 1 )
	{
		for ( int i = 1; i < argc; i = i + 2 )
		{
			std::string k = std::string(argv[i]), v = std::string(argv[i+1]);
			if ( argvs.find(k) != argvs.end() )
				argvs.erase(k);
			argvs[k] = v;
		}
		std::cout 
			<< vnd::copyright << "\n"
			<< vnd::version  << "\n"
			<< "waiting for commands...\n";
	}
	else
	{
		std::cout 
			<< vnd::copyright << "\n"
			<< vnd::version  << "\n"
			<< "syntax: vnd -n <device_name> -f <conf_file> -d <log_dir>\n"
			<< "using default args...\n";
	}
	if ( medeb::active ) medeb::init();
	boost::asio::io_service io_service;
	//network_device v = network_device(argvs); 
	// v is constructed by default before the temporarily created network_device(argvs) is copied into it and there is no proper default ctor!
	vnd::network_device v(argvs); // ctor, v is constructed directly
	v.io_serv = &io_service;
	v.stran = new boost::asio::io_service::strand(*v.io_serv);
	v.run();
	if ( medeb::active ) medeb::dump();
	return EXIT_SUCCESS;
}
