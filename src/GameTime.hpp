#pragma once

namespace GameTime
{
	int hour();
	int min();
	int sec();
	int day();

	bool isNight();

	void setCurrentTime(double current);
	double getCurrentTime();

	void update();
}