#pragma once

#include "DTAPI_AvFifo.h"

#include <array>
#include <cstdint>
#include <string>


// ------------------------------------------------------------
// Device configuration
// ------------------------------------------------------------

// Device type: 2110 = DTA-2110, 2125 = DTA-2125
constexpr int CfgDeviceType{ 2110 };

// Device number: 0 = first device in system
constexpr int CfgDeviceNo{ 0 };


// ------------------------------------------------------------
// Video configuration
// ------------------------------------------------------------

constexpr int FrameWidth{ 1920 };
constexpr int FrameHeight{ 1080 };

constexpr double FramesPerSecond{ 50.0 };

constexpr std::int64_t FramePeriodNs{
    20'000'000
};


// ------------------------------------------------------------
// Channel configuration
// ------------------------------------------------------------

struct ChannelConfig
{
    const char* Label;

    AvFifo::IpPars IpPars;
};


constexpr std::size_t NumChannels{ 4 };


const std::array<ChannelConfig, NumChannels>
Channels
{ {
    {
        "Channel 1",
        {
            .IpAddr{239, 10, 10, 10},
            .IpVersion{
                AvFifo::IpProtocolVersion::IPv4
            },
            .Port{5000},
            .RtpPayloadType{96},
            .TransportProtocol{
                AvFifo::IpTransportProtocol::Rtp
            }
        }
    },

    {
        "Channel 2",
        {
            .IpAddr{239, 10, 10, 11},
            .IpVersion{
                AvFifo::IpProtocolVersion::IPv4
            },
            .Port{5000},
            .RtpPayloadType{96},
            .TransportProtocol{
                AvFifo::IpTransportProtocol::Rtp
            }
        }
    },

    {
        "Channel 3",
        {
            .IpAddr{239, 10, 10, 12},
            .IpVersion{
                AvFifo::IpProtocolVersion::IPv4
            },
            .Port{5000},
            .RtpPayloadType{96},
            .TransportProtocol{
                AvFifo::IpTransportProtocol::Rtp
            }
        }
    },

    {
        "Channel 4",
        {
            .IpAddr{239, 10, 10, 13},
            .IpVersion{
                AvFifo::IpProtocolVersion::IPv4
            },
            .Port{5000},
            .RtpPayloadType{96},
            .TransportProtocol{
                AvFifo::IpTransportProtocol::Rtp
            }
        }
    }
} };

// Temporary compatibility with the existing single-channel transmitter.
// Channel 0 is used until AvFifo_VideoTx.cpp is converted to multi-channel operation.
const AvFifo::IpPars& AvFifo_VideoTx_IpPars =
Channels[0].IpPars;


// ------------------------------------------------------------
// SMPTE 2110-20 format
// ------------------------------------------------------------

const AvFifo::St2110::TxConfigVideo
AvFifo_VideoTx_Config
{
    .Format{
        AvFifo::St2110::TxFrameFormat::Uyvy422_10b
    },

    .Packing{
        false,
        AvFifo::St2110::PackingMode::General,
        -1
    },

    .Resolution{
        FrameWidth,
        FrameHeight
    },

    .Timing{
        {50, 1},
        AvFifo::St2110::Scheduling::Linear,
        AvFifo::St2110::VideoScanning::Progressive
    }
};


