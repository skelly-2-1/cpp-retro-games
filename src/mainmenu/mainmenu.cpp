/*
@file

    mainmenu.cpp

@purpose

    Handling of our main menu
*/

#include <ctime>
#include <stdio.h>
#include "mainmenu.h"
#include "imgui/imgui.h"
#include "misc/macros.h"
#include "imgui/imgui_user.h"

#if defined(PLATFORM_WINDOWS) || defined(PLATFORM_LINUX)
#include "util/util.h"
/*#ifdef PLATFORM_WINDOWS
#include <Windows.h>
#endif*/
#else
#ifdef NS_ENABLE_NXLINK
#include "misc/trace.h"
#endif
#endif

// games
#include "games/snake/snake.h"
#include "games/pingpong/pingpong.h"

/*
@brief

    Called before any rendering is done
*/
void retrogames::mainmenu_t::initialize(settings_t* settings)
{
    global_scaling = UI_SCALE;

    // Save a pointer to the settings
    this->settings = settings;

    // Add any ImGui fonts we may need
    create_fonts();

    // Create the games manager
    games_manager = std::make_unique<games_manager_t>(settings, &default_font_small, &default_font_mid, &default_font_big);

    // Here we add our games. Each game can have a pointer to it's icon,
    // version numbering, and name.
    games_manager->add_game<games::snake_t>("snake");
    games_manager->add_game<games::pingpong_t>("pingpong");

    selected_game_name = &settings->create("main_last_selected_game", "none");

    auto selected_game_name_str = selected_game_name->get<std::string>();

    selected_game = selected_game_name_str.compare("none") == 0 ? nullptr : games_manager->select_game(selected_game_name_str);
}

/*
@brief

    Creates needed fonts
*/
void retrogames::mainmenu_t::create_fonts(void)
{
    auto& io = ImGui::GetIO();

    ImFontConfig default_font_big_config;

    default_font_big_config.SizePixels = std::ceil(static_cast<float>(settings->get_main_settings().resolution_area.height) / 25.f) * global_scaling;
    default_font_big = io.Fonts->AddFontDefault(&default_font_big_config);

    ImFontConfig default_font_small_config;

    default_font_small_config.SizePixels = std::ceil(default_font_big_config.SizePixels * .5f);
    default_font_small = io.Fonts->AddFontDefault(&default_font_small_config);

    ImFontConfig default_font_mid_config;

    default_font_mid_config.SizePixels = std::ceil(default_font_big_config.SizePixels * .75f);
    default_font_mid = io.Fonts->AddFontDefault(&default_font_mid_config);

#if defined(PLATFORM_NS) && defined(NS_ENABLE_NXLINK)
    TRACE("Resolution area: %ux%u, displaysize: %.0fx%.0f", settings->get_main_settings().resolution_area.width, settings->get_main_settings().resolution_area.height, io.DisplaySize.x, io.DisplaySize.y);
#endif
}

/*
@brief

    Called on reset
*/
void retrogames::mainmenu_t::reset(void)
{
    create_fonts();

    if (reset_game)
    {
        games_manager->get_current_game()->base_reset(settings, true);

        reset_game = false;
    }
}

