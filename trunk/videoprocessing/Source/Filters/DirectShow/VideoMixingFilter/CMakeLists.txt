# CMakeLists.txt for <VideoMixingFilter> 

INCLUDE_DIRECTORIES(
${vppIncludes}
)

# Lib directories
LINK_DIRECTORIES(
${vppLink}
)

SET(FLT_HDRS
stdafx.h
VideoMixingFilter.h
)

SET(FLT_SRCS 
DLLSetup.cpp
stdafx.cpp
VideoMixingFilter.cpp
VideoMixingFilter.def
)

ADD_LIBRARY(
VideoMixingFilter SHARED ${FLT_SRCS} ${FLT_HDRS})

TARGET_LINK_LIBRARIES (
VideoMixingFilter
${vppLibs}
vpp
) 

INSTALL(
  TARGETS VideoMixingFilter
  ARCHIVE DESTINATION bin
  LIBRARY DESTINATION bin
)

IF (REGISTER_DS_FILTERS)
ADD_CUSTOM_COMMAND(
TARGET VideoMixingFilter 
POST_BUILD COMMAND 
regsvr32 /s \"$(TargetPath)\"
)
ENDIF(REGISTER_DS_FILTERS)
