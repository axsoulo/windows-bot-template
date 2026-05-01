#include "ASCIIMenu.h"
#include <iostream>
#include <sstream>

// Generate the ascii gamepad
std::string ASCIIMenu::generate_display() {
	std::ostringstream ss;

	// Time since last command
	auto now = std::chrono::steady_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now -
		last_command_time).count();

	// Determine which button to highlight
	std::string up_btn = "  UP  ";
	std::string down_btn = "DOWN";
	std::string left_btn = "LEFT";
	std::string right_btn = "RIGHT";
	std::string a_btn = "  A  ";
	std::string b_btn = "  B  ";
	std::string rb_btn = "  RB  ";
	std::string lb_btn = "  LB  ";
	std::string select_btn = "SELECT";
	std::string start_btn = "START";

	// Highlight for command changes
	if (last_command == "u") {
		up_btn = "[UP]";
	}
	else if (last_command == "d") {
		down_btn = "[DOWN]";
	}
	else if (last_command == "l") {
		left_btn = "[LEFT]";
	}
	else if (last_command == "r") {
		right_btn = "[RIGHT]";
	}
	else if (last_command == "a") {
		a_btn = "[A]";
	}
	else if (last_command == "b") {
		b_btn = "[B]";
	}
	else if (last_command == "rb") {
		rb_btn = "[RB]";
	}
	else if (last_command == "lb") {
		lb_btn = "[LB]";
	}
	else if (last_command == "select") {
		select_btn = "[SELECT]";
	}
	else if (last_command == "start") {
		start_btn = "[START]";
	}

	ss << "```\n";
	ss << "-----------------------------------\n";
	ss << "|                                 |\n";
	ss << "|          " << up_btn << "                 |\n";
	ss << "|                                 |\n";
	ss << "|    " << left_btn << "  " << down_btn << "  " << right_btn << "            |\n";
	ss << "|                                 |\n";
	ss << "|        " << lb_btn << "      " << rb_btn << "       |\n";
	ss << "|                                 |\n";
	ss << "|      " << a_btn << "    " << b_btn << "             |\n";
	ss << "|                                 |\n";
	ss << "|   " << select_btn << "    " << start_btn << "               |\n";
	ss << "|                                 |\n";
	ss << "-----------------------------------\n";
	ss << "```\n";

	ss << "**Last Command:** " << (last_command.empty() ? "None" : last_command) << "\n";
	ss << "**Time since last input:** " << elapsed << "ms\n";

	return ss.str();
}

void ASCIIMenu::create_initial_display() {
	std::lock_guard<std::recursive_mutex> lock(display_mutex);

	bot.message_create(dpp::message(channel_id, "Initializing display..."),
		[this](const dpp::confirmation_callback_t& callback) {
			if (!callback.is_error()) {
				dpp::message msg = callback.get<dpp::message>();
				message_id = msg.id;
				display_initialized = true;

				// Update the display proper
				update_display();
				std::cout << "ASCII display created. Message ID: " << message_id << std::endl;
			}
			else {
				std::cerr << "Failed to create display: " << callback.get_error().message << std::endl;
			}
		});
}

// Constructor
ASCIIMenu::ASCIIMenu(dpp::cluster& bot_ref, dpp::snowflake channel)
	: bot(bot_ref), channel_id(channel), message_id(0),
	last_command(""), display_initialized(false) {
	last_command_time = std::chrono::steady_clock::now();
}

// Update the display with the current state
void ASCIIMenu::update_display() {
	std::lock_guard<std::recursive_mutex> lock(display_mutex);

	if (!display_initialized || message_id == 0) {
		return;
	}

	std::string display_content = generate_display();

	dpp::message edit_msg(channel_id, display_content);
	edit_msg.id = message_id;

	bot.message_edit(edit_msg, [this](const dpp::confirmation_callback_t& callback) {
		if (callback.is_error()) {
			// If message is deleted recreate it
			std::cerr << "Failed to edit display, recreating" << std::endl;
			display_initialized = false;
			message_id = 0;
			create_initial_display();
		}
		});
}

// Record an executed command
void ASCIIMenu::record_command(const std::string& command) {
	std::lock_guard<std::recursive_mutex> lock(display_mutex);
	last_command = command;
	last_command_time = std::chrono::steady_clock::now();
	update_display();
}

// Initialize the display
void ASCIIMenu::initialize() {
	create_initial_display();
}

// Periodic refresh
void ASCIIMenu::periodic_refresh() {
	std::lock_guard<std::recursive_mutex> lock(display_mutex);
	if (display_initialized) {
		update_display();
	}
}
