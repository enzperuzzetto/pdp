
// memory output (medeb)
// Copyright (C) 2012-2016 Damien Magoni

#ifndef MEDEB_HPP
#define MEDEB_HPP

#include "util.hpp"

namespace medeb
{
	extern bool active;

	class entry
	{
	public:
		int byte_size;
		std::string file_name;
		int line_number;
	};

	void * newm(void * p, int byte_size, char const * file_name, int line_number);
	void delm(void * p);
	void init();
	void dump();
}

#endif
