#pragma once

#include <cstdint>

namespace retrogames
{

    struct area_size_t final
    {

        uint32_t width, height;

        area_size_t(uint32_t width = 0, uint32_t height = 0) : width(width), height(height) {}

    };

}