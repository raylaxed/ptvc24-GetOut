
#pragma once

#include <thread>
#include <chrono>


/*
A simple timer Class that works like a stopwatch. 
This Class is mainly used to execute tasks after a curtain time without blocking the the simulation

IMPORTANT:
The timer/stopwatch starts automatically after creation
*/
class Timer
{
private:

		
	std::chrono::time_point<std::chrono::steady_clock> m_StartTime;

public:

	Timer();

	//Returns the elapsed time since the timer was created
	float Duration();

	//Resets the timer and immediatly starts it aigain
	void Reset();
	
};