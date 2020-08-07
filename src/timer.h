#pragma once

#include <string>
#include <chrono>

class FuncTimer
{
public:
	FuncTimer(const char* name);
	~FuncTimer();
private:
	// When you think about it, we give the name as a static string
	// almost always for timing.
	const char* m_Name;
	std::chrono::time_point<std::chrono::high_resolution_clock> m_Start;
};

class FpsTimer
{
public:
	FpsTimer();
	~FpsTimer() {}
	void Update();
private:
	int m_Frames;
	double m_Lasttime;
};