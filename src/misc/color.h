/*
@file

	color.h

@purpose

	Easy management of colors (RGBA)
*/

#pragma once

#include <cstdint>

namespace retrogames
{

	class color_t final
	{

	protected:



	private:

		uint8_t colors[4];

	public:

		/*
		@brief

			Constructor, sets the color
		*/
		color_t(uint8_t r = 255, uint8_t g = 255, uint8_t b = 255, uint8_t a = 255)
		{
			colors[0] = r;
			colors[1] = g;
			colors[2] = b;
			colors[3] = a;
		}

		/*
		@brief

			Retrieves RGBA colors
		*/
		uint8_t r(void) const { return colors[0]; }
		uint8_t g(void) const { return colors[1]; }
		uint8_t b(void) const { return colors[2]; }
		uint8_t a(void) const { return colors[3]; }

		/*
		@brief

			Gets the raw color (const)
		*/
		const uint8_t* get_raw_color(void) const { return colors; }

		/*
		@brief

			Gets the raw color
		*/
		uint8_t* get_raw_color(void) { return colors; }

	};

}