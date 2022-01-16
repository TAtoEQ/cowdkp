#pragma once
#include <filesystem>
#include <map>
#include <vector>
#include <regex>


using LogCallbackType = void (*)(const std::smatch&);

class LogParser
{
private:
	volatile bool running = false;
	void parseLine(const std::string& line, const std::vector<std::pair<std::regex, LogCallbackType>>& callbacks);

public:
	LogParser();

	void start(const std::vector<std::pair<std::regex, LogCallbackType>>& callbacks) noexcept;
	void stop() noexcept;

	bool isRunning() const noexcept { return running; }
};