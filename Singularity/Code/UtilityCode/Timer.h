//	----==== TIMER.H ====----
//
//	Author:			Jeffrey Kiah
//					y2kiah@hotmail.com
//	Version:		1
//	Date:			1/04
//	Description:	Sets up and gives functionality to the high speed Windows timer
//	--------------------------------------------------------------------------------

#ifndef TIMER_H
#define TIMER_H

#include "singleton.h"


/*---------------
---- DEFINES ----
---------------*/

#define timer	Timer::instance()


/*------------------
---- STRUCTURES ----
------------------*/


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
class Timer : public Singleton<Timer> {
	private:
		
		_int64		timerFrequency, currentTime, lastTime, fpsCurrentTime, fpsLastTime;
		float		timeFix, timerFrequencyFix, fps;
		int			numFrames;

		float		getTimePassed(void);

	public:

		//-----------------------------------------------------------------------------------
		//  Must call before using the timer
		//-----------------------------------------------------------------------------------
		void		initTimer(void);

		//-----------------------------------------------------------------------------------
		//	These return and calculate the current frames per second
		//-----------------------------------------------------------------------------------
		float		getFPS(void) const	{ return fps; }
		void		calcFPS(void);		

		//-----------------------------------------------------------------------------------
		//	Units returned are (1.0 / seconds). Multiply by variable to get per-second units
		//-----------------------------------------------------------------------------------
		float		getTimeFix(void) const { return timeFix; }		

		//-----------------------------------------------------------------------------------
		//	Returns time when getTimePassed was last called (usually once per frame)
		//-----------------------------------------------------------------------------------
		_int64		getCurrentTime(void) const { return currentTime; }

		//-----------------------------------------------------------------------------------
		//	Queries the system timer for exact current time
		//-----------------------------------------------------------------------------------
		_int64		queryCurrentTime(void) const;

		//-----------------------------------------------------------------------------------
		//	Returns the number of seconds since time t
		//-----------------------------------------------------------------------------------
		float		calcNumSecondsSinceTime(_int64 t) const;

		//-----------------------------------------------------------------------------------
		//	Multiplier for time based calculations. This should be called once per frame
		//	within the loop. Use getTimeFix to retrieve the result.
		//-----------------------------------------------------------------------------------
		void		calcTimeFix(void) { timeFix = getTimePassed(); } // call once per frame
		
		//-----------------------------------------------------------------------------------
		//	Call this after application is inactive for a while
		//-----------------------------------------------------------------------------------
		void		resetLastTime(void) {	lastTime = fpsLastTime = queryCurrentTime();
											numFrames = 0;
										} 
		
		explicit Timer() : Singleton<Timer>(*this)
		{
			timerFrequency = currentTime = lastTime = fpsCurrentTime = fpsLastTime = 0;
			timeFix = timerFrequencyFix = fps = 0;
			numFrames = 0;
		}
};

#endif