#pragma once
#include <regex>
#include "logparser.h"

const inline std::regex bidRe{ R"x(^.{26} ([A-Z][a-z]+) (tells?|says?)? ?([ A-Za-z\d]+)(:\d+)?, '(\d+) ?(ra|<20|app)? ?(ra|<20|app)? ?(ra|<20|app)?'$)x" };
void onBid(const std::smatch& match);

const inline std::regex startBidRe{ R"x(^.{26} ([A-Z][a-z]+) tells you, 'start (.+)'$)x" };
void onStartBid(const std::smatch& match);

const inline std::regex pauseBidsRe{ R"x(^.{26} ([A-Z][a-z]+) tells you, '(pause|resume)'$)x" };
void onPause(const std::smatch& match);

const inline std::vector<std::pair<std::regex, LogCallbackType>> logCallbacks =
{
	{bidRe, onBid},
	{startBidRe, onStartBid},
	{pauseBidsRe, onPause}
};