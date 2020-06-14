/*
@file

    cfgvalue.h

@purpose

    Defining config variables' value
*/
        
#pragma once

#include <string>
#include <stdint.h>
#include "color.h"
#include "macros.h"

#ifdef SETTINGS_ENABLE_DEBUGGING
#ifdef PLATFORM_WINDOWS
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN
#endif
#include <Windows.h>
#define CFGVALUE_STRINGIZE(x) #x
#define CFGVALUE_STR(x) CFGVALUE_STRINGIZE(x)
#define CFGVALUE_ABORT() { MessageBoxA(nullptr, (std::string("Config value manager encountered an error at line ") + CFGVALUE_STR(__LINE__) + " in file: " + CFGVALUE_STR(__FILE__)).c_str(), "error", MB_SETFOREGROUND | MB_ICONERROR); TerminateProcess(GetCurrentProcess(), 0); }
#else
#ifndef PLATFORM_EMSCRIPTEN
#error no
#define CFGVALUE_STRINGIZE(x) #x
#define CFGVALUE_STR(x) CFGVALUE_STRINGIZE(x)
#define CFGVALUE_ABORT() { fprintf(stderr, (std::string("Config value manager encountered an error at line ") + CFGVALUE_STR(__LINE__) + " in file: " + CFGVALUE_STR(__FILE__) + "\n").c_str()); std::abort(); }
#else
#define CFGVALUE_ABORT() std::abort()
#endif
#endif
#else
#define CFGVALUE_ABORT() std::abort()
#endif

namespace retrogames
{

    /*
	@brief

		Defines the type of variables
	*/
	enum class cfgvalue_e
	{

		value_null,
		value_integer,
		value_unsigned_integer,
		value_boolean,
		value_string,
		value_float,
		value_color

	};

    /*
    @brief

        Defines a config variables' value
    */
    class cfgvalue_t final
    {

    protected:



    private:

        // type (to know what we're supposed to access)
        cfgvalue_e type;
        // string (stored as pointer to save space)
        std::string* string;
        // integer value
        int64_t integer;
        // unsigned integer value
        uint64_t unsigned_integer;
        // boolean
        bool boolean;
        // float
        float number_float;
        // color
        color_t color;

        // copies from another instance
        void copy_from(const cfgvalue_t& other)
        {
            switch (other.get_type())
            {
                case cfgvalue_e::value_boolean:
                {
                    boolean = other.get<bool>();

                    break;
                }
                case cfgvalue_e::value_integer:
                {
                    integer = other.get<int64_t>();

                    break;
                }
                case cfgvalue_e::value_string:
                {
                    if (string != nullptr)
                    {
                        *string = other.get<std::string>();
                    }
                    else
                    {
                        string = new std::string(other.get<std::string>());
                    }

                    break;
                }
                case cfgvalue_e::value_unsigned_integer:
                {
                    unsigned_integer = other.get<uint64_t>();

                    break;
                }
                case cfgvalue_e::value_float:
                {
                    number_float = other.get<float>();

                    break;
                }
                case cfgvalue_e::value_color:
                {
                    color = other.get<color_t>();

                    break;
                }
                default:
                {
                    break;
                }
            }

            type = other.get_type();
        }

    public:

        // destructor, cleans up our string if it was created
        ~cfgvalue_t()
        {
            if (string != nullptr)
            {
                delete string;

                string = nullptr;
            }
        }

        // copy constructor
        cfgvalue_t(const cfgvalue_t& other) { copy_from(other); }
        // default constructor
        cfgvalue_t() : type(cfgvalue_e::value_null), string(nullptr) {}
        // constructor for booleans
        cfgvalue_t(bool v) : boolean(v), string(nullptr), type(cfgvalue_e::value_boolean) {}
        // constructor for numbers (integer)
        cfgvalue_t(int64_t v) : integer(v), string(nullptr), type(cfgvalue_e::value_integer) {}
        // constructor for numbers (32bit integer)
        cfgvalue_t(int32_t v) : integer(static_cast<int64_t>(v)), string(nullptr), type(cfgvalue_e::value_integer) {}
        // constructor for unsigned numbers (unsigned integer)
        cfgvalue_t(uint64_t v) : unsigned_integer(v), string(nullptr), type(cfgvalue_e::value_unsigned_integer) {}
        // constructor for unsigned numbers (32bit unsigned integer)
        cfgvalue_t(uint32_t v) : unsigned_integer(static_cast<uint64_t>(v)), string(nullptr), type(cfgvalue_e::value_unsigned_integer) {}
        // constructor for floats
        cfgvalue_t(float v) : number_float(v), string(nullptr), type(cfgvalue_e::value_float) {}
        // constructor for colors
        cfgvalue_t(color_t col) : color(col), string(nullptr), type(cfgvalue_e::value_color) {}
        // constructor for empty values of a given type
        cfgvalue_t(cfgvalue_e t)
        {
            switch (t)
            {
                case cfgvalue_e::value_boolean:
                {
                    boolean = false;
                    type = cfgvalue_e::value_boolean;

                    break;
                }
                case cfgvalue_e::value_string:
                {
                    string = nullptr;
                    type = cfgvalue_e::value_string;

                    break;
                }
                case cfgvalue_e::value_integer:
                {
                    integer = 0;
                    type = cfgvalue_e::value_integer;

                    break;
                }
                case cfgvalue_e::value_unsigned_integer:
                {
                    unsigned_integer = 0;
                    type = cfgvalue_e::value_unsigned_integer;

                    break;
                }
                case cfgvalue_e::value_float:
                {
                    number_float = 0;
                    type = cfgvalue_e::value_float;

                    break;
                }
                case cfgvalue_e::value_color:
                {
                    color = color_t(255, 255, 255, 255);
                    type = cfgvalue_e::value_color;

                    break;
                }
                case cfgvalue_e::value_null:
                {
                    type = cfgvalue_e::value_null;

                    break;
                }
                default:
                {
                    CFGVALUE_ABORT();

                    break;
                }
            }
        }

