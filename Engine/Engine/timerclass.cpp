#include "timerclass.h"

TimerClass::TimerClass()
{}
TimerClass::TimerClass(const TimerClass& other)
{}

TimerClass::~TimerClass()
{}


bool TimerClass::Initialize()
{
	//check if system supports high performance timers
	QueryPerformanceFrequency((LARGE_INTEGER*)&m_frequency);
	if(m_frequency == 0)
		return false;

	//get ticks per MS
	m_ticksPerMs = (float)(m_frequency / 1000);

	//save current time in start time variable
	QueryPerformanceCounter((LARGE_INTEGER*)&m_startTime);

	return true;
}

//calc the frame time
void TimerClass::Frame()
{
	//get the current time
	INT64 currentTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currentTime);

	//calc time difference to last time + resulting frame time
	float timeDifference = 0.0f;
	timeDifference = (float)(currentTime - m_startTime);
	m_frameTime = timeDifference / m_ticksPerMs;

	//set start timer to current time
	m_startTime = currentTime;

	return;
}

float TimerClass::GetTime()
{
	return m_frameTime;
}