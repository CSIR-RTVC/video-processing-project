# CMakeLists.txt for <H264EncoderFilter> 

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
H264EncoderFilter.h
H264EncoderProperties.h
resource.h
stdafx.h
)

SET(FLT_SRCS 
DLLSetup.cpp
H264EncoderFilter.cpp
H264EncoderFilter.def
H264EncoderFilter.rc
stdafx.cpp
)

ADD_LIBRARY(
H264EncoderFilter SHARED ${FLT_SRCS} ${FLT_HDRS})

TARGET_LINK_LIBRARIES (
H264EncoderFilter
H264v2
${vppLibs}
vpp
) 

INSTALL(
  TARGETS H264EncoderFilter
  ARCHIVE DESTINATION bin
  LIBRARY DESTINATION bin
)

IF (REGISTER_DS_FILTERS)
ADD_CUSTOM_COMMAND(
TARGET H264EncoderFilter 
POST_BUILD COMMAND 
regsvr32 /s \"$(TargetPath)\"
)
ENDIF(REGISTER_DS_FILTERS)
