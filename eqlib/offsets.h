#pragma once
namespace Offsets
{
	namespace Raid
	{
		static constexpr int WINDOW_ADDR = 0xadeef4;
		static constexpr int WINDOW_OFFSET = 0;
		static constexpr int WINDOW_LOCKED_OFFSET = 0xb96ce0;
		static constexpr int WINDOW_GROUP_BUTTON_BASE_OFFSET = 0x254;
		static constexpr int WINDOW_SELECTED_RAIDER_ADDR = 0xb96814;

		static constexpr int WINDOW_COLOR_ADDR = 0xadeee8;
		static constexpr int WINDOW_COLOR_BASE_OFFSET = 0x25c;
		static constexpr int WINDOW_COLOR_OFFSET = 0x50;
		static constexpr int WINDOW_OPEN_OFFSET = 0x20c;

		static constexpr int LIST_ADDR = 0xb937a0;
		static constexpr int MY_NAME_ADDR = 0xb931a0;
		static constexpr int NAME_OFFSET = 0;
		static constexpr int LEVEL_OFFSET = 0x80;
		static constexpr int CLASS_OFFSET = 0x84;
		static constexpr int GROUP_OFFSET = 0x90;
		static constexpr int DEAD_OFFSET = 0x95;
		static constexpr int RAIDLEAD_OFFSET = 0x88;
		static constexpr int GROUPLEAD_OFFSET = 0x89;
		static constexpr int ASSIST_OFFSET = 0x8a;
		static constexpr int MARKER_OFFSET = 0x8b;
		static constexpr int LOOTER_OFFSET = 0x8c;
	}

	namespace EQ
	{
		static inline constexpr int INST_ADDR = 0xb9fcec;
		static inline constexpr int CHAR_ADDR = 0xb90d68;
	}
}