        // constructor for strings
        cfgvalue_t(const std::string& str)
        {
            string = new std::string(str);

            type = cfgvalue_e::value_string;
        }

        // assignment operator
        cfgvalue_t& operator = (const cfgvalue_t& other) { copy_from(other); return *this; }

        // gets the type
        const cfgvalue_e& get_type(void) const { return type; }

        // creates as bool
        template <typename type>
        static typename std::enable_if<std::is_same<std::remove_cv_t<type>, bool>::value, cfgvalue_t>::type create(type default_value)
        {
            return cfgvalue_t(default_value);
        }

        // creates as arithmetic value (except bool and const char*/char*)
        template <typename type>
        static typename std::enable_if< std::is_arithmetic<type>::value &&
                                        !std::is_same<std::remove_cv_t<type>, bool>::value &&
                                        !std::is_same<type, char*>::value &&
                                        !std::is_same<type, const char*>::value, cfgvalue_t>::type create(type default_value)
        {
            return cfgvalue_t(default_value);
        }

        // creates as string (const char*/char*)
        template <typename type>
        static typename std::enable_if< std::is_constructible<type, const char*>::value &&
                                        (std::is_same<type, const char*>::value || std::is_same<type, char*>::value), cfgvalue_t>::type create(type default_value)
        {
            return cfgvalue_t(std::string(default_value));
        }

        // creates as string (std::string)
        template <typename type>
        static typename std::enable_if< std::is_constructible<type, const char*>::value &&
                                        std::is_same<std::remove_cv_t<type>, std::string>::value, cfgvalue_t>::type create(type default_value)
        {
            return cfgvalue_t(default_value);
        }

        // creates as color
        template <typename type>
        static typename std::enable_if<std::is_class<type>::value && std::is_same<std::remove_cv_t<type>, color_t>::value, cfgvalue_t>::type create(type default_value)
        {
            return cfgvalue_t(default_value);
        }

        // gets an arithmetic value (except bool and const char*/char*)
        template <typename type>
        typename std::enable_if<    std::is_arithmetic<type>::value &&
                                    !std::is_same<std::remove_cv_t<type>, bool>::value &&
                                    !std::is_same<type, char*>::value &&
                                    !std::is_same<type, const char*>::value,
                                    type>::type get(void) const
        {
            switch (this->type)
            {
                case cfgvalue_e::value_float:
                {
                    return static_cast<type>(number_float);
                }
                case cfgvalue_e::value_integer:
                {
                    return static_cast<type>(integer);
                }
                case cfgvalue_e::value_unsigned_integer:
                {
                    return static_cast<type>(unsigned_integer);
                }
                default:
                {
                    break;
                }
            }

            CFGVALUE_ABORT();

            return 0;
        }

        // gets a boolean value
        template <typename type>
        typename std::enable_if<std::is_same<std::remove_cv_t<type>, bool>::value, type>::type get(void) const
        {
            if (this->type != cfgvalue_e::value_boolean) CFGVALUE_ABORT();

            return boolean;
        }

        // gets a color value
        template <typename type>
        typename std::enable_if<std::is_class<type>::value && std::is_same<std::remove_cv_t<type>, color_t>::value, type>::type get(void) const
        {
            if (this->type != cfgvalue_e::value_color) CFGVALUE_ABORT();

            return color;
        }

        // gets a string value with an object
        template<typename type>
        typename std::enable_if<std::is_constructible<type, const char*>::value && !std::is_pointer<type>::value && !std::is_same<std::remove_cv_t<type>, bool>::value, type>::type get(void) const
        {
            if (this->type != cfgvalue_e::value_string) CFGVALUE_ABORT();

            return string != nullptr ? *string : "";
        }

