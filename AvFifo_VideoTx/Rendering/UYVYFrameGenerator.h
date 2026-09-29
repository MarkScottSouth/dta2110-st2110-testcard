#pragma once

#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>

enum class TestPattern
{
    SMPTEBars,
    GrayRamp,
    SolidBlue,
    Checkerboard
};

struct YUV
{
    uint8_t y;
    uint8_t u;
    uint8_t v;
};

class UYVYFrameGenerator
{
public:
    UYVYFrameGenerator(int width, int height, double fps);

    const std::vector<uint8_t>& renderBasePattern(TestPattern pattern);

    const std::vector<uint8_t>& renderFromBase(
        const std::vector<uint8_t>& baseFrame,
        const std::string& label,
        int boxX,
        int boxY);

    int width() const { return W; }
    int height() const { return H; }
    double fps() const { return FPS; }
    std::size_t frameSizeBytes() const { return uyvy.size(); }

private:
    int W;
    int H;
    double FPS;

    std::vector<uint8_t> uyvy;
    uint64_t frameNumber = 0;

    int boxSize = 96;

    void fillSMPTEBars();
    void fillGrayRamp();
    void fillSolidBlue();
    void fillCheckerboard();

    void drawBouncingBox(int boxX, int boxY);
    void drawLabelPlaceholder(const std::string& label);

    // Draws the embedded DekTec logo onto the current UYVY frame.
    // This is called while preparing the static channel backgrounds,
    // so it has no cost in the real-time transmit loop.
    void drawDektecLogo();

    void setPixelPair(int x, int y, YUV p0, YUV p1);
    void drawRect(int x0, int y0, int w, int h, YUV colour);

    void drawCharacter(
        int x,
        int y,
        char character,
        int scale,
        YUV colour);

    void drawText(
        int x,
        int y,
        const std::string& text,
        int scale,
        YUV colour);
};
