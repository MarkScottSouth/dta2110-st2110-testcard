#include "DektecImage.h"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <stdexcept>


void Image::ConvertToUyvy10(
    uint8_t* output)
{
    if (output == nullptr)
    {
        throw std::runtime_error(
            "ConvertToUyvy10 received a null output pointer");
    }

    if (b.w <= 0 || b.h <= 0)
    {
        throw std::runtime_error(
            "ConvertToUyvy10 received invalid image dimensions");
    }

    if ((b.w % 2) != 0)
    {
        throw std::runtime_error(
            "ConvertToUyvy10 requires an even image width");
    }

    const std::size_t expectedInputSize =
        static_cast<std::size_t>(b.w) *
        static_cast<std::size_t>(b.h) *
        3;

    if (Pixels.size() != expectedInputSize)
    {
        throw std::runtime_error(
            "ConvertToUyvy10 input buffer size mismatch");
    }

    const std::size_t expectedOutputSize =
        static_cast<std::size_t>(b.w) *
        static_cast<std::size_t>(b.h) *
        5 / 2;

    std::size_t inputOffset = 0;
    std::size_t outputOffset = 0;

    for (int y = 0; y < b.h; ++y)
    {
        for (int x = 0; x < b.w; x += 2)
        {
            if ((inputOffset + 6) >
                Pixels.size())
            {
                throw std::runtime_error(
                    "ConvertToUyvy10 input buffer overrun");
            }

            if ((outputOffset + 5) >
                expectedOutputSize)
            {
                throw std::runtime_error(
                    "ConvertToUyvy10 output buffer overrun");
            }

            const uint8_t* yuv8 =
                Pixels.data() + inputOffset;

            uint32_t value{ 0 };

            value =
                static_cast<uint32_t>(
                    yuv8[1] + yuv8[4]) << 1;

            value |=
                static_cast<uint32_t>(
                    yuv8[0]) << 12;

            value |=
                static_cast<uint32_t>(
                    yuv8[2] + yuv8[5]) << 21;

            std::memcpy(
                output + outputOffset,
                &value,
                sizeof(value));

            output[outputOffset + 4] =
                yuv8[3];

            inputOffset += 6;
            outputOffset += 5;
        }
    }

    if (inputOffset != expectedInputSize)
    {
        throw std::runtime_error(
            "ConvertToUyvy10 did not consume "
            "the expected input size");
    }

    if (outputOffset != expectedOutputSize)
    {
        throw std::runtime_error(
            "ConvertToUyvy10 did not produce "
            "the expected output size");
    }
}