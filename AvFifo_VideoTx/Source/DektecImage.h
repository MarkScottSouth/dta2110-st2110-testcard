#pragma once

#include <cstdint>
#include <vector>

// Simple width/height structure used by Image.
struct Size
{
    int w;
    int h;
};

// Intermediate 8-bit YUV image used by the DekTec
// 10-bit packing conversion.
//
// Each pixel occupies three bytes:
//     Y, U, V
//
// DektecFrameConverter populates this image from the
// renderer's 8-bit UYVY frame before ConvertToUyvy10()
// packs it into the format required by the DekTec FIFO.
struct Image
{
    Size b{};

    std::vector<uint8_t> Pixels;

    void Init(Size size)
    {
        b = size;

        Pixels.resize(
            static_cast<std::size_t>(b.w) *
            static_cast<std::size_t>(b.h) *
            3);
    }

    void ConvertToUyvy10(
        uint8_t* output);
};