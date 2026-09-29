// *#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#* AvFifo_VideoTx.cpp *#*#*#*#*#*#*
//
// DekTec AvFifo SMPTE 2110-20 transmitter using the L2tek UYVY test-card renderer.
//
// This version prepares one separately identified test card per channel at startup.
// It also prepares one packed 96x96 box patch at startup.
//
// The real-time transmit loop:
//   1. copies each channel's pre-packed static ident into the FIFO frame;
//   2. stamps the same small moving box patch onto all four frames;
//   3. transmits them.
//
// No full-frame rendering or UYVY8->UYVY10 conversion occurs in the real-time loop.

#include "Configuration.h"
#include "UYVYFrameGenerator.h"
#include "DektecFrameConverter.h"
#include "DTAPI_AvFifo.h"
#include "DektecImage.h"

#include <array>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

const char* Version{ "Aug2026-4Ident-BouncingBox" };

using namespace std;
using namespace chrono_literals;

void AvFifo_VideoTx();
void VideoTxLoop(DtDevice& Device);

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    cout
        << "\n"
        << "--------------------------------------------------------------------------\n"
        << "DekTec AvFifo - SMPTE 2110-20 four-channel test-card transmitter\n"
        << "Version " << Version << "\n"
        << "--------------------------------------------------------------------------\n\n"
        << "DTA-" << CfgDeviceType << ": " << CfgDeviceNo << "\n"
        << "Video format: 10-bit packed 4:2:2 UYVY, "
        << FrameWidth << "x" << FrameHeight << " at 50 fps\n"
        << "Configured channels: " << NumChannels << "\n\n"
        << "Close this window to stop transmission.\n";

    try
    {
        AvFifo_VideoTx();
    }
    catch (const AvFifo::UsageError& e)
    {
        cerr << "Usage error: " << e.what() << '\n';
    }
    catch (const AvFifo::DriverError& e)
    {
        cerr << "Driver error: " << e.what() << '\n';
    }
    catch (const exception& e)
    {
        cerr << "Exception: " << e.what() << '\n';
    }
    catch (...)
    {
        cerr << "Caught an unknown exception\n";
    }

    this_thread::sleep_for(3000ms);
    return 0;
}

void AvFifo_VideoTx()
{
    DtDevice Device{};

    DTAPI_RESULT result =
        Device.AttachToType(
            CfgDeviceType,
            CfgDeviceNo);

    if (result != DTAPI_OK)
    {
        throw runtime_error(
            "Attach failed: " +
            string(DtapiResult2Str(result)));
    }

    try
    {
        result =
            Device.IsNetworkCardOperational(
                1,
                Channels[0].IpPars.IpVersion ==
                AvFifo::IpProtocolVersion::IPv4,
                Channels[0].IpPars.IpVersion ==
                AvFifo::IpProtocolVersion::IPv6);

        if (result != DTAPI_OK)
        {
            throw runtime_error(
                "Networking error: " +
                string(DtapiResult2Str(result)));
        }

        VideoTxLoop(Device);
    }
    catch (...)
    {
        Device.Detach();
        throw;
    }

    Device.Detach();
}

const std::size_t frameSize =
static_cast<std::size_t>(FrameWidth) *
static_cast<std::size_t>(FrameHeight) *
20 / 8;

UYVYFrameGenerator generator(
    FrameWidth,
    FrameHeight,
    FramesPerSecond);

DektecFrameConverter converter(
    FrameWidth,
    FrameHeight);

const std::vector<uint8_t> baseFrame =
generator.renderBasePattern(
    TestPattern::SMPTEBars);

void VideoTxLoop(DtDevice& Device)
{
    std::array<AvFifo::TxFifo, NumChannels> videoTxFifos{};

    std::array<bool, NumChannels> fifoAttached{};
    std::array<bool, NumChannels> fifoStarted{};

    std::array<std::vector<uint8_t>, NumChannels> packedFrames;

    // The same small packed box image is stamped onto every channel at runtime.
    constexpr int BoxSize = 96;
    constexpr int BoxPatchSourceX = 100;
    constexpr int BoxPatchSourceY = 100;

    // Keep the box away from the visible image while preparing the static idents.
    constexpr int OffscreenBoxX = FrameWidth + BoxSize;
    constexpr int OffscreenBoxY = FrameHeight + BoxSize;

    const std::size_t packedRowBytes =
        static_cast<std::size_t>(FrameWidth) * 5 / 2;

    const std::size_t boxPatchRowBytes =
        static_cast<std::size_t>(BoxSize) * 5 / 2;

    std::vector<uint8_t> boxPatch(
        boxPatchRowBytes *
        static_cast<std::size_t>(BoxSize));

    // Common box animation state.
    int boxX = 100;
    int boxY = 100;
    int boxVX = 6;
    int boxVY = 4;

    try
    {
        const std::size_t expectedUyvy8Size =
            static_cast<std::size_t>(FrameWidth) *
            static_cast<std::size_t>(FrameHeight) *
            2;

        const std::size_t expectedUyvy10Size =
            static_cast<std::size_t>(FrameWidth) *
            static_cast<std::size_t>(FrameHeight) *
            5 / 2;

        if (baseFrame.size() != expectedUyvy8Size)
        {
            throw std::runtime_error(
                "Base frame has an unexpected UYVY8 size");
        }

        if (frameSize != expectedUyvy10Size)
        {
            throw std::runtime_error(
                "Configured packed UYVY10 frame size is unexpected");
        }

        cout << "\nPreparing channel test cards...\n";

        // ---------------------------------------------------------------------
        // Prepare four static idented backgrounds.
        //
        // renderFromBase() normally also draws the box. For these static
        // backgrounds the box coordinates are deliberately placed off-screen.
        // ---------------------------------------------------------------------

        for (std::size_t ch = 0;
            ch < NumChannels;
            ++ch)
        {
            packedFrames[ch].resize(frameSize);

            const std::vector<uint8_t>& testFrame =
                generator.renderFromBase(
                    baseFrame,
                    Channels[ch].Label,
                    OffscreenBoxX,
                    OffscreenBoxY);

            if (testFrame.size() != expectedUyvy8Size)
            {
                throw std::runtime_error(
                    "Renderer returned an unexpected UYVY8 frame size "
                    "while preparing channel " +
                    std::to_string(ch + 1));
            }

            converter.Convert(
                testFrame,
                packedFrames[ch].data());

            cout
                << "Prepared channel "
                << ch + 1
                << ": "
                << Channels[ch].Label
                << '\n';
        }

        // ---------------------------------------------------------------------
        // Prepare one packed box patch.
        //
        // We let the existing renderer draw the box once, convert that frame
        // once, and extract only the 96x96 packed region. This preserves the
        // exact box appearance without needing any UYVY10 packing code here.
        // ---------------------------------------------------------------------

        std::vector<uint8_t> packedBoxSource(frameSize);

        const std::vector<uint8_t>& boxSourceFrame =
            generator.renderFromBase(
                baseFrame,
                "",
                BoxPatchSourceX,
                BoxPatchSourceY);

        if (boxSourceFrame.size() != expectedUyvy8Size)
        {
            throw std::runtime_error(
                "Renderer returned an unexpected UYVY8 frame size "
                "while preparing the moving box patch");
        }

        converter.Convert(
            boxSourceFrame,
            packedBoxSource.data());

        for (int y = 0; y < BoxSize; ++y)
        {
            const std::size_t sourceOffset =
                static_cast<std::size_t>(BoxPatchSourceY + y) *
                packedRowBytes +
                static_cast<std::size_t>(BoxPatchSourceX / 2) * 5;

            const std::size_t patchOffset =
                static_cast<std::size_t>(y) *
                boxPatchRowBytes;

            std::memcpy(
                boxPatch.data() + patchOffset,
                packedBoxSource.data() + sourceOffset,
                boxPatchRowBytes);
        }

        cout
            << "Prepared common moving-box patch: "
            << BoxSize
            << "x"
            << BoxSize
            << " pixels\n";

        cout << "\nConfiguring transmit channels...\n";

        for (std::size_t ch = 0;
            ch < NumChannels;
            ++ch)
        {
            videoTxFifos[ch].Attach(
                Device,
                1,
                AvFifo::HwOrSwPipe::PreferHwPipe);

            fifoAttached[ch] = true;

            videoTxFifos[ch].SetIpPars(
                Channels[ch].IpPars);

            videoTxFifos[ch].Configure(
                AvFifo_VideoTx_Config);

            cout
                << "Configured channel "
                << ch + 1
                << ": "
                << Channels[ch].Label
                << " -> "
                << static_cast<int>(Channels[ch].IpPars.IpAddr[0]) << "."
                << static_cast<int>(Channels[ch].IpPars.IpAddr[1]) << "."
                << static_cast<int>(Channels[ch].IpPars.IpAddr[2]) << "."
                << static_cast<int>(Channels[ch].IpPars.IpAddr[3])
                << ":"
                << Channels[ch].IpPars.Port
                << '\n';
        }

        DtTimeOfDay ToD{};

        Device.GetTimeOfDay(ToD);
        ToD += 500'000'000;

        for (std::size_t ch = 0;
            ch < NumChannels;
            ++ch)
        {
            if (!videoTxFifos[ch].Start())
            {
                throw std::runtime_error(
                    "Failed to start FIFO for channel " +
                    std::to_string(ch + 1));
            }

            fifoStarted[ch] = true;

            cout
                << "Started channel "
                << ch + 1
                << '\n';
        }

        cout
            << "All "
            << NumChannels
            << " transmissions started\n";

        std::uint64_t frameCount = 0;
        bool quit = false;

        while (!quit)
        {
            bool fifoReady = true;

            for (std::size_t ch = 0;
                ch < NumChannels;
                ++ch)
            {
                const AvFifo::FifoStatus status =
                    videoTxFifos[ch].GetStatus();

                if (status != AvFifo::FifoStatus::Ok)
                {
                    cerr
                        << "FIFO failure on channel "
                        << ch + 1
                        << ": "
                        << AvFifo::FifoStatusToMessage(status)
                        << '\n';

                    quit = true;
                    fifoReady = false;
                    break;
                }

                if (videoTxFifos[ch].GetFifoLoad() >=
                    videoTxFifos[ch].GetMaxSize())
                {
                    fifoReady = false;
                }
            }

            if (quit)
            {
                continue;
            }

            if (!fifoReady)
            {
                this_thread::sleep_for(1ms);
                continue;
            }

            ToD = AvFifo::Tod2Grid_Video(
                ToD,
                { 50, 1 });

            // -------------------------------------------------------------
            // Advance one common box position for all four channels.
            //
            // boxX remains even (100 start, +/-6 movement), which is important
            // because packed UYVY10 stores two pixels in each 5-byte group.
            // -------------------------------------------------------------

            boxX += boxVX;
            boxY += boxVY;

            if (boxX < 0)
            {
                boxX = 0;
                boxVX = -boxVX;
            }
            else if (boxX > FrameWidth - BoxSize)
            {
                boxX = FrameWidth - BoxSize;
                boxVX = -boxVX;
            }

            if (boxY < 0)
            {
                boxY = 0;
                boxVY = -boxVY;
            }
            else if (boxY > FrameHeight - BoxSize)
            {
                boxY = FrameHeight - BoxSize;
                boxVY = -boxVY;
            }

            const auto processingStart =
                std::chrono::high_resolution_clock::now();

            for (std::size_t ch = 0;
                ch < NumChannels;
                ++ch)
            {
                AvFifo::Frame* frame =
                    videoTxFifos[ch].GetFrameFromMemPool(
                        frameSize);

                if (frame == nullptr)
                {
                    throw std::runtime_error(
                        "Failed to allocate transmit frame "
                        "for channel " +
                        std::to_string(ch + 1));
                }

                frame->ToD = ToD;

                frame->RtpTime =
                    AvFifo::St2110::Tod2Rtp_Video(
                        ToD);

                // Start every transmitted frame from the channel's clean,
                // separately identified static background.
                std::memcpy(
                    frame->Data(),
                    packedFrames[ch].data(),
                    packedFrames[ch].size());

                // Stamp the small moving box patch onto the packed frame.
                // Only 96 x 96 pixels are touched rather than reconverting
                // the complete 1920 x 1080 image.
                for (int y = 0; y < BoxSize; ++y)
                {
                    const std::size_t destinationOffset =
                        static_cast<std::size_t>(boxY + y) *
                        packedRowBytes +
                        static_cast<std::size_t>(boxX / 2) * 5;

                    const std::size_t patchOffset =
                        static_cast<std::size_t>(y) *
                        boxPatchRowBytes;

                    std::memcpy(
                        frame->Data() + destinationOffset,
                        boxPatch.data() + patchOffset,
                        boxPatchRowBytes);
                }

                frame->NumValidBytes =
                    static_cast<int>(
                        packedFrames[ch].size());

                videoTxFifos[ch].Write(frame);
            }

            const auto processingEnd =
                std::chrono::high_resolution_clock::now();

            const double processingMs =
                std::chrono::duration<double, std::milli>(
                    processingEnd -
                    processingStart).count();

            ++frameCount;

            ToD += FramePeriodNs;

            if ((frameCount % 250) == 0)
            {
                cout
                    << "Frames written: "
                    << frameCount
                    << ", box: "
                    << boxX
                    << ","
                    << boxY
                    << ", copy+box processing: "
                    << processingMs
                    << " ms";

                for (std::size_t ch = 0;
                    ch < NumChannels;
                    ++ch)
                {
                    cout
                        << ", FIFO"
                        << ch + 1
                        << ": "
                        << videoTxFifos[ch].GetFifoLoad()
                        << "/"
                        << videoTxFifos[ch].GetMaxSize();
                }

                cout << '\n';
            }
        }
    }
    catch (...)
    {
        for (std::size_t ch = NumChannels;
            ch-- > 0;)
        {
            if (fifoStarted[ch])
            {
                videoTxFifos[ch].Stop();
                fifoStarted[ch] = false;
            }

            if (fifoAttached[ch])
            {
                videoTxFifos[ch].Detach();
                fifoAttached[ch] = false;
            }
        }

        throw;
    }

    for (std::size_t ch = NumChannels;
        ch-- > 0;)
    {
        if (fifoStarted[ch])
        {
            videoTxFifos[ch].Stop();
        }

        if (fifoAttached[ch])
        {
            videoTxFifos[ch].Detach();
        }
    }
}