#pragma once
#include <filesystem>
#include <string>
#include <vector>

enum class Verbosity
{
	none = 0,
	error =  1 << 1,
	winner = 1 << 2,
	status = 1 << 3
};

inline Verbosity operator|(Verbosity a, Verbosity b)
{
	return static_cast<Verbosity>(static_cast<int>(a) | static_cast<int>(b));
}

inline Verbosity operator&(Verbosity a, Verbosity b)
{
	return static_cast<Verbosity>(static_cast<int>(a) & static_cast<int>(b));
}

class settings
{
public:
	static const inline std::filesystem::path settingsPath = "cowdkp/settings.ini";

	static inline std::string logPath = "logs/";
	static inline std::string character = "";
	static inline std::string server = "";
	static inline float auctionTime = 30;
	static inline float auctionBidTimeInc = 3;
	static inline float auctionGoingTime = 5;
	static inline std::vector<std::string> bidChannels;
	static inline std::vector<std::string> mods;
	static inline Verbosity verbose = Verbosity::none;

	static bool load() noexcept;
	static bool isMod(const std::string& name) noexcept;
};