/*
@brief

    Runs every frame, we do all required rendering here

@params

    @should_render:     True if we should render, false if not.
                        This would be the case if the window
                        isn't in the foreground, for example.
                        You can still handle anything other
                        than drawing in the meanwhile though.
    @reset_video_mode:  Returns information to the caller if
                        the video mode should be reset
                        (when we changed settings)

@return

    True if the application should exit, false if not
*/
bool retrogames::mainmenu_t::run(bool should_render, bool& reset_video_mode)
{
#if defined(PLATFORM_WINDOWS) || defined(PLATFORM_LINUX)
    // To check if any video settings changed
    struct video_settings_t final
    {

        uint32_t fps;

        bool vsync, fullscreen;

        std::string resolution;

        bool draw_fps, draw_frametime, draw_playtime;

        std::string draw_position;

        uint32_t timeout_time;

        cfgvalue_t* cfgvalue_fps, *cfgvalue_vsync, *cfgvalue_fullscreen, *cfgvalue_resolution;
        cfgvalue_t* cfgvalue_draw_fps, *cfgvalue_draw_frametime, *cfgvalue_draw_playtime, *cfgvalue_draw_position, *cfgvalue_timeout_time;

        void load_from_cfgvalues(void)
        {
            fps = cfgvalue_fps->get<uint32_t>();
            vsync = cfgvalue_vsync->get<bool>();
            fullscreen = cfgvalue_fullscreen->get<bool>();
            resolution = cfgvalue_resolution->get<std::string>();
            draw_fps = cfgvalue_draw_fps->get<bool>();
            draw_frametime = cfgvalue_draw_frametime->get<bool>();
            draw_playtime = cfgvalue_draw_playtime->get<bool>();
            draw_position = cfgvalue_draw_position->get<std::string>();
            timeout_time = cfgvalue_timeout_time->get<uint32_t>();
        }

        video_settings_t(settings_t* settings) :
            cfgvalue_fps(nullptr),
            cfgvalue_vsync(nullptr),
            cfgvalue_fullscreen(nullptr),
            cfgvalue_resolution(nullptr),
            cfgvalue_draw_fps(nullptr),
            cfgvalue_draw_frametime(nullptr),
            cfgvalue_draw_playtime(nullptr),
            cfgvalue_draw_position(nullptr),
            cfgvalue_timeout_time(nullptr)
        {
            fps = settings->get_main_settings().fps->get<uint32_t>();
            vsync = settings->get_main_settings().vsync->get<bool>();
            fullscreen = settings->get_main_settings().fullscreen->get<bool>();
            resolution = settings->get_main_settings().resolution->get<std::string>();
            draw_fps = settings->get_main_settings().draw_fps->get<bool>();
            draw_frametime = settings->get_main_settings().draw_frametime->get<bool>();
            draw_playtime = settings->get_main_settings().draw_playtime->get<bool>();
            draw_position = settings->get_main_settings().draw_position->get<std::string>();
            timeout_time = settings->get_main_settings().timeout_time->get<uint32_t>();
        }

        video_settings_t(uint32_t fps, bool vsync, bool fullscreen, std::string resolution, bool draw_fps, bool draw_frametime, bool draw_playtime, std::string draw_position, uint32_t timeout_time) :
            cfgvalue_fps(nullptr),
            cfgvalue_vsync(nullptr),
            cfgvalue_fullscreen(nullptr),
            cfgvalue_resolution(nullptr),
            fps(fps),
            vsync(vsync),
            fullscreen(fullscreen),
            resolution(resolution),
            draw_fps(draw_fps),
            draw_frametime(draw_frametime),
            draw_playtime(draw_playtime),
            draw_position(draw_position),
            timeout_time(timeout_time) {}

        video_settings_t(cfgvalue_t* fps, cfgvalue_t* vsync, cfgvalue_t* fullscreen, cfgvalue_t* resolution, cfgvalue_t* cfgvalue_draw_fps, cfgvalue_t* cfgvalue_draw_frametime, cfgvalue_t* cfgvalue_draw_playtime, cfgvalue_t* cfgvalue_draw_position, cfgvalue_t* cfgvalue_timeout_time) :
            cfgvalue_fps(fps),
            cfgvalue_vsync(vsync),
            cfgvalue_fullscreen(fullscreen),
            cfgvalue_resolution(resolution),
            cfgvalue_draw_fps(cfgvalue_draw_fps),
            cfgvalue_draw_frametime(cfgvalue_draw_frametime),
            cfgvalue_draw_playtime(cfgvalue_draw_playtime),
            cfgvalue_draw_position(cfgvalue_draw_position),
            cfgvalue_timeout_time(cfgvalue_timeout_time)
        {
            load_from_cfgvalues();
        }

        bool differs(const video_settings_t& other)
        {
            return fps != other.fps || vsync != other.vsync || fullscreen != other.fullscreen || resolution.compare(other.resolution) != 0;
        }

    };

    static auto original_video_settings = video_settings_t(settings);

    // If a game is running, run that
    static bool last_game_running = false;
    static bool main_reset_video_settings = false;

    bool should_run_game = game_running && selected_game != nullptr;

    if (last_game_running != should_run_game)
    {
        last_game_running = should_run_game;

        if (!should_run_game)
        {
            // Reset video mode to original
            if (main_reset_video_settings)
            {
                reset_video_mode = true;
                main_reset_video_settings = false;
            }

            return false;
        }
        else
        {
            main_reset_video_settings = video_settings_t(settings).differs(original_video_settings);

            // Set the global video settings back to the default
            settings->get_main_settings().fps->set(original_video_settings.fps);
            settings->get_main_settings().fullscreen->set(original_video_settings.fullscreen);
            settings->get_main_settings().resolution->set(original_video_settings.resolution);
            settings->get_main_settings().vsync->set(original_video_settings.vsync);
        }
    }

    if (should_run_game)
    {
        ImGui::PushFont(default_font_big);

        if (selected_game->base_draw(should_render)) game_running = false;
        if (selected_game->is_paused() && should_render && selected_game->draw_pause_menu(default_font_big)) game_running = false;

        ImGui::PopFont();

        return false;
    }
#else
    if (game_running && selected_game != nullptr)
    {
        ImGui::PushFont(default_font_big);

        if (selected_game->base_draw(should_render)) game_running = false;
        if (selected_game->is_paused() && selected_game->draw_pause_menu(default_font_big)) game_running = false;

        ImGui::PopFont();

        return false;
    }
#endif

    // We shouldn't render, so the window is out of focus
    if (!should_render) return false;

	/*static bool show_demo_window = true;
    static bool show_another_window = false;
    static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
    if (show_demo_window)
        ImGui::ShowDemoWindow(&show_demo_window);

    // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
    {
        static float f = 0.0f;
        static int counter = 0;

        ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

        ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
        ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
        ImGui::Checkbox("Another Window", &show_another_window);

        ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
        ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

        if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
            counter++;
        ImGui::SameLine();
        ImGui::Text("counter = %d", counter);

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();
    }

    // 3. Show another simple window.
    if (show_another_window)
    {
        ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
        ImGui::Text("Hello from another window!");
        if (ImGui::Button("Close Me"))
            show_another_window = false;
        ImGui::End();
    }*/

    // Should we exit?
    static bool should_exit = false;

    // Viewing game options?
    enum class start_game_e
    {

        START_GAME_STATE_MAIN,
        START_GAME_STATE_OPTIONS,
        START_GAME_STATE_CONTROLS,
        START_GAME_STATE_INFORMATION

    };

    static start_game_e start_state = start_game_e::START_GAME_STATE_MAIN;

    // set up our left hand collection
    enum class selection_e
    {
        selection_start,
        selection_select_game,
        selection_options,

        selection_credits,
        selection_exit,
        selection_size
    };

    static auto& selected_item_cfgvalue = [&]() -> cfgvalue_t&
    {
        auto& cfgvalue = settings->create("main_last_selected_item", 0u);
        auto val = cfgvalue.get<uint32_t>();

        if (cfgvalue.get<uint32_t>() >= static_cast<uint32_t>(selection_e::selection_size)) cfgvalue = 0u;

        return cfgvalue;
    }();

    static selection_e selected_item = static_cast<selection_e>(selected_item_cfgvalue.get<uint32_t>());
    static std::string selection_names[static_cast<uint8_t>(selection_e::selection_size)] = {

        "Start game",
        "Select game",
        "Options",
        "Credits",
        "Exit"

    };

    // get a list of games and find the currently selected one
    static const auto& games = games_manager->get_games();

    // set up positioning of our GUI items
    static auto background_color = color_t(40, 40, 40);
    static auto window_dampening_multiplier = .8f;
    
    auto& resolution_area = settings->get_main_settings().resolution_area;
    auto indent_multiplier_width = 70.f;
    auto indent_multiplier_height = 15.f;
    auto indent_height = std::ceil(static_cast<float>(resolution_area.height) / indent_multiplier_height) * global_scaling;
    auto indent_width = std::ceil(static_cast<float>(resolution_area.height) / indent_multiplier_width) * global_scaling;
    auto left_selection_size_width = std::ceil(static_cast<float>(resolution_area.width) / 6.f) * global_scaling;
    auto selection_pos = ImVec2(indent_width, indent_height);
    auto selection_size = ImVec2(left_selection_size_width, static_cast<float>(resolution_area.height) - indent_height * 2.f);

    // Notification system
    static std::string current_notification;
    static bool notification_modified = false;
    static auto set_notification = [&](const std::string& info) { current_notification = info; notification_modified = true; };
    static auto draw_notification = [&](void)
    {
        static std::chrono::high_resolution_clock::time_point fade_start_time;
        static constexpr uint8_t notification_length = 5; // How many seconds the notification should be active
        static std::string last_notification; // To see if the notification message changed
        static constexpr float fade_time = .5f; // How many seconds it takes for the notification to fade in/out

        if (last_notification.compare(current_notification) != 0)
        {
            // Notification changed
            if (last_notification.empty() && !current_notification.empty())
            {
                // Notification has just been set
                fade_start_time = std::chrono::high_resolution_clock::now();
            }
            else if (!current_notification.empty() && !last_notification.empty())
            {
                // Notification text changed, don't fade in again
                if (std::chrono::high_resolution_clock::now() - fade_start_time > std::chrono::milliseconds(static_cast<int64_t>(fade_time * 1000.f))) fade_start_time = std::chrono::high_resolution_clock::now() - std::chrono::milliseconds(static_cast<int64_t>(fade_time * 1000.f));
            }

            last_notification = current_notification;
            notification_modified = false;
        }
        else if (notification_modified)
        {
            // Notification recreated, don't fade in again
            if (std::chrono::high_resolution_clock::now() - fade_start_time > std::chrono::milliseconds(static_cast<int64_t>(fade_time * 1000.f))) fade_start_time = std::chrono::high_resolution_clock::now() - std::chrono::milliseconds(static_cast<int64_t>(fade_time * 1000.f));

            notification_modified = false;
        }

        // Check if we have a notification
        if (current_notification.empty()) return;

        // Check if the time is over
        if (std::chrono::high_resolution_clock::now() - fade_start_time >= std::chrono::seconds(notification_length))
        {
            current_notification.clear();
            last_notification.clear();

            return;
        }

        static auto raw_border_color = ImGui::GetStyleColorVec4(ImGuiCol_Border);
        static auto raw_text_color = ImGui::GetStyleColorVec4(ImGuiCol_Text);
        static auto raw_background_color = ImGui::GetStyleColorVec4(ImGuiCol_WindowBg);

        auto background_color = raw_background_color;
        auto text_color = raw_text_color;
        auto border_color = raw_border_color;
        auto expired = std::chrono::high_resolution_clock::now() - fade_start_time;

        if (expired < std::chrono::milliseconds(static_cast<int64_t>(fade_time * 1000.f)))
        {
            // fade in
            static constexpr auto total = static_cast<uint32_t>(fade_time * 1000.f);

            auto expired_uint32_t = static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::milliseconds>(expired).count());
            auto mult = (static_cast<float>(expired_uint32_t) / static_cast<float>(total));

            background_color.w *= mult;
            text_color.w *= mult;
            border_color.w *= mult;
        }
        else if (expired > std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::seconds(notification_length)) - std::chrono::milliseconds(static_cast<int64_t>(fade_time * 1000.f)))
        {
            // fade out
            static constexpr auto total = static_cast<uint32_t>(fade_time * 1000.f);

            auto expired_uint32_t = static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::milliseconds>(expired).count());

            expired_uint32_t -= static_cast<uint32_t>((std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::seconds(notification_length)) - std::chrono::milliseconds(static_cast<int64_t>(fade_time * 1000.f))).count());

            auto mult = 1.f - (static_cast<float>(expired_uint32_t) / static_cast<float>(total));

            background_color.w *= mult;
            text_color.w *= mult;
            border_color.w *= mult;
        }

        ImGui::PushFont(default_font_small);

        auto size = ImVec2{ImGui::CalcTextSize(current_notification.c_str()).x + ImGui::GetStyle().FramePadding.x * 2.f + ImGui::GetStyle().ItemSpacing.x,ImGui::GetFrameHeight() + ImGui::GetStyle().ItemSpacing.y * 2.f};
        auto pos = ImVec2{resolution_area.width * .5f - size.x * .5f, (resolution_area.height - indent_height * .5f) - size.y * .5f};

        ImGui::SetNextWindowPos(pos, ImGuiCond_Always);
        ImGui::SetNextWindowSize(size, ImGuiCond_Always);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.f);
        ImGui::PushStyleColor(ImGuiCol_WindowBg, background_color);
        ImGui::PushStyleColor(ImGuiCol_Text, text_color);
        ImGui::PushStyleColor(ImGuiCol_Border, border_color);

        if (ImGui::Begin("##notification", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoNavInputs | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoInputs))
        {
            ImGui::TextUnformatted(current_notification.c_str());
            ImGui::End();
        }

        ImGui::PopStyleColor(3);
        ImGui::PopStyleVar(2);
        ImGui::PopFont();
    };

    static bool focus_set = false;

    if (!focus_set)
    {
        ImGui::SetNextWindowFocus();

        focus_set = true;
    }

    auto main_window_bg_color = ImVec4{(static_cast<float>(background_color.r()) / 255.f) * window_dampening_multiplier, (static_cast<float>(background_color.g()) / 255.f) * window_dampening_multiplier, (static_cast<float>(background_color.b()) / 255.f) * window_dampening_multiplier, 1.f};

    ImGui::SetNextWindowPos(selection_pos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(selection_size, ImGuiCond_Always);
    ImGui::GetBackgroundDrawList()->AddRectFilled(ImVec2(0.f, 0.f), ImVec2(static_cast<float>(resolution_area.width), static_cast<float>(resolution_area.height)), ImGuiUser::color_to_imgui_color_u32(background_color));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, main_window_bg_color);
    ImGui::PushStyleColor(ImGuiCol_Border, {0.f,0.f,0.f,0.f});
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
    ImGui::PushFont(default_font_big);

    if (ImGui::Begin("Menu selection", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoMove))
    {
        auto button_size = ImVec2(ImGui::GetWindowContentRegionWidth(), 0.f);

        for (uint8_t i = 0; i < static_cast<uint8_t>(selection_e::selection_size); i++)
        {
            if (ImGui::Button(selection_names[i].c_str(), button_size))
            {
                if (i == static_cast<uint8_t>(selection_e::selection_start)) start_state = start_game_e::START_GAME_STATE_MAIN;

                selected_item = static_cast<selection_e>(i);

                if (i < static_cast<uint8_t>(selection_e::selection_credits)) selected_item_cfgvalue = static_cast<uint32_t>(i);

                ImGui::SetNextWindowFocus();
            }
        }

        ImGui::End();
    }

    static std::string subwindow_button_text, subwindow_button_text_2;
    static bool subwindow_button_pressed = false, subwindow_button_pressed_2 = false;

    auto window_y_offset = ImGui::GetStyle().ItemSpacing.y;//0.f;//ImGui::GetStyle().WindowPadding.y*2.f+ImGui::GetStyle().ItemSpacing.y;
    auto main_window_pos = ImVec2(indent_width + selection_size.x, selection_pos.y+1.f);
    auto main_window_size = ImVec2((static_cast<float>(resolution_area.width) - indent_width * 2.f) - selection_size.x, selection_size.y - 2.f);
    auto draw_bottom_button = (selected_item == selection_e::selection_start && start_state != start_game_e::START_GAME_STATE_MAIN && selected_game != nullptr) || selected_item == selection_e::selection_options;

    if (draw_bottom_button)
    {
        // Make room for another window at the bottom, which will contain the bottom button
        //main_window_size.y -= (ImGui::GetFrameHeightWithSpacing() + window_y_offset) + ImGui::GetStyle().WindowPadding.y;

        //ImGui::GetBackgroundDrawList()->AddRectFilled({main_window_pos.x,main_window_pos.y+main_window_size.y}, ImVec2{main_window_pos.x+main_window_size.x,main_window_pos.y+main_window_size.y+ImGui::GetStyle().WindowPadding.y+1.f}, ImGui::GetColorU32(main_window_bg_color));

        if (selected_item == selection_e::selection_options)
        {
            subwindow_button_text = "Apply";
            subwindow_button_text_2.clear();
        }
        else if (start_state == start_game_e::START_GAME_STATE_CONTROLS || start_state == start_game_e::START_GAME_STATE_INFORMATION)
        {
            subwindow_button_text = "Back";
            subwindow_button_text_2.clear();
        }
        else
        {
            subwindow_button_text = "Back";
            subwindow_button_text_2 = "Apply defaults";
        }
    }
    else if (!subwindow_button_text.empty())
    {
        subwindow_button_text.clear();
    }

    ImGui::SetNextWindowPos(main_window_pos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(main_window_size, ImGuiCond_Always);
    ImGui::PushStyleColor(ImGuiCol_Border,{1.f,1.f,1.f,1.f});
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.f);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);

    static bool load_align_combo = false;
    static bool load_resolution = false;

    if (ImGui::Begin("Main window", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove))
    {
        if (draw_bottom_button)
        {
            ImGui::PushStyleColor(ImGuiCol_WindowBg,{0.f,0.f,0.f,0.f});
            ImGui::SetNextWindowPos({main_window_pos.x+ImGui::GetStyle().WindowPadding.x,main_window_pos.y+ImGui::GetStyle().WindowPadding.y}, ImGuiCond_Always);
        }

        if (!draw_bottom_button || ImGui::BeginChild("Main child window #1", ImVec2(main_window_size.x-ImGui::GetStyle().WindowPadding.x*2.f, (main_window_size.y-((ImGui::GetFrameHeightWithSpacing() + window_y_offset) + ImGui::GetStyle().WindowPadding.y))-ImGui::GetStyle().WindowPadding.y*2.f), false, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove/* | ImGuiWindowFlags_NoNav*/))
        {
            if (selected_item == selection_e::selection_start)
            {
                if (selected_game == nullptr)
                {
                    ImGui::TextWrapped("Here you can start the selected game or change settings.");
                    ImGui::Separator();
                    ImGui::TextWrapped("No game selected!");

                    start_state = start_game_e::START_GAME_STATE_MAIN;
                }
                else
                {
                    auto button_size = ImVec2{ImGui::GetContentRegionAvailWidth(),0.f};

                    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.f);

                    if (start_state == start_game_e::START_GAME_STATE_MAIN)
                    {
                        ImGui::TextWrapped("Here you can start the selected game or change settings.");
                        ImGui::Separator();

                        if (ImGui::Button("Show information", button_size)) start_state = start_game_e::START_GAME_STATE_INFORMATION;
                        if (ImGui::Button("Show controls", button_size)) start_state = start_game_e::START_GAME_STATE_CONTROLS;
                        if (ImGui::Button("View options", button_size)) { start_state = start_game_e::START_GAME_STATE_OPTIONS; load_align_combo = load_resolution = true; }
                        if (ImGui::Button("Play", button_size))
                        {
#ifndef PLATFORM_NS
                            const auto& gamename = selected_game->get_information().name;

                            video_settings_t game_video_settings(
                                &settings->get(gamename + "_video_fps"),
                                &settings->get(gamename + "_video_vsync"),
                                &settings->get(gamename + "_video_fullscreen"),
                                &settings->get(gamename + "_video_resolution"),
                                &settings->get(gamename + "_draw_fps"),
                                &settings->get(gamename + "_draw_frametime"),
                                &settings->get(gamename + "_draw_playtime"),
                                &settings->get(gamename + "_draw_position_alignment"),
                                &settings->get(gamename + "_lostfocus_timeout_time")
                            );

                            // Grab the wanted resolution from our settings
                            auto old_resolution = area_size_t(1280, 720); // 720p default
                            {
                                auto res = settings->get_main_settings().resolution->get<std::string>();
                                auto pos = res.find_first_of('x');

                                if (pos != std::string::npos)
                                {
                                    old_resolution.width = std::stoi(res.substr(0, pos));
                                    old_resolution.height = std::stoi(res.substr(pos + 1));
                                }
                            }

                            auto wanted_resolution = area_size_t(1280, 720); // 720p default
                            {
                                auto res = game_video_settings.resolution;
                                auto pos = res.find_first_of('x');

                                if (pos != std::string::npos)
                                {
                                    wanted_resolution.width = std::stoi(res.substr(0, pos));
                                    wanted_resolution.height = std::stoi(res.substr(pos + 1));
                                }
                            }

                            settings->get_main_settings().resolution_area = wanted_resolution;

                            if (original_video_settings.differs(game_video_settings))
                            {
                                settings->get_main_settings().fps->set(game_video_settings.fps);
                                settings->get_main_settings().fullscreen->set(game_video_settings.fullscreen);
                                settings->get_main_settings().resolution->set(game_video_settings.resolution);
                                settings->get_main_settings().vsync->set(game_video_settings.vsync);

                                reset_video_mode = reset_game = true;
                            }
                            else
                            {
                                selected_game->base_reset(settings, false);
                            }

                            settings->get_main_settings().resolution_area = old_resolution;
#endif

                            game_running = true;
                        }
                    }
                    else if (start_state == start_game_e::START_GAME_STATE_OPTIONS)
                    {
                        static auto window_bg_color = main_window_bg_color;
                        static auto frame_bg_color = ImVec4{window_bg_color.x*window_dampening_multiplier,window_bg_color.y*window_dampening_multiplier,window_bg_color.z*window_dampening_multiplier,window_bg_color.w*window_dampening_multiplier};
                        static auto text_selected_bg = ImVec4{(static_cast<float>(background_color.r()) / 255.f) * 1.5f, (static_cast<float>(background_color.g()) / 255.f) * 1.5f, (static_cast<float>(background_color.b()) / 255.f) * 1.5f, static_cast<float>(background_color.a()) / 255.f};
                        static auto slidergrab_color = text_selected_bg;
                        static auto slidergrab_color_active = ImVec4{(static_cast<float>(background_color.r()) / 255.f) * 2.f, (static_cast<float>(background_color.g()) / 255.f) * 2.f, (static_cast<float>(background_color.b()) / 255.f) * 2.f, static_cast<float>(background_color.a()) / 255.f};

#if defined(PLATFORM_LINUX) || defined(PLATFORM_WINDOWS)
                        ImGui::TextWrapped("Video options");
                        ImGui::Separator();
                        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.f);
                        ImGui::PushStyleColor(ImGuiCol_FrameBg, frame_bg_color);
                        ImGui::PushStyleColor(ImGuiCol_TextSelectedBg, text_selected_bg);
                        ImGui::PushStyleColor(ImGuiCol_SliderGrab, slidergrab_color);
                        ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, slidergrab_color_active);
                        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, slidergrab_color_active);
                        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, text_selected_bg);

                        const auto& gamename = selected_game->get_information().name;

                        video_settings_t game_video_settings(
                            &settings->get(gamename + "_video_fps"),
                            &settings->get(gamename + "_video_vsync"),
                            &settings->get(gamename + "_video_fullscreen"),
                            &settings->get(gamename + "_video_resolution"),
                            &settings->get(gamename + "_draw_fps"),
                            &settings->get(gamename + "_draw_frametime"),
                            &settings->get(gamename + "_draw_playtime"),
                            &settings->get(gamename + "_draw_position_alignment"),
                            &settings->get(gamename + "_lostfocus_timeout_time")
                        );

                        ImGuiUser::inputslider_uint32_t(game_video_settings.cfgvalue_fps, "FPS", 1000u, 0u, "Sets the framerate limit. This setting will be ignored if vertical sync is enabled.", global_scaling);
                        ImGuiUser::inputslider_uint32_t(game_video_settings.cfgvalue_timeout_time, "Timeout (focus lost/start of game)", 10u, 0u, "The time in seconds the game will pause when starting the game or tabbing back into it. 0 means no timeout!");

                        ImGui::PopStyleColor(6);

                        // booleans
                        ImGui::Separator();

                        ImGuiUser::toggle_button(game_video_settings.cfgvalue_fullscreen, "Fullscreen", "Turns on/off fullscreen video mode.");
                        ImGuiUser::toggle_button(game_video_settings.cfgvalue_vsync, "Vertical sync", "Turns on/off vertical sync. Reduces screen tearing, although framerate will be limited to the refresh rate of your monitor.");
                        ImGuiUser::toggle_button(game_video_settings.cfgvalue_draw_fps, "Draw FPS", "If enabled, the FPS will be drawn in games using the specified alignment.");
                        ImGuiUser::toggle_button(game_video_settings.cfgvalue_draw_frametime, "Draw frametime", "If enabled, the frametime (in ms) will be drawn in games using the specified alignment.");
                        ImGuiUser::toggle_button(game_video_settings.cfgvalue_draw_playtime, "Draw playtime", "If enabled, the playtime (hh:mm:ss:ms) will be drawn in games using the specified alignment.");

                        // Combos
                        ImGui::Separator();

                        static auto& supported_resolutions = util::get_supported_resolutions();
                        static auto selected_resolution_func = [&]() -> int32_t
                        {
                            // Grab the wanted resolution from our settings
                            auto resolution = area_size_t(1280, 720); // 720p default
                            {
                                auto res = game_video_settings.resolution;
                                auto pos = res.find_first_of('x');

                                if (pos != std::string::npos)
                                {
                                    resolution.width = std::stoi(res.substr(0, pos));
                                    resolution.height = std::stoi(res.substr(pos + 1));
                                }
                            }

                            // The default resolution in case the selected one wasn't found
                            constexpr int32_t default_resolution = 0;

                            // Return the found resolution
                            auto found = std::find_if(supported_resolutions.first.begin(), supported_resolutions.first.end(), [&resolution](const std::tuple<uint16_t, uint16_t, uint16_t>& p) -> bool
                            {
                                return static_cast<uint32_t>(std::get<0>(p)) == resolution.width && static_cast<uint32_t>(std::get<1>(p)) == resolution.height;
                            });

                            return found != supported_resolutions.first.end() ? static_cast<int32_t>(std::distance(supported_resolutions.first.begin(), found)) : default_resolution;
                        };

                        static int32_t selected_resolution = 0;

                        if (load_resolution)
                        {
                            load_resolution = false;

                            selected_resolution = selected_resolution_func();
                        }

                        ImGui::TextUnformatted("Resolution:");
                        ImGui::SameLine();

                        ImGuiUser::help_marker("NOTE: Only 16:9 resolutions are supported (standard widescreen format).");

                        ImGui::PushItemWidth(ImGui::GetContentRegionAvailWidth());
                        ImGui::PushStyleColor(ImGuiCol_FrameBg, {window_bg_color.x*window_dampening_multiplier,window_bg_color.y*window_dampening_multiplier,window_bg_color.z*window_dampening_multiplier,window_bg_color.w*window_dampening_multiplier});
                        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, {(static_cast<float>(background_color.r()) / 255.f) * 2.f, (static_cast<float>(background_color.g()) / 255.f) * 2.f, (static_cast<float>(background_color.b()) / 255.f) * 2.f, static_cast<float>(background_color.a()) / 255.f});
                        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, {(static_cast<float>(background_color.r()) / 255.f) * 1.5f, (static_cast<float>(background_color.g()) / 255.f) * 1.5f, (static_cast<float>(background_color.b()) / 255.f) * 1.5f, static_cast<float>(background_color.a()) / 255.f});
                        ImGui::PushStyleColor(ImGuiCol_Header, {1.f,1.f,1.f,0.f});
                        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, {(static_cast<float>(background_color.r()) / 255.f) * 1.5f, (static_cast<float>(background_color.g()) / 255.f) * 1.5f, (static_cast<float>(background_color.b()) / 255.f) * 1.5f, static_cast<float>(background_color.a()) / 255.f});
                        ImGui::PushStyleColor(ImGuiCol_HeaderActive, {(static_cast<float>(background_color.r()) / 255.f) * 2.f, (static_cast<float>(background_color.g()) / 255.f) * 2.f, (static_cast<float>(background_color.b()) / 255.f) * 2.f, static_cast<float>(background_color.a()) / 255.f});

                        if (ImGui::Combo("##res", &selected_resolution, supported_resolutions.second.c_str()))
                        {
                            auto& res = supported_resolutions.first.at(static_cast<std::size_t>(selected_resolution));

                            game_video_settings.cfgvalue_resolution->set(std::to_string(std::get<0>(res)) + 'x' + std::to_string(std::get<1>(res)));
                        }

                        ImGui::TextUnformatted("Info alignment:");
                        ImGui::SameLine();

                        ImGuiUser::help_marker("FPS, frametime and playtime to be drawn via this alignment (if any of them are enabled).");

                        static int32_t align_item = 0;

                        if (load_align_combo)
                        {
                            load_align_combo = false;

                            auto alignment = game_video_settings.cfgvalue_draw_position->get<std::string>();

                            std::transform(alignment.begin(), alignment.end(), alignment.begin(), ::tolower);

                            if (alignment.length() >= 7)
                            {
                                if (alignment.compare("topright") == 0) align_item = 1;
                                else if (alignment.compare("bottomleft") == 0) align_item = 2;
                                else if (alignment.compare("bottomright") == 0) align_item = 3;
                                else if (alignment.compare("topcenter") == 0) align_item = 4;
                                else if (alignment.compare("bottomcenter") == 0) align_item = 5;
                            }
                        }

                        static const char* alignment_items[] = { "Top left", "Top right", "Bottom left", "Bottom right", "Top center", "Bottom center" };

                        if (ImGui::Combo("##ialign", &align_item, alignment_items, 6))
                        {
                            std::string new_alignment = alignment_items[align_item];

                            new_alignment.erase(std::remove(new_alignment.begin(), new_alignment.end(), ' '), new_alignment.end());

                            std::transform(new_alignment.begin(), new_alignment.end(), new_alignment.begin(), tolower);

                            game_video_settings.cfgvalue_draw_position->set(new_alignment);
                        }

                        ImGui::PopItemWidth();
                        ImGuiUser::frame_height_spacing();
