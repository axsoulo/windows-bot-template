#include <dpp/dpp.h>
#include <dpp/unicode_emoji.h>
#include <windows.h>
#include <unordered_map>
#include <iostream>
#include <string>
#include <cctype>
#include "ASCIIMenu.h"
#include "Keylogger.h"
#include "MyBot.h"
#include <fstream>

/* Be sure to place your token in the line below.
 * Follow steps here to get a token:
 * https://dpp.dev/creating-a-bot-application.html
 * When you invite the bot, be sure to invite it with the
 * scopes 'bot' and 'applications.commands', e.g.
 * https://discord.com/oauth2/authorize?client_id=940762342495518720&scope=bot+applications.commands&permissions=139586816064
 */

std::string read_token() {
	std::ifstream file("config.txt");
	std::string token;
	std::getline(file, token);
	return token;
}

const std::string    BOT_TOKEN = read_token();

//key press and release
void press_key(WORD virtual_key) {
	INPUT ip = { 0 };
	ip.type = INPUT_KEYBOARD;
	ip.ki.wVk = virtual_key;

	//press key
	SendInput(1, &ip, sizeof(INPUT));

	//delay
	Sleep(20);

	//release key
	ip.ki.dwFlags = KEYEVENTF_KEYUP;
	SendInput(1, &ip, sizeof(INPUT));
}

int main()
{
	/* Create bot cluster */
	dpp::cluster bot(BOT_TOKEN, dpp::i_default_intents | dpp::i_message_content);

	//too lazy to figure out how to pull the channel id -- nvm
	dpp::snowflake history_channel_id = 1499165401781899386;
	dpp::snowflake display_channel_id = 1417673123821522997;
	ASCIIMenu display(bot, display_channel_id);
	Keylogger key_logger;
	std::string BOT_TOKEN = read_token_from_config();

	/* Output simple log messages to stdout */
	bot.on_log(dpp::utility::cout_logger());

	if (key_logger.is_ready()) {
		std::cout << "Logging to:" << key_logger.get_log_path() << std::endl;
	}

	/* Register slash command here in on_ready */
	bot.on_ready([&](const dpp::ready_t& event) {

		/* Wrap command registration in run_once to make sure it doesnt run on every full reconnection */
		if (dpp::run_once<struct register_bot_commands>()) {
			std::vector<dpp::slashcommand> slash_commands;
			slash_commands.emplace_back("ping", "Ping pong!", bot.me.id); //test command
			slash_commands.emplace_back("u", "Press the UP button", bot.me.id);
			slash_commands.emplace_back("l", "Press the LEFT button", bot.me.id);
			slash_commands.emplace_back("d", "Press the DOWN button", bot.me.id);
			slash_commands.emplace_back("r", "Press the RIGHT button", bot.me.id);
			slash_commands.emplace_back("a", "Press the A button", bot.me.id);
			slash_commands.emplace_back("b", "Press the B button", bot.me.id);
			slash_commands.emplace_back("rb", "Press the RIGHT BUMPER button", bot.me.id);
			slash_commands.emplace_back("lb", "Press the LEFT BUMPER button", bot.me.id);
			slash_commands.emplace_back("select", "Press the SELECT button", bot.me.id);
			slash_commands.emplace_back("start", "Press the START button", bot.me.id);


			bot.global_bulk_command_create(slash_commands);
		}

		display.initialize();

		bot.start_timer([&](dpp::timer timer) {
			display.periodic_refresh();
			}, 5000); //5 seconds

		});

	static std::unordered_map<std::string, WORD> commands = {
		{"u", VK_UP},
		{"l", VK_LEFT},
		{"d", VK_DOWN},
		{"r", VK_RIGHT},
		{"a", 0x41},
		{"b", 0x42},
		{"rb", 0x45},
		{"lb", 0x57},
		{"select", VK_SHIFT},
		{"start", VK_RETURN}
	};

	/* Handle slash command with the most recent addition to D++ features, coroutines! */
	bot.on_slashcommand([&](const dpp::slashcommand_t& event) -> dpp::task<void> {
		std::string command_name = event.command.get_command_name();
		static std::unordered_map<std::string, std::pair<WORD, std::string>> slash_map = {

			{"u", {VK_UP, "u"}},
			{"l", {VK_LEFT, "l"}},
			{"d", {VK_DOWN, "d"}},
			{"r", {VK_RIGHT, "r"}},
			{"a", {0x41, "a"}},
			{"b", {0x42, "b"}},
			{"rb", {0x45, "rb"}},
			{"lb", {0x57, "lb"}},
			{"select", {VK_SHIFT, "select"}},
			{"start", {VK_RETURN, "start"}}
		};

		auto it = slash_map.find(command_name);
		if (it != slash_map.end()) {

			press_key(it->second.first);
			bot.message_create(dpp::message(history_channel_id, it->second.second));

			display.record_command(it->second.second);
			Sleep(200);
			event.co_reply("Executed");
		}
		else if (command_name == "ping") {
			co_await event.co_reply("Pong!");
		}

		co_return;
		});

	//Bot activates whenever a message is created
	bot.on_message_create([&](const dpp::message_create_t& event) {

		//make sure bot doesnt infinitely reply to itself
		if (event.msg.author.id == bot.me.id) {
			return;
		}

		std::string content = event.msg.content;

		// convert to lowercase
		std::string lower_content = content;
		for (char& c : lower_content) {
			c = std::tolower(c);
		}

		auto it = commands.find(lower_content);

		if (it == commands.end()) {
			it = commands.find(content);
		}

		if (it != commands.end()) { //makes sure that something is found

			Sleep(66); //delay to help input register

			//key press
			press_key(it->second);
			key_logger.log_key(it->first, event.msg.author.username);
			bot.message_create(dpp::message(history_channel_id, it->first));
			bot.message_add_reaction(event.msg.id, event.msg.channel_id, dpp::unicode_emoji::white_check_mark);
			display.record_command(it->first);
			std::cout << "Executed: " << it->first << " from: " << event.msg.author.username << std::endl;
		}
		else {
			bot.message_add_reaction(event.msg.id, event.msg.channel_id, dpp::unicode_emoji::cross_mark);
			bot.message_create(dpp::message(event.msg.channel_id, "Input failed, probably rate limited or an invalid input."));
			//std::cout << "Input Failed" << std::endl; (removed because console output already happens)
		}
		});


	/* Start the bot */
	bot.start(dpp::st_wait);

	return 0;
}