#include "Timer.h"

Timer::Timer() 
{
	m_StartTime = std::chrono::high_resolution_clock::now();
}


float Timer::Duration()
{
	std::chrono::duration<float> duration = std::chrono::high_resolution_clock::now() - m_StartTime;
	return duration.count();
}

void Timer::Reset()
{
	m_StartTime = std::chrono::high_resolution_clock::now();
}