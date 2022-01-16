#pragma once
#include <filesystem>
#include <string>
#include <vector>

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

	static bool load() noexcept;
	static bool isMod(const std::string& name) noexcept;
};