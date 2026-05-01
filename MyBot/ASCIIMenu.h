#pragma once
#include <dpp/dpp.h>
#include <string>
#include <chrono>
#include <mutex>

class ASCIIMenu {
private:
	dpp::cluster& bot;
	dpp::snowflake channel_id;
	dpp::snowflake message_id;
	std::string last_command;
	std::chrono::steady_clock::time_point last_command_time; //measure duration from last command
	std::recursive_mutex display_mutex; //need recursive to prevent deadlocking (multiple threads trying to access the same code)
	bool display_initialized;

	// generate the ascii gamepad
	std::string generate_display();

	void create_initial_display();

public:
	ASCIIMenu(dpp::cluster& bot_ref, dpp::snowflake channel);

	// update the display with the curent state
	void update_display();

	// record an executed command
	void record_command(const std::string& command);

	// initialize the display
	void initialize();

	// periodic refresh
	void periodic_refresh();
};