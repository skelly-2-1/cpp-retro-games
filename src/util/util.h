/*
@file

	util.h

@purpose

	Miscellaneous functionality that doesn't deserve it's own class
*/

#pragma once

#include <vector>
#include <cstdint>
#include <string>
#include "misc/area_size.h"

namespace retrogames
{

	namespace util
	{
		
#if defined(PLATFORM_WINDOWS) || defined(PLATFORM_LINUX)
		/*
		@brief

			Grabs the supported monitor resolutions (with the defined aspect ratio)

		@arguments

			The aspect ratio we want. 0 on both means all resolutions should be added.

		@return

			vector<std::pair<uint16_t, uint16_t>>: vector of X and Y resolutions
			std::string: Zero-separated string of resolutions (for use in ImGui)
		*/
		const std::pair<std::vector<std::tuple<uint16_t, uint16_t, uint16_t>>, std::string>& get_supported_resolutions(const uint8_t aspect_x = 16, const uint8_t aspect_y = 9);

		/*
		@brief

			Checks if the given @size is the given aspect ratio
		*/
		bool check_aspect_ratio(const area_size_t& size, const uint8_t aspect_x = 16, const uint8_t aspect_y = 9);
#endif

	}

}