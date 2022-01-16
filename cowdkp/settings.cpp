#include <fmt/core.h>
#include <fstream>
#include "settings.h"

bool settings::load() noexcept
{
	try
	{
		std::ifstream in(settings::settingsPath);
		if (!in.good())
		{
			fmt::print("Error opening settings file. {}\n", settings::settingsPath.string());
			return false;
		}

		std::stringstream ss;
		ss << in.rdbuf();
		in.close();

		std::string key;
		std::string value;
		std::string line;

		while (std::getline(ss, line, '\n'))
		{
			if (line.length() <= 0) continue;
			std::stringstream lineSteam(line);
			std::getline(lineSteam, key, '=');
			std::getline(lineSteam, value, '\n');

			if (key == "character") settings::character = value;
			else if (key == "server") settings::server = value;
			else if (key == "auction_time") settings::auctionTime = std::stof(value);
			else if (key == "auction_bid_time_increment") settings::auctionBidTimeInc = std::stof(value);
			else if (key == "auction_going_time") settings::auctionGoingTime = std::stof(value);
			else if (key == "bid_channel") settings::bidChannels.push_back(value);
		}
	}
	catch (const std::exception& e)
	{
		fmt::print("Error loading settings: {}\n", std::string(e.what()));
		return false;
	}
	return true;
}