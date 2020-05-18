/*
@file

	util.h

@purpose

	Miscellaneous functionality that doesn't deserve it's own class
*/

#ifdef PLATFORM_WINDOWS
#include <Windows.h>
#elif defined(PLATFORM_LINUX)
#include <GLFW/glfw3.h>
#endif

#include <unordered_map>
#include <algorithm>
#include <cmath>
#include "util.h"

namespace retrogames
{

    static auto get_aspect_remainder = [](uint32_t target_width, uint32_t target_height)
    {
        if (target_height == 0) return target_width;

        int32_t reminder = 0;

        while (target_height != 0)
        {
            reminder = static_cast<int32_t>(std::fmod<uint32_t, uint32_t>(target_width, target_height));

            target_width = target_height;
            target_height = reminder;
        }

        return target_width;
    };

    namespace detail
    {

        // Used to make an unordered_map with std::pair as key
        struct hash_pair_t final
        {
            template <class T1, class T2>
            std::size_t operator()(const std::pair<T1, T2>& p) const
            {
                return std::hash<T1>{}(std::get<0>(p)) ^ std::hash<T2>{}(std::get<1>(p));
            }
        };

    }

}

/*
@brief

	Grabs the supported monitor resolutions (with the defined aspect ratio)

@arguments

	The aspect ratio we want. 0 on both means all resolutions should be added.

@return

	vector<std::pair<uint16_t, uint16_t>>: vector of X and Y resolutions
	std::string: Zero-separated string of resolutions (for use in ImGui)
*/
#if defined(PLATFORM_LINUX) || defined(PLATFORM_WINDOWS)
const std::pair<std::vector<std::tuple<uint16_t, uint16_t, uint16_t>>, std::string>& retrogames::util::get_supported_resolutions(const uint8_t aspect_x/* = 16*/, const uint8_t aspect_y/* = 9*/)
{
    static bool tried_finding_resolutions = false;
    static auto supported_resolutions = std::make_pair<std::vector<std::tuple<uint16_t, uint16_t, uint16_t>>, std::string>({}, "");
    
    if (tried_finding_resolutions) return supported_resolutions;

    std::unordered_map<std::pair<uint16_t, uint16_t>, std::vector<uint16_t>, retrogames::detail::hash_pair_t> supported_resolutions_map;

#ifdef PLATFORM_LINUX
    int mode_count;

    auto modes = glfwGetVideoModes(glfwGetPrimaryMonitor(), &mode_count);

    if (mode_count == 0)
    {
        tried_finding_resolutions = true;

        return supported_resolutions;
    }

    for (uint16_t num = 0; num < static_cast<uint16_t>(mode_count); num++)
    {
        auto mode = modes[num];

        // Only add the right aspect-ratio resolutions
        if ((aspect_x != 0 || aspect_y != 0) && std::round(static_cast<double>(mode.width) / static_cast<double>(get_aspect_remainder(mode.width, mode.height))) != static_cast<double>(aspect_x) || std::round(static_cast<double>(mode.height) / static_cast<double>(get_aspect_remainder(mode.width, mode.height))) != static_cast<double>(aspect_y)) continue;

        // 16:9 resolution found, add it
        supported_resolutions_map[std::make_pair(static_cast<uint16_t>(mode.width), static_cast<uint16_t>(mode.height))].push_back(static_cast<uint16_t>(mode.refreshRate));
    }
#else
    auto dm = DEVMODE{};

    dm.dmSize = sizeof(dm);

    for (uint16_t num = 0; EnumDisplaySettings(NULL, num, &dm) != 0; num++)
    {
        // Only add the right aspect-ratio resolutions
        if ((aspect_x != 0 || aspect_y != 0) && std::round(static_cast<double>(dm.dmPelsWidth) / static_cast<double>(get_aspect_remainder(dm.dmPelsWidth, dm.dmPelsHeight))) != static_cast<double>(aspect_x) || std::round(static_cast<double>(dm.dmPelsHeight) / static_cast<double>(get_aspect_remainder(dm.dmPelsWidth, dm.dmPelsHeight))) != static_cast<double>(aspect_y)) continue;

        // 16:9 resolution found, add it
        supported_resolutions_map[std::make_pair(static_cast<uint16_t>(dm.dmPelsWidth), static_cast<uint16_t>(dm.dmPelsHeight))].push_back(static_cast<uint16_t>(dm.dmDisplayFrequency));
    }
#endif

    // Now, add those resolutions to our vector (to avoid duplicates, we made that map)
    for (const auto& res : supported_resolutions_map)
    {
        // Find the highest refresh rate
        uint16_t refresh_rate = 0;

        for (const auto& rate : res.second)
        {
            if (rate > refresh_rate) refresh_rate = rate;
        }

        supported_resolutions.first.push_back(std::make_tuple(res.first.first, res.first.second, refresh_rate));
    }

    // Sort the supported_resolutions_vec so lowest resolutions come first and highest last
    std::sort(supported_resolutions.first.begin(), supported_resolutions.first.end(), [](const std::tuple<uint16_t, uint16_t, uint16_t>& left, const std::tuple<uint16_t, uint16_t, uint16_t>& right)
    {
        return std::get<0>(left) < std::get<0>(right);
    });

    // Turn the resolutions to a string and add them to our unique_ptr
    if (!supported_resolutions.first.empty())
    {
        for (const auto& res : supported_resolutions.first)
        {
            supported_resolutions.second += std::to_string(std::get<0>(res)) + "x" + std::to_string(std::get<1>(res));
            supported_resolutions.second.push_back('\0');
        }
    }

    tried_finding_resolutions = true;

    return supported_resolutions;
}

/*
@brief

    Checks if the given @size is the given aspect ratio
*/
bool retrogames::util::check_aspect_ratio(const area_size_t& size, const uint8_t aspect_x/* = 16*/, const uint8_t aspect_y/* = 9*/)
{
    return  std::round(static_cast<double>(size.width) / static_cast<double>(get_aspect_remainder(size.width, size.height))) == static_cast<double>(aspect_x) &&
            std::round(static_cast<double>(size.height) / static_cast<double>(get_aspect_remainder(size.width, size.height))) == static_cast<double>(aspect_y);
}
#endif