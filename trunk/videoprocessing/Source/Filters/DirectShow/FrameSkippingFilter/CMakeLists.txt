# CMakeLists.txt for <FrameSkippingFilter> 

INCLUDE_DIRECTORIES(
${vppIncludes}
)

# Lib directories
LINK_DIRECTORIES(
${vppLink}
)

SET(FLT_HDRS
FrameSkippingFilter.h
FrameSkippingProperties.h
resource.h
stdafx.h
)

SET(FLT_SRCS 
DLLSetup.cpp
FrameSkippingFilter.cpp
FrameSkippingFilter.def
FrameSkippingFilter.rc
stdafx.cpp
)

ADD_LIBRARY(
FrameSkippingFilter SHARED ${FLT_SRCS} ${FLT_HDRS})

TARGET_LINK_LIBRARIES (
FrameSkippingFilter
${vppLibs}
vpp
) 

INSTALL(
  TARGETS FrameSkippingFilter
  ARCHIVE DESTINATION bin
  LIBRARY DESTINATION bin
)

IF (REGISTER_DS_FILTERS)
ADD_CUSTOM_COMMAND(
TARGET FrameSkippingFilter 
POST_BUILD COMMAND 
regsvr32 /s \"$(TargetPath)\"
)
ENDIF(REGISTER_DS_FILTERS)

