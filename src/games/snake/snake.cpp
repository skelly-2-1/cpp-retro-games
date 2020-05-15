/*
@file

    snake.cpp

@purpose

    Actual snake game and GUI functionality
*/

#include <array>
#include <deque>
#include <random>
#include <algorithm>
#include "snake.h"
#include "misc/area_size.h"
#include "misc/macros.h"
#include "imgui/imgui_user.h"

// Some defines we'll use
#define with(decl)\
    for (bool __f = true; __f; ) \
    for (decl; __f; __f = false)
#define modal_popup(args...)\
    with(auto __modal = modal_popup_t(args))\
    if (__modal.success())

// Set the current modal popup id to 0
uint8_t retrogames::games::snake_t::current_modal_popup_id = 0;

/*
@brief

    Called from our renderer thread when we need to draw
*/
bool retrogames::games::snake_t::draw(bool render)
{
    if (!render) return false;

    // Calculate the space we have available to the right and left of the actual snake playing field.
    // We have a 16:9 resolution so we won't have a perfect square.
    // We can use the left and right sides to display some information while playing.
    const uint16_t left_right_distance = (static_cast<uint16_t>(resolution_area.width) - static_cast<uint16_t>(resolution_area.height)) / 2;
    const ImVec2 snake_playing_field_position = ImVec2(left_right_distance, 0);
    const ImVec2 snake_playing_field_size = ImVec2(resolution_area.height, resolution_area.height);
    const ImVec2 snake_playing_field_end = ImVec2(snake_playing_field_position.x + snake_playing_field_size.x, snake_playing_field_position.y + snake_playing_field_size.y);

    static auto imgui_window = [this](bool no_padding, const uint8_t id, const ImVec2 pos, const ImVec2 size, auto func, ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar)
    {
        char fmt[256];

        sprintf(fmt, "##playingfield_%i", id);

        // Create an imgui window at our snake playing field to draw to
        if (no_padding)
        {
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.0f, 0.0f });
        }

        ImGui::PushStyleColor(ImGuiCol_Border, { 0.0f, 0.0f, 0.0f, 0.0f });
        ImGui::PushStyleColor(ImGuiCol_BorderShadow, { 0.0f, 0.0f, 0.0f, 0.0f });
        ImGui::PushStyleColor(ImGuiCol_WindowBg, { 0.0f, 0.0f, 0.0f, 0.0f });
        ImGui::Begin(fmt, nullptr, flags | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoNav);
        ImGui::SetWindowPos(pos, ImGuiCond_Always);
        ImGui::SetWindowSize(size, ImGuiCond_Always);

        // Draw our snake playing field
        func();

        // End the window
        ImGui::End();
        ImGui::PopStyleColor(3);

        if (no_padding) ImGui::PopStyleVar(2);
    };

    // Create an imgui window at our snake playing field to draw to
    imgui_window(true, 0, snake_playing_field_position, snake_playing_field_size, [this]() { draw_field(); }, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoInputs);

    // Create imgui windows for the left/right sides of the window to draw to
    imgui_window(false, 1, ImVec2(0.f, 0.f), ImVec2(static_cast<float>(left_right_distance), resolution_area.height), [this]() { draw_left_window(); });
    imgui_window(false, 2, ImVec2(static_cast<float>(left_right_distance) + snake_playing_field_size.x, 0.f), ImVec2(static_cast<float>(left_right_distance), resolution_area.height), [this]() { draw_right_window(); });
    
    if (should_exit)
    {
        should_exit = false;

        return true;
    }

    return false;
}