#endif

                        ImGui::TextWrapped("Game options");
                        ImGui::Separator();
                        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.f);
                        ImGui::PushStyleColor(ImGuiCol_FrameBg, frame_bg_color);
                        ImGui::PushStyleColor(ImGuiCol_TextSelectedBg, text_selected_bg);
                        ImGui::PushStyleColor(ImGuiCol_SliderGrab, slidergrab_color);
                        ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, slidergrab_color_active);
                        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, slidergrab_color_active);
                        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, text_selected_bg);

#ifdef PLATFORM_NS
                        ImGui::PushStyleColor(ImGuiCol_Header, {1.f,1.f,1.f,0.f});
                        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, {(static_cast<float>(background_color.r()) / 255.f) * 1.5f, (static_cast<float>(background_color.g()) / 255.f) * 1.5f, (static_cast<float>(background_color.b()) / 255.f) * 1.5f, static_cast<float>(background_color.a()) / 255.f});
                        ImGui::PushStyleColor(ImGuiCol_HeaderActive, {(static_cast<float>(background_color.r()) / 255.f) * 2.f, (static_cast<float>(background_color.g()) / 255.f) * 2.f, (static_cast<float>(background_color.b()) / 255.f) * 2.f, static_cast<float>(background_color.a()) / 255.f});
