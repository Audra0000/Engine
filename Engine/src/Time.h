#pragma once
#include "Module.h"

class Time : public Module
{
public:
	Time();
	~Time();

	bool Start() override;
	bool PreUpdate() override;

	float GetDeltaTime() const { return deltaTime; }
	float GetTotalTime() const { return totalTime; }

private:
	float deltaTime;
	float totalTime;
	float lastFrame;
};