/*
@brief

    Draws the snake playing field and handles all the logic
*/
void retrogames::games::snake_t::draw_field(void)
{
    // Get the window size
    auto window_size = ImVec2(resolution_area.height, resolution_area.height);

    // Fill the playing field background
    draw_filled_rect(ImVec2(0, 0), window_size, color_t(0, 0, 0));

    // List of parts for the snake, so we can first draw the outline and then the snake on top
    static std::deque<std::tuple<ImVec2, color_t, color_t>> parts;

    // Clear the parts list
    parts.clear();

    // Helper function to add a snake part to the draw list
    static auto add_part = [this](const ImVec2& box_position, const float& offset, const DIRECTION& direction, color_t fill_color, color_t outline_color)
    {
        auto pos = ImVec2(box_position.x * static_cast<float>(box_size), box_position.y * static_cast<float>(box_size));

        static std::array<ImVec2, 4> direction_offsets = {

            ImVec2(0.f, -1.f),
            ImVec2(0.f, 1.f),
            ImVec2(-1.f, 0.f),
            ImVec2(1.f, 0.f)

        };

        if (offset > 0.f)
        {
            // Now, we can grab the offset with the current direction
            auto calculated_offset = direction_offsets[static_cast<uint8_t>(direction) - 1];

            // Add the scaled direction offset to the pos
            pos.x += (calculated_offset.x * static_cast<float>(box_size)) * offset;
            pos.y += (calculated_offset.y * static_cast<float>(box_size)) * offset;
        }

        parts.push_back(std::make_tuple(pos, fill_color, outline_color));
    };

    // Outline and snake colors
    // Outline gets changed if we're dead
    static auto outline_color_original = color_t(200, 200, 200);

    auto outline_color = outline_color_original;

    static auto snake_color = color_t(0, 200, 0);
    static auto snake_head_color = color_t(0, 150, 0);

    if (dead)
    {
        // We died, pulsate the outline color instead (every second)
        auto ms_elapsed = static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - death_time).count());
        auto angle = (static_cast<double>(static_cast<double>(ms_elapsed) - std::floor(static_cast<double>(ms_elapsed) / 1000.0) * 1000.0) / 1000.0) * 360.0;
        auto scale = 0.5 + std::sin(angle * 0.0174533) * 0.5; // 0.f to 1.f

        outline_color = color_t(255 / 2 + static_cast<uint8_t>(scale * (255 / 2)), 0, 0);
    }

    // Draw the valid snake positions
    auto positions_color = color_t(100, 100, 100, 100);

    for (uint32_t i = 1; i < box_amount; i++)
    {
        draw_line(ImVec2(0.f, static_cast<float>(i * box_size)), ImVec2(resolution_area.height, static_cast<float>(i * box_size)), positions_color);
        draw_line(ImVec2(static_cast<float>(i * box_size), 0.f), ImVec2(static_cast<float>(i * box_size), resolution_area.height), positions_color);
    }

    // Calculate the delay between snake move-frames
    // Also check if we're able to move or not (not mid-animation)
    float scale = 0.f;
    bool can_move = false;
    {
        auto& update_interval = fpsmanager.get_update_interval();
        auto& next_frame = fpsmanager.get_next_frame_time_point();
        auto now = std::chrono::high_resolution_clock::now();

        if (now < next_frame)
        {
            scale = 1.f - (static_cast<float>((next_frame - now).count()) / static_cast<float>(update_interval.count()));
        }
        else
        {
            can_move = true;
        }
    }

    // Move the snake if need be, also remember if we've moved or not (needed later)
    bool moved = false;

    if (can_move)
    {
        if (hit_pause)
        {
            toggle_pause();

            hit_pause = false;
        }

        if (!is_paused())
        {
            auto dir = think();

            if (dir != DIRECTION::SNAKE_DIRECTION_NONE)
            {
                static_vars.direction = dir;

                moved = true;
            }
        }
        else
        {
            scale = 1.f;
        }
    }
    else if (is_in_timeout())
    {
        scale = 0.f;
    }

    // Move one more time if we died (since we're one frame behind)
    if (static_vars.last_dead_needs_reset)
    {
        static_vars.last_dead = dead;
        static_vars.last_dead_needs_reset = false;
    }

    if (static_vars.last_dead != dead)
    {
        static_vars.last_dead = dead;

        if (dead) static_vars.interpolate_last_time = true;
    }

    if (dead)
    {
        if (!static_vars.interpolate_last_time) scale = 0.f;
        else
        {
            if (scale == 0.f)
            {
                last_head = head;

                static_vars.interpolate_last_time = false;
            }
        }
    }

    // Add the snake parts
    for (int32_t i = static_cast<int32_t>(last_position_history.size()); i >= 0; i--)
    {
        auto dir = DIRECTION::SNAKE_DIRECTION_DEFAULT;

        if (i == last_position_history.size())
        {
            // Fill in corners
            if (!last_position_history.empty()) add_part(last_head, 0.f, dir, snake_color, outline_color);

            // Add the head (interpolated)
            add_part(last_head, static_vars.direction == DIRECTION::SNAKE_DIRECTION_NONE ? 0.f : scale, static_vars.direction, snake_head_color, outline_color);

            continue;
        }
        else if (i == 0 && !last_position_history.empty() && move_eat_counter == move_counter)
        {
            // We've just eaten and this is the tail, smooth out the animation of it popping out.
            ImVec2 next_box_position{}; // One block after our tail

            if (last_position_history.size() > 1)
            {
                auto& pos = last_position_history[static_cast<uint64_t>(i) + 1];

                next_box_position = ImVec2{ static_cast<float>(pos.first), static_cast<float>(pos.second) };
            }
            else
            {
                next_box_position = last_head;
            }

            add_part(next_box_position, 0.f, dir, snake_color, outline_color);

            continue;
        }

        auto& pos = last_position_history[i];
        auto box_position = ImVec2{ static_cast<float>(pos.first), static_cast<float>(pos.second) };

        if (i == last_position_history.size() - 1)
        {
            if (last_head.x == box_position.x + 1) dir = DIRECTION::SNAKE_DIRECTION_RIGHT;
            else if (last_head.x == box_position.x - 1) dir = DIRECTION::SNAKE_DIRECTION_LEFT;
            else if (last_head.y == box_position.y + 1) dir = DIRECTION::SNAKE_DIRECTION_DOWN;
            else if (last_head.y == box_position.y - 1) dir = DIRECTION::SNAKE_DIRECTION_UP;
        }
        else
        {
            auto& next_pos = last_position_history[static_cast<uint64_t>(i) + 1];
            auto next_box_position = ImVec2{ static_cast<float>(next_pos.first), static_cast<float>(next_pos.second) };

            if (next_box_position.x == box_position.x + 1) dir = DIRECTION::SNAKE_DIRECTION_RIGHT;
            else if (next_box_position.x == box_position.x - 1) dir = DIRECTION::SNAKE_DIRECTION_LEFT;
            else if (next_box_position.y == box_position.y + 1) dir = DIRECTION::SNAKE_DIRECTION_DOWN;
            else if (next_box_position.y == box_position.y - 1) dir = DIRECTION::SNAKE_DIRECTION_UP;

            // To fill in corners (looks kinda trippy otherwise)
            add_part(next_box_position, 0.f, dir, snake_color, outline_color);
        }

        add_part(box_position, scale, dir, snake_color, outline_color);
    }

    // Helper function to get outline position and size
    const auto get_outline_pos_and_size = [this](const ImVec2& pos) -> std::pair<ImVec2, ImVec2>
    {
        auto _pos = ImVec2(pos.x - 1.f, pos.y - 1.f);
        auto size = ImVec2(ImVec2(static_cast<float>(box_size) + 2.f, static_cast<float>(box_size) + 2.f));

        if (_pos.x < 0)
        {
            _pos.x = 0.f;
            size.x -= 1.f;
        }
        else if (pos.x + size.x >= static_cast<float>(box_amount) * static_cast<float>(box_size))
        {
            size.x -= 2.f;
        }

        if (_pos.y < 0)
        {
            _pos.y = 0.f;
            size.y -= 1.f;
        }
        else if (pos.y + size.y >= static_cast<float>(box_amount) * static_cast<float>(box_size))
        {
            size.y -= 2.f;
        }

        return std::make_pair(_pos, size);
    };

    // Helper function to draw an outline
    const auto draw_outline = [this, get_outline_pos_and_size](const ImVec2& pos, const color_t& color)
    {
        auto _pos = get_outline_pos_and_size(pos);

        draw_rect(_pos.first, _pos.second, color);
    };

    // Draw foods
    static const auto food_color = color_t(200, 0, 0);

    // Outline
    for (const auto& food : foods)
    {
        draw_outline(ImVec2(static_cast<float>(std::get<0>(food) * box_size), static_cast<float>(std::get<1>(food) * box_size)), outline_color_original);
    }

    // Fill
    for (const auto& food : foods)
    {
        draw_filled_rect(ImVec2(static_cast<float>(std::get<0>(food) * box_size), static_cast<float>(std::get<1>(food) * box_size)), ImVec2(static_cast<float>(box_size), static_cast<float>(box_size)), food_color);
    }

    // Cached foods (to smooth out the animation of eating the food)
    if (!cached_foods.empty())
    {
        // Outline
        for (const auto& food : cached_foods)
        {
            draw_outline(ImVec2(static_cast<float>(std::get<0>(food) * box_size), static_cast<float>(std::get<1>(food) * box_size)), outline_color_original);
        }

        // Fill
        for (auto it = cached_foods.begin(); it != cached_foods.end();)
        {
            auto food = *it;

            if (std::get<2>(food) == move_counter)
            {
                draw_filled_rect(ImVec2(static_cast<float>(std::get<0>(food) * box_size), static_cast<float>(std::get<1>(food) * box_size)), ImVec2(static_cast<float>(box_size), static_cast<float>(box_size)), food_color);

                it++;
            }
            else
            {
                it = cached_foods.erase(it);
            }
        }
    }

    // List of outer snake parts, due to our rendering method we have to fix the outside of
    // the playing field not flashing red when dying. We do that with this.
    static std::vector<std::pair<uint16_t, uint16_t>> outside_parts;

    outside_parts.clear();

    // Draw the snake outline
    for (const auto& part : parts)
    {
        const auto& pos = std::get<0>(part);
        const auto& outline_color = std::get<2>(part);

        draw_outline(pos, outline_color);

        // Check if we have an outside snake part if we're dead (due to the rendering order)
        if (dead)
        {
            const auto outside_pos = static_cast<float>(box_size) * (static_cast<float>(box_amount) - 1.f);

            if (pos.x == 0.f || pos.y == 0.f || pos.x == outside_pos || pos.y == outside_pos) outside_parts.push_back(std::make_pair(static_cast<uint16_t>(pos.x), static_cast<uint16_t>(pos.y)));
        }
    }

    // Fill in the snake
    for (const auto& part : parts)
    {
        const auto& pos = std::get<0>(part);
        const auto& fill_color = std::get<1>(part);

        draw_filled_rect(pos, ImVec2(static_cast<float>(box_size), static_cast<float>(box_size)), fill_color);
    }
    
    // Draw the field outline
    draw_rect(ImVec2(0.f, 0.f), window_size, outline_color_original);

    // Replace parts of the field outline with flashing red if we died
    if (!outside_parts.empty())
    {
        for (const auto& pos : outside_parts)
        {
            const auto outside_pos = static_cast<float>(box_size) * (static_cast<float>(box_amount) - 1.f);
            const auto outside_draw_pos = window_size.y - 1.f;

            if (pos.first == 0.f) draw_line(ImVec2(0.f, static_cast<float>(pos.second)), ImVec2(0.f, static_cast<float>(pos.second) + static_cast<float>(box_size)), outline_color);
            else if (pos.first == outside_pos) draw_line(ImVec2(outside_draw_pos, static_cast<float>(pos.second)), ImVec2(outside_draw_pos, static_cast<float>(pos.second) + static_cast<float>(box_size)), outline_color);

            if (pos.second == 0.f) draw_line(ImVec2(static_cast<float>(pos.first), 0.f), ImVec2(static_cast<float>(pos.first) + static_cast<float>(box_size), 0.f), outline_color);
            else if (pos.second == outside_pos) draw_line(ImVec2(static_cast<float>(pos.first), outside_draw_pos), ImVec2(static_cast<float>(pos.first) + static_cast<float>(box_size), outside_draw_pos), outline_color);
        }
    }

    // If the player is dead, draw the death menu
    // and if the game is paused, draw a pause menu
    if (dead) draw_death_menu();
}

