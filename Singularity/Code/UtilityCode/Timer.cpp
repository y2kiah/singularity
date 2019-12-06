//	----==== TIMER.CPP ====----
//
//	Author:			Jeffrey Kiah
//					y2kiah@hotmail.com
//	Version:		1
//	Date:			1/04
//	Description:	Sets up and gives functionality to the high speed Windows timer
//	--------------------------------------------------------------------------------

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include "timer.h"

/*-----------------
---- FUNCTIONS ----
-----------------*/

////////// class Timer //////////

//-----------------------------------------------------------------------
//	Updates member variables for current time, and time difference since
//	the last time the function was called. Private since it is called by
//	calcTimeFix()
//-----------------------------------------------------------------------
float Timer::getTimePassed(void) {
	QueryPerformanceCounter((LARGE_INTEGER *)&currentTime);
	float offset = (float)(currentTime - lastTime) * timerFrequencyFix;
	lastTime = currentTime;
	
	return offset;
}


//-----------------------------------------------------------------------
//	Calculates the frames per second, updates every 75 frames
//-----------------------------------------------------------------------
void Timer::calcFPS(void)
{
	++numFrames;

	if (numFrames == 60) {
		QueryPerformanceCounter((LARGE_INTEGER *)&fpsCurrentTime);
		fps = 76.0f / ((float)(fpsCurrentTime - fpsLastTime) * timerFrequencyFix);
		fpsLastTime = fpsCurrentTime;
		numFrames = 0;
	}
}


//-----------------------------------------------------------------------
//	Sets up the high speed Windows timer. If this feature is not
//	available on a system this function may cause the program to crash.
//-----------------------------------------------------------------------
void Timer::initTimer(void)
{
	QueryPerformanceFrequency((LARGE_INTEGER *)&timerFrequency);
	timerFrequencyFix = 1.0f / (float)timerFrequency;

	QueryPerformanceCounter((LARGE_INTEGER *)&fpsLastTime);
	timeFix = getTimePassed();
}


//-----------------------------------------------------------------------
//	Returns current time directly from system clock
//-----------------------------------------------------------------------
_int64 Timer::queryCurrentTime(void) const
{
	_int64 returnTime;
	QueryPerformanceCounter((LARGE_INTEGER *)&returnTime);
	
	return returnTime;
}


float Timer::calcNumSecondsSinceTime(_int64 t) const
{
	_int64 now;
	QueryPerformanceCounter((LARGE_INTEGER *)&now);
	
	return (float)(now - t) * timerFrequencyFix;
}