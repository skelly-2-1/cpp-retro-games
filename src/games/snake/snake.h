/*
@file

	snake.h

@purpose

	Snake game and GUI functionality
*/

#pragma once

#include <cstdint>
#include <chrono>
#include <memory>
#include <chrono>
#include <tuple>
#include <deque>
#include "misc/color.h"
#include "imgui/imgui.h"
#include "fpsmanager/fpsmanager.h"
#include "misc/unique_ptr_array_matrix.h"
#include "misc/settings.h"
#include "misc/timer.h"
#include "games/base/base.h"

namespace retrogames
{

    namespace games
    {

        class snake_t final : public game_base_t
        {

        protected:



        private:

            // Settings
            cfgvalue_t& setting_field_size;
            cfgvalue_t& setting_speed;

            // Tells us about the current state of the death menu
            enum class DEATH_STATE : uint8_t
            {

                DEATH_STATE_MAIN,
                DEATH_STATE_CONFIRM_CLOSE

            };

            // Tells us about the state of a box in our playing field
            enum class POSITION_STATE : uint8_t
            {

                POSITION_STATE_NOTHING,
                POSITION_STATE_SNAKE,
                POSITION_STATE_FOOD

            };

            // Tells us about the direction of our snake
            enum class DIRECTION : uint8_t
            {

                SNAKE_DIRECTION_NONE, // Only used for forcing (@force_direction)
                SNAKE_DIRECTION_UP,
                SNAKE_DIRECTION_DOWN,
                SNAKE_DIRECTION_LEFT,
                SNAKE_DIRECTION_RIGHT,
                
                // Default direction of the snake when starting the game/after dying
                SNAKE_DIRECTION_DEFAULT = SNAKE_DIRECTION_DOWN

            };

            // Static vars
            struct static_vars_t final
            {

                DIRECTION direction;

                bool last_dead, last_dead_needs_reset;
                bool last_death, last_death_needs_reset;
                bool last_render;
                bool interpolate_last_time;

                void reset(void)
                {
                    last_render = true;
                    interpolate_last_time = false;
                    direction = DIRECTION::SNAKE_DIRECTION_DEFAULT;
                }

                static_vars_t() { reset(); }

            };

            static_vars_t static_vars;

            // The current state of the death menu
            DEATH_STATE death_state;

            // The target FPS of our snake moving
            uint8_t snake_fps;

            // We don't want to move the snake 60 times a second, so
            // we create another fpsmanager to be able to see at every draw frame
            // if we want to move.
            fpsmanager_t fpsmanager;

            // Used to pulsate colors
            std::chrono::high_resolution_clock::time_point death_time;

            // How many boxes we have per axis
            uint32_t box_amount;

            // The size of our boxes
            float box_size;

            // Head position of our snake
            ImVec2 head;

            // Last head position of our snake
            ImVec2 last_head;

            // The direction stack of our snake
            std::deque<DIRECTION> direction_stack;

            // Needed in order to not skip a frame when we choose a direction
            DIRECTION force_direction;

            // Array of snake positions
            unique_ptr_array_matrix_t<POSITION_STATE> positions;

            // Position history of our snake
            std::deque<std::pair<int16_t, int16_t>> position_history;

            // Last position history of our snake (used for interpolating)
            std::deque<std::pair<int16_t, int16_t>> last_position_history;

            // For drawing all the foods
            using food_type = std::tuple<int16_t, int16_t, timer_t>;
            using cached_food_type = std::tuple<uint16_t, uint16_t, uint64_t>;

            std::deque<food_type> foods;
            std::deque<cached_food_type> cached_foods;

            // Tells us if the snake died or not
            bool dead;

            // Tells us if we just started
            bool just_started;

            // Did we just eat?
            bool eaten;

            // Should we exit the application?
            bool should_exit;

            // Stores the time in the current game we survived
            playtime_t time_survived;

            // Did we hit pause?
            bool hit_pause;

            // How many times we moved
            uint64_t move_counter;

            // The last frame we ate at
            uint16_t move_eat_counter;

            // The resolution of our playing field
            uint16_t resolution;

            // Resolution area
            area_size_t resolution_area;

            /*
            @brief

                Draws a filled rectangle at the @pos and @size in the current ImGui window
            */
            void draw_filled_rect(const ImVec2& pos, const ImVec2& size, color_t color);

            /*
            @brief

                Draws a rectangle at the @pos and @size in the current ImGui window
            */
            void draw_rect(const ImVec2& pos, const ImVec2& size, color_t color);

            /*
            @brief

                Draws a line between @pos_1 and @pos_2 given the @thickness
                in the current ImGui window
            */
            void draw_line(const ImVec2& pos_1, const ImVec2& pos_2, color_t color, float thickness = 1.f);

            /*
            @brief
            
                Kills the snake
            */
            void kill(void);

            /*
            @brief

                Resets all the variables associated with the snake
            */
            void do_reset(void);

            /*
            @brief

                Draws the snake playing field and handles all the logic
            */
            void draw_field(void);

            /*
            @brief

                Draws the left window (playing field is square, so we have left and right windows
                since our resolution is 16:9)
            */
            void draw_left_window(void);

            /*
            @brief

                Draws the right window (playing field is square, so we have left and right windows
                since our resolution is 16:9)
            */
            void draw_right_window(void);

            /*
            @brief

                Draws the death menu
            */
            void draw_death_menu(void);

            /*
            @brief

                Handles game logic, moves the snake if need be
            */
            DIRECTION think(void);

            /*
            @brief

                Moves our snake. Returns false if we didn't hit something (we died),
                and true if we did. Uses the @dir argument to move in it's direction.
            */
            bool move(const DIRECTION dir);

            /*
            @brief

                Needs to be called when the snake eats
            */
            void eat(void);

            /*
            @brief

                Generates food at random coordinates (where there aren't any yet)
            */
            void generate_food(void);

        public:

            /*
            @brief

                Constructor
            */
            snake_t(settings_t* settings, const std::string& name, ImFont** default_font_small, ImFont** default_font_mid, ImFont** default_font_big, const std::string& version = "1.0", uint8_t* icon = nullptr);

            /*
            @brief

                Destructor
            */
            ~snake_t();

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