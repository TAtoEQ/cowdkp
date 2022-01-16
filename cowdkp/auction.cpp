#include <algorithm>
#include <game.h>
#include "auction.h"
#include "settings.h"
#include <fmt/core.h>

bool Bid::isValid(const Bid& o) const noexcept
{
	// main > ra > sub20 = app
	static std::vector<std::pair<int, int>> minIncs =
	{
		{1000, 100},
		{500, 50},
		{250, 25},
		{0, 10}
	};

	bool none = flags == BidFlags::none;
	bool ra = (flags & BidFlags::ra) == BidFlags::ra;
	bool app = (flags & BidFlags::app) == BidFlags::app;
	bool sub20 = (flags & BidFlags::sub20) == BidFlags::sub20;

	bool none2 = o.flags == BidFlags::none;
	bool ra2 = (o.flags & BidFlags::ra) == BidFlags::ra;
	bool app2 = (o.flags & BidFlags::app) == BidFlags::app;
	bool sub202 = (o.flags & BidFlags::sub20) == BidFlags::sub20;

	//if (name == o.name) return false;

	int lootPrio = 3;
	if (ra) lootPrio = 2;
	else if (app || sub20) lootPrio = 1;

	int lootPrio2 = 3;
	if (ra2) lootPrio2 = 2;
	else if (app2 || sub202) lootPrio2 = 1;

	if (lootPrio2 > lootPrio && o.bid > 0) return false;
	else if (!ra && bid < 100) return false;
	else if (ra && bid < 20) return false;
	else if (lootPrio > lootPrio2) return true;

	int minInc = 0;
	for (const auto [amount, inc] : minIncs)
	{
		if (o.bid >= amount)
		{
			minInc = inc;
			break;
		}
	}

	if (bid >= o.bid + minInc) return true;
	return false;
}

Auction::Auction(const Item& item, const std::string& channel)  noexcept
	: item(item), channel(channel)
{
	std::string msg = fmt::format("{} {} bids go ({})", channelToText[channel], item.link, item.starter);
	Game::hookedCommandFunc(0, 0, 0, msg.c_str());
}

std::string Auction::getOpenChannel() noexcept
{
	for (const auto& chan : settings::bidChannels)
	{
		Auction* a = auctionForChannel(chan);
		if (!a)
		{
			return chan;
		}
	}
	return "";
}

Auction* Auction::auctionForChannel(const std::string& channel) noexcept
{
	std::scoped_lock sl{ lock };
	for (size_t i=0; i<auctions.size(); ++i)
	{
		if (auctions[i].channel == channel)
			return &auctions[i];
	}
	return nullptr;
}

std::string Auction::addItemToQueue(const std::string& starter, const std::string& item) noexcept
{
	std::string link = Game::findLinkForItem(item);
	if (!link.empty())
	{
		std::scoped_lock sl{ lock };
		q.emplace_back(item, link, starter);
		return link;
	}
	else
	{
		fmt::print("No item link found for {}.\n", item);
	}
	return "";
}

void Auction::update(float dt) noexcept
{
	if (state == AuctionState::expired) return;

	timeLeft -= dt;
	if (timeLeft <= 0.0f)
	{
		std::string bidFlagStr = "";
		if ((winningBid.flags & BidFlags::ra) == BidFlags::ra) bidFlagStr += " ra";
		if ((winningBid.flags & BidFlags::sub20) == BidFlags::sub20) bidFlagStr += " app";
		if ((winningBid.flags & BidFlags::app) == BidFlags::app) bidFlagStr += " <20";

		switch (state)
		{
			case AuctionState::open:
			{
				state = AuctionState::closingOnce;
				std::string msg;
				if (winningBid.bid == 0)
				{
					msg = fmt::format("{} {} bank once", channelToText[channel], item.link);
				}
				else
				{
					msg = fmt::format("{} {} {} {}{} once", channelToText[channel], item.link, winningBid.name, winningBid.bid, bidFlagStr);
				}
				Game::hookedCommandFunc(0, 0, 0, msg.c_str());
				timeLeft = settings::auctionGoingTime;
				break;
			}
			case AuctionState::closingOnce:
			{
				state = AuctionState::closingTwice;
				std::string msg;
				if (winningBid.bid == 0)
				{
					msg = fmt::format("{} {} bank twice", channelToText[channel], item.link);
				}
				else
				{
					msg = fmt::format("{} {} {} {}{} twice", channelToText[channel], item.link, winningBid.name, winningBid.bid, bidFlagStr);
				}
				Game::hookedCommandFunc(0, 0, 0, msg.c_str());
				timeLeft = settings::auctionGoingTime;
				break;
			}
			case AuctionState::closingTwice:
			{
				state = AuctionState::sold;
				std::string msg;
				if (winningBid.bid == 0)
				{
					msg = fmt::format("{} {} bank sold", channelToText[channel], item.link);
				}
				else
				{
					msg = fmt::format("{} {} {} {} sold", channelToText[channel], item.link, winningBid.name, winningBid.bid);
				}
				Game::hookedCommandFunc(0, 0, 0, msg.c_str());
				timeLeft = 3;
				break;
			}
			case AuctionState::sold:
			{
				std::string msg;
				if (winningBid.bid == 0)
				{
					msg = fmt::format(";t {} {} banked.", item.starter, item.link, winningBid.name, winningBid.bid);
				}
				else
				{
					msg = fmt::format(";t {} {} won by {} for {} dkp.", item.starter, item.link, winningBid.name, winningBid.bid);
				}
				Game::hookedCommandFunc(0, 0, 0, msg.c_str());

				if (winningBid.bid > 0)
				{
					msg = fmt::format("{} {}-{}@{} GRATSS ({})", channelToText[channel], item.link, winningBid.name, winningBid.bid, item.starter);
					Game::hookedCommandFunc(0, 0, 0, msg.c_str());
				}
				state = AuctionState::expired;
			}
		}
	}
}

bool Auction::addBid(const Bid& b) noexcept
{
	std::scoped_lock sl{ lock };
	if (state == AuctionState::sold || state == AuctionState::expired) return false;
	if (!b.isValid(winningBid)) return false;
	winningBid = b;
	state = AuctionState::open;
	timeLeft = std::min(timeLeft + settings::auctionBidTimeInc, settings::auctionTime);
	return true;
}

void Auction::removeAuction(Auction& a) noexcept
{

}

void Auction::resetTime() noexcept
{
	if (state != AuctionState::sold && state != AuctionState::expired)
	{
		timeLeft = settings::auctionTime;
		state = AuctionState::open;
	}
}

void Auction::pauseBids(bool p) noexcept
{
	paused = p;
	if (!paused)
	{
		for (auto& a : auctions)
		{
			a.resetTime();
		}
	}
}

void Auction::updateAuctions(float dt) noexcept
{
	if (paused) return;
	std::scoped_lock sl{ lock };
	for (auto& a : auctions)
	{
		a.update(dt);
	}
	auctions.erase(std::remove_if(auctions.begin(), auctions.end(), [](const auto& a) { return a.state == AuctionState::expired; }), auctions.end());
	
	while (auctions.size() < settings::bidChannels.size() && !q.empty())
	{
		bool stop = true;
		for (size_t i = 0; i < q.size(); ++i)
		{
			Item& item = q[i];
			bool found = false;
			for (const auto& a : auctions)
			{
				if (a.item.item == item.item)
				{
					found = true;
					break;
				}
			}
			if (!found)
			{
				auto it = q.begin() + i;
				std::string channel = getOpenChannel();
				auctions.emplace_back(item, channel);
				fmt::print("Starting auction for {} in channel {} by {}.\n", item.item, channel, item.starter);
				q.erase(it);
				stop = false;
				break;
			}
		}
		if (stop) break;
	}
}