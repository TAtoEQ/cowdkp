#pragma once
#include <fmt/core.h>
#include <fstream>
#include <sstream>
#include <thread>
#include <chrono>

#include "logparser.h"
#include "settings.h"

LogParser::LogParser()
{

}

void LogParser::parseLine(const std::string& line, const std::vector<std::pair<std::regex, LogCallbackType>>& callbacks)
{
	std::smatch match;
	for (const auto& [re, cb] : callbacks)
	{
		if (std::regex_match(line, match, re))
		{
			cb(match);
			return;
		}
	}
}

void LogParser::start(const std::vector<std::pair<std::regex, LogCallbackType>>& callbacks) noexcept
{
	using namespace std::chrono_literals;
	std::string logFile = fmt::format("{}eqlog_{}_{}.txt", settings::logPath, settings::character, settings::server);

	running = true;
	std::ifstream log(logFile);
	if (!log.is_open() || !log.good())
	{
		fmt::print("Error opening logfile {}\n", logFile);
		return;
	}
	log.seekg(0, std::ios::end);
	std::streampos lastPos = log.tellg();

	while (running)
	{
		try
		{
			/*if (!log.is_open() || !log.good())
			{
				log = std::ifstream(logPath);
				log.seekg(0, std::ios::end);
			}*/
			if (log.eof())
			{
				log.clear();
			}
			std::string line;
			std::getline(log, line);
			if (line.length() > 0)
			{
				parseLine(line, callbacks);
			}
			else
			{
				std::this_thread::sleep_for(.1s);
			}
		}
		catch (const std::exception& e)
		{
			fmt::print("{}\n", std::string(e.what()));
		}
	}
}

void LogParser::stop() noexcept
{
	running = false;
}