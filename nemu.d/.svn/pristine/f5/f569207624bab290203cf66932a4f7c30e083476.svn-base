
// mesap (messenger application)
// Copyright (C) 2012 Damien Magoni. All rights reserved.

#if !defined(MESAP_H)
#define MESAP_H

#include <fstream>
#include "util.h"

namespace mesap
{
	typedef byte messages_output_type;
	const messages_output_type console = 0x01;
	const messages_output_type logging = 0x02;

	class messenger
	{
	public:
		std::ofstream log_file;
		messages_output_type output_mode;
		bool pause_mode;

		messenger();
		messenger(std::string filename, messages_output_type startup_output_mode, bool startup_pause_mode);
		~messenger();
		messenger & operator<<(const std::string & s);
		messenger & operator<<(const char * s);
		messenger & operator<<(const int & i);
		messenger & operator<<(const long & i);
		messenger & operator<<(const unsigned char & i);
		messenger & operator<<(const unsigned int & i);
		messenger & operator<<(const unsigned long & i);
		messenger & operator<<(const double & d);
		void flush();
		void precision(int p);
	};
}

#endif
