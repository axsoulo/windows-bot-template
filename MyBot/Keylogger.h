#pragma once
#include <string>
#include <fstream>
#include <mutex>


class Keylogger
{
private:
	std::ofstream log_file;
	std::mutex log_mutex;
	bool is_initialized;
	std::string current_log_path;

	//Get the current timestamp as a formatted string
	std::string get_timestamp();

	std::string get_file_timestamp();

public:
	//constructor
	Keylogger();

	//destructor
	~Keylogger();

	//log key press
	void log_key(const std::string& command, const std::string& username);

	// get the current log file path
	std::string get_log_path() const;

	//check if logger is ready
	bool is_ready() const;

	//flush dat file
	void flush();
};

