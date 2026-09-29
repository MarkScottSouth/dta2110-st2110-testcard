#include "DektecFrameConverter.h"

#include <cstddef>
#include <iostream>
#include <stdexcept>

DektecFrameConverter::DektecFrameConverter(
    int width,
    int height)
{
    if (width <= 0 || height <= 0)
    {
        throw std::invalid_argument(
            "Invalid Dektec image dimensions");
    }

    if ((width % 2) != 0)
    {
        throw std::invalid_argument(
            "UYVY image width must be even");
    }

    m_Image.Init({ width, height });

    const std::size_t expectedPixelSize =
        static_cast<std::size_t>(width) *
        static_cast<std::size_t>(height) *
        3;

    if (m_Image.Pixels.size() != expectedPixelSize)
    {
        throw std::runtime_error(
            "Dektec Image pixel allocation is incorrect");
    }
}

void DektecFrameConverter::Convert(
    const std::vector<uint8_t>& frame,
    uint8_t* output)
{
    if (output == nullptr)
    {
        throw std::invalid_argument(
            "Dektec output pointer is null");
    }

    CopyUyvy8ToImage(frame);

    m_Image.ConvertToUyvy10(output);
}

void DektecFrameConverter::CopyUyvy8ToImage(
    const std::vector<uint8_t>& frame)
{
    const int width = m_Image.b.w;
    const int height = m_Image.b.h;

    if (width <= 0 || height <= 0)
    {
        throw std::runtime_error(
            "Dektec Image dimensions are invalid");
    }

    if ((width % 2) != 0)
    {
        throw std::runtime_error(
            "Dektec Image width must be even");
    }

    const std::size_t expectedFrameSize =
        static_cast<std::size_t>(width) *
        static_cast<std::size_t>(height) *
        2;

    if (frame.size() != expectedFrameSize)
    {
        throw std::runtime_error(
            "Unexpected 8-bit UYVY frame size");
    }

    const std::size_t expectedPixelSize =
        static_cast<std::size_t>(width) *
        static_cast<std::size_t>(height) *
        3;

    if (m_Image.Pixels.size() != expectedPixelSize)
    {
        throw std::runtime_error(
            "Unexpected Dektec Image buffer size");
    }

    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; x += 2)
        {
            const std::size_t sourceIndex =
                (static_cast<std::size_t>(y) *
                    static_cast<std::size_t>(width) +
                    static_cast<std::size_t>(x)) *
                2;

            const uint8_t u =
                frame.at(sourceIndex + 0);

            const uint8_t y0 =
                frame.at(sourceIndex + 1);

            const uint8_t v =
                frame.at(sourceIndex + 2);

            const uint8_t y1 =
                frame.at(sourceIndex + 3);

            const std::size_t firstPixelIndex =
                (static_cast<std::size_t>(y) *
                    static_cast<std::size_t>(width) +
                    static_cast<std::size_t>(x)) *
                3;

            const std::size_t secondPixelIndex =
                firstPixelIndex + 3;

            // Image format is Y, U, V per pixel.
            m_Image.Pixels.at(firstPixelIndex + 0) = y0;
            m_Image.Pixels.at(firstPixelIndex + 1) = u;
            m_Image.Pixels.at(firstPixelIndex + 2) = v;

            m_Image.Pixels.at(secondPixelIndex + 0) = y1;
            m_Image.Pixels.at(secondPixelIndex + 1) = u;
            m_Image.Pixels.at(secondPixelIndex + 2) = v;
        }
    }
}

void DektecFrameConverter::PrintDebugState() const
{
    std::cout
        << "DektecFrameConverter state\n";

    std::cout
        << "Image width: "
        << m_Image.b.w
        << '\n';

    std::cout
        << "Image height: "
        << m_Image.b.h
        << '\n';

    std::cout
        << "Pixels size: "
        << m_Image.Pixels.size()
        << '\n';

    std::cout
        << "Pixels capacity: "
        << m_Image.Pixels.capacity()
        << '\n';

    std::cout
        << "Pixels data address: "
        << static_cast<const void*>(
            m_Image.Pixels.data())
        << '\n';
}
