/*
@file

	fpsmanager.h

@purpose

	Limiting FPS to a given framerate
*/

#pragma once

#include <chrono>

namespace retrogames
{

	class fpsmanager_t final
	{

	protected:



	private:

		std::chrono::nanoseconds update_interval;
		std::chrono::high_resolution_clock::time_point next_frame;

		bool update_time_set;
		bool zero_delay;

	public:

		/*
		@brief

			Constructor, calculates sleep_interval
		*/
		fpsmanager_t(const uint16_t target_fps);

		/*
		@brief

			Sleeps until the next frame (blocking)
		*/
		void run(void);

		/*
		@brief

			Checks if we should run (non-blocking)
		*/
		bool should_run(void);

		/*
		@brief

			Calculates the delay (in nanoseconds) of a specific FPS (frame-to-frame)
		*/
		static std::chrono::nanoseconds calculate_delay(uint16_t target_fps);

		/*
		@brief

			Resets the next_frame (so it runs NOW)
		*/
		void reset(void);

		/*
		@brief

			Gets the time point of the next frame
		*/
		const std::chrono::high_resolution_clock::time_point& get_next_frame_time_point(void) const { return next_frame; }

		/*
		@brief

			Gets the update interval
		*/
		const std::chrono::nanoseconds& get_update_interval(void) const { return update_interval; }

	};

}