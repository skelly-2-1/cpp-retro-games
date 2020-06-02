/*
@file

	pingpong.cpp

@purpose

	Ping pong game and GUI functionality
*/

#include "pingpong.h"
#include "imgui/imgui_user.h"
#include "misc/macros.h"

namespace retrogames
{

    namespace detail
    {

        // turns a difficulty string to an index (DIFFICULTY)
        auto difficulty_string_to_index = [](std::string difficulty_name) -> retrogames::games::pingpong_t::DIFFICULTY
        {
            std::transform(difficulty_name.begin(), difficulty_name.end(), difficulty_name.end(), ::tolower);

            for (uint8_t index = 0; index < static_cast<uint8_t>(retrogames::games::pingpong_t::DIFFICULTY::DIFFICULTY_SIZE); index++)
            {
                std::string name = retrogames::games::pingpong_t::get_difficulty_name(static_cast<retrogames::games::pingpong_t::DIFFICULTY>(index));
                std::transform(name.begin(), name.end(), name.end(), ::tolower);

                if (difficulty_name.compare(name) == 0) return static_cast<retrogames::games::pingpong_t::DIFFICULTY>(index);
            }

            // default difficulty in case the string wasn't found
            return retrogames::games::pingpong_t::DIFFICULTY::DIFFICULTY_DEFAULT;
        };

    }

}

/*
@brief

    Constructor
*/
retrogames::games::pingpong_t::pingpong_t(settings_t* settings, const std::string& name, ImFont** default_font_small, ImFont** default_font_mid, ImFont** default_font_big, const std::string& version, uint8_t* icon) :
    game_base_t(game_information_t::create(name, version, icon), settings, default_font_small, default_font_mid, default_font_big),
    cfgvalue_ping_scale_x(settings->create("pingpong_ping_scale_x", 1.f)),
    cfgvalue_ping_scale_y(settings->create("pingpong_ping_scale_y", 1.f)),
    cfgvalue_initial_paddle_speed(settings->create("pingpong_initial_paddle_speed", 1000.f)),
    cfgvalue_initial_ball_speed(settings->create("pingpong_initial_ball_speed", 800.f)),
    cfgvalue_ball_scale(settings->create("pingpong_ball_scale", 1.f)),
    cfgvalue_cpu_difficulty(settings->create("pingpong_cpu_difficulty", get_difficulty_name(DIFFICULTY::DIFFICULTY_DEFAULT))),
    cfgvalue_max_score(settings->create("pingpong_max_score", 7u)),
    settings(settings),
    left_paddle(nullptr),
    right_paddle(nullptr),
    time_scale(0.1),
    main_font(nullptr),
    difficultymanager(nullptr)
{
    reset(settings, true);
}

/*
@brief

    Creates our paddles
*/
void retrogames::games::pingpong_t::create_paddles(void)
{
    auto x_offset = static_cast<double>(settings->get_main_settings().resolution_area.width) / 20.;
    auto size_x = x_offset / 5.;
    auto size_y = static_cast<double>(settings->get_main_settings().resolution_area.height) * .1;

    size_y = std::floor(size_y * static_cast<double>(cfgvalue_ping_scale_y.get<float>()));
    
    auto ping_scale_x = static_cast<double>(cfgvalue_ping_scale_x.get<float>());
    auto x_scale_offset = x_offset - (x_offset * ping_scale_x);

    size_x = std::floor(size_x * ping_scale_x);
    x_offset = std::floor(x_offset - x_scale_offset);

    left_paddle.reset(new paddle_t(static_cast<uint32_t>(x_offset), area_size_t(static_cast<uint32_t>(std::floor(size_x)), static_cast<uint32_t>(std::floor(size_y))), initial_paddle_speed, settings->get_main_settings().resolution_area, true));
    right_paddle.reset(new paddle_t(static_cast<uint32_t>(x_offset), area_size_t(static_cast<uint32_t>(std::floor(size_x)), static_cast<uint32_t>(std::floor(size_y))), initial_paddle_speed, settings->get_main_settings().resolution_area, false));
}

/*
@brief

    Creates our ball
*/
void retrogames::games::pingpong_t::create_ball(void)
{
    auto ball_size = 20. * (static_cast<double>(settings->get_main_settings().resolution_area.height) / 1080.);

    ball_size = std::floor(ball_size * ball_scale);
    ball.reset(new ball_t(initial_ball_speed, static_cast<uint32_t>(ball_size), settings->get_main_settings().resolution_area));
}

/*
@brief

    Called from our renderer thread when we need to draw
*/
bool retrogames::games::pingpong_t::draw(bool render)
{
    if (!render) return false;

    // set the time scale
    time_scale = ImGui::GetIO().DeltaTime * (static_cast<float>(resolution_area.width) / 1280.f);

    // function to move our ball
    // returns true if one side
    // scored a point
    auto move_ball = [&](bool calculate_only = false) -> bool
    {
        // move the ball
        bool paddle_hit = false;

        ball->x += ball->speed_x * time_scale;
        ball->y += ball->speed_y * time_scale;

        // clamp the X value in case something goes horribly wrong
        ball->x = std::max(static_cast<double>(ball->size) / 2, ball->x);
        ball->x = std::min((static_cast<double>(resolution_area.width) - static_cast<double>(ball->size / 2) - 1.), ball->x);

        if (ball->y < 0)
        {
            // if the ball hits the ceiling, send it back down
            ball->y = 0.;
            ball->speed_y *= -1.;
        }
        else if (ball->y > resolution_area.height - ball->size / 2)
        {
            // if the ball hits the bottom, send it back up
            ball->y = static_cast<double>((resolution_area.height - 1) - ball->size / 2);
            ball->speed_y *= -1.;
        }

        // check if the ball hits a paddle
        if (calculate_only)
        {
            // if we only want to calculate, return true if the X position passes the cpu paddle's X
            if ((/*right_paddle->is_cpu && */ball->speed_x > 0. && ball->x >= right_paddle->x) ||
                (/*left_paddle->is_cpu && */ball->speed_x < 0. && ball->x <= left_paddle->x + static_cast<double>(left_paddle->size.width))) return true;
        }
        else
        {
            // check if the new position hits a paddle
            if ((ball->speed_x > 0. && right_paddle->intersect(ball.get())) || (ball->speed_x < 0. && left_paddle->intersect(ball.get())))
            {
                paddle_hit = true;

                // Revert the x direction
                ball->speed_x *= -1.;

                // Add a spin to it if our paddle is moving
                auto paddle = (ball->speed_x < 0.) ? right_paddle.get() : left_paddle.get();

                if (paddle->direction != paddle_t::DIRECTION::DIRECTION_NONE)
                {
                    // multiply the paddle speed with the distance between the ball hitting and the center of the paddle
                    auto ball_y = ball->y;

                    ball_y = std::max(ball_y, paddle->y);
                    ball_y = std::min(ball_y, paddle->y + static_cast<double>(paddle->size.height));

                    // (?) Do we want abs here?
                    auto mult = 1. + std::abs((ball_y - (paddle->y + static_cast<double>(paddle->size.height) * .5)) / (static_cast<double>(paddle->size.height) * .5));

                    ball->speed_y = paddle->speed * mult;
                }
            }
        }

        // check if the ball leaves the playing field (one side lost)
        if (calculate_only) return false;

        if (!paddle_hit)
        {
            if (ball->x <= static_cast<double>(std::ceil(ball->size / 2)) + 1.)
            {
                // hit left side
                right_paddle->reset(resolution_area, ++right_paddle->points);
                left_paddle->reset(resolution_area, left_paddle->points);
                ball->reset(resolution_area, right_paddle->points+left_paddle->points);

                // start timeout
                start_timeout();
            }
            else if (ball->x + static_cast<double>(std::ceil(ball->size / 2)) >= static_cast<double>(resolution_area.width) - 1.)
            {
                // hit right side
                left_paddle->reset(resolution_area, ++left_paddle->points);
                right_paddle->reset(resolution_area, right_paddle->points);
                ball->reset(resolution_area, right_paddle->points+left_paddle->points);

                // start timeout
                start_timeout();
            }
        }

        return paddle_hit;
    };

    // function to move a paddle
    static auto move_paddle = [&](paddle_t* paddle, double min_position_multiplier, double end_multiplier, bool target_calculated = false)
    {
        paddle->moving_to_calculated_position = target_calculated;

        // check if we want to move it
        if ((paddle->direction == paddle_t::DIRECTION::DIRECTION_NONE && !target_calculated) || (target_calculated && paddle->calculated_position_set)) return;

        // our base paddle speed
        auto& base_paddle_speed = paddle->base_speed_scaled;

        // set our paddle speed
        if (!target_calculated)
        {
            // normal
            paddle->speed = (paddle->direction == paddle_t::DIRECTION::DIRECTION_DOWN) ? base_paddle_speed : -base_paddle_speed;
        }
        else
        {
            // cpu (calculated)
            auto center_y = static_cast<uint32_t>(paddle->y) + paddle->size.height / 2;

            paddle->speed = (center_y > static_cast<uint32_t>(paddle->calculated_y)) ? -base_paddle_speed : base_paddle_speed;
            paddle->direction = (center_y > static_cast<uint32_t>(paddle->calculated_y)) ? paddle_t::DIRECTION::DIRECTION_UP : paddle_t::DIRECTION::DIRECTION_DOWN;
        }

        if (min_position_multiplier < 1.0)
        {
            auto calc = (!target_calculated) ? ball->y : static_cast<double>(paddle->calculated_y);
            auto pos = paddle->y + static_cast<double>(paddle->size.height / 2);
            auto div = (pos <= calc) ? pos / calc : calc / pos;

            paddle->speed *= (min_position_multiplier + ((1. - div) * (1. - min_position_multiplier)) * end_multiplier);
        }

        // calculate new paddle coordinates
        auto new_y = paddle->y + paddle->speed * time_scale;

        // don't let our paddle get outside of our screen
        new_y = std::max(new_y, 0.);
        new_y = std::min(new_y, static_cast<double>((resolution_area.height - 1) - paddle->size.height));

        // now, set the paddle position (height)
        if (target_calculated)
        {
            // prevent cpu paddle from flickering up and down
            auto old_center_y = static_cast<uint32_t>(paddle->y) + paddle->size.height / 2;
            auto new_center_y = static_cast<uint32_t>(new_y) + paddle->size.height / 2;

            if ((old_center_y <= static_cast<uint32_t>(paddle->calculated_y_clamped) && new_center_y >= static_cast<uint32_t>(paddle->calculated_y_clamped)) || (old_center_y >= static_cast<uint32_t>(paddle->calculated_y_clamped) && new_center_y <= static_cast<uint32_t>(paddle->calculated_y_clamped)))
            {
                new_y = static_cast<uint32_t>(paddle->calculated_y_clamped) - paddle->size.height / 2;

                // don't let our paddle get outside of our screen
                new_y = std::max(new_y, 0.);
                new_y = std::min(new_y, static_cast<double>((resolution_area.height - 1) - paddle->size.height));

                // reset the direction since it's set
                paddle->direction = paddle_t::DIRECTION::DIRECTION_NONE;

                // tell the cpu that we've hit the proper position
                paddle->calculated_position_set = true;
            }
        }

        // apply the new position
        paddle->y = new_y;
    };

    // Sets the direction for a cpu paddle
    static auto set_cpu_paddle_direction = [&](paddle_t* paddle, bool move_to_calculated_position)
    {
        if (!paddle->is_cpu) return;

        paddle->direction = paddle_t::DIRECTION::DIRECTION_NONE;

        if (paddle->calculated_position_set) return;

        auto current_ball_y = move_to_calculated_position ? static_cast<double>(paddle->calculated_y) : ball->y;

        if ((paddle->left && ball->speed_x > 0.) || (!paddle->left && ball->speed_x < 0.)) return;

        auto center = paddle->y + static_cast<double>(paddle->size.height / 2);
        auto clipped_screen =   (static_cast<int32_t>(paddle->y) == 0 && static_cast<int32_t>(current_ball_y) <= paddle->size.height) ||
                                (static_cast<int32_t>(paddle->y) >= static_cast<int32_t>((resolution_area.height - 1) - paddle->size.height) && static_cast<int32_t>(current_ball_y) >= static_cast<int32_t>(paddle->y));

        if (!clipped_screen && std::abs(center - current_ball_y) > 5.) paddle->direction = (center > current_ball_y) ? paddle_t::DIRECTION::DIRECTION_UP : paddle_t::DIRECTION::DIRECTION_DOWN;
    };

    // Sets the direction for a player paddle
    static auto set_player_paddle_direction = [&](paddle_t* paddle, control_keys_e up_key, control_keys_e down_key)
    {
        auto up_pressed = control_keys.is_pressed(up_key);
        auto down_pressed = control_keys.is_pressed(down_key);

        if ((up_pressed || down_pressed) && paddle->is_cpu) paddle->is_cpu = false;

        if (!up_pressed && down_pressed)
        {
            paddle->direction = paddle_t::DIRECTION::DIRECTION_DOWN;
        }
        else if (!down_pressed && up_pressed)
        {
            paddle->direction = paddle_t::DIRECTION::DIRECTION_UP;
        }
        else if (down_pressed && up_pressed)
        {
            paddle->direction = (control_keys.down_timer[static_cast<uint8_t>(down_key)].get_elapsed() < control_keys.down_timer[static_cast<uint8_t>(up_key)].get_elapsed()) ? paddle_t::DIRECTION::DIRECTION_DOWN : paddle_t::DIRECTION::DIRECTION_UP;
        }
        else
        {
            paddle->direction = paddle_t::DIRECTION::DIRECTION_NONE;
        }
    };

    // Sets the direction of a paddle (handles both - cpus and players)
    static auto set_paddle_direction = [&](paddle_t* paddle, bool move_to_calculated_position) -> void
    {
        set_player_paddle_direction(paddle, paddle->left ? control_keys_e::KEY_W : control_keys_e::KEY_UPARROW, paddle->left ? control_keys_e::KEY_S : control_keys_e::KEY_DOWNARROW);
        set_cpu_paddle_direction(paddle, move_to_calculated_position);
    };

    // only handle game logic if we're not paused/in timeout
    // and if we don't have a winner yet
    if (can_continue() && winner_paddle == nullptr)
    {
        // store the old ball position
        ball->old_x = ball->x;
        ball->old_y = ball->y;

        // move the left paddle
        bool    left_paddle_move_to_calculated_position =
                left_paddle->is_cpu &&
                left_paddle->calculated_y != -1 &&
                !left_paddle->calculated_position_set &&
                difficultymanager->get_current_difficulty()->should_paddle_go_to_calculated_position(resolution_area.width, ball->x, true);

        set_paddle_direction(left_paddle.get(), left_paddle_move_to_calculated_position);

        move_paddle(left_paddle.get(), .5, 1., left_paddle_move_to_calculated_position);

        // move the right paddle
        bool    right_paddle_move_to_calculated_position =
                right_paddle->is_cpu &&
                right_paddle->calculated_y != -1 &&
                !right_paddle->calculated_position_set &&
                difficultymanager->get_current_difficulty()->should_paddle_go_to_calculated_position(resolution_area.width, ball->x, false);

        set_paddle_direction(right_paddle.get(), right_paddle_move_to_calculated_position);

        move_paddle(right_paddle.get(), .5, 1., right_paddle_move_to_calculated_position);

        // move the ball
        auto old_left_points = left_paddle->points;
        auto old_right_points = right_paddle->points;
        auto paddle_hit = move_ball();

        // check if we need to calculate the position where the ball will land on the other side
        if (paddle_hit)
        {
            auto target_paddle = ball->speed_x > 0. ? right_paddle.get() : left_paddle.get();

            if (target_paddle->is_cpu)
            {
                auto current_difficulty = difficultymanager->get_current_difficulty();

                // only calculate if needed
                if (current_difficulty->calculated_pos_moving_chance > 0.)
                {
                    auto old_calculated_y = target_paddle->calculated_y;
                    auto old_clamped_calculated_y = target_paddle->calculated_y_clamped;
                    auto old_ball_x = ball->x;
                    auto old_ball_y = ball->y;
                    auto old_ball_speed_x = ball->speed_x;
                    auto old_ball_speed_y = ball->speed_y;
                    auto last_ball_y = ball->y;

                    // move the ball until it hits the other side
                    while (true)
                    {
                        if (move_ball(true))
                        {
                            // average it out
                            target_paddle->calculated_y = (int)((ball->y + last_ball_y) / 2);

                            break;
                        }

                        last_ball_y = ball->y;
                    }

                    ball->x = old_ball_x;
                    ball->y = old_ball_y;
                    ball->speed_x = old_ball_speed_x;
                    ball->speed_y = old_ball_speed_y;

                    if (target_paddle->calculated_y != old_calculated_y)
                    {
                        // clamp the new calculated y
                        auto size = std::ceil(static_cast<double>(target_paddle->size.height) / 2);

                        target_paddle->calculated_y_clamped = std::max(target_paddle->calculated_y, static_cast<int32_t>(size));
                        target_paddle->calculated_y_clamped = std::min(target_paddle->calculated_y_clamped, (static_cast<int32_t>(resolution_area.height) - static_cast<int32_t>(size)) - 1);

                        if (target_paddle->calculated_y_clamped != old_clamped_calculated_y)
                        {
                            // since the position changed, we're no longer in the right position
                            target_paddle->calculated_position_set = false;
                        }
                    }

                    // generate new random numbers for the cpu difficulty
                    current_difficulty->generate_numbers();

                    // set the paddle speed
                    target_paddle->speed = target_paddle->base_speed_scaled * current_difficulty->current_paddle_speed_multiplier;
                }
            }

            // play ding sound
            play_sound_effect(snd_t::sounds_e::SOUND_DING);
        }
        else
        {
            // check if someone won
            auto is_left = (old_left_points < left_paddle->points);

            if (is_left || old_right_points < right_paddle->points)
            {
                auto scored_paddle = is_left ? left_paddle.get() : right_paddle.get();
                auto max_score = cfgvalue_max_score.get<uint32_t>();

                if (max_score > 0u && scored_paddle->points >= max_score)  winner_paddle = scored_paddle;
            }
        }
    }

    // debugging
    /*auto draw_calculated_position = [&](paddle_t* paddle)
    {
        if (paddle->calculated_y == -1) return;

        constexpr auto size = 12u;

        ImGui::GetBackgroundDrawList()->AddRectFilled(
            ImVec2{static_cast<float>((paddle->left ? paddle->x : paddle->x + static_cast<double>(paddle->size.width))-static_cast<double>(size/2)), static_cast<float>((paddle->calculated_y-static_cast<double>(size/2)))},
            ImVec2{static_cast<float>((paddle->left ? paddle->x : paddle->x + static_cast<double>(paddle->size.width))+static_cast<double>(size/2)), static_cast<float>((paddle->calculated_y+static_cast<double>(size/2)))},
            ImGuiUser::color_to_imgui_color_u32(color_t(255, 0, 0))
        );
    };

    draw_calculated_position(left_paddle.get());
    draw_calculated_position(right_paddle.get());*/

    // Now moved to base functionality
    /*// draw the playtime
    static auto draw_playtime = [&](void)
    {
        static char playtime_buffer[80];

        auto playtime = get_playtime();

        sprintf(playtime_buffer, "%02i:%02i:%02i:%03i", playtime.hours, playtime.minutes, playtime.seconds, playtime.milliseconds);

        auto target_y = (resolution_area.height - resolution_area.height / 20u) - static_cast<uint32_t>(std::floor(ImGui::GetFontSize() * .5f));
        auto target_x = resolution_area.width / 2u;

        target_x -= static_cast<uint32_t>(std::floor(ImGui::CalcTextSize(playtime_buffer).x * .5f));

        ImGui::GetBackgroundDrawList()->AddText(ImVec2{static_cast<float>(target_x), static_cast<float>(target_y)}, ImGuiUser::color_to_imgui_color_u32(color_t(200, 200, 200)), playtime_buffer);
    };

    draw_playtime();*/

    // draw the scores
    static auto draw_score = [&](paddle_t* paddle)
    {
        auto score = std::to_string(paddle->points);
        auto middle_x = resolution_area.width / 2u;
        auto offset_x = static_cast<uint32_t>((static_cast<float>(resolution_area.width) / 20.f) * UI_SCALE);
        auto target_x = middle_x + (paddle->left ? -offset_x : offset_x);
        auto target_y = static_cast<uint32_t>((static_cast<float>(resolution_area.height) / 15.f) * UI_SCALE);

        target_y -= static_cast<uint32_t>(std::floor(ImGui::GetFontSize() * .5f));
        target_x -= static_cast<uint32_t>(std::floor(ImGui::CalcTextSize(score.c_str()).x * .5f));

        ImGui::GetBackgroundDrawList()->AddText(ImVec2{static_cast<float>(target_x), static_cast<float>(target_y)}, ImGuiUser::color_to_imgui_color_u32(color_t(200, 200, 200)), score.c_str());
    };

    ImGui::PushFont(main_font);

    draw_score(left_paddle.get());
    draw_score(right_paddle.get());

    ImGui::PopFont();

    // draw the middle line
    static auto draw_dashline = [](float x0, float y0, float x1, float y1, float* spacing, int spacing_size, int line_width, color_t color) 
    { 
        auto lerp = [](float a, float b, float f)
        {
            return (a * (1.f - f)) + (b * f);
        };

        auto dist = [](int x1, int y1, int x2, int y2)
        {
            return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2) * 1.f);
        };

        auto distance = dist(x0, y0, x1, y1);
        auto x_spacing = new float[spacing_size];
        auto y_spacing = new float[spacing_size];

        float drawn = 0.f;
        
        if (distance > 0)
        { 
            int i;

            auto draw_line = true;

            for (i = 0; i < spacing_size; i++) 
            { 
                x_spacing[i] = lerp(0.f, (x1 - x0), spacing[i] / distance); 
                y_spacing[i] = lerp(0.f, (y1 - y0), spacing[i] / distance); 
            } 
        
            i = 0; 

            while (drawn < distance) 
            { 
                //if (draw_line) line(x0, y0, x0 + x_spacing[i], y0 + y_spacing[i]);
                if (draw_line) ImGui::GetBackgroundDrawList()->AddLine({x0, y0}, {x0+x_spacing[i],y0+y_spacing[i]}, ImGuiUser::color_to_imgui_color_u32(color), line_width);

                x0 += x_spacing[i];
                y0 += y_spacing[i];

                drawn = drawn + std::sqrt(x_spacing[i] * x_spacing[i] + y_spacing[i] * y_spacing[i]);

                i = (i + 1) % spacing_size;  // cycle through array

                draw_line = !draw_line;  // switch between dash and gap
            }
        }
    };
 
    auto line_size = static_cast<float>(resolution_area.height) / 15.f;

    float pattern[] = {line_size,line_size};

    auto dash_line_start = ImVec2{static_cast<float>(resolution_area.width / 2), 0.f};
    auto dash_line_end = ImVec2{static_cast<float>(resolution_area.width / 2), static_cast<float>(resolution_area.height)};

    draw_dashline(dash_line_start.x, dash_line_start.y, dash_line_end.x, dash_line_end.y, pattern, 2, static_cast<int32_t>(static_cast<float>(resolution_area.width) / 200.f), color_t(200, 200, 200));

    // draw the ball and paddles
    ball->draw();
    left_paddle->draw();
    right_paddle->draw();

    // draw the modal for winning
    if (winner_paddle != nullptr)
    {
        unpause(); // unpause if we're paused
        dont_draw_pause_menu(); // prevent pause menu from drawing

        IMGUI_MODAL_POPUP(winner, true) // true = with darkening
        {
            if (!confirm_exit_game)
            {
                auto button_size = ImVec2{ImGui::CalcTextSize("Back to main menu").x+ImGui::GetStyle().FramePadding.x*2.f,0.f};

                ImGui::Text("%s side won!", (winner_paddle->left ? "Left" : "Right"));

                if (ImGui::Button("Restart", button_size))
                {
                    // reset the paddles and the ball
                    left_paddle->points = right_paddle->points = 0;
                    right_paddle->reset(resolution_area, 0u);
                    left_paddle->reset(resolution_area, 0u);
                    ball->reset(resolution_area, 0);

                    // don't ask for a confirm next time someone wins
                    confirm_exit_game = false;

                    // we have no winner anymore
                    winner_paddle = nullptr;

                    // start timeout
                    start_timeout();
                }

                if (ImGui::Button("Back to main menu", button_size)) confirm_exit_game = true;
            }
            else
            {
                ImGui::TextUnformatted("Are you sure?");

                auto button_size = ImVec2{((ImGui::CalcTextSize("Are you sure?").x+ImGui::GetStyle().FramePadding.x*2.f)*.5f)-ImGui::GetStyle().ItemInnerSpacing.x*2.f,0.f};

                if (ImGui::Button("Yes", button_size)) should_exit = true;

                ImGui::SameLine();

                if (ImGui::Button("No", button_size)) confirm_exit_game = false;
            }
        }
    }

    return should_exit;
}

/*
@brief

    Handles key down and up messages
*/
void retrogames::games::pingpong_t::handle_key(ImGuiKey key, bool pressed)
{
    if (key == ImGuiKey_S || key == ImGuiKey_W || key == ImGuiKey_DownArrow || key == ImGuiKey_UpArrow)
    {
        auto control_key = (key == ImGuiKey_S) ? control_keys_e::KEY_S : (key == ImGuiKey_W ? control_keys_e::KEY_W : (key == ImGuiKey_UpArrow ? control_keys_e::KEY_UPARROW : control_keys_e::KEY_DOWNARROW));

        control_keys.pressed[static_cast<uint8_t>(control_key)] = pressed;

        if (pressed)
        {
            auto& timer = control_keys.down_timer[static_cast<uint8_t>(control_key)];

            timer.stop();
            timer.start();
        }
    }
    else if (key == ImGuiKey_Escape && pressed)
    {
        toggle_pause();
    }
}

/*
@brief

    Draws the options menu
*/
void retrogames::games::pingpong_t::draw_options(float scaling)
{
    // sliders
    ImGuiUser::inputslider_float(&cfgvalue_ping_scale_x, "Paddle scale (width)", 5.f, 0.3f, "The paddle width will be scaled by this value.", scaling, .1f, .2f);
    ImGuiUser::inputslider_float(&cfgvalue_ping_scale_y, "Paddle scale (height)", 5.f, 0.3f, "The paddle height will be scaled by this value.", scaling, .1f, .2f);
    ImGuiUser::inputslider_float(&cfgvalue_initial_paddle_speed, "Paddle speed", 2000.f, 400.f, "The speed of the paddle.", scaling, .1f, 5.f);
    ImGuiUser::inputslider_float(&cfgvalue_initial_ball_speed, "Ball speed", 2000.f, 400.f, "The speed of the ball.", scaling, .1f, 5.f);
    ImGuiUser::inputslider_float(&cfgvalue_ball_scale, "Ball scale", 5.f, 0.3f, "The ball size will be scaled by this value.", scaling, .1f, .2f);
    ImGuiUser::inputslider_uint32_t(&cfgvalue_max_score, "Max score", 20u, 0u, "The player/cpu that reaches this score wins. 0 means unlimited, no winner.", scaling);

    // combos
    ImGui::PushItemWidth(ImGui::GetContentRegionAvailWidth());
    ImGui::Separator();
    ImGui::TextUnformatted("Difficulty:");
    ImGui::SameLine();

    ImGuiUser::help_marker("The CPU difficulty setting.");

    static auto diff_item = static_cast<int32_t>(detail::difficulty_string_to_index(cfgvalue_cpu_difficulty.get<std::string>()));

    if (ImGui::Combo("##diff", &diff_item, get_difficulty_names(), static_cast<int32_t>(DIFFICULTY::DIFFICULTY_SIZE)))
    {
        cfgvalue_cpu_difficulty.set<std::string>(get_difficulty_name(static_cast<DIFFICULTY>(diff_item)));

        //printf("chose: %s - %s\n",get_difficulty_name(static_cast<DIFFICULTY>(diff_item)),settings->get("pingpong_cpu_difficulty").get<std::string>().c_str());
    }

    ImGui::PopItemWidth();
}

/*
@brief

    Resets information (when we re-start the game)
*/
void retrogames::games::pingpong_t::reset(settings_t* settings, bool create_fonts)
{
    initial_paddle_speed = static_cast<double>(cfgvalue_initial_paddle_speed.get<float>());
    initial_ball_speed = static_cast<double>(cfgvalue_initial_ball_speed.get<float>());
    ball_scale = static_cast<double>(cfgvalue_ball_scale.get<float>());
    resolution_area = settings->get_main_settings().resolution_area;

    create_paddles();
    create_ball();

    if (create_fonts) create_main_font(UI_SCALE);

    control_keys.reset();

    winner_paddle = nullptr;

    should_exit = confirm_exit_game = false;

    // create difficulties if that hasn't happened yet
    if (difficultymanager == nullptr)
    {
        difficultymanager = std::make_unique<difficultymanager_t>(resolution_area.width);

        // easy, set paddle speed to half and disable going to the calculated position entirely
        difficultymanager->create_difficulty(DIFFICULTY::DIFFICULTY_EASY,
                                            false, false,
                                            .5, .8,
                                            .2, 1.,
                                            .2);
        // medium, set paddle speed to half (minimum) and 80% (maximum) and give it a 20% chance to go to the calculated position (but only when 80-90% towards the other side)
        difficultymanager->create_difficulty(DIFFICULTY::DIFFICULTY_MEDIUM,
                                            true, true,
                                            .7, .95,
                                            .3, .2,
                                            .5);
        // hard, set paddle speed to 100% (minimum) and 120% (maximum) and go to the calculated position between 60-70% of the ball getting to the paddle
        // Also a 95% chance to move to the calculated position when 30-40% within the range of the cpu paddle
        difficultymanager->create_difficulty(DIFFICULTY::DIFFICULTY_HARD,
                                            true, true,
                                            1., 1.2,
                                            .4, .3,
                                            .95);
        // impossible, set the paddle speed to 100% and always go to the calculated position
        difficultymanager->create_difficulty(DIFFICULTY::DIFFICULTY_IMPOSSIBLE,
                                            false, false,
                                            1., 1.,
                                            0., 0.);
        // choose the difficulty we want
        difficultymanager->choose_difficulty(detail::difficulty_string_to_index(cfgvalue_cpu_difficulty.get<std::string>()));
    }
    else
    {
        // reset screen width
        difficultymanager->set_screen_width(resolution_area.width);

        // choose the difficulty we want
        difficultymanager->choose_difficulty(detail::difficulty_string_to_index(cfgvalue_cpu_difficulty.get<std::string>()));
    }
}

/*
@brief

    Draws controls
*/
void retrogames::games::pingpong_t::draw_controls(float scaling)
{
    ImGui::BulletText("W/S - Move left paddle");
    ImGui::BulletText("Arrow up/down - Move right paddle");
    ImGui::BulletText("Escape - Pause");
}

/*
@brief

    Draws some information
*/
void retrogames::games::pingpong_t::draw_information(float scaling)
{

}

/*
@brief

    Draws the ball
*/
void retrogames::games::pingpong_t::ball_t::draw(void)
{
    static auto ball_color = color_t(200, 200, 200);

    ImGui::GetBackgroundDrawList()->AddRectFilled(ImVec2{static_cast<float>(x)-static_cast<float>(size / 2), static_cast<float>(y)-static_cast<float>(size / 2)}, ImVec2{static_cast<float>(x+static_cast<float>(size / 2)),static_cast<float>(y+static_cast<float>(size / 2))}, ImGuiUser::color_to_imgui_color_u32(ball_color));
}

/*
@brief

    Draws a paddle
*/
void retrogames::games::pingpong_t::paddle_t::draw(void)
{
    static auto paddle_color = color_t(220, 220, 220);
    //static auto paddle_color_moving_to_position = color_t(220, 50, 50);

    auto current_paddle_color = /*moving_to_calculated_position ? paddle_color_moving_to_position : */paddle_color;

    ImGui::GetBackgroundDrawList()->AddRectFilled(ImVec2{static_cast<float>(x), static_cast<float>(y)}, ImVec2{static_cast<float>(x + static_cast<double>(size.width)), static_cast<float>(y + static_cast<double>(size.height))}, ImGuiUser::color_to_imgui_color_u32(current_paddle_color));
}

/*
@brief

    Creates the main font
*/
void retrogames::games::pingpong_t::create_main_font(float scaling)
{
    ImFontConfig font_config;

    font_config.SizePixels = std::ceil((static_cast<float>(resolution_area.height) / 10.f) * scaling);

    main_font = ImGui::GetIO().Fonts->AddFontDefault(&font_config);
}

/*
@brief

    Checks if the ball intersects the paddle
*/
bool retrogames::games::pingpong_t::paddle_t::intersect(ball_t* ball)
{
    auto do_boxes_intersect_or_touch = [](double b1_x, double b1_y, double b1_width, double b1_height, double b2_x, double b2_y, double b2_width, double b2_height)
    {
        return !(b2_x >= b1_x + b1_width || b2_x + b2_width <= b1_x || b2_y >= b1_y + b1_height || b2_y + b2_height <= b1_y);
    };

    // check if the ball position changed
    if (ball->x != ball->old_x || ball->y != ball->old_y)
    {
        // check if the ball flew through a paddle
        auto past_x = left ?    ball->old_x > x + static_cast<double>(size.width) && ball->x < x : // left paddle
                                ball->old_x < x && ball->x > x + static_cast<double>(size.width);  // right paddle
        auto past_y = ball->old_y >= y && ball->old_y <= y + static_cast<double>(size.height) && (ball->y < y || ball->y > y + static_cast<double>(size.height));

        if (past_x || past_y)
        {
            // ball flew through a paddle, check all the inbetween positions for intersection too
            auto offset_x = ball->x - ball->old_x;
            auto offset_y = ball->y - ball->old_y;
            auto amount_x = static_cast<uint32_t>(std::abs(offset_x) / static_cast<double>(ball->size));
            auto amount_y = static_cast<uint32_t>(std::abs(offset_y) / static_cast<double>(ball->size));

            if (amount_x > 0 || amount_y > 0)
            {
                auto highest_amount = std::max(amount_x, amount_y);

                for (uint32_t i = 0; i < highest_amount; i++)
                {
                    double mult_x = 0., mult_y = 0.;
                    
                    if (highest_amount == amount_x)
                    {
                        mult_x = static_cast<double>(i + 1.) / static_cast<double>(amount_x);
                        mult_y = mult_x;
                    }
                    else
                    {
                        mult_y = static_cast<double>(i + 1.) / static_cast<double>(amount_y);
                        mult_x = mult_y;
                    }

                    auto ball_x = ball->old_x + offset_x * static_cast<double>(mult_x);
                    auto ball_y = ball->old_y + offset_y * static_cast<double>(mult_y);
                    auto old_ball_x = ball->x;
                    auto old_ball_y = ball->y;
                    
                    ball->x = ball_x;
                    ball->y = ball_y;

                    if (do_boxes_intersect_or_touch(ball->x - static_cast<double>(ball->size / 2), ball->y - static_cast<double>(ball->size / 2), static_cast<double>(ball->size), static_cast<double>(ball->size), x, y, static_cast<double>(size.width), static_cast<double>(size.height))) return true;

                    ball->x = old_ball_x;
                    ball->y = old_ball_y;
                }

                // all inbetween and final positions already checked, no intersection
                return false;
            }
        }
    }

    // ball didn't fly through a paddle, simply check for intersection
    return do_boxes_intersect_or_touch(ball->x - static_cast<double>(ball->size / 2), ball->y - static_cast<double>(ball->size / 2), static_cast<double>(ball->size), static_cast<double>(ball->size), x, y, static_cast<double>(size.width), static_cast<double>(size.height));
}