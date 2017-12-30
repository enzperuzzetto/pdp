
// medeb (memory debugging)
// Copyright (C) 2012 Damien Magoni. All rights reserved.

#if !defined(MEDEB_H)
#define MEDEB_H

#include "util.h"

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
