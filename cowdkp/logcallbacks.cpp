#include <regex>
#include <fmt/core.h>
#include <game.h>
#include "settings.h"
#include "logcallbacks.h"
#include "auction.h"

void onBid(const std::smatch& match)
{
	static std::string appStr = "app";
	static std::string raStr = "ra";
	static std::string sub20Str = "<20";

	std::string name = match.str(1);
	if (name == "You") name = settings::character;
	if (!match.str(10).empty()) name = match.str(10);
	std::string channel = match.str(3);
	std::string bid = match.str(5);

	Auction* a = Auction::auctionForChannel(channel);
	if (a)
	{
		BidFlags flags = BidFlags::none;
		for (size_t i = 6; i < match.size(); ++i)
		{
			if (match.str(i) == appStr) flags = flags | BidFlags::app;
			else if (match.str(i) == raStr) flags = flags | BidFlags::ra;
			else if (match.str(i) == sub20Str) flags = flags | BidFlags::sub20;
		}
		std::chrono::high_resolution_clock::time_point now = std::chrono::high_resolution_clock::now();
		a->addBid({ name, std::stoi(bid), flags, now});
	}
	else
	{
		fmt::print("No auction for channel {}.\n", channel);
	}
}

void onStartBid(const std::smatch& match)
{
	if (!settings::isMod(match.str(1))) return;
	std::string link = Auction::addItemToQueue(match.str(1), match.str(2));
	if (!link.empty())
	{
		auto pos = Auction::itemsInQueue();
		fmt::print("Adding item to queue {} {}. ({})\n", match.str(1), match.str(2), pos);
		if ((settings::verbose & Verbosity::status) == Verbosity::status)
		{
			Game::hookedCommandFunc(0, 0, 0, fmt::format(";t {} Added {} to queue. ({})", match.str(1), link, pos).c_str());
		}
	}
	else
	{
		if ((settings::verbose & Verbosity::error) == Verbosity::error)
		{
			Game::hookedCommandFunc(0, 0, 0, fmt::format(";t {} Unable to find link for {}.", match.str(1), match.str(2)).c_str());
		}
	}
}

void onPause(const std::smatch& match)
{
	if (!settings::isMod(match.str(1))) return;
	std::string name = match.str(1);
	bool pause = match.str(2) == "pause";
	Auction::pauseBids(pause);
}

void onCancel(const std::smatch& match)
{
	if (!settings::isMod(match.str(1))) return;
	Auction::cancelAuctionInChannel(match.str(2));
}

void onRetract(const std::smatch& match)
{
	std::string name = match.str(1);
	if (name == "You") name = settings::character;
	std::string channel = match.str(3);
	Auction* a = Auction::auctionForChannel(channel);
	if (a)
	{
		a->retractBid(name);
	}
}