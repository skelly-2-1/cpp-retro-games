/*
@file

	fpsmanager.cpp

@purpose

	Limiting FPS to a given framerate
*/

#include <thread>
#include "fpsmanager.h"

/*
@brief

	Calculates the delay (in nanoseconds) of a specific FPS (frame-to-frame)
*/
std::chrono::nanoseconds retrogames::fpsmanager_t::calculate_delay(uint16_t target_fps)
{
	return std::chrono::nanoseconds(static_cast<uint64_t>(1000000000.0 / (double)target_fps));
}

/*
@brief

	Constructor, calculates sleep_interval
*/
retrogames::fpsmanager_t::fpsmanager_t(const uint16_t target_fps) :
	zero_delay(target_fps == 0)
{
	if (zero_delay) return;

	update_interval = calculate_delay(target_fps);
	update_time_set = false;
}

/*
@brief

	Sleeps until the next frame
*/
void retrogames::fpsmanager_t::run(void)
{
	if (zero_delay) return;

	if (!update_time_set)
	{
		update_time_set = true;

		next_frame = std::chrono::high_resolution_clock::now() + update_interval;

		return;
	}

	if (std::chrono::high_resolution_clock::now() > next_frame)
	{
		//while (std::chrono::high_resolution_clock::now() > next_frame) next_frame += update_interval;

		next_frame = std::chrono::high_resolution_clock::now() + update_interval;
	}
	else
	{
		std::this_thread::sleep_until(next_frame);
	}

	next_frame += update_interval;
}

/*
@brief

	Checks if we should run (non-blocking)
*/
bool retrogames::fpsmanager_t::should_run(void)
{
	if (zero_delay) return true;

	if (!update_time_set)
	{
		update_time_set = true;

		next_frame = std::chrono::high_resolution_clock::now() + update_interval;

		return true;
	}

	auto now = std::chrono::high_resolution_clock::now();

	if (now >= next_frame)
	{
		while (next_frame <= now) next_frame += update_interval;

		return true;
	}

	return false;
}

/*
@brief

	Resets the next_frame (so it runs NOW)
*/
void retrogames::fpsmanager_t::reset(void)
{
	update_time_set = false;
}