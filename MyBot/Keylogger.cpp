#include "Keylogger.h"
#include <chrono>
#include <iomanip>
#include <sstream>
#include <filesystem>

//Current timestamp as a string
std::string Keylogger::get_timestamp() {
	auto now = std::chrono::system_clock::now();
	auto in_time_t = std::chrono::system_clock::to_time_t(now);

	std::tm bt;
	localtime_s(&bt, &in_time_t);

	std::ostringstream ss;
	ss << std::put_time(&bt, "%Y-%m-%d %H:%M:%S");

	return ss.str();
}

//Timestamp for filename
std::string Keylogger::get_file_timestamp() {
	auto now = std::chrono::system_clock::now();
	auto in_time_t = std::chrono::system_clock::to_time_t(now);

	std::tm bt;
	localtime_s(&bt, &in_time_t);

	std::ostringstream ss;
	ss << std::put_time(&bt, "%Y-%m-%d_%H-%M-%S");

	return ss.str();
}

//Constructor
Keylogger::Keylogger() : is_initialized(false) {
	// create logs directory if it doesnt exist
	if (!std::filesystem::exists("logs")) {
		std::filesystem::create_directories("logs");
	}

	//create log file
	current_log_path = "logs/input_log_" + get_file_timestamp() + ".txt";
	log_file.open(current_log_path, std::ios::out | std::ios::app);

	if (log_file.is_open()) {
		is_initialized = true;

		//writing da header
		log_file << "KEY INPUT LOG\n";
		log_file << "Started at: " << get_timestamp() << "\n";
		log_file << "||||||||||||||||||||||||||||||||||||||||||\n\n";
		log_file << std::left
				<< std::setw(25) << "TIMESTAMP"
				<< std::setw(15) << "COMMAND"
				<< std::setw(20) << "USER" << "\n";
		log_file << std::string(60, '-') << "\n";
		log_file.flush(); //to push data out of memory
	}
}

Keylogger::~Keylogger() { //doesnt work idk why
	if (log_file.is_open()) {
		log_file << "\n||||||||||||||||||||||||||||\n";
		log_file << "Session is over at: " << get_timestamp() << "\n";
		log_file << "||||||||||||||||||||||||||||\n";
		log_file.close();
	}
}

// log key press
void Keylogger::log_key(const std::string& command, const std::string& username) {
	std::lock_guard<std::mutex> lock(log_mutex);

	if (!is_initialized || !log_file.is_open()) {
		return;
	}

	log_file << std::left
		<< std::setw(25) << get_timestamp()
		<< std::setw(15) << command
		<< std::setw(20) << username
		<< "\n";
	log_file.flush();
}

// get current log file path
std::string Keylogger::get_log_path() const {
	return current_log_path;
}

// check if logger ready
bool Keylogger::is_ready() const {
	return is_initialized && log_file.is_open();
}

// manually flush the log file
void Keylogger::flush() {
	std::lock_guard<std::mutex> lock(log_mutex);
	if (log_file.is_open()) {
		log_file.flush();
	}
}