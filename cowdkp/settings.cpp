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

			if (key[0] == '#') continue;
			else if (key == "character") settings::character = value;
			else if (key == "server") settings::server = value;
			else if (key == "auction_time") settings::auctionTime = std::stof(value);
			else if (key == "auction_bid_time_increment") settings::auctionBidTimeInc = std::stof(value);
			else if (key == "auction_going_time") settings::auctionGoingTime = std::stof(value);
			else if (key == "bid_channel") settings::bidChannels.push_back(value);
			else if (key == "mod") settings::mods.push_back(value);
			else if (key == "verbose")
			{
				if (value == "status") settings::verbose = settings::verbose | Verbosity::status;
				else if (value == "error") settings::verbose = settings::verbose | Verbosity::error;
				else if (value == "winner") settings::verbose = settings::verbose | Verbosity::winner;
			}
		}
	}
	catch (const std::exception& e)
	{
		fmt::print("Error loading settings: {}\n", std::string(e.what()));
		return false;
	}
	return true;
}

bool settings::isMod(const std::string& name) noexcept
{
	return std::find(mods.cbegin(), mods.cend(), name) != mods.cend();
}