#pragma once
#include "..\\Window&Render\WindowContainer.h"
#include "..\\Externals\Timer.h"
#include <chrono>
#include <cstdint>

class PowerEngine : WindowContainer
{
public:
	bool Initialize(HINSTANCE hInstance, std::string window_title, std::string window_class, int width, int height);
	bool ProcessMessages();
	void Update();
	void RenderFrame();
	bool IsWindowed = FALSE;
	int maxFPS = 60;
private:
	Timer timer;
	uint64_t timeSinceEpochMillisec();
};