#endif

                        selected_game->draw_options(global_scaling);

#ifndef PLATFORM_NS
                        ImGui::PopStyleVar(2);
                        ImGui::PopStyleColor(12);
#else
                        ImGui::PopStyleVar();
                        ImGui::PopStyleColor(9);
#endif


                        if (subwindow_button_pressed)
                        {
                            // Back
                            start_state = start_game_e::START_GAME_STATE_MAIN;
                            
                            subwindow_button_pressed = false;
                        }
                        else if (subwindow_button_pressed_2)
                        {
#ifndef PLATFORM_NS
                            // Apply defaults
                            game_video_settings.cfgvalue_draw_fps->set(settings->get_main_settings().draw_fps->get<bool>());
                            game_video_settings.cfgvalue_draw_frametime->set(settings->get_main_settings().draw_frametime->get<bool>());
                            game_video_settings.cfgvalue_draw_playtime->set(settings->get_main_settings().draw_playtime->get<bool>());
                            game_video_settings.cfgvalue_draw_position->set(settings->get_main_settings().draw_position->get<std::string>());
                            game_video_settings.cfgvalue_timeout_time->set(settings->get_main_settings().timeout_time->get<uint32_t>());
                            game_video_settings.cfgvalue_fps->set(settings->get_main_settings().fps->get<uint32_t>());
                            game_video_settings.cfgvalue_fullscreen->set(settings->get_main_settings().fullscreen->get<bool>());
                            game_video_settings.cfgvalue_resolution->set(settings->get_main_settings().resolution->get<std::string>());
                            game_video_settings.cfgvalue_vsync->set(settings->get_main_settings().vsync->get<bool>());
                            game_video_settings.load_from_cfgvalues();
#else
                            const auto& gamename = selected_game->get_information().name;

                            settings->get(gamename + "_draw_fps").set(settings->get_main_settings().draw_fps->get<bool>());
                            settings->get(gamename + "_draw_frametime").set(settings->get_main_settings().draw_frametime->get<bool>());
                            settings->get(gamename + "_draw_playtime").set(settings->get_main_settings().draw_playtime->get<bool>());
                            settings->get(gamename + "_draw_position_alignment").set(settings->get_main_settings().draw_position->get<std::string>());
                            settings->get(gamename + "_lostfocus_timeout_time").set(settings->get_main_settings().timeout_time->get<uint32_t>());
#endif

                            load_align_combo = load_resolution = true;
                        }
                    }
                    else if (start_state == start_game_e::START_GAME_STATE_CONTROLS)
                    {
                        selected_game->draw_controls(global_scaling);

                    if (subwindow_button_pressed) { start_state = start_game_e::START_GAME_STATE_MAIN; subwindow_button_pressed = false; }
                    }
                    else
                    {
                        selected_game->draw_information(global_scaling);

                        if (subwindow_button_pressed) { start_state = start_game_e::START_GAME_STATE_MAIN; subwindow_button_pressed = false; }
                    }

                    ImGui::PopStyleVar();
                }
            }
            else if (selected_item == selection_e::selection_select_game)
            {
                ImGui::Text("Available games:");
                ImGui::Separator();
                //ImGui::Columns(4, nullptr, false);

                auto border_size = 1.f;
                auto button_size_box = std::ceil(((static_cast<float>(resolution_area.width) - indent_width * 2.f) - selection_size.x) / 4.f) - (ImGui::GetStyle().ItemSpacing.x/* + border_size * 2.f*/);
                auto button_size = ImVec2(button_size_box, button_size_box);

                ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, border_size);

                uint8_t count = 0;

                for (const auto& game : games)
                {
                    if (count != 0 && count != 4) ImGui::SameLine();

                    const auto& game_name = game.second->get_information().name;

                    if (ImGui::Button(game_name.c_str(), button_size))
                    {
                        set_notification(std::string("Game selected: ") + game_name);

                        selected_game = games_manager->select_game(game_name);
                        selected_game_name->set(game_name);

                        //ImGui::NextColumn();
                    }

                    count++;
                }

                ImGui::PopStyleVar();
                //ImGui::Columns(1);
            }
            else if (selected_item == selection_e::selection_options)
            {
                ImGui::TextWrapped("These are the default settings. Games will use these settings on first load, although they can be modified per-game.");
                ImGui::Separator();

                static auto window_bg_color = main_window_bg_color;
                static auto frame_bg_color = ImVec4{window_bg_color.x*window_dampening_multiplier,window_bg_color.y*window_dampening_multiplier,window_bg_color.z*window_dampening_multiplier,window_bg_color.w*window_dampening_multiplier};
                static auto text_selected_bg = ImVec4{(static_cast<float>(background_color.r()) / 255.f) * 1.5f, (static_cast<float>(background_color.g()) / 255.f) * 1.5f, (static_cast<float>(background_color.b()) / 255.f) * 1.5f, static_cast<float>(background_color.a()) / 255.f};
                static auto slidergrab_color = text_selected_bg;
                static auto slidergrab_color_active = ImVec4{(static_cast<float>(background_color.r()) / 255.f) * 2.f, (static_cast<float>(background_color.g()) / 255.f) * 2.f, (static_cast<float>(background_color.b()) / 255.f) * 2.f, static_cast<float>(background_color.a()) / 255.f};

#ifdef PLATFORM_NS
                // sliders
                ImGui::PushStyleColor(ImGuiCol_FrameBg, frame_bg_color);
                ImGui::PushStyleColor(ImGuiCol_TextSelectedBg, text_selected_bg);
                ImGui::PushStyleColor(ImGuiCol_SliderGrab, slidergrab_color);
                ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, slidergrab_color_active);
                ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, slidergrab_color_active);
                ImGui::PushStyleColor(ImGuiCol_FrameBgActive, text_selected_bg);

                ImGuiUser::inputslider_uint32_t(settings->get_main_settings().timeout_time, "Timeout (focus lost/start of game)", 10u, 0u, "The time in seconds the game will pause when starting the game or tabbing back into it. 0 means no timeout!");

                ImGui::PopStyleColor(6);

                // booleans
                ImGuiUser::toggle_button(settings->get_main_settings().draw_fps, "Draw FPS", "If enabled, the FPS will be drawn in games using the specified alignment.");
                ImGuiUser::toggle_button(settings->get_main_settings().draw_frametime, "Draw frametime", "If enabled, the frametime (in ms) will be drawn in games using the specified alignment.");
                ImGuiUser::toggle_button(settings->get_main_settings().draw_playtime, "Draw playtime", "If enabled, the playtime (hh:mm:ss:ms) will be drawn in games using the specified alignment.");

                // Combos
                ImGui::Separator();

