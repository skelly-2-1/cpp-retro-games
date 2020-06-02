/*
@file

	settings.h

@purpose
	
	Save and load settings from a config file using JSON
*/

#pragma once

#include <cstdint>
#include <functional>
#include <unordered_map>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <string>
#include "json.hpp"
#include "cfgvalue.h"
#include "macros.h"
#include "area_size.h"

#ifdef PLATFORM_NS
#include <switch.h>
#ifdef NS_ENABLE_NXLINK
#include "misc/trace.h"
#endif
#endif

#ifdef SETTINGS_ENABLE_DEBUGGING
#ifdef PLATFORM_WINDOWS
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN
#endif
#include <Windows.h>
#define SETTINGS_STRINGIZE(x) #x
#define SETTINGS_STR(x) SETTINGS_STRINGIZE(x)
#define SETTINGS_ABORT() { MessageBoxA(nullptr, (std::string("Settings manager encountered an error at line ") + SETTINGS_STR(__LINE__) + " in file: " + SETTINGS_STR(__FILE__)).c_str(), "error", MB_SETFOREGROUND | MB_ICONERROR); TerminateProcess(GetCurrentProcess(), 0); }
#else
#define SETTINGS_STRINGIZE(x) #x
#define SETTINGS_STR(x) SETTINGS_STRINGIZE(x)
#define SETTINGS_ABORT() { fprintf(stderr, (std::string("Settings manager encountered an error at line ") + SETTINGS_STR(__LINE__) + " in file: " + SETTINGS_STR(__FILE__) + "\n").c_str()); std::abort(); }
#endif
#else
#define SETTINGS_ABORT() std::abort()
#endif

namespace retrogames
{

	class settings_t;

	// Settings that may get used across multiple files
	struct main_settings_t final
	{

#if defined(PLATFORM_WINDOWS) || defined(PLATFORM_LINUX)
		cfgvalue_t* vsync;
		cfgvalue_t* fullscreen;
		cfgvalue_t* resolution;
		cfgvalue_t* fps;
#endif

		area_size_t resolution_area;

		cfgvalue_t* draw_fps, *draw_frametime, *draw_playtime, *draw_position;
		cfgvalue_t* timeout_time;
		cfgvalue_t* sound_effect_volume;

		main_settings_t()
		{
#if defined(PLATFORM_WINDOWS) || defined(PLATFORM_LINUX)
			vsync = fullscreen = resolution = fps = nullptr;
#endif

			resolution_area = area_size_t(1280, 720);

			draw_fps = draw_frametime = draw_playtime = draw_position = nullptr;
			timeout_time = nullptr;
			sound_effect_volume = nullptr;
		}

	};

	class settings_t final
	{

	protected:



	private:

		std::unordered_map<std::string, cfgvalue_t> settings;

		nlohmann::json json;

		std::string path;

		main_settings_t main_settings;

	public:

		/*
		@brief

			Gets main settings
		*/
		main_settings_t& get_main_settings(void) { return main_settings; }

		/*
		@brief

			Constructor
		*/
		settings_t(const std::string& path) { load(path); }

		/*
		@brief

			Loads a JSON from the given path
		*/
		void load(const std::string& path)
		{
			this->path = path;

#ifndef PLATFORM_NS
			try
			{
#endif
				std::ifstream stream(path);

				if (stream.is_open())
				{
					stream >> json;
					stream.close();
				}
#ifndef PLATFORM_NS
			}
			catch (...) {}
#endif

#ifndef PLATFORM_NS
			main_settings.vsync = &create("main_vsync", false);
			main_settings.fullscreen = &create("main_fullscreen", false);
			main_settings.resolution = &create("main_resolution", "1280x720");
			main_settings.fps = &create("main_fps", 60u);
			main_settings.resolution_area = area_size_t(1280, 720); // 720p default
			{
				auto res = main_settings.resolution->get<std::string>();
				auto pos = res.find_first_of('x');

				if (pos != std::string::npos)
				{
					main_settings.resolution_area.width = std::stoi(res.substr(0, pos));
					main_settings.resolution_area.height = std::stoi(res.substr(pos + 1));
				}
			}
#else
			main_settings.resolution_area = area_size_t(static_cast<uint32_t>(FB_WIDTH), static_cast<uint32_t>(FB_HEIGHT));
#if defined(PLATFORM_NS) && defined(NS_ENABLE_NXLINK)
			TRACE("resolution area: %ux%u. 720p: %ux%u", main_settings.resolution_area.width, main_settings.resolution_area.height, 1280u, 720u);
#endif
#endif

			main_settings.draw_fps = &create("main_draw_fps", true);
			main_settings.draw_frametime = &create("main_draw_frametime", true);
			main_settings.draw_playtime = &create("main_draw_playtime", true);
			main_settings.draw_position = &create("main_draw_position_alignment", "topright");
			main_settings.timeout_time = &create("main_lostfocus_timeout_time", 3u);
			main_settings.sound_effect_volume = &create("main_sound_effect_volume", 50.f);
		}