        // gets a raw string value
        template <typename type>
        typename std::enable_if<std::is_constructible<type, const char*>::value && std::is_pointer<type>::value, type>::type get(void) const
        {
            if (this->type != cfgvalue_e::value_string) CFGVALUE_ABORT();

            return string != nullptr ? string->c_str() : 0;
        }

        // sets an arithmetic value
        template <typename type>
        typename std::enable_if<std::is_arithmetic<type>::value && !std::is_same<std::remove_cv_t<type>, bool>::value, void>::type set(type t)
        {
            switch (this->type)
            {
                case cfgvalue_e::value_integer:
                {
                    integer = static_cast<int64_t>(t);

                    break;
                }
                case cfgvalue_e::value_float:
                {
                    number_float = static_cast<float>(t);

                    break;
                }
                case cfgvalue_e::value_unsigned_integer:
                {
                    unsigned_integer = static_cast<uint64_t>(t);

                    break;
                }
                default:
                {
                    CFGVALUE_ABORT();

                    break;
                }
            }
        }

        // sets a boolean value
        template <typename type>
        typename std::enable_if<std::is_same<std::remove_cv_t<type>, bool>::value, void>::type set(type t)
        {
            if (this->type != cfgvalue_e::value_boolean)
            {
                CFGVALUE_ABORT();

                return;
            }

            boolean = t;
        }

        // sets a color value
        template <typename type>
        typename std::enable_if<std::is_class<type>::value && std::is_same<std::remove_cv_t<type>, color_t>::value, void>::type set(type t)
        {
            if (this->type != cfgvalue_e::value_color)
            {
                CFGVALUE_ABORT();

                return;
            }

            color = t;
        }

        // sets a string value with a std::string object or a const char*
        template <typename type>
        typename std::enable_if<std::is_same<type, std::string>::value || std::is_same<type, const char*>::value, void>::type set(type t)
        {
            if (this->type != cfgvalue_e::value_string)
            {
                CFGVALUE_ABORT();

                return;
            }

            if (string == nullptr)
            {
                string = new std::string(t);
            }
            else
            {
                *string = t;
            }
        }

        // compares an arithmetic value
        template <typename type>
        typename std::enable_if<std::is_arithmetic<type>::value && !std::is_same<std::remove_cv_t<type>, bool>::value, bool>::type operator == (const type cmp) const
        {
            return get<type>() == cmp;
        }

        // compares a boolean value
        template <typename type>
        typename std::enable_if<std::is_same<std::remove_cv_t<type>, bool>::value, type>::type operator == (const type cmp) const
        {
            return get<type>() == cmp;
        }

        // compares a color value
        template <typename type>
        typename std::enable_if<std::is_class<type>::value && std::is_same<std::remove_cv_t<type>, color_t>::value, type>::type operator == (const type cmp) const
        {
            auto col = get<type>();

            return col[0] == cmp[0] && col[1] == cmp[1] && col[2] == cmp[2] && col[3] == cmp[3];
        }

        // compares a std::string value
        template <typename type>
        typename std::enable_if<std::is_same<type, std::string>::value, bool>::type operator == (const type cmp) const
        {
            return strcmp(get<type>().c_str(), cmp.c_str()) == 0;
        }

        // compares a raw string value
        template <typename type>
        typename std::enable_if<std::is_same<type, const char*>::value, bool>::type operator == (const type cmp) const
        {
            return strcmp(get<type>().c_str(), cmp) == 0;
        }

        // sets an arithmetic value
        template <typename type>
        typename std::enable_if<std::is_arithmetic<type>::value && !std::is_same<type, bool>::value, cfgvalue_t&>::type operator = (const type v)
        {
            set<type>(v);

            return *this;
        }

        // sets a boolean value
        template <typename type>
        typename std::enable_if<std::is_same<type, bool>::value, cfgvalue_t&>::type operator = (const type v)
        {
            set<bool>(v);

            return *this;
        }

        // sets a color value
        template <typename type>
        typename std::enable_if<std::is_class<type>::value && std::is_same<type, color_t>::value, cfgvalue_t&>::type operator = (const type v)
        {
            set<color_t>(v);

            return *this;
        }

        // sets a string value
        template <typename type>
        typename std::enable_if<std::is_same<type, std::string>::value || std::is_same<type, const char*>::value, cfgvalue_t&>::type operator = (const type v)
        {
            set<type>(v);

            return *this;
        }

    };

}

#if (defined(PLATFORM_WINDOWS) || defined(PLATFORM_LINUX)) && defined(SETTINGS_ENABLE_DEBUGGING)
#undef CFGVALUE_STRINGIZE
#undef CFGVALUE_STR
#endif
#undef CFGVALUE_ABORT