#ifndef PLATFORM_NS
                auto window_bg_color = main_window_bg_color;
#endif

                ImGui::PushItemWidth(ImGui::GetContentRegionAvailWidth());
                ImGui::PushStyleColor(ImGuiCol_FrameBg, {window_bg_color.x*window_dampening_multiplier,window_bg_color.y*window_dampening_multiplier,window_bg_color.z*window_dampening_multiplier,window_bg_color.w*window_dampening_multiplier});
                ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, {(static_cast<float>(background_color.r()) / 255.f) * 2.f, (static_cast<float>(background_color.g()) / 255.f) * 2.f, (static_cast<float>(background_color.b()) / 255.f) * 2.f, static_cast<float>(background_color.a()) / 255.f});
                ImGui::PushStyleColor(ImGuiCol_FrameBgActive, {(static_cast<float>(background_color.r()) / 255.f) * 1.5f, (static_cast<float>(background_color.g()) / 255.f) * 1.5f, (static_cast<float>(background_color.b()) / 255.f) * 1.5f, static_cast<float>(background_color.a()) / 255.f});
                ImGui::PushStyleColor(ImGuiCol_Header, {1.f,1.f,1.f,0.f});
                ImGui::PushStyleColor(ImGuiCol_HeaderHovered, {(static_cast<float>(background_color.r()) / 255.f) * 1.5f, (static_cast<float>(background_color.g()) / 255.f) * 1.5f, (static_cast<float>(background_color.b()) / 255.f) * 1.5f, static_cast<float>(background_color.a()) / 255.f});
                ImGui::PushStyleColor(ImGuiCol_HeaderActive, {(static_cast<float>(background_color.r()) / 255.f) * 2.f, (static_cast<float>(background_color.g()) / 255.f) * 2.f, (static_cast<float>(background_color.b()) / 255.f) * 2.f, static_cast<float>(background_color.a()) / 255.f});
                ImGui::TextUnformatted("Info alignment:");
                ImGui::SameLine();

                ImGuiUser::help_marker("FPS, frametime and playtime to be drawn via this alignment (if any of them are enabled).");

                static int align_item = [&]()
                {
                    auto alignment = settings->get_main_settings().draw_position->get<std::string>();

                    std::transform(alignment.begin(), alignment.end(), alignment.begin(), ::tolower);

                    if (alignment.length() >= 7)
                    {
                        if (alignment.compare("topright") == 0) return 1;
                        else if (alignment.compare("bottomleft") == 0) return 2;
                        else if (alignment.compare("bottomright") == 0) return 3;
                        else if (alignment.compare("topcenter") == 0) return 4;
                        else if (alignment.compare("bottomcenter") == 0) return 5;
                    }

                    return 0;
                }();

                static const char* alignment_items[] = { "Top left", "Top right", "Bottom left", "Bottom right", "Top center", "Bottom center" };

                if (ImGui::Combo("##ialign", &align_item, alignment_items, 6))
                {
                    std::string new_alignment = alignment_items[align_item];

                    new_alignment.erase(std::remove(new_alignment.begin(), new_alignment.end(), ' '), new_alignment.end());

                    std::transform(new_alignment.begin(), new_alignment.end(), new_alignment.begin(), tolower);

                    settings->get_main_settings().draw_position->set(new_alignment);
                }

                ImGui::PopStyleColor(6);
                ImGui::PopItemWidth();
    #else
                // sliders
                ImGui::PushStyleColor(ImGuiCol_FrameBg, frame_bg_color);
                ImGui::PushStyleColor(ImGuiCol_TextSelectedBg, text_selected_bg);
                ImGui::PushStyleColor(ImGuiCol_SliderGrab, slidergrab_color);
                ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, slidergrab_color_active);
                ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, slidergrab_color_active);
                ImGui::PushStyleColor(ImGuiCol_FrameBgActive, text_selected_bg);

                ImGuiUser::inputslider_uint32_t(settings->get_main_settings().fps, "FPS", 1000u, 0u, "Sets the framerate limit. This setting will be ignored if vertical sync is enabled.", global_scaling);
                ImGuiUser::inputslider_uint32_t(settings->get_main_settings().timeout_time, "Timeout (focus lost/start of game)", 10u, 0u, "The time in seconds the game will pause when starting the game or tabbing back into it. 0 means no timeout!");

                ImGui::PopStyleColor(6);

                // booleans
                ImGui::Separator();

                ImGuiUser::toggle_button(settings->get_main_settings().fullscreen, "Fullscreen", "Turns on/off fullscreen video mode.");
                ImGuiUser::toggle_button(settings->get_main_settings().vsync, "Vertical sync", "Turns on/off vertical sync. Reduces screen tearing, although framerate will be limited to the refresh rate of your monitor.");
                ImGuiUser::toggle_button(settings->get_main_settings().draw_fps, "Draw FPS", "If enabled, the FPS will be drawn in games using the specified alignment.");
                ImGuiUser::toggle_button(settings->get_main_settings().draw_frametime, "Draw frametime", "If enabled, the frametime (in ms) will be drawn in games using the specified alignment.");
                ImGuiUser::toggle_button(settings->get_main_settings().draw_playtime, "Draw playtime", "If enabled, the playtime (hh:mm:ss:ms) will be drawn in games using the specified alignment.");

                // Combos
                ImGui::Separator();

                static auto& supported_resolutions = util::get_supported_resolutions();
                static auto selected_resolution = [&]() -> int32_t
                {
                    // Grab the wanted resolution from our settings
                    auto resolution = settings->get_main_settings().resolution_area;

                    // The default resolution in case the selected one wasn't found
                    constexpr int32_t default_resolution = 0;

                    // Return the found resolution
                    auto found = std::find_if(supported_resolutions.first.begin(), supported_resolutions.first.end(), [&resolution](const std::tuple<uint16_t, uint16_t, uint16_t>& p) -> bool
                    {
                        return static_cast<uint32_t>(std::get<0>(p)) == resolution.width && static_cast<uint32_t>(std::get<1>(p)) == resolution.height;
                    });

                    return found != supported_resolutions.first.end() ? static_cast<int32_t>(std::distance(supported_resolutions.first.begin(), found)) : default_resolution;
                }();

                ImGui::TextUnformatted("Resolution:");
                ImGui::SameLine();

                ImGuiUser::help_marker("NOTE: Only 16:9 resolutions are supported (standard widescreen format).");

                ImGui::PushItemWidth(ImGui::GetContentRegionAvailWidth());
                ImGui::PushStyleColor(ImGuiCol_FrameBg, {window_bg_color.x*window_dampening_multiplier,window_bg_color.y*window_dampening_multiplier,window_bg_color.z*window_dampening_multiplier,window_bg_color.w*window_dampening_multiplier});
                ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, {(static_cast<float>(background_color.r()) / 255.f) * 2.f, (static_cast<float>(background_color.g()) / 255.f) * 2.f, (static_cast<float>(background_color.b()) / 255.f) * 2.f, static_cast<float>(background_color.a()) / 255.f});
                ImGui::PushStyleColor(ImGuiCol_FrameBgActive, {(static_cast<float>(background_color.r()) / 255.f) * 1.5f, (static_cast<float>(background_color.g()) / 255.f) * 1.5f, (static_cast<float>(background_color.b()) / 255.f) * 1.5f, static_cast<float>(background_color.a()) / 255.f});
                ImGui::PushStyleColor(ImGuiCol_Header, {1.f,1.f,1.f,0.f});
                ImGui::PushStyleColor(ImGuiCol_HeaderHovered, {(static_cast<float>(background_color.r()) / 255.f) * 1.5f, (static_cast<float>(background_color.g()) / 255.f) * 1.5f, (static_cast<float>(background_color.b()) / 255.f) * 1.5f, static_cast<float>(background_color.a()) / 255.f});
                ImGui::PushStyleColor(ImGuiCol_HeaderActive, {(static_cast<float>(background_color.r()) / 255.f) * 2.f, (static_cast<float>(background_color.g()) / 255.f) * 2.f, (static_cast<float>(background_color.b()) / 255.f) * 2.f, static_cast<float>(background_color.a()) / 255.f});

                if (ImGui::Combo("##res", &selected_resolution, supported_resolutions.second.c_str()))
                {
                    auto& res = supported_resolutions.first.at(static_cast<std::size_t>(selected_resolution));

                    settings->get_main_settings().resolution->set(std::to_string(std::get<0>(res)) + 'x' + std::to_string(std::get<1>(res)));
                }

                ImGui::TextUnformatted("Info alignment:");
                ImGui::SameLine();

                ImGuiUser::help_marker("FPS, frametime and playtime to be drawn via this alignment (if any of them are enabled).");

                static int align_item = [&]()
                {
                    auto alignment = settings->get_main_settings().draw_position->get<std::string>();

                    std::transform(alignment.begin(), alignment.end(), alignment.begin(), ::tolower);

                    if (alignment.length() >= 7)
                    {
                        if (alignment.compare("topright") == 0) return 1;
                        else if (alignment.compare("bottomleft") == 0) return 2;
                        else if (alignment.compare("bottomright") == 0) return 3;
                        else if (alignment.compare("topcenter") == 0) return 4;
                        else if (alignment.compare("bottomcenter") == 0) return 5;
                    }

                    return 0;
                }();

                static const char* alignment_items[] = { "Top left", "Top right", "Bottom left", "Bottom right", "Top center", "Bottom center" };

                if (ImGui::Combo("##ialign", &align_item, alignment_items, 6))
                {
                    std::string new_alignment = alignment_items[align_item];

                    new_alignment.erase(std::remove(new_alignment.begin(), new_alignment.end(), ' '), new_alignment.end());

                    std::transform(new_alignment.begin(), new_alignment.end(), new_alignment.begin(), tolower);

                    settings->get_main_settings().draw_position->set(new_alignment);
                }

                ImGui::PopStyleColor(6);
                ImGui::PopItemWidth();

                // Apply button, move to bottom of the window
                if (subwindow_button_pressed)
                {
                    subwindow_button_pressed = false;

                    auto new_video_settings = video_settings_t(settings);

                    if (original_video_settings.differs(new_video_settings))
                    {
                        reset_video_mode = true;

                        original_video_settings = new_video_settings;

                        set_notification("Video settings changed!");
                    }
                    else
                    {
                        set_notification("No video settings changed!");
                    }
                }
