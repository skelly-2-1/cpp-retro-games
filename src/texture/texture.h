/*
@file

    texture.h

@purpose

    Managing of textures
*/

#pragma once

#include <stdint.h>

namespace retrogames
{

    class texture_t final
    {

    protected:



    private:

        void* texture;

        uint32_t width, height;

    public:

        /*
        @brief

            Constructor, size and data should be the raw data and size of
            a PNG file
        */
        texture_t(const uint32_t size, const uint32_t* data);

        /*
        @brief

            Destructor, cleans up the texture
        */
        ~texture_t();

    };

}