#include "UYVYFrameGenerator.h"
#include <algorithm>
#include <string>
#include <cstddef>
#include <cstring>

#include "Font5x7.h"
#include "DektecLogoData.h"

#include "DektecFrameConverter.h"

#include <iostream>
#include <stdexcept>


UYVYFrameGenerator::UYVYFrameGenerator(int width, int height, double fps)
    : W(width),
    H(height),
    FPS(fps),
    uyvy(width* height * 2)
{
}

const std::vector<uint8_t>& UYVYFrameGenerator::renderBasePattern(
    TestPattern pattern)
{
    switch (pattern)
    {
    case TestPattern::SMPTEBars:
        fillSMPTEBars();
        break;

    case TestPattern::GrayRamp:
        fillGrayRamp();
        break;

    case TestPattern::SolidBlue:
        fillSolidBlue();
        break;

    case TestPattern::Checkerboard:
        fillCheckerboard();
        break;
    }

    return uyvy;
}

const std::vector<uint8_t>& UYVYFrameGenerator::renderFromBase(
    const std::vector<uint8_t>& baseFrame,
    const std::string& label,
    int boxX,
    int boxY)
{
    uyvy = baseFrame;

    drawDektecLogo();
    drawBouncingBox(boxX, boxY);
    drawLabelPlaceholder(label);

    ++frameNumber;

    return uyvy;
}

void UYVYFrameGenerator::setPixelPair(int x, int y, YUV p0, YUV p1)
{
    if (x < 0 || x + 1 >= W || y < 0 || y >= H)
        return;

    if (x & 1)
        x--;

    std::size_t offset =
        static_cast<std::size_t>(y) * static_cast<std::size_t>(W) * 2 +
        static_cast<std::size_t>(x) * 2;

    uint8_t U = static_cast<uint8_t>(
        (static_cast<int>(p0.u) + static_cast<int>(p1.u)) / 2);

    uint8_t V = static_cast<uint8_t>(
        (static_cast<int>(p0.v) + static_cast<int>(p1.v)) / 2);

    uyvy[offset + 0] = U;
    uyvy[offset + 1] = p0.y;
    uyvy[offset + 2] = V;
    uyvy[offset + 3] = p1.y;
}

void UYVYFrameGenerator::fillSMPTEBars()
{
    const YUV bars[7] =
    {
        {235, 128, 128},
        {210,  16, 146},
        {170, 166,  16},
        {145,  54,  34},
        {106, 202, 222},
        { 81,  90, 240},
        { 41, 240, 110}
    };

    const int barHeight = (H * 2) / 3;

    for (int y = 0; y < barHeight; ++y)
    {
        for (int x = 0; x < W; x += 2)
        {
            int bar = (x * 7) / W;
            if (bar > 6) bar = 6;

            setPixelPair(x, y, bars[bar], bars[bar]);
        }
    }

    for (int y = barHeight; y < H; ++y)
    {
        for (int x = 0; x < W; x += 2)
        {
            uint8_t y0 =
                static_cast<uint8_t>(16 + ((x * 219) / (W - 1)));

            uint8_t y1 =
                static_cast<uint8_t>(16 + (((x + 1) * 219) / (W - 1)));

            setPixelPair(
                x,
                y,
                YUV{ y0, 128, 128 },
                YUV{ y1, 128, 128 });
        }
    }
}

void UYVYFrameGenerator::fillGrayRamp()
{
    for (int y = 0; y < H; ++y)
    {
        for (int x = 0; x < W; x += 2)
        {
            uint8_t y0 =
                static_cast<uint8_t>(16 + ((x * 219) / (W - 1)));

            uint8_t y1 =
                static_cast<uint8_t>(16 + (((x + 1) * 219) / (W - 1)));

            setPixelPair(
                x,
                y,
                YUV{ y0, 128, 128 },
                YUV{ y1, 128, 128 });
        }
    }
}

void UYVYFrameGenerator::fillSolidBlue()
{
    YUV blue{ 41, 240, 110 };

    for (int y = 0; y < H; ++y)
    {
        for (int x = 0; x < W; x += 2)
        {
            setPixelPair(x, y, blue, blue);
        }
    }
}

void UYVYFrameGenerator::fillCheckerboard()
{
    const int square = 80;

    YUV white{ 235, 128, 128 };
    YUV black{ 16, 128, 128 };

    for (int y = 0; y < H; ++y)
    {
        for (int x = 0; x < W; x += 2)
        {
            bool isWhite =
                ((x / square) + (y / square)) % 2 == 0;

            YUV colour = isWhite ? white : black;

            setPixelPair(x, y, colour, colour);
        }
    }
}

void UYVYFrameGenerator::drawRect(
    int x0,
    int y0,
    int w,
    int h,
    YUV colour)
{
    int x1 = x0 + w;
    int y1 = y0 + h;

    if (x0 < 0) x0 = 0;
    if (y0 < 0) y0 = 0;
    if (x1 > W) x1 = W;
    if (y1 > H) y1 = H;

    if (x0 & 1)
        x0--;

    for (int y = y0; y < y1; ++y)
    {
        for (int x = x0; x < x1; x += 2)
        {
            setPixelPair(x, y, colour, colour);
        }
    }
}

void UYVYFrameGenerator::drawBouncingBox(int boxX, int boxY)
{
    YUV blue{ 41, 240, 110 };
    YUV white{ 235, 128, 128 };

    drawRect(boxX, boxY, boxSize, boxSize, blue);

    drawRect(boxX, boxY, boxSize, 4, white);
    drawRect(boxX, boxY + boxSize - 4, boxSize, 4, white);
    drawRect(boxX, boxY, 4, boxSize, white);
    drawRect(boxX + boxSize - 4, boxY, 4, boxSize, white);
}

void UYVYFrameGenerator::drawLabelPlaceholder(
    const std::string& label)
{
    YUV black{ 16, 128, 128 };
    YUV white{ 235, 128, 128 };

    const int x = 32;
    const int y = 32;
    const int w = 220;
    const int h = 40;

    drawRect(x, y, w, h, black);

    drawText(
        x + 10,
        y + 8,
        label,
        3,
        white);
}

void UYVYFrameGenerator::drawDektecLogo()
{
    constexpr int rightMargin = 40;
    constexpr int topMargin = 40;

    int logoX =
        W - DektecLogo::Width - rightMargin;

    const int logoY = topMargin;

    // UYVY 4:2:2 works in two-pixel groups, so keep X even.
    if (logoX & 1)
        --logoX;

    if (logoX < 0 ||
        logoY < 0 ||
        logoX + DektecLogo::Width > W ||
        logoY + DektecLogo::Height > H)
    {
        return;
    }

    for (int y = 0;
         y < DektecLogo::Height;
         ++y)
    {
        const std::size_t destinationOffset =
            static_cast<std::size_t>(logoY + y) *
                static_cast<std::size_t>(W) * 2 +
            static_cast<std::size_t>(logoX) * 2;

        const std::size_t sourceOffset =
            static_cast<std::size_t>(y) *
            DektecLogo::RowBytes;

        std::memcpy(
            uyvy.data() + destinationOffset,
            DektecLogo::Uyvy.data() + sourceOffset,
            DektecLogo::RowBytes);
    }
}

void UYVYFrameGenerator::drawCharacter(
    int x,
    int y,
    char character,
    int scale,
    YUV colour)
{
    if (character < 32 || character > 127)
        character = '?';

    const uint8_t* glyph =
        Font5x7[static_cast<unsigned char>(character) - 32];

    for (int column = 0; column < 5; ++column)
    {
        uint8_t bits = glyph[column];

        for (int row = 0; row < 7; ++row)
        {
            if (bits & (1u << row))
            {
                drawRect(
                    x + column * scale,
                    y + row * scale,
                    scale,
                    scale,
                    colour);
            }
        }
    }
}
void UYVYFrameGenerator::drawText(
    int x,
    int y,
    const std::string& text,
    int scale,
    YUV colour)
{
    int cursorX = x;

    for (char character : text)
    {
        drawCharacter(
            cursorX,
            y,
            character,
            scale,
            colour);

        cursorX += 6 * scale;
    }
}