#endif
            }
            else if (selected_item == selection_e::selection_exit)
            {
                ImGui::TextWrapped("Until next time, hope you enjoyed ;)");
                ImGui::Separator();

                auto button_size = ImVec2{ImGui::GetContentRegionAvailWidth(),0.f};

                ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.f);

                if (ImGui::Button("Exit", button_size)) should_exit = true;

                ImGui::PopStyleVar();
            }
            else if (selected_item == selection_e::selection_credits)
            {
                ImGui::TextWrapped("Every external influence to this project is listed here. I'm sorry if I forgot anyone or anything, feel free to remind me though.");
                ImGui::Separator();
                ImGui::BulletText("ocornut - Dear ImGui");
                ImGui::BulletText("devkitpro/libnx - compiler toolchain/SDK for the Switch");
                ImGui::BulletText("RetroArch - Design inspiration");
                ImGui::BulletText("nlohmann - JSON library");
                ImGui::BulletText("Microsoft - DirectX SDK (Windows backend) + Win32API");
                ImGui::BulletText("GLFW - Linux backend");
                ImGui::BulletText("Probably various other things I forgot");
            }

            if (draw_bottom_button)
            {
                ImGui::EndChild();
                ImGui::PopStyleColor();
            }
        }

        if (draw_bottom_button)
        {
            ImGui::SetNextWindowPos(ImVec2(indent_width + selection_size.x + ImGui::GetStyle().WindowPadding.x, (selection_pos.y+(selection_size.y-ImGui::GetFrameHeightWithSpacing()))-window_y_offset), ImGuiCond_Always);
            ImGui::PushStyleColor(ImGuiCol_Border,{1.f,1.f,1.f,1.f});
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.f);
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,{ImGui::GetStyle().ItemSpacing.x,0.f});
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,{ImGui::GetStyle().WindowPadding.x,0.f});

            if (ImGui::BeginChild("Main child window #2", ImVec2(((static_cast<float>(resolution_area.width) - indent_width * 2.f) - selection_size.x)-ImGui::GetStyle().WindowPadding.x*2.f, ImGui::GetFrameHeightWithSpacing()+window_y_offset), false, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove/* | ImGuiWindowFlags_NoNav*/))
            {
                if (subwindow_button_text_2.empty())
                {
                    if (ImGui::Button(subwindow_button_text.c_str(), ImVec2{ImGui::GetContentRegionAvailWidth(),0.f})) subwindow_button_pressed = true;
                }
                else
                {
                    //ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,{ImGui::GetStyle().ItemSpacing.x,0.f});

                    auto button_size = ImVec2{ImGui::GetContentRegionAvailWidth()*.5f-ImGui::GetStyle().ItemSpacing.x*.5f,0.f};

                    if (ImGui::Button(subwindow_button_text.c_str(), button_size)) subwindow_button_pressed = true;

                    ImGui::SameLine();

                    if (ImGui::Button(subwindow_button_text_2.c_str(), button_size)) subwindow_button_pressed_2 = true;

                    //ImGui::PopStyleVar();
                }

                ImGui::EndChild();
            }

            ImGui::PopStyleColor();
            ImGui::PopStyleVar(5);
        }

        ImGui::End();
    }

    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor();
    ImGui::PopFont();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(2);

    // draw any notifications we might have set
    draw_notification();

    // draw the time
    ImGui::PushFont(default_font_small);

    time_t rawtime;
    tm* timeinfo;

    time(&rawtime);

    timeinfo = localtime (&rawtime);

    char datetime_buffer[80];

    strftime(datetime_buffer, 80, "%d/%m %I:%M%p", timeinfo);

    auto date_text_size = ImGui::CalcTextSize(datetime_buffer);

    ImGui::GetBackgroundDrawList()->AddText(ImVec2((resolution_area.width - indent_width * 2.f) - date_text_size.x, indent_height * .5f - ImGui::GetFontSize() * .5f), ImGui::GetColorU32({1.f,1.f,1.f,1.f}), datetime_buffer);
    ImGui::PopFont();

    // draw the currently selected item
    ImGui::PushFont(default_font_big);
    ImGui::GetBackgroundDrawList()->AddText(ImVec2(indent_width * 2.f, indent_height * .5f - ImGui::GetFontSize() * .5f), ImGui::GetColorU32({1.f,1.f,1.f,1.f}), selection_names[static_cast<uint8_t>(selected_item)].c_str());
    ImGui::PopFont();

    // lines at the top and bottom
    ImGui::GetBackgroundDrawList()->AddLine(ImVec2(indent_width, indent_height), ImVec2(resolution_area.width - indent_width, indent_height), ImGui::GetColorU32({1.f, 1.f, 1.f, 1.f}));
    ImGui::GetBackgroundDrawList()->AddLine(ImVec2(indent_width, indent_height + selection_size.y), ImVec2(resolution_area.width - indent_width, indent_height + selection_size.y), ImGui::GetColorU32({1.f,1.f,1.f,1.f}));

    // selected game
    ImGui::PushFont(default_font_small);
    ImGui::GetBackgroundDrawList()->AddText(ImVec2(indent_width * 2.f, static_cast<float>(resolution_area.height) - indent_height * .5f - ImGui::GetFontSize() * .5f), ImGui::GetColorU32({1.f,1.f,1.f,1.f}), (std::string("v") + CPP_RETRO_GAMES_VERSION + " - selected game: " + (selected_game == nullptr ? "none" : selected_game->get_information().name)).c_str());
    ImGui::PopFont();

    // fps
    ImGui::PushFont(default_font_small);

    char fps_text[80];

    sprintf(fps_text, "fps: %.0f", ImGui::GetIO().Framerate);

    ImGui::GetBackgroundDrawList()->AddText(ImVec2((resolution_area.width - indent_width * 2.f) - ImGui::CalcTextSize(fps_text).x, static_cast<float>(resolution_area.height) - indent_height * .5f - ImGui::GetFontSize() * .5f), ImGui::GetColorU32({1.f,1.f,1.f,1.f}), fps_text);
    ImGui::PopFont();

    return should_exit;
}

/*
@brief

    Handles key events
*/
void retrogames::mainmenu_t::handle_key(bool down, ImGuiKey key)
{
    if (selected_game != nullptr && game_running) selected_game->handle_key(key, down);
}