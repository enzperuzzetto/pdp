
// messenger application (mesap)
// Copyright (C) 2012-2016 Damien Magoni

#include "mesap.hpp"

namespace mesap
{
	messenger::messenger()
	{
		output_mode = console;
		pause_mode = false;
	}

	messenger::messenger(std::string filename, messages_output_type startup_output_mode, bool startup_pause_mode)
	{
		output_mode = startup_output_mode;
		pause_mode = startup_pause_mode;
		if ( output_mode & logging )
		{
			log_file.open(filename.c_str());
			if ( !log_file.is_open() )
			{
				std::cout << "could not create log file\n";
			}
		}
	}

	messenger::~messenger()
	{
		if ( ( output_mode & logging ) && log_file.is_open() )
			log_file.close();
	}

	messenger & messenger::operator<<(const std::string & s)
	{
		if ( output_mode & console )
		{
			std::cout << s;
			std::cout.flush();
		}
		if ( output_mode & logging )
		{
			log_file << s;
			log_file.flush();
		}
		if ( pause_mode )
			util::pause();
		return *this;
	}

	messenger & messenger::operator<<(const char * s)
	{
		if ( output_mode & console )
		{
			std::cout << s;
			std::cout.flush();
		}
		if ( output_mode & logging )
		{
			log_file << s;
			log_file.flush();
		}
		if ( pause_mode )
			util::pause();
		return *this;
	}

	messenger & messenger::operator<<(const int & i)
	{
		if ( output_mode & console )
		{
			std::cout << i;
			std::cout.flush();
		}
		if ( output_mode & logging )
		{
			log_file << i;
			log_file.flush();
		}
		if ( pause_mode )
			util::pause();
		return *this;
	}

	messenger & messenger::operator<<(const unsigned char & i)
	{
		if ( output_mode & console )
		{
			std::cout << i;
			std::cout.flush();
		}
		if ( output_mode & logging )
		{
			log_file << i;
			log_file.flush();
		}
		if ( pause_mode )
			util::pause();
		return *this;
	}

	messenger & messenger::operator<<(const unsigned int & i)
	{
		if ( output_mode & console )
		{
			std::cout << i;
			std::cout.flush();
		}
		if ( output_mode & logging )
		{
			log_file << i;
			log_file.flush();
		}
		if ( pause_mode )
			util::pause();
		return *this;
	}

	messenger & messenger::operator<<(const unsigned long & i)
	{
		if ( output_mode & console )
		{
			std::cout << i;
			std::cout.flush();
		}
		if ( output_mode & logging )
		{
			log_file << i;
			log_file.flush();
		}
		if ( pause_mode )
			util::pause();
		return *this;
	}

	messenger & messenger::operator<<(const double & d)
	{
		if ( output_mode & console )
		{
			std::cout << d;
			std::cout.flush();
		}
		if ( output_mode & logging )
		{
			log_file << d;
			log_file.flush();
		}
		if ( pause_mode )
			util::pause();
		return *this;
	}

	void messenger::flush()
	{
		log_file.flush();
		std::cout.flush();
	}

	void messenger::precision(int p)
	{
		log_file.precision(p);
		std::cout.precision(p);
	}
}
