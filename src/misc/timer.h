/*
@file

	timer.h

@purpose

	Timing
*/

#pragma once

#include <chrono>

namespace retrogames
{

	class timer_t final
	{

	protected:



	private:

		std::chrono::high_resolution_clock::time_point start_time, pause_time;

		bool started_;
		bool paused_;
		bool start_time_set;

	public:

		/*
		@brief

			Offsets the time by std::chrono::high_resolution_clock::now() - @point
		*/
		void offset_by_time(const std::chrono::high_resolution_clock::time_point& point)
		{
			if (!started_) return;

			auto offset = std::chrono::high_resolution_clock::now() - point;

			/*if (paused_) pause_time += offset;
			else start_time += offset;*/

			start_time += offset;
		}

		/*
		@brief

			Offsets the time by duration
		*/
		template <typename T> void offset_by_duration(const T& duration)
		{
			if (paused_) pause_time += duration;
			else start_time += duration;
		}

		/*
		@brief

			Constructor, starts the timer if needed
		*/
		timer_t(bool start = false) : started_(start), start_time_set(false), paused_(false)
		{
			if (!start) return;
			
			this->start();
		}

		/*
		@brief

			Pauses the timer
		*/
		void pause(void)
		{
			if (paused_ || !started_) return;

			paused_ = true;
			pause_time = std::chrono::high_resolution_clock::now();
		}

		/*
		@brief

			Unpauses the timer
		*/
		void unpause(void)
		{
			if (!paused_ || !started_) return;

			paused_ = false;
			start_time += std::chrono::high_resolution_clock::now() - pause_time;
		}

		/*
		@brief

			Gets the elapsed time in whatever form you may want (chrono types)
			Also takes into account if the timer is paused
		*/
		template <typename T = std::chrono::milliseconds> T get_elapsed(void) const
		{
			if (!start_time_set) return T(0);

			return std::chrono::duration_cast<T>(std::chrono::high_resolution_clock::now() - get_time_point());
		}

		/*
		@brief

			(Re)starts the timer
		*/
		void start(void)
		{
			start_time = std::chrono::high_resolution_clock::now();

			started_ = start_time_set = true;
			paused_ = false;
		}

		/*
		@brief

			Stops the timer
		*/
		void stop(void)
		{
			started_ = paused_ = false;
		}

		/*
		@brief

			Tells the caller if the timer has been started

		*/
		bool started(void) const
		{
			return started_;
		}

		/*
		@brief

			Tells the caller if the timer has been paused
		*/
		bool paused(void) const
		{
			return paused_;
		}

		/*
		@brief

			Gets the raw time point when our timer started
		*/
		std::chrono::high_resolution_clock::time_point get_time_point(void) const
		{
			if (!started_) return start_time;
			if (paused_) return start_time + (std::chrono::high_resolution_clock::now() - pause_time);

			return start_time;
		}

	};

}