/*
@brief

    Draws a filled rectangle at the @pos and @size in the current ImGui window
*/
void retrogames::games::snake_t::draw_filled_rect(const ImVec2& pos, const ImVec2& size, color_t color)
{
    auto window_pos = ImGui::GetWindowPos();

    ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(window_pos.x + pos.x, window_pos.y + pos.y), ImVec2(window_pos.x + pos.x + size.x, window_pos.y + pos.y + size.y), ImGuiUser::color_to_imgui_color_u32(color));
}

/*
@brief

    Draws a rectangle at the @pos and @size in the current ImGui window
*/
void retrogames::games::snake_t::draw_rect(const ImVec2& pos, const ImVec2& size, color_t color)
{
    auto window_pos = ImGui::GetWindowPos();

    ImGui::GetWindowDrawList()->AddRect(ImVec2(window_pos.x + pos.x, window_pos.y + pos.y), ImVec2(window_pos.x + pos.x + size.x, window_pos.y + pos.y + size.y), ImGuiUser::color_to_imgui_color_u32(color));
}

/*
@brief

    Draws a line between @pos_1 and @pos_2 given the @thickness
    in the current ImGui window
*/
void retrogames::games::snake_t::draw_line(const ImVec2& pos_1, const ImVec2& pos_2, color_t color, float thickness/* = 1.f*/)
{
    auto window_pos = ImGui::GetWindowPos();

    ImGui::GetWindowDrawList()->AddLine(ImVec2(window_pos.x + pos_1.x, window_pos.y + pos_1.y), ImVec2(window_pos.x + pos_2.x, window_pos.y + pos_2.y), ImGuiUser::color_to_imgui_color_u32(color), thickness);
}

