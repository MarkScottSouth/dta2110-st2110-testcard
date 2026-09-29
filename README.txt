==========================================================================================
AvFifoExamples - DekTec AvFifo Code Examples for Windows
==========================================================================================

Thank you for downloading the AvFifoExamples package.

This zip file contains the source code for examples related to the AvFifo API (part of
DTAPI), designed to be compiled and run on Windows for DekTec NICs that support the
AvFifo API (currently the DTA-2110 10GbE NIC and the DTA-2125 25GbE NIC).

  1. AvFifo_AncRx: A SMPTE 2110-40 ancillary data receiver.
  2. AvFifo_AncTx: A SMPTE 2110-40 ancillary data transmitter.
  3. AvFifo_AudioRx: A SMPTE 2110-30 audio receiver with SDL renderer.
  4. AvFifo_AudioTx: A SMPTE 2110-30 audio transmitter.
  5. AvFifo_FastMetadataRx: A SMPTE 2110-41 fast metadata receiver.
  6. AvFifo_FastMetadataTx: A SMPTE 2110-41 fast metadata transmitter.
  7. AvFifo_PktTiming: Access arrival timestamps of SMPTE 2110 IP packets.
  8. AvFifo_VideoRx: A SMPTE 2110-20 video receiver with SDL renderer.
  9. AvFifo_VideoTx: A SMPTE 2110-20 video test generator.

The AvFifo_VideoRx and AvFifo_VideoTx examples serve as the primary reference
implementations with extensive comments describing the AvFifo usage in detail. The other
examples contain fewer comments and demonstrate specific aspects of the AvFifo API.

NOTE: To run Rx and Tx examples on a single PC with two DTA-2110s (or DTA-2125s), set
      CfgDeviceNo to 0 in the Tx application, and to 1 in the Rx application.

The package includes the following:

  - Source code for the AvFifo code examples.
  - Required SDL library headers and binaries for AvFifo_VideoRx and AvFifo_AudioRx.
  - Solution and project files for Visual Studio 2019 and 2022.
  - An overview and compilation instructions (this README.txt file).
  - The AvFifo section of the DTAPI reference manuals.

Prerequisites:

  1. Download and install the latest DekTec Windows SDK (DTAPI) from the following page:
     https://www.dektec.com/downloads/SDK.
  2. Ensure that Visual Studio 2019 or 2022 is installed.
  
Instructions to compile:

  1. Open the solution file "AvFifoExamples.sln" in Visual Studio 2019 or 2022.
  2. Take a look at the fixed configuration constants, and update if you need to:
      - AvFifo_AncRx\Source\AvFifo_AncRx.cpp for AvFifo_AncRx
      - AvFifo_AncTx\Source\AvFifo_AncTx.cpp for AvFifo_AncTx
      - AvFifo_AudioRx\Source\AvFifo_AudioRx.cpp for AvFifo_AudioRx
      - AvFifo_AudioTx\Source\AvFifo_AudioTx.cpp for AvFifo_AudioTx
      - AvFifo_FastMetadataRx\Source\AvFifo_FastMetadataRx.cpp for AvFifo_FastMetadataRx
      - AvFifo_FastMetadataTx\Source\AvFifo_FastMetadataTx.cpp for AvFifo_FastMetadataTx
      - AvFifo_PktTiming\Source\AvFifo_PktTiming.cpp for AvFifo_PktTiming;
      - AvFifo_VideoRx\Source\AvFifo_VideoRx.cpp for AvFifo_VideoRx;
      - AvFifo_VideoTx\Source\Configuration.h for AvFifo_VideoTx.
  3. Build the solution (Ctrl+Shift+B in Visual Studio).

If you encounter any issues or have further queries or suggestions, please contact 
DekTec support (info@dektec.com). 

The DekTec Team
