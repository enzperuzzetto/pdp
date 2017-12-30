
// utilities (util)
// Copyright (C) 2012-2016 Damien Magoni

#include "util.hpp"

namespace util
{
	std::string print_ascii(std::vector<byte> v)
	{
		std::string s;
		s.assign(v.begin(), v.end());
		return s;
	}

	std::string print_hexa(std::vector<byte> v)
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
		os << "\n"; 
		os.flags(std::ios::right|std::ios::dec);
		os.flush();
		return os.str();
	}

	void word_to_bytes(word u, std::vector<byte> & v) // MSB first
	{
		v.push_back(u >>  8 & 0xFF);
		v.push_back(u >>  0 & 0xFF);
	}

	void dword_to_bytes(dword u, std::vector<byte> & v) // MSB first
	{
		v.push_back(u >> 24 & 0xFF);
		v.push_back(u >> 16 & 0xFF);
		v.push_back(u >>  8 & 0xFF);
		v.push_back(u >>  0 & 0xFF);
	}

	void qword_to_bytes(qword u, std::vector<byte> & v) // MSB first
	{
		v.push_back(u >> 56 & 0xFF);
		v.push_back(u >> 48 & 0xFF);
		v.push_back(u >> 40 & 0xFF);
		v.push_back(u >> 32 & 0xFF);
		v.push_back(u >> 24 & 0xFF);
		v.push_back(u >> 16 & 0xFF);
		v.push_back(u >>  8 & 0xFF);
		v.push_back(u >>  0 & 0xFF);
	}

	void pause()
	{
		std::cout << "interactive pause: hit any key to continue\n";
		getchar();
	}

	std::string timef(time_format format) // = time_date_option)
	{
		time_t t1;
		time(&t1);
		struct tm * t2 = localtime(&t1); // should be replaced by boost/C++11 code
		std::ostringstream ss;
		if ( format == time_option )
			ss << t2->tm_hour << ":" << t2->tm_min << ":" << t2->tm_sec;
		else if ( format == date_option )
			ss << t2->tm_mday << "/" << t2->tm_mon + 1 << "/" << t2->tm_year - 100;
		else if ( format == time_date_option )
			ss << t2->tm_hour << ":" << t2->tm_min << ":" << t2->tm_sec << "-" <<
			t2->tm_mday << "/" << t2->tm_mon + 1 << "/" << t2->tm_year - 100;
		else if ( format == filename_suffix_option )
			ss << t2->tm_hour << "h" << t2->tm_min << "m" << t2->tm_sec << "-" <<
			t2->tm_mday << "-" << t2->tm_mon + 1 << "-" << t2->tm_year - 100;
		else
			ss << t2->tm_hour << ":" << t2->tm_min << ":" << t2->tm_sec << "-" <<
			t2->tm_mday << "/" << t2->tm_mon + 1 << "/" << t2->tm_year - 100;
		ss << '\0'; // must add '\0'
		std::string s(ss.str());
		return s;
	}
}