/*
@brief

    Handles key down and up messages
*/
void retrogames::games::snake_t::handle_key(ImGuiKey key, bool pressed)
{
    if (!pressed) return;

    // Escape (un)pauses the game
    if (key == ImGuiKey_Escape && !dead) hit_pause = true;
    if (is_paused() || (!is_paused() && hit_pause)) return;

    // Helper function to add a direction to our direction stack (thread-safe)
    static auto add_direction = [this](DIRECTION dir)
    {
        // Determine the current direction
        auto current_dir = force_direction == DIRECTION::SNAKE_DIRECTION_NONE ? direction_stack.at(0) : force_direction;

        // Already set
        if (dir == current_dir) return;

        // Check if it goes against itself
        if ((dir == DIRECTION::SNAKE_DIRECTION_RIGHT && current_dir == DIRECTION::SNAKE_DIRECTION_LEFT ||
            dir == DIRECTION::SNAKE_DIRECTION_LEFT && current_dir == DIRECTION::SNAKE_DIRECTION_RIGHT ||
            dir == DIRECTION::SNAKE_DIRECTION_UP && current_dir == DIRECTION::SNAKE_DIRECTION_DOWN ||
            dir == DIRECTION::SNAKE_DIRECTION_DOWN && current_dir == DIRECTION::SNAKE_DIRECTION_UP))
        {
            if (!position_history.empty()) return;
        }

        // All good, add the direction to our stack
        direction_stack.push_back(dir);

        // We can now reset the force direction because we have a stack
        force_direction = DIRECTION::SNAKE_DIRECTION_NONE;
    };

    switch (key)
    {
		case ImGuiKey_LeftArrow:
		{
			add_direction(DIRECTION::SNAKE_DIRECTION_LEFT);

			break;
		}
		case ImGuiKey_RightArrow:
		{
			add_direction(DIRECTION::SNAKE_DIRECTION_RIGHT);

			break;
		}
		case ImGuiKey_UpArrow:
		{
			add_direction(DIRECTION::SNAKE_DIRECTION_UP);

			break;
		}
		case ImGuiKey_DownArrow:
		{
			add_direction(DIRECTION::SNAKE_DIRECTION_DOWN);

			break;
		}
		default:
		{
			break;
		}
    }
}