		/*
		@brief

			Saves the (possibly modified) JSON
		*/
		void save(const std::string& save_path = "")
		{
			auto _path = save_path.empty() ? path : save_path;

			for (const auto& setting : settings)
			{
				switch (setting.second.get_type())
				{
					case cfgvalue_e::value_unsigned_integer:
					{
						json[setting.first] = setting.second.get<uint32_t>();

						break;
					}
					case cfgvalue_e::value_string:
					{
						json[setting.first] =  setting.second.get<std::string>();

						break;
					}
					case cfgvalue_e::value_integer:
					{
						json[setting.first] = setting.second.get<int32_t>();

						break;
					}
					case cfgvalue_e::value_float:
					{
						json[setting.first] = setting.second.get<float>();

						break;
					}
					case cfgvalue_e::value_color:
					{
						auto color = setting.second.get<color_t>();
						auto color_string = std::to_string(color.r()) + ',' + std::to_string(color.g()) + ',' + std::to_string(color.b()) + ',' + std::to_string(color.a());

						json[setting.first] = color_string;

						break;
					}
					case cfgvalue_e::value_boolean:
					{
						json[setting.first] = setting.second.get<bool>();

						break;
					}
					default:
					{
						// Unknown type
						std::abort();

						break;
					}
				}
			}

#ifndef PLATFORM_NS
			try
			{
#endif
				std::ofstream o(_path);

				if (!o.is_open()) return;

				o << std::setw(4) << json << std::endl;
				o.close();
#ifndef PLATFORM_NS
			}
			catch (...) {}
#endif
		}

		/*
		@brief

			Checks if settings are empty (nothing created yet)
		*/
		bool empty(void) const { return settings.empty(); }

		/*
		@brief
		
			Checks if a setting exists
		*/
		bool exists(const std::string& name)
		{
			return !settings.empty() && settings.find(name) != settings.end();
		}

		/*
		@brief

			Gets a setting
		*/
		cfgvalue_t& get(const std::string& name)
		{
			if (settings.empty()) SETTINGS_ABORT();

			auto f = settings.find(name);

			if (f == settings.end()) SETTINGS_ABORT();

			return f->second;
		}

		/*
		@brief

			Creates a setting

		@notes

			We explicitly don't allow cfgvalue_t as type here (doesn't work)
			because otherwise, const char* would be interpreted as something
			else, for example. This way, the user doesn't have to call
			cfgvalue_t::create themselves.
		*/
		template <typename T>
		retrogames::cfgvalue_t& create(const std::string& name, T default_value)
		{
			// create the value
			auto value = cfgvalue_t::create(default_value);

			// check if the value has a proper type
			if (value.get_type() == cfgvalue_e::value_null) SETTINGS_ABORT();

			// add the value
			auto& added_value = settings[name];

			added_value = value;

			// Check if the value already exists in JSON
			if (!json.empty())
			{
				auto f = json.find(name);

				if (f != json.end())
				{
					// Entry found, overwrite the value with the existing one
					switch (added_value.get_type())
					{
						case cfgvalue_e::value_unsigned_integer:
						{
							added_value.set<uint32_t>(f->get<uint32_t>());

							break;
						}
						case cfgvalue_e::value_string:
						{
							added_value.set<std::string>(f->get<std::string>());

							break;
						}
						case cfgvalue_e::value_integer:
						{
							added_value.set<int32_t>(f->get<int32_t>());

							break;
						}
						case cfgvalue_e::value_float:
						{
							added_value.set<float>(f->get<float>());

							break;
						}
						case cfgvalue_e::value_color:
						{
							auto color_string = f->get<std::string>();

							std::vector<uint8_t> rgba;
							std::size_t previous = 0;

							auto current = color_string.find(',');

							while (current != std::string::npos)
							{
								rgba.push_back(static_cast<uint8_t>(std::atoi(color_string.substr(previous, current - previous).c_str())));

								previous = current + 1;
								current = color_string.find(',', previous);
							}

							if (rgba.size() != 4) SETTINGS_ABORT();

							added_value.set<color_t>(color_t(rgba.at(0), rgba.at(1), rgba.at(2), rgba.at(3)));

							break;
						}
						case cfgvalue_e::value_boolean:
						{
							added_value.set<bool>(f->get<bool>());

							break;
						}
						default:
						{
							// Unknown type
							SETTINGS_ABORT();

							break;
						}
					}
				}
			}

			return added_value;
		}

	};

}

#if (defined(PLATFORM_WINDOWS) || defined(PLATFORM_LINUX)) && defined(SETTINGS_ENABLE_DEBUGGING)
#undef SETTINGS_STRINGIZE
#undef SETTINGS_STR
#endif
#undef SETTINGS_ABORT