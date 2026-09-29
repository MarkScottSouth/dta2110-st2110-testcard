# DTA-2110 ST 2110 Test Card Transmitter

A four-channel SMPTE ST 2110-20 test-card transmitter for the **DekTec
DTA-2110**, built with the DekTec DTAPI/AvFifo API.

This project was developed by **L2Tek** as part of an ST 2110
demonstration system for IBC 2026. It began with the DekTec AvFifo
example code and was extended with a software test-pattern renderer,
four independently identified multicast outputs, pre-rendered 10-bit
video frames, and a lightweight animated overlay.

## What it does

The application generates four simultaneous:

-   1920 × 1080 progressive video streams
-   50 frames/s
-   YUV 4:2:2 10-bit video
-   SMPTE ST 2110-20 multicast outputs
-   independently labelled channels
-   independently configurable multicast destinations
-   common animated 96 × 96 bouncing box to make it obvious that the
    streams are live

The supplied renderer also supports several base patterns:

-   SMPTE colour bars
-   grey ramp
-   solid blue
-   checkerboard

## Architecture

``` text
Configuration
     |
     +---- multicast addresses / ports
     +---- channel labels
     |
     v
UYVYFrameGenerator
     |
     +---- base test pattern
     +---- Channel 1 / 2 / 3 / 4 ident
     |
     v
Four static UYVY8 images
     |
     v
DektecFrameConverter
     |
     v
Four packed UYVY10 backgrounds
     |
     +---- shared 96 x 96 moving-box patch
     |
     v
4 x AvFifo TxFifo
     |
     v
DekTec DTA-2110
     |
     v
4 x SMPTE ST 2110-20 multicast streams
```

## Why the frames are pre-rendered

An early four-channel version independently rendered and converted a
complete 1920 × 1080 image for every channel on every frame.

At 50 fps there is only a 20 ms frame period. Rendering and converting
four complete images took approximately 19--23 ms, eventually allowing
the transmit FIFOs to drain.

The current implementation instead:

1.  generates each channel's labelled test card once at startup;
2.  converts each complete image to packed 10-bit UYVY once;
3.  retains the four packed frames as static backgrounds;
4.  copies the appropriate background into each DekTec FIFO frame;
5.  adds only a small pre-generated moving-box patch at runtime.

On the development system this reduced live processing to approximately
**1.7--2.2 ms per four-channel frame cycle**, with approximately **4%
CPU utilisation** and **200 MB memory usage** observed during testing.

These figures describe the development system and should not be
interpreted as guaranteed performance on other hardware.

## DekTec AvFifo flow

At a high level the transmit side follows this sequence:

``` cpp
Device.AttachToType(...);

fifo.Attach(
    Device,
    1,
    AvFifo::HwOrSwPipe::PreferHwPipe);

fifo.SetIpPars(...);
fifo.Configure(...);
fifo.Start();

Device.GetTimeOfDay(ToD);

while (running)
{
    ToD = AvFifo::Tod2Grid_Video(ToD, { 50, 1 });

    AvFifo::Frame* frame =
        fifo.GetFrameFromMemPool(frameSize);

    frame->ToD = ToD;
    frame->RtpTime =
        AvFifo::St2110::Tod2Rtp_Video(ToD);

    std::memcpy(
        frame->Data(),
        packedVideo,
        frameSize);

    frame->NumValidBytes = frameSize;

    fifo.Write(frame);

    ToD += 20'000'000;
}

fifo.Stop();
fifo.Detach();
Device.Detach();
```

The application uses `AvFifo::HwOrSwPipe::PreferHwPipe` so that DTAPI
can select an appropriate hardware or software pipe. This was important
when running four outputs on hardware with fewer available hardware Tx
pipes than requested streams.

## Video conversion

The renderer works internally in 8-bit UYVY 4:2:2 because that makes
drawing test patterns, text and simple graphics straightforward.

Before transmission, `DektecFrameConverter` converts the generated frame
into the packed 10-bit UYVY representation required by the transmit
path.

The expensive full-frame conversion is performed during startup rather
than in the real-time transmit loop.

## Project layout

``` text
AvFifo_VideoTx/
|
+-- Rendering/
|   +-- Channel.h
|   +-- Font5x7.cpp
|   +-- Font5x7.h
|   +-- UYVYFrameGenerator.cpp
|   +-- UYVYFrameGenerator.h
|
+-- Source/
|   +-- AvFifo_VideoTx.cpp
|   +-- Configuration.h
|   +-- DektecFrameConverter.cpp
|   +-- DektecFrameConverter.h
|   +-- DektecImage.cpp
|   +-- DektecImage.h
|
+-- AvFifo_VideoTx.vcxproj
```

The repository also contains the Visual Studio solution file.

## Requirements

This project was developed on Windows using Microsoft Visual Studio and
requires:

-   a compatible DekTec ST 2110 network interface;
-   the DekTec Windows SDK;
-   DTAPI / AvFifo headers and libraries;
-   the corresponding DekTec device drivers.

The DekTec SDK itself is **not distributed by this repository**. Obtain
the current SDK and drivers directly from DekTec.

## Building

1.  Install the DekTec Windows SDK and the required device driver.
2.  Open `Dektec_Test_Card.sln` in Visual Studio.
3.  Check the DekTec SDK include/library paths expected by the project.
4.  Review `AvFifo_VideoTx/Source/Configuration.h` and set the required
    network and multicast parameters.
5.  Build the **Release** configuration.
6.  Run the application with a compatible DekTec interface.

The application was designed and performance-tested primarily as a
Release build. Debug builds may not maintain the required real-time
timing.

## Network configuration

Each channel has its own label and multicast destination. Example
addresses used during development included:

``` text
Channel 1 -> 239.10.10.10:5000
Channel 2 -> 239.10.10.11:5000
Channel 3 -> 239.10.10.12:5000
Channel 4 -> 239.10.10.13:5000
```

These are examples only. Adjust `Configuration.h` for the network on
which the application is deployed.

## Timing

For 1080p50, the application advances the presentation time by:

``` cpp
20'000'000   // nanoseconds = 20 ms
```

Frames are aligned to the video timing grid using:

``` cpp
AvFifo::Tod2Grid_Video(...)
```

and the RTP timestamp is derived using:

``` cpp
AvFifo::St2110::Tod2Rtp_Video(...)
```

Keeping the FIFOs supplied sufficiently ahead of transmission time is
essential. During development, FIFO load monitoring proved particularly
useful for detecting processing that could not sustain the 20 ms frame
period.

## Status

The IBC 2026 development version successfully produced four
independently identified 1080p50 ST 2110-20 multicast streams
simultaneously from a DTA-2110.

The public version removes event-specific branding while retaining the
four-channel transmitter, channel identification and animated live
indicator.

## DekTec SDK and example code

This project was developed from example code supplied with the DekTec
SDK and uses the DekTec DTAPI/AvFifo API.

The test-pattern renderer, four-channel application work, static-frame
optimisation and lightweight dynamic overlay were developed as part of
the L2Tek project.

DekTec SDK components are subject to DekTec's own licence terms. This
repository is intended to contain the application project, not a
redistribution of the complete DekTec SDK.

DekTec and associated product names are trademarks of their respective
owner.

## About L2Tek

L2Tek supplies specialist semiconductor, broadcast-video and IP-media
technology and provides technical support and demonstration systems for
professional video applications.

This project was developed as a practical ST 2110 demonstration and
engineering exercise.
