/*
@file

	pingpong.h

@purpose

	Ping pong game and GUI functionality
*/

#include <memory.h>
#include <random>
#include <array>
#include "games/base/base.h"
#include "misc/area_size.h"
#include "misc/color.h"
#include "misc/settings.h"
#include "misc/timer.h"
#include "util/util.h"

namespace retrogames
{

    namespace games
    {

        class pingpong_t final : public game_base_t
        {

        protected:



        private:

            enum class control_keys_e
            {

                KEY_W,
                KEY_S,
                KEY_DOWNARROW,
                KEY_UPARROW,
                KEY_SIZE

            };

            struct control_keys_t final
            {

                bool is_pressed(control_keys_e key) { return pressed[static_cast<uint8_t>(key)]; }

                bool pressed[static_cast<uint8_t>(control_keys_e::KEY_SIZE)]{};

                timer_t down_timer[static_cast<uint8_t>(control_keys_e::KEY_SIZE)];

                void reset(void)
                {
                    memset(&pressed, 0, sizeof(pressed));

                    for (auto& timer : down_timer) timer.stop();
                }

            };

        public:

            // all of our difficulties (the values have to be defined later)
            enum class DIFFICULTY
            {

                DIFFICULTY_EASY,
                DIFFICULTY_MEDIUM,
                DIFFICULTY_HARD,
                DIFFICULTY_IMPOSSIBLE,
                DIFFICULTY_SIZE,
                DIFFICULTY_DEFAULT = DIFFICULTY_IMPOSSIBLE

            };

            /*
            @brief

                Gets all difficulty names
            */
            static const char** get_difficulty_names(void)
            {
                // names of the difficulties
                static const char* difficulty_names[static_cast<uint8_t>(DIFFICULTY::DIFFICULTY_SIZE)] = {

                    "Easy",
                    "Medium",
                    "Hard",
                    "Impossible"

                };

                return difficulty_names;
            }

            /*
            @brief

                Gets a difficulty name
            */
            static const char* get_difficulty_name(DIFFICULTY diff)
            {
                return get_difficulty_names()[static_cast<uint8_t>(diff)];
            }

        private:

            // the values stored in all the difficulties
            class difficulty_t final
            {

            protected:



            private:

                bool go_to_calculated_position; // flag for the chance to move to the calculated position

                std::string name; // name of the difficulty

            public:

                bool enable_paddle_speed_minmax_variance; // enable variance of paddle speed multiplier?
                bool enable_calculated_pos_minmax_variance; // enable variance of calculated pos multiplier?

                bool is_calculated_pos_multiplier_dynamic; // for outside calculations to check if our calculated pos multiplier is dynamic or not
                bool is_paddle_speed_multiplier_dynamic; // for outside calculations to check if our paddle speed multiplier is dynamic or not

                double paddle_speed_multiplier_min; // cpu paddle will be scaled by this (minimum)
                double paddle_speed_multiplier_max; // cpu paddle will be scaled by this (maximum)
                double min_calculated_pos_multiplier; // example: 0.8 = 80% = Paddle will just go to the calculated position if 80% of the time/distance has been passed
                double max_calculated_pos_multiplier; // ^
                double current_paddle_speed_multiplier; // calculated later
                double current_calculated_pos_multiplier; // calculated later
                double calculated_pos_moving_chance; // the chance to move to the calculated position at one time

                difficulty_t(   std::string name,
                                bool enable_paddle_speed_minmax_variance,
                                bool enable_calculated_pos_minmax_variance,
                                double paddle_speed_multiplier_min,
                                double paddle_speed_multiplier_max,
                                double min_calculated_pos_multiplier,
                                double max_calculated_pos_multiplier,
                                double calculated_pos_moving_chance) :
                                current_calculated_pos_multiplier(0.),
                                current_paddle_speed_multiplier(1.),
                                go_to_calculated_position(false),
                                name(name),
                                enable_paddle_speed_minmax_variance(enable_paddle_speed_minmax_variance),
                                enable_calculated_pos_minmax_variance(enable_calculated_pos_minmax_variance),
                                paddle_speed_multiplier_min(paddle_speed_multiplier_min),
                                paddle_speed_multiplier_max(paddle_speed_multiplier_max),
                                min_calculated_pos_multiplier(min_calculated_pos_multiplier),
                                max_calculated_pos_multiplier(max_calculated_pos_multiplier),
                                calculated_pos_moving_chance(calculated_pos_moving_chance)
                {
                    if (!enable_calculated_pos_minmax_variance && min_calculated_pos_multiplier == max_calculated_pos_multiplier)
                    {
                        is_calculated_pos_multiplier_dynamic = false;

                        current_calculated_pos_multiplier = max_calculated_pos_multiplier;
                    }
                    else
                    {
                        is_calculated_pos_multiplier_dynamic = true;
                    }

                    if (!enable_paddle_speed_minmax_variance && paddle_speed_multiplier_min == paddle_speed_multiplier_max)
                    {
                        is_paddle_speed_multiplier_dynamic = false;

                        current_paddle_speed_multiplier = paddle_speed_multiplier_max;
                    }
                    else
                    {
                        is_paddle_speed_multiplier_dynamic = true;
                    }
                }

