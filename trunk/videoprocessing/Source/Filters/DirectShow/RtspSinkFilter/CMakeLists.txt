# CMakeLists.txt for <RtspSinkFilter> 

INCLUDE_DIRECTORIES(
${vppIncludes}
${vppCodecIncludes}
${vppUtilIncludes}
$ENV{LIVE_MEDIA_EXT_ROOT}/include
$ENV{LIVE_MEDIA_EXT_ROOT}/externals/glog/src/windows
)
 
# Lib directories
LINK_DIRECTORIES(
${vppLink}
)

SET(FLT_HDRS
RtspSinkFilter.h
RtspSinkGuids.h
RtspSinkInputPin.h
stdafx.h
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
