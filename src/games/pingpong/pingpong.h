/*
@file

	pingpong.h

@purpose

	Ping pong game and GUI functionality
*/

#include <memory.h>
#include <random>
#include "games/base/base.h"
#include "misc/area_size.h"
#include "misc/color.h"
#include "misc/settings.h"
#include "misc/timer.h"

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

            struct difficultymanager_t final
            {



            };

            struct ball_t final
            {

                double x, y, old_x, old_y;
                double initial_speed;
                double speed_x, speed_y;

                uint32_t size;

                void reset(area_size_t resolution_area, uint32_t total_points = 0)
                {
                    auto random_number = [](const int32_t& min, const int32_t& max) -> int32_t
                    {
                        static std::random_device rd; // obtain a random number from hardware
                        static std::mt19937 eng(rd()); // seed the generator

                        std::uniform_int_distribution<> distr(min, max); // define the range

                        return distr(eng);
                    };

                    x = std::floor(static_cast<double>(resolution_area.width) * .5 - static_cast<double>(size) * .5);
                    y = std::floor(static_cast<double>(resolution_area.height) * .5 - static_cast<double>(size) * .5);

                    speed_x = random_number(0, 100) < 50 ? initial_speed + static_cast<double>(total_points) : -initial_speed - static_cast<double>(total_points);
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