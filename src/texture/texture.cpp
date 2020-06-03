/*
@file

    texture.cpp

@purpose

    Managing of textures
*/

#define STB_IMAGE_IMPLEMENTATION

#include "texture.h"
#include "stb_image.h"
#include <utility>

#ifdef PLATFORM_WINDOWS
#include <d3d9.h>
#include <d3d9.h>

namespace retrogames
{

    // from imgui/wrappers/dx9/dx9.h
    extern IDirect3DDevice9* global_d3d9device;

}

#endif

// from imgui_draw.cpp
extern unsigned int stb_decompress_length(const unsigned char *input);
extern unsigned int stb_decompress(unsigned char *output, const unsigned char *input, unsigned int length);

/*
@brief

    Constructor, size and data should be the stb-compressed data and size of
    a PNG file
*/
retrogames::texture_t::texture_t(const uint32_t size, const uint32_t* data) : texture(nullptr), width(0), height(0)
{
    auto decompressed = [](const uint32_t size, const uint32_t* data) -> std::pair<uint32_t, uint8_t*>
    {
        const unsigned int buf_decompressed_size = stb_decompress_length((const unsigned char*)data);
        unsigned char* buf_decompressed_data = new unsigned char[buf_decompressed_size];
        stb_decompress(buf_decompressed_data, (const unsigned char*)data, (unsigned int)size);

        return std::make_pair<uint32_t, uint8_t*>(static_cast<uint32_t>(buf_decompressed_size), static_cast<uint8_t*>(buf_decompressed_data));
    }(size, data);

    int32_t width=0,height=0;

    // load texture from disk into memory
    auto imgbuf = stbi_load_from_memory(decompressed.second, decompressed.first, &width, &height, nullptr, 4);

    if (imgbuf == 0) return;

#ifdef PLATFORM_WINDOWS
    PDIRECT3DTEXTURE9 texture = nullptr;

    // create empty texture
    global_d3d9device->CreateTexture(static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1u, 0u, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &texture, nullptr);

    if (texture == nullptr)
    {
        stbi_image_free(imgbuf);

        delete[] decompressed.second;

        return;
    }

    // write to the texture
    D3DLOCKED_RECT rect;

    texture->LockRect(0, &rect, 0, D3DLOCK_DISCARD);

    auto dest = static_cast<unsigned char*>(rect.pBits);

    memcpy(dest, imgbuf, sizeof(unsigned char) * width * height * 4);

    texture->UnlockRect(0);

    // set texture pointer and width, height
    this->texture = reinterpret_cast<void*>(texture);
    this->width = static_cast<uint32_t>(width);
    this->height = static_cast<uint32_t>(height);
#endif

    stbi_image_free(imgbuf);

    delete[] decompressed.second;
}

/*
@brief

    Destructor, cleans up the texture
*/
retrogames::texture_t::~texture_t()
{
    if (texture == nullptr) return;

#ifdef PLATFORM_WINDOWS
    reinterpret_cast<IDirect3DDevice9*>(texture)->Release();

    texture = nullptr;
#endif
}