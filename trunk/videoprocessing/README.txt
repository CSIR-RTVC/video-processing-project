Video Processing Project: http://videoprocessing.sourceforge.net/
--------------------------------------------------------------------------------------------------
The aim of the Video Processing Project is to create atomic video processing tools such as image croppers, scalers, colour converters and video coding utilties that provide a readable software implementation, whilst retaining fast implementations. These tools are targetted both at the Windows and Linux platforms and have been implemented in C++. The source code is freely available under a BSD licence. 

Another goal of the project is to establish a framework in within which faster video processing algorithms and implementations can be developed and tested by the open source community.

Related tools have been grouped together as libraries, which can then be integrated to create plug-ins for multimedia frameworks such as DirectShow and GStreamer. 

Base classes and interfaces that have been developed during the course of the project to simplify certain aspects of DirectShow filter development and have also been released under the BSD license.

--------------------------------------------------------------------------------------------------
Library:
- ImageUtils - a library containing various video processing utilities
- DirectShow - DirectShow utility classes
- CodecUtils - Encoding/decoding related classes
- H264v2     - H264 codec implementation

--------------------------------------------------------------------------------------------------
Released multimedia plug-ins:

DirectShow filters:
- Audio mixing filter
- Crop filter
- Framerate display
- H264 decoder
- H264 encoder
- H264 source filter
- Pic in pic filter
- RGB to YUV420 Planar color converter
- Rotate filter
- RTSP/RTP Source filter (8/16 bit PCM AMR MP3, H264)
- Scale filter
- Tee
- Timestamp logger
- Video mixing filter
- YUV420 Planar to RGB color converter
- YUV source

Filter configuration: 
- Use the property pages to configure the crop, scale or RTSP source filter from graph edit.
The RTSP source filter can be tested using the GraphEdit like GraphStudio tool which has the ability to open a URL source address. 
- Use the COM interface ISettingsInterface to configure the filters programmatically.


GStreamer:
N/A

--------------------------------------------------------------------------------------------------
Contact:

rtvc@meraka.org.za