/*
@brief

    Kills the snake
*/
void retrogames::games::snake_t::kill(void)
{
    dead = true;
    death_time = std::chrono::high_resolution_clock::now();
    death_state = DEATH_STATE::DEATH_STATE_MAIN;
}

/*
@brief

    Resets all the variables associated with the snake
*/
void retrogames::games::snake_t::do_reset(void)
{
    // Reset the direction
    {
        // Force another direction
        force_direction = DIRECTION::SNAKE_DIRECTION_DEFAULT;

        // Clear the direction stack
        direction_stack.clear();
    }

    // Clear the position history
    position_history.clear();
    last_position_history.clear();

    // Set the head position to the middle
    head.x = head.y = last_head.x = last_head.y = static_cast<float>(box_amount / 2);

    // Reset the position states
    std::memset(&positions.at(0, 0), static_cast<int32_t>(POSITION_STATE::POSITION_STATE_NOTHING), positions.size());

    // Set the heads' snake position state
    positions.at(static_cast<uint64_t>(head.x), static_cast<uint64_t>(head.y)) = POSITION_STATE::POSITION_STATE_SNAKE;

    // We didn't die yet
    dead = false;

    // Didn't hit false
    hit_pause = false;

    // And finally, reset our fps manager
    fpsmanager.reset();

    // No foods anymore
    foods.clear();

    // Didn't move yet
    move_counter = move_eat_counter = 0;

    // We just started
    just_started = true;
}

/*
@brief

    Constructor, loads settings among other things
*/
retrogames::games::snake_t::snake_t(settings_t* settings, const std::string& name, ImFont** default_font_small, ImFont** default_font_mid, ImFont** default_font_big, const std::string& version/* = "1.0"*/, uint8_t* icon/* = nullptr*/) :
    game_base_t(game_information_t::create(name, version, icon), settings, default_font_small, default_font_mid, default_font_big),
    eaten(false),
    just_started(true),
    death_state(DEATH_STATE::DEATH_STATE_MAIN),
    should_exit(false),
    hit_pause(false),
    move_counter(0),
    move_eat_counter(0),
    resolution_area(settings->get_main_settings().resolution_area),
    setting_timeout(settings->create("snake_timeout", 3u)),
    setting_field_size(settings->create("snake_field_size", 10u)),
    setting_speed(settings->create("snake_speed", 10u)),
    snake_fps(static_cast<uint8_t>(setting_speed.get<uint32_t>())),
    fpsmanager(snake_fps),
    box_amount(setting_field_size.get<uint32_t>() * 2),
    positions(setting_field_size.get<uint32_t>() * 2, setting_field_size.get<uint32_t>() * 2)
{
    resolution = static_cast<uint16_t>(resolution_area.height);
    box_size = static_cast<float>(resolution) / (static_cast<float>(setting_field_size.get<uint32_t>() * 2));

    // Set the head position to the middle
    head.x = head.y = last_head.x = last_head.y = static_cast<float>(box_amount / 2);

    // Set the heads' snake position state
    positions.at(static_cast<uint64_t>(head.x), static_cast<uint64_t>(head.y)) = POSITION_STATE::POSITION_STATE_SNAKE;

    // We didn't die yet
    dead = false;

    // Reset the direction (default)
    force_direction = DIRECTION::SNAKE_DIRECTION_DEFAULT;

    // Set the ImGui style we want
    auto& style = ImGui::GetStyle();

    style.Colors[ImGuiCol_Button] = { 0.f, 0.f, 0.f, 0.f };
    style.Colors[ImGuiCol_ButtonActive] = { .8f, .8f, .8f, 1.f };
    style.Colors[ImGuiCol_ButtonHovered] = { .2f, .2f, .2f, 1.f };
}

/*
@brief

    Destructor
*/
retrogames::games::snake_t::~snake_t()
{

}

/*
@brief

    Needs to be called when the snake eats
*/
void retrogames::games::snake_t::eat(void)
{
    // We just ate (@move() will make use of this)
    eaten = true;

    // Also set this in order to smooth out the animation of the tail growing
    move_eat_counter = move_counter;

    // Remove the food from our list
    if (foods.empty()) return;

	// Create a new element
    generate_food();

	// Find the entry
    auto it = std::find_if(foods.begin(), foods.end(), [this](const food_type& tuple) -> bool
    {
        return std::get<0>(tuple) == head.x && std::get<1>(tuple) == head.y;
    });

    // Check if we found it
    if (it == foods.end()) return;

	// Add it to the cached food list (to smooth out the animation of eating the food)
    cached_foods.push_back(std::make_tuple(std::get<0>(*it), std::get<1>(*it), move_counter));

    // Remove the element
    foods.erase(it);
}

/*
@brief

    Handles game logic, moves the snake if need be
*/
retrogames::games::snake_t::DIRECTION retrogames::games::snake_t::think(void)
{
    // Check for timeout
    if (is_in_timeout()) return DIRECTION::SNAKE_DIRECTION_NONE;

    // Check if we want to move our snake. We don't want to do that every frame
    // (60 moves a second would be too fast)
    // Also check if we're dead, the game is paused or we're in the start timeout
    if (!fpsmanager.should_run() || dead || is_paused()) return DIRECTION::SNAKE_DIRECTION_NONE;

    // If we just started, generate food
    if (just_started)
    {
        generate_food();

        just_started = false;
    }

    // Determine the direction
    auto direction = (force_direction == DIRECTION::SNAKE_DIRECTION_NONE) ? direction_stack.at(0) : force_direction;

    // Remove the direction from our stack if need be
    if (force_direction == DIRECTION::SNAKE_DIRECTION_NONE)
    {
        // We have a stack, remove an element
        direction_stack.erase(direction_stack.begin());

        // If the direction stack is empty, set the force direction to
        // the newest element in the stack.
        if (direction_stack.empty()) force_direction = direction;
    }

    // Move the snake
    if (move(direction))
    {
        // We died. Shame.
        kill();

        // Remove the last element from our last_position array (would extend our tail otherwise on death by one)
        if (!last_position_history.empty()) last_position_history.erase(last_position_history.begin());
    }

    return direction;
}

/*
@brief

    Moves our snake. Returns false if we didn't hit something (we died),
    and true if we did. Uses the @dir argument to move in it's direction.
*/
bool retrogames::games::snake_t::move(const DIRECTION dir)
{
    // Add the current position to the position history
    position_history.push_back(std::make_pair(static_cast<uint16_t>(head.x), static_cast<uint16_t>(head.y)));

    // Add the last position to the position history
    last_position_history.push_back(std::make_pair(static_cast<uint16_t>(last_head.x), static_cast<uint16_t>(last_head.y)));

    // Helper function to check if the current head + offset would mean that we die
    static auto check_death = [this](ImVec2 offset)
    {
        // Calculate the new position
        auto new_pos = ImVec2(head.x + offset.x, head.y + offset.y);

        // Check if we're now out of bounds
        if (new_pos.x < 0.f || new_pos.y < 0.f ||
            new_pos.x >= static_cast<float>(box_amount) || new_pos.y >= static_cast<float>(box_amount)) return true;

        // Check if we've hit our own snake
        if (positions.at(static_cast<uint64_t>(new_pos.x), static_cast<uint64_t>(new_pos.y)) == POSITION_STATE::POSITION_STATE_SNAKE) return true;

        // All good!
        return false;
    };

    /*
    Enum implemented like this (after NONE):

        SNAKE_DIRECTION_UP,
        SNAKE_DIRECTION_DOWN,
        SNAKE_DIRECTION_LEFT,
        SNAKE_DIRECTION_RIGHT

        So, to get an offset, we can just do:
    */
    static std::array<ImVec2, 4> direction_offsets = {

        ImVec2(0.f, -1.f),
        ImVec2(0.f, 1.f),
        ImVec2(-1.f, 0.f),
        ImVec2(1.f, 0.f)

    };

    // Now, we can grab the offset with the current direction
    auto offset = direction_offsets[static_cast<uint8_t>(dir) - 1];

    // Check if we would die by moving by our offset
    if (check_death(offset)) return true;

    // Save the last head position
    last_head = head;

    // Add the offset to the head
    head.x += offset.x;
    head.y += offset.y;

    // We've moved!
    move_counter++;

    // Check if we hit a food block
    auto& pos = positions.at(static_cast<uint16_t>(head.x), static_cast<uint16_t>(head.y));

    if (pos == POSITION_STATE::POSITION_STATE_FOOD) eat();

    // Snake didn't die, remove the last position
    {
        const auto& last_pos = position_history[0];

        if (!eaten)
        {
            // We haven't eaten, remove one block from our tail
            positions.at(last_pos.first, last_pos.second) = POSITION_STATE::POSITION_STATE_NOTHING;

            // Remove the last element from the history since it's now invalid
            position_history.erase(position_history.begin());
            last_position_history.erase(last_position_history.begin());
        }
        else
        {
            // We've eaten, don't remove a block from our tail (tail will grow by one block)
            eaten = false;
        }
    }

    // Set the new position to be part of the snake
    pos = POSITION_STATE::POSITION_STATE_SNAKE;

    return false;
}

/*
@brief

    Draws the left window (playing field is square, so we have left and right windows
    since our resolution is 16:9)
*/
void retrogames::games::snake_t::draw_left_window(void)
{
    if (static_vars.last_death != dead)
    {
        static_vars.last_death = dead;

        if (dead) time_survived = get_playtime();
    }

    if (!dead && !is_paused() && !is_in_timeout())
    {
        time_survived = get_playtime();
    }

    // Display the score
    ImGui::Text("Score: %i", static_cast<int32_t>(position_history.size()));
}

/*
@brief

    Draws the right window (playing field is square, so we have left and right windows
    since our resolution is 16:9)
*/
void retrogames::games::snake_t::draw_right_window(void)
{
    
}

/*
@brief

    Constructor (from modal_popup_wrapper_t)
*/
retrogames::games::snake_t::modal_popup_t::modal_popup_t(const char* name, bool darkening/* = false*/) : started_popup(false), darkening(false)
{
    // Create a dummy window for the ImGui context (needed when we switch between windowed and fullscreen)
    // don't ask me why...
    if (!(started_window = ImGui::Begin((std::string("##no") + std::to_string(current_modal_popup_id++)).c_str(), nullptr, ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove))) return;

    if (!(this->darkening = darkening))
    {
        ImGui::PushStyleColor(ImGuiCol_ModalWindowDarkening, { 0.f, 0.f, 0.f, 0.f });
        ImGui::PushStyleColor(ImGuiCol_ModalWindowDimBg, { 0.f, 0.f, 0.f, 0.f });
    }

    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.0f, 0.0f });
    ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, 0.f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
    ImGui::OpenPopup(name);

    started_popup = ImGui::BeginPopupModal(name, nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar);
}

/*
@brief

    Destructor (from modal_popup_wrapper_t)
*/
retrogames::games::snake_t::modal_popup_t::~modal_popup_t()
{
    current_modal_popup_id--;

    if (!started_window) return;
    if (started_popup) ImGui::EndPopup();
    if (!darkening) ImGui::PopStyleColor(2);

    ImGui::PopStyleVar(4);
    ImGui::End();
}

/*
@brief

    Draws the death menu
*/
void retrogames::games::snake_t::draw_death_menu(void)
{
    auto& colors = ImGui::GetStyle().Colors;

    modal_popup("##deathmenu")
    {
        if (death_state == DEATH_STATE::DEATH_STATE_MAIN)
        {
            static auto button_size = ImVec2(100.f * 3.f, 0.f);

            ImGui::Text("You died.");
            ImGui::Text("Time alive:");
            ImGui::Text("%02i:%02i:%02i:%03i", time_survived.hours, time_survived.minutes, time_survived.seconds, time_survived.milliseconds);

            if (ImGui::Button("Retry", button_size))
            {
                // Reset all variables in case we've already played
                do_reset();

                // Time out again
                start_timeout();

                // Reset the playtime
                reset_playtime();

                ImGui::CloseCurrentPopup();
            }

            if (ImGui::Button("Back to main menu", button_size))
            {
                death_state = DEATH_STATE::DEATH_STATE_CONFIRM_CLOSE;

                ImGui::CloseCurrentPopup();
            }
        }
        else
        {
            ImGui::Text("Are you sure?");

            if (ImGui::Button("Yes", ImVec2(65.f, 0.)))
            {
                // Close the game
                should_exit = true;
            }

            ImGui::SameLine();

            if (ImGui::Button("No", ImVec2(65.f, 0.))) death_state = DEATH_STATE::DEATH_STATE_MAIN;
        }
    }
}

/*
@brief

    Draws the options menu
*/
void retrogames::games::snake_t::draw_options(float scaling)
{
    ImGuiUser::inputslider_uint32_t(&setting_timeout, "Timeout (in seconds)", 10u, 0u, "How many seconds the game is in timeout when starting or tabbing back into the game.", scaling);
    ImGuiUser::inputslider_uint32_t(&setting_field_size, "Resolution (X2)", 25u, 3u, "How many boxes are in one axis * 2 (x, y) * 2 = games' field resolution.", scaling);
    ImGuiUser::inputslider_uint32_t(&setting_speed, "Speed", 60u, 1u, "How many times the snake moves from one box to another in a single second.", scaling);
}

/*
@brief

    Generates food at random coordinates (where there aren't any yet)
*/
void retrogames::games::snake_t::generate_food(void)
{
    auto random_number = [](const int32_t& min, const int32_t& max) -> int32_t
    {
        static std::random_device rd; // obtain a random number from hardware
        static std::mt19937 eng(rd()); // seed the generator

        std::uniform_int_distribution<> distr(min, max); // define the range

        return distr(eng);
    };

    //for (uint64_t tries = 0;; tries++)
    while (true)
    {
        auto x = random_number(0, box_amount - 1);
        auto y = random_number(0, box_amount - 1);

        auto& pos = positions.at(static_cast<uint64_t>(x), static_cast<uint64_t>(y));

        if (pos != POSITION_STATE::POSITION_STATE_NOTHING) continue;

        pos = POSITION_STATE::POSITION_STATE_FOOD;

        foods.push_back(std::make_tuple(static_cast<uint16_t>(x), static_cast<uint16_t>(y), timer_t(true)));

        break;
    }
}

/*
@brief

    Resets information (when we re-start the game)
*/
void retrogames::games::snake_t::reset(settings_t* settings, bool create_fonts)
{
    // Reset everything that has to do with video settings
    resolution_area = settings->get_main_settings().resolution_area;
    resolution = static_cast<uint16_t>(resolution_area.height);
    box_size = static_cast<float>(resolution) / (static_cast<float>(setting_field_size.get<uint32_t>() * 2));
    snake_fps = static_cast<uint8_t>(setting_speed.get<uint32_t>());
    fpsmanager = fpsmanager_t(snake_fps);
    box_amount = setting_field_size.get<uint32_t>() * 2;
    positions = unique_ptr_array_matrix_t<POSITION_STATE>(setting_field_size.get<uint32_t>() * 2, setting_field_size.get<uint32_t>() * 2);

    // Misc
    death_state = DEATH_STATE::DEATH_STATE_MAIN;
    should_exit = false;
    hit_pause = false;
    move_counter = 0;
    move_eat_counter = 0;

    // Now do a full reset of everything else
    do_reset();

    static_vars.reset();
}

/*
@brief

    Draws controls
*/
void retrogames::games::snake_t::draw_controls(float scaling)
{
    ImGui::TextWrapped("Controls for snake");
    ImGui::Separator();

#ifndef PLATFORM_NS
    ImGui::BulletText("WASD/Arrow keys - Move");
    ImGui::BulletText("Escape - Pause");
#else
    ImGui::BulletText("DPAD/Arrows - Move");
    ImGui::BulletText("Plus - Pause");
#endif
}

/*
@brief

    Draws some information
*/
void retrogames::games::snake_t::draw_information(float scaling)
{
    ImGui::TextWrapped("Snake (taken from wikipedia don't judge me)");
    ImGui::Separator();
    ImGui::TextWrapped("Snake is the common name for a video game concept where the player maneuvers a line which grows in length, with the line itself being a primary obstacle. The concept originated in the 1976 arcade game Blockade, and the ease of implementing Snake has led to hundreds of versions (some of which have the word snake or worm in the title) for many platforms. After a variant was preloaded on Nokia mobile phones in 1998, there was a resurgence of interest in the snake concept as it found a larger audience. There are over 300 Snake-like games for iOS alone.");
    /*ImGuiUser::frame_height_spacing();
    ImGui::TextWrapped("Gameplay");
    ImGui::Separator();
    ImGui::TextWrapped("The player controls a dot, square, or object on a bordered plane. As it moves forward, it leaves a trail behind, resembling a moving snake. In some games, the end of the trail is in a fixed position, so the snake continually gets longer as it moves. In another common scheme, the snake has a specific length, so there is a moving tail a fixed number of units away from the head. The player loses when the snake runs into the screen border, a trail or other obstacle, or itself.");*/
}