
// memory output (medeb)
// Copyright (C) 2012-2016 Damien Magoni

#include "medeb.hpp"

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
		{
			std::cout << "no memory leaks\n";
			return;
		}
		std::ofstream ofs("mem-dbg.txt");
		ofs << "number of mem objects = " << map.size() << "\n";
		std::map<void * , entry>::iterator it = map.begin();
		while ( it != map.end() )
		{
			ofs.flags(std::ios::right|std::ios::hex|std::ios::showbase);
			ofs << "-> 0x" << std::setw(8) << it->first;
			ofs.flags(std::ios::right|std::ios::dec);
			ofs << " [" << it->second.file_name << ":" << it->second.line_number << "] " << it->second.byte_size << " Bytes\n";
			it++;
		}
		ofs.close();
	}
}
