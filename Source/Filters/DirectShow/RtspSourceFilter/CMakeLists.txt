# CMakeLists.txt for <RtspSourceFilter> 

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_BUILD_FOR_SHORT")

INCLUDE_DIRECTORIES(
${vppIncludes}
${vppCodecIncludes}
${vppUtilIncludes}
)

# Lib directories
LINK_DIRECTORIES(
${vppLink}
)

SET(FLT_HDRS
#AsyncRtspClientAdapter.h
#AsyncRtspClientSessionManager.h
MediaPacketManager.h
MediaPacketQueue.h
MediaParameterExtractor.h
resource.h
RtspClientSessionManager.h
RtspProperties.h
RtspSourceFilter.h
RtspSourceOutputPin.h
RtvcRtpSink.h
stdafx.h
)

SET(FLT_SRCS 
#AsyncRtspClientAdapter.cpp
#AsyncRtspClientSessionManager.cpp
DllSetup.cpp
MediaPacketManager.cpp
MediaPacketQueue.cpp
MediaParameterExtractor.cpp
RtspClientSessionManager.cpp
RtspSourceFilter.cpp
RtspSourceFilter.def
RtspSourceFilter.rc
RtspSourceOutputPin.cpp
stdafx.cpp
)

ADD_LIBRARY(
RtspSourceFilter SHARED ${FLT_SRCS} ${FLT_HDRS})

TARGET_LINK_LIBRARIES (
RtspSourceFilter
${vppLibs}
vpp
groupsock
liveMedia
UsageEnvironment
BasicUsageEnvironment
) 

INSTALL(
  TARGETS RtspSourceFilter
  ARCHIVE DESTINATION bin
  LIBRARY DESTINATION bin
)

IF (REGISTER_DS_FILTERS)
ADD_CUSTOM_COMMAND(
TARGET RtspSourceFilter 
POST_BUILD COMMAND 
regsvr32 /s \"$(TargetPath)\"
)
ENDIF(REGISTER_DS_FILTERS)
