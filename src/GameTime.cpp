#include "StdAfx.hpp"
#include "GameTime.hpp"

#define TIMESPEED 10

namespace GameTime
{
	namespace
	{
		int s_hour = 0;
		int s_min = 0;
		int s_sec = 0;
		int s_day = 0;
		bool s_night = false;
		double s_beginTime = 0.0;
		double s_currentTime = 0.0;
	}

	void setCurrentTime(double current)
	{
		s_currentTime = current;
		s_beginTime = emscripten_get_now();
	}

	double getCurrentTime()
	{
		return emscripten_get_now() - s_beginTime + s_currentTime;
	}

	void update()
	{
		const int curTime = (int)getCurrentTime() / TIMESPEED;

		const int sec = curTime / 60;
		s_sec = curTime % 60;
		const int min = sec / 60;
		s_min = sec % 60;
		const int hour = min / 24;
		s_hour = min % 24;
		const int day = hour / 30;
		s_day = hour % 30;

		s_hour++; // 1 based
		s_day++; // 1 based

		s_night = s_hour >= 21 || s_hour <= 6;
	}

	int hour()
	{
		return s_hour;
	}

	int min()
	{
		return s_min;
	}

	int sec()
	{
		return s_sec;
	}

	int day()
	{
		return s_day;
	}

	bool isNight()
	{
		return s_night;
	}
}