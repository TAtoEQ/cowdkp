#include <algorithm>
#include <game.h>
#include "auction.h"
#include "settings.h"
#include <fmt/core.h>

bool Bid::isValid(const Bid& winning) const noexcept
{
	// main > ra > sub20 = app

	bool none = flags == BidFlags::none;
	bool ra = (flags & BidFlags::ra) == BidFlags::ra;
	bool app = (flags & BidFlags::app) == BidFlags::app;
	bool sub20 = (flags & BidFlags::sub20) == BidFlags::sub20;

	bool none2 = winning.flags == BidFlags::none;
	bool ra2 = (winning.flags & BidFlags::ra) == BidFlags::ra;
	bool app2 = (winning.flags & BidFlags::app) == BidFlags::app;
	bool sub202 = (winning.flags & BidFlags::sub20) == BidFlags::sub20;

	//if (name == o.name) return false;

	int lootPrio = 3;
	if (ra) lootPrio = 2;
	else if (app || sub20) lootPrio = 1;

	int lootPrio2 = 3;
	if (ra2) lootPrio2 = 2;
	else if (app2 || sub202) lootPrio2 = 1;

	if (lootPrio2 > lootPrio && winning.bid > 0) return false;
	else if (!ra && bid < 100) return false;
	else if (ra && bid < 20) return false;
	else if (lootPrio > lootPrio2) return true;

	int minInc = 0;
	for (const auto [amount, inc] : minIncs)
	{
		if (winning.bid >= amount)
		{
			minInc = inc;
			break;
		}
	}

	if (bid >= winning.bid + minInc) return true;
	return false;
}

Auction::Auction(const Item& item, const std::string& channel)  noexcept
	: item(item), channel(channel)
{
	std::string msg = fmt::format("{} {} bids go ({})", chatTextForChannel(channel), item.link, item.starter);
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
		const Bid& winningBid = (!bids.empty()) ? bids[bids.size() - 1] : Bid();
		if ((winningBid.flags & BidFlags::ra) == BidFlags::ra) bidFlagStr += " ra";
		if ((winningBid.flags & BidFlags::sub20) == BidFlags::sub20) bidFlagStr += " <20";
		if ((winningBid.flags & BidFlags::app) == BidFlags::app) bidFlagStr += " app";

		switch (state)
		{
			case AuctionState::open:
			{
				state = AuctionState::closingOnce;
				std::string msg;
				if (winningBid.bid == 0)
				{
					msg = fmt::format("{} {} bank once", chatTextForChannel(channel), item.link);
				}
				else
				{
					msg = fmt::format("{} {} {} {}{} once", chatTextForChannel(channel), item.link, winningBid.name, winningBid.bid, bidFlagStr);
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
					msg = fmt::format("{} {} bank twice", chatTextForChannel(channel), item.link);
				}
				else
				{
					msg = fmt::format("{} {} {} {}{} twice", chatTextForChannel(channel), item.link, winningBid.name, winningBid.bid, bidFlagStr);
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
					msg = fmt::format("{} {} bank sold", chatTextForChannel(channel), item.link);
				}
				else
				{
					msg = fmt::format("{} {} {} {} sold", chatTextForChannel(channel), item.link, winningBid.name, winningBid.bid);
				}
				Game::hookedCommandFunc(0, 0, 0, msg.c_str());
				timeLeft = 3;
				break;
			}
			case AuctionState::sold:
			{
				std::string msg;
				if ((settings::verbose & Verbosity::winner) == Verbosity::winner )
				{
					if (winningBid.bid == 0)
					{
						msg = fmt::format(";t {} {} banked.", item.starter, item.link, winningBid.name, winningBid.bid);
					}
					else
					{
						msg = fmt::format(";t {} {} won by {} for {} dkp.", item.starter, item.link, winningBid.name, winningBid.bid);
					}
					Game::hookedCommandFunc(0, 0, 0, msg.c_str());
				}

				if (winningBid.bid > 0)
				{
					msg = fmt::format("{} {}-{}@{} GRATSS ({})", chatTextForChannel(channel), item.link, winningBid.name, winningBid.bid, item.starter);
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
	const Bid& winningBid = (!bids.empty()) ? bids[bids.size() - 1] : Bid();
	if (!b.isValid(winningBid)) return false;
	bids.push_back(b);
	state = AuctionState::open;
	timeLeft = std::min(timeLeft + settings::auctionBidTimeInc, settings::auctionTime);
	return true;
}

void Auction::retractBid(const std::string& name) noexcept
{
	if (bids.empty() || state == AuctionState::sold || state == AuctionState::expired) return;
	std::string currentWinner = bids[bids.size() - 1].name;
	bids.erase(std::remove_if(bids.begin(), bids.end(), [&name](const auto& b) { return b.name == name; }), bids.end());
	if (bids.size() > 1)
	{
		for (size_t i = bids.size() - 1; i > 0; --i)
		{
			if (bids[i].name == bids[i - 1].name)
			{
				bids.pop_back();
			}
			else
			{
				break;
			}
		}
	}
	const Bid& newWinner = (!bids.empty()) ? bids[bids.size() - 1] : Bid();
	std::string msg;
	if (newWinner.bid == 0)
	{
		msg = fmt::format("{} {} bids at 0", chatTextForChannel(channel), item.link);
	}
	else
	{
		if (currentWinner != newWinner.name)
		{
			state = AuctionState::open;
		}
		msg = fmt::format("{} {} bids at {} by {}", chatTextForChannel(channel), item.link, newWinner.bid, newWinner.name);
	}
	timeLeft = settings::auctionTime;
	Game::hookedCommandFunc(0, 0, 0, msg.c_str());
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

std::string Auction::chatTextForChannel(const std::string& str) noexcept
{
	auto it = channelToText.find(str);
	if (it != channelToText.end())
	{
		return it->second;
	}
	std::string s = "/chat #";
	return s + str;
}

void Auction::cancelAuctionInChannel(const std::string& channel) noexcept
{
	std::scoped_lock sl{ lock };
	if (channel == "all")
	{
		for (auto& a : auctions)
		{
			std::string msg = fmt::format("{} {} canceled.", chatTextForChannel(a.channel), a.item.link);
			Game::hookedCommandFunc(0, 0, 0, msg.c_str());
		}
		auctions.clear();
		q.clear();
	}
	else
	{
		Auction* a = auctionForChannel(channel);
		if (a)
		{
			std::string msg = fmt::format("{} {} canceled.", chatTextForChannel(a->channel), a->item.link);
			Game::hookedCommandFunc(0, 0, 0, msg.c_str());
			auctions.erase(std::remove_if(auctions.begin(), auctions.end(), [a](const auto& auc) {return a == &auc; }));
		}
	}
}