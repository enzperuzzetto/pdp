
// medeb (memory debugging)
// Copyright (C) 2012 Damien Magoni. All rights reserved.

#include <fstream>
#include "medeb.h"

namespace medeb
{
	bool active = true;
	std::map<void *, entry> map;

	void * newm(void * p, int size, char const * file, int line)
	{
		entry d;
		d.byte_size = size;
		d.file_name = file;
		d.line_number = line;
		map[p] = d;
		return 0;//p;
		//void * p = malloc(size);
		//return ::new char [size];
	}

	void delm(void * p)
	{
		map.erase(p);
		//free(p);
	}

	void init()
	{
		map.clear();
	}

	void dump()
	{
		if ( map.empty() )
			return;
		std::ofstream o("mem-dbg.txt");
		o << "number of mem objects = " << map.size() << "\n";
		std::map<void * , entry>::iterator it = map.begin();
		while ( it != map.end() )
		{
			o.flags(std::ios::right|std::ios::hex|std::ios::showbase);
			o << "-> 0x" << std::setw(8) << it->first;
			o.flags(std::ios::right|std::ios::dec);
			o << " [" << it->second.file_name << ":" << it->second.line_number << "] " << it->second.byte_size << " Bytes\n";
			it++;
		}
		o.close();
	}
}
