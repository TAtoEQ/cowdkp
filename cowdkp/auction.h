#pragma once
#include <map>
#include <vector>
#include <string>
#include <chrono>
#include <mutex>
#include "settings.h"

using namespace std::chrono_literals;

enum class AuctionState
{
	open = 0,
	closingOnce = 1,
	closingTwice = 2,
	sold = 3,
	expired = 4
};

enum class BidFlags
{
	none = 0,
	ra = 1 << 1,
	sub20 = 1 << 2,
	app = 1 << 3
}; 

inline BidFlags operator|(BidFlags a, BidFlags b)
{
	return static_cast<BidFlags>(static_cast<int>(a) | static_cast<int>(b));
}

inline BidFlags operator&(BidFlags a, BidFlags b)
{
	return static_cast<BidFlags>(static_cast<int>(a) & static_cast<int>(b));
}

struct Bid
{
	static inline std::vector<std::pair<int, int>> minIncs =
	{
		{1000, 100},
		{500, 50},
		{250, 25},
		{0, 10}
	};

	std::string name;
	int bid = 0;
	BidFlags flags = BidFlags::none;
	std::chrono::high_resolution_clock::time_point time = std::chrono::high_resolution_clock::now();
	bool isValid(const Bid& winning) const noexcept;
};

struct Item
{
	std::string item = "";
	std::string link = "";
	std::string starter = "";
};

class Auction
{
private:
	static inline std::recursive_mutex lock;
	static inline std::vector<Auction> auctions;
	static inline std::vector<Item> q;
	static volatile inline bool paused = false;

	Item item;
	std::string channel;
	AuctionState state = AuctionState::open;
	int currentBid = 0;
	std::vector<Bid> bids;
	std::vector<Bid> reservedBids;

	float timeLeft = settings::auctionTime;
	
	void update(float dt) noexcept;
	void resetTime() noexcept;
	static void removeAuction(Auction& a) noexcept;

public:
	static inline std::map<std::string, std::string> channelToText
	{
		{"say", "/say"},
		{"auction", "/auc"},
		{"shout", "/shout"},
		{"out of character", "/ooc"},
		{"guild", "/gu"},
		{"raid", "/rs"},
		{"group", "/g"}
	};

	Auction(const Item& item, const std::string& channel) noexcept;
	bool addBid(const Bid& b, bool checkReserve=true) noexcept;
	void retractBid(const std::string& name) noexcept;

	static std::string chatTextForChannel(const std::string& str) noexcept;
	static Auction* auctionForChannel(const std::string& channel) noexcept;
	static std::string addItemToQueue(const std::string& starter, const std::string& item) noexcept;
	static void updateAuctions(float dt) noexcept;
	static std::string getOpenChannel() noexcept;
	static size_t itemsInQueue() noexcept { return q.size(); }
	static void pauseBids(bool paused) noexcept;
	static void cancelAuctionInChannel(const std::string& channel) noexcept;
};