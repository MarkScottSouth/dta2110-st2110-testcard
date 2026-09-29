#pragma once

#include <cstdint>
#include <vector>

#include "DektecImage.h"

class DektecFrameConverter
{
public:
    DektecFrameConverter(
        int width,
        int height);

    DektecFrameConverter(
        const DektecFrameConverter&) = delete;

    DektecFrameConverter& operator=(
        const DektecFrameConverter&) = delete;

    void Convert(
        const std::vector<uint8_t>& frame,
        uint8_t* output);

    void PrintDebugState() const;

private:
    Image m_Image;

    void CopyUyvy8ToImage(
        const std::vector<uint8_t>& frame);
};
