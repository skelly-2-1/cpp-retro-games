/*
@file

	unique_ptr_array_matrix.h

@purpose

	Being able to treat unique_ptr arrays as multidimensional ones
*/

#pragma once

#include <memory>
#include <cstdint>

namespace retrogames
{

    template <typename T> class unique_ptr_array_matrix_t final
    {
        
    protected:



    private:

        // The actual array
        std::unique_ptr<T[]> array;

        // The width and height of our array
        uint64_t width, height;

        /*
        @brief
            
            Gets the index of a x and y coordinate of our matrix
        */
        uint64_t index(uint64_t x, uint64_t y) const { return x + width * y; }

    public:

        /*
        @brief

            Constructor, initializes the array and saves the width of our array
        */
        unique_ptr_array_matrix_t(uint64_t w, uint64_t h) : width(w), height(h), array(new T[w * h]{}) {}
        
        /*
        @brief

            Const-accessor of our array
        */
        T at(uint64_t x, uint64_t y) const { return array[index(x, y)]; }

        /*
        @brief

            Non-const accessor of our array
        */
        T& at(uint64_t x, uint64_t y) { return array[index(x, y)]; }

        /*
        @brief

            Gets the size of our array
        */
        size_t size(void) const { return width * height; }

    };

}