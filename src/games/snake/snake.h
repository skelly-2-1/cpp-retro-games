/*
@file

	snake.h

@purpose

	Actual snake game and GUI functionality
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
#include "games/base.h"

namespace retrogames
{

	class settings_t;

    namespace games
    {

        class snake_t final : public game_base_t
        {

        protected:



        private:

            // Settings
            cfgvalue_t& setting_timeout;
            cfgvalue_t& setting_field_size;
            cfgvalue_t& setting_speed;

            // Tells us about the current modal popup ID (in case we want to stack them)
            static uint8_t current_modal_popup_id;

            /*
            @brief

                Wrapper for ImGui's modal popups (just makes the code smaller)
            */
            struct modal_popup_t final
            {

                // Did we successfully open the dummy window?
                bool started_window;

                // Did we successfully open the popup?
                bool started_popup;

                // Do we want to darken the background?
                bool darkening;

                /*
                @brief

                    Constructor
                */
                modal_popup_t(const char* name, bool darkening = false);

                /*
                @brief

                    Destructor
                */
                ~modal_popup_t();

                /*
                @brief

                    Tells the caller if we started the popup successfully or not
                */
                bool success(void) const { return started_popup; }

            };

            // Tells us about the current state of the game
            enum class GAME_STATE : uint8_t
            {

                GAME_STATE_MAIN_MENU,
                GAME_STATE_OPTIONS_MENU,
                GAME_STATE_HIGHSCORES_MENU,
                GAME_STATE_PLAYING,

                // Default game state/starting screen
                GAME_STATE_DEFAULT = GAME_STATE_MAIN_MENU

            };

            // Tells us about the current state of the main menu
            enum class MAIN_MENU_STATE : uint8_t
            {

                MAIN_MENU_STATE_MAIN,
                MAIN_MENU_STATE_OPTIONS,

                // Default main menu state
                MAIN_MENU_STATE_DEFAULT = MAIN_MENU_STATE_MAIN

            };

            // Tells us about the current state of the pause menu
            enum class PAUSE_STATE : uint8_t
            {

                PAUSE_STATE_MAIN,
                PAUSE_STATE_CONFIRM_MAIN_MENU,
                PAUSE_STATE_CONFIRM_CLOSE_GAME

            };

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
                bool last_pause, last_pause_needs_reset;
                bool last_render;
                bool last_timeout_timer_started, last_timeout_timer_started_needs_reset;
                bool interpolate_last_time;

                void reset(void)
                {
                    last_pause = last_death = last_dead = false;
                    last_pause_needs_reset = last_death_needs_reset = last_dead_needs_reset = true;
                    last_render = true;
                    last_timeout_timer_started = false;
                    last_timeout_timer_started_needs_reset = true;
                    interpolate_last_time = false;
                    direction = DIRECTION::SNAKE_DIRECTION_DEFAULT;
                }

                static_vars_t() { reset(); }

            };

            static_vars_t static_vars;

            // The current state of the game
            GAME_STATE game_state;

            // The current state of the main menu
            MAIN_MENU_STATE main_menu_state;

            // The current state of the pause menu
            PAUSE_STATE pause_state;

            // The current state of the death menu
            DEATH_STATE death_state;

            // The target FPS of our snake moving
            uint8_t snake_fps;

            // We don't want to move the snake 60 times a second, so
            // we create another fpsmanager to be able to see at every draw frame
            // if we want to move.
            fpsmanager_t fpsmanager;

            // The start timer (starts when we press play)
            timer_t start_timer;

            // Used to pulsate colors
            std::chrono::high_resolution_clock::time_point death_time;

            // Did we pause the game?
            bool paused;

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

            // The time since starting (death timeout)
            timer_t timeout_timer;

            // Our settings
            settings_t* settings;

            // Did we just eat?
            bool eaten;

            // Should we exit the application?
            bool should_exit;

            // Start the timeout timer?
            bool start_timeout_timer;

            // Stores the time in the current game we survived
            std::tuple<uint16_t, uint16_t, uint16_t, uint16_t> time_survived;

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

                Converts a color_t to an ImU32 (used by ImGui for rendering)
            */
            ImU32 to_imgui_color(color_t color);

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

                Draws the pause menu
            */
            void draw_pause_menu(void);

            /*
            @brief

                Draws the death menu
            */
            void draw_death_menu(void);

            /*
            @brief

                Draws the main menu
            */
            void draw_main_menu(void);

            /*
            @brief

                Draws the highscores menu
            */
            void draw_highscores_menu(void);

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

                Returns the time elapsed since a chrono::high_resolution_clock::time_point.
                Hours, minutes, seconds, milliseconds
            */
            std::tuple<uint16_t, uint16_t, uint16_t, uint16_t> get_time_elapsed(const std::chrono::high_resolution_clock::time_point& point);

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
            snake_t(settings_t* settings, const std::string& name, const std::string& version = "1.0", uint8_t* icon = nullptr);

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
            virtual void reset(settings_t* settings) override;

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