                void generate_numbers(void)
                {
                    if (!enable_calculated_pos_minmax_variance)
                    {
                        current_calculated_pos_multiplier = max_calculated_pos_multiplier;
                    }
                    else
                    {
                        current_calculated_pos_multiplier = (min_calculated_pos_multiplier != max_calculated_pos_multiplier) ? min_calculated_pos_multiplier + util::random(0., 1.) * (max_calculated_pos_multiplier - min_calculated_pos_multiplier) : max_calculated_pos_multiplier;
                    }

                    if (!enable_paddle_speed_minmax_variance)
                    {
                        current_paddle_speed_multiplier = paddle_speed_multiplier_max;
                    }
                    else
                    {
                        current_paddle_speed_multiplier = (paddle_speed_multiplier_min != paddle_speed_multiplier_max) ? paddle_speed_multiplier_min + util::random(0., 1.) * (paddle_speed_multiplier_max - paddle_speed_multiplier_min) : paddle_speed_multiplier_max;
                    }

                    if (calculated_pos_moving_chance < 1.)
                    {
                        go_to_calculated_position = util::random(0., 1.) >= 1. - calculated_pos_moving_chance;
                    }
                    else if (!go_to_calculated_position)
                    {
                        go_to_calculated_position = true;
                    }
                }

                bool should_paddle_go_to_calculated_position(uint32_t screen_width, double ball_x, bool left)
                {
                    if (!go_to_calculated_position) return false;
                    if (!is_calculated_pos_multiplier_dynamic && min_calculated_pos_multiplier == 0. && max_calculated_pos_multiplier == 0.) return true;

                    return !left ? (ball_x >= (static_cast<double>(screen_width) * (1. - current_calculated_pos_multiplier))) : (ball_x <= static_cast<double>(screen_width) - (static_cast<double>(screen_width) * (1. - current_calculated_pos_multiplier)));
                }

            };

            // the class that manages our difficulties
            class difficultymanager_t final
            {

            protected:



            private:

                std::array<std::unique_ptr<difficulty_t>, static_cast<uint8_t>(DIFFICULTY::DIFFICULTY_SIZE)> difficulties;

                DIFFICULTY current_difficulty;

                bool difficulty_set = false;

                uint32_t playable_area_width;

            public:

                void set_screen_width(uint32_t screen_width) { playable_area_width = screen_width; }

                difficultymanager_t(uint32_t screen_width) :
                    difficulty_set(false),
                    current_difficulty(DIFFICULTY::DIFFICULTY_IMPOSSIBLE) {}

                void create_difficulty(     DIFFICULTY diff_num,
                                            bool enable_paddle_speed_minmax_variance,
                                            bool enable_calculated_pos_minmax_variance,
                                            double paddle_speed_multiplier_min,
                                            double paddle_speed_multiplier_max,
                                            double min_calculated_pos_multiplier,
                                            double max_calculated_pos_multiplier,
                                            double calculated_pos_moving_chance = 1.)
                {
                    if (static_cast<int32_t>(diff_num) < 0 || static_cast<int32_t>(diff_num) >= static_cast<int32_t>(DIFFICULTY::DIFFICULTY_SIZE)) return;

                    difficulties[static_cast<int32_t>(diff_num)] = std::make_unique<difficulty_t>(  get_difficulty_name(diff_num),
                                                                                                    enable_paddle_speed_minmax_variance,
                                                                                                    enable_calculated_pos_minmax_variance,
                                                                                                    paddle_speed_multiplier_min,
                                                                                                    paddle_speed_multiplier_max,
                                                                                                    min_calculated_pos_multiplier,
                                                                                                    max_calculated_pos_multiplier,
                                                                                                    calculated_pos_moving_chance);
                }

                void choose_difficulty(DIFFICULTY diff_num)
                {
                    if (static_cast<int32_t>(diff_num) < 0 || static_cast<int32_t>(diff_num) >= static_cast<int32_t>(DIFFICULTY::DIFFICULTY_SIZE)) return;

                    current_difficulty = diff_num;
                    difficulty_set = true;
                }

                difficulty_t* get_current_difficulty(void)
                {
                    return difficulty_set ? difficulties[static_cast<int8_t>(current_difficulty)].get() : nullptr;
                }

            };

            std::unique_ptr<difficultymanager_t> difficultymanager;

            // all the info for the ball
            struct ball_t final
            {

                double x, y, old_x, old_y;
                double initial_speed;
                double speed_x, speed_y;

                uint32_t size;

                void reset(area_size_t resolution_area, uint32_t total_points = 0)
                {
                    x = std::floor(static_cast<double>(resolution_area.width) * .5);
                    y = std::floor(static_cast<double>(resolution_area.height) * .5);

                    speed_x = util::random(1, 100) < 50 ? initial_speed + static_cast<double>(total_points) : -initial_speed - static_cast<double>(total_points);
                    speed_y = 0.f;
                }

                ball_t(double initial_speed, uint32_t size, area_size_t resolution_area) :
                    size(size),
                    initial_speed(initial_speed)
                {
                    reset(resolution_area);
                }

                /*
                @brief

                    Draws the ball
                */
                void draw(void);

            };

            struct paddle_t final
            {

                enum class DIRECTION
                {

                    DIRECTION_NONE,
                    DIRECTION_UP,
                    DIRECTION_DOWN

                };

                uint32_t x_offset;

                double x, y;
                double speed;
                double base_speed, base_speed_scaled;

                area_size_t size;

                DIRECTION direction;

                uint32_t points;

                int32_t calculated_y, calculated_y_clamped;

                bool is_cpu;
                bool left;
                bool moving_to_calculated_position;
                bool calculated_position_set;

                void reset(area_size_t resolution_area, uint32_t total_points = 0)
                {
                    x = std::floor(left ? static_cast<double>(x_offset) : static_cast<double>((resolution_area.width - x_offset) - size.width));
                    y = std::floor(static_cast<double>(resolution_area.height / 2 - size.height / 2));

                    direction = DIRECTION::DIRECTION_NONE;

                    base_speed_scaled = base_speed + static_cast<double>(total_points);

                    calculated_y = -1;

                    is_cpu = true;
                    moving_to_calculated_position = calculated_position_set = false;
                }

                paddle_t(uint32_t x_offset, area_size_t size, double base_speed, area_size_t resolution_area, bool left = true) :
                    x_offset(x_offset),
                    size(size),
                    base_speed(base_speed),
                    left(left),
                    points(0u),
                    speed(0.)
                {
                    reset(resolution_area);
                }

                /*
                @brief

                    Checks if the ball intersects the paddle
                */
                bool intersect(ball_t* ball);

                /*
                @brief

                    Draws a paddle
                */
                void draw(void);

            };

            std::unique_ptr<paddle_t> left_paddle, right_paddle;
            std::unique_ptr<ball_t> ball, old_ball;

            settings_t* settings;

            area_size_t resolution_area;

            control_keys_t control_keys;

            cfgvalue_t& cfgvalue_ping_scale_x;
            cfgvalue_t& cfgvalue_ping_scale_y;
            cfgvalue_t& cfgvalue_initial_paddle_speed;
            cfgvalue_t& cfgvalue_initial_ball_speed;
            cfgvalue_t& cfgvalue_ball_scale;
            cfgvalue_t& cfgvalue_cpu_difficulty;

            double initial_paddle_speed;
            double initial_ball_speed;
            double ball_scale;
            double time_scale;

            ImFont* main_font;

            /*
            @brief

                Creates the main font
            */
            void create_main_font(float scaling);

            /*
            @brief

                Creates our paddles
            */
            void create_paddles(void);

            /*
            @brief

                Creates our ball
            */
            void create_ball(void);
    
        public:

            /*
            @brief

                Constructor
            */
            pingpong_t(settings_t* settings, const std::string& name, ImFont** default_font_small, ImFont** default_font_mid, ImFont** default_font_big, const std::string& version = "1.0", uint8_t* icon = nullptr);

            /*
            @brief

                Called from our renderer thread when we need to draw
            */
            virtual bool draw(bool render) override;

            /*
            @brief

                Handles key down and up messages
            */
            virtual void handle_key(ImGuiKey key, bool pressed) override;

            /*
            @brief

                Draws the options menu
            */
            virtual void draw_options(float scaling) override;

            /*
            @brief

                Resets information (when we re-start the game)
            */
            virtual void reset(settings_t* settings, bool create_fonts) override;

            /*
            @brief

                Draws controls
            */
            virtual void draw_controls(float scaling) override;

            /*
            @brief

                Draws some information
            */
            virtual void draw_information(float scaling) override;

        };

    }

}