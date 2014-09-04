# CMakeLists.txt for <RtspSinkFilter> 

INCLUDE_DIRECTORIES(
${vppIncludes}
${vppCodecIncludes}
${vppUtilIncludes}
$ENV{LIVE_MEDIA_EXT_ROOT}/include
$ENV{LIVE_MEDIA_EXT_ROOT}/externals/glog/src/windows
$ENV{LIVE_MEDIA_EXT_ROOT}/externals/cpp-util/include
)
 
# Lib directories
LINK_DIRECTORIES(
${vppLink}
)

SET(FLT_HDRS
DirectShowNetworkCodecControlInterface.h
INetworkCodecControlInterface.h
RtspSinkFilter.h
RtspSinkGuids.h
RtspSinkInputPin.h
stdafx.h
StepBasedRateController.h
)

SET(FLT_SRCS 
DllSetup.cpp
RtspSinkFilter.cpp
RtspSinkFilter.def
RtspSinkInputPin.cpp
stdafx.cpp
)

ADD_LIBRARY(
RtspSinkFilter SHARED ${FLT_SRCS} ${FLT_HDRS})

TARGET_LINK_LIBRARIES (
RtspSinkFilter
${vppLibs}
vpp
LiveMediaExt
SimpleRateAdaptation
groupsock
liveMedia
UsageEnvironment
BasicUsageEnvironment
glog
) 

INSTALL(
  TARGETS RtspSinkFilter
  ARCHIVE DESTINATION bin
  LIBRARY DESTINATION bin
)
