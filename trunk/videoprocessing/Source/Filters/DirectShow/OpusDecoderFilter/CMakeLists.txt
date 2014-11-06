# CMakeLists.txt for <OpusDecoderFilter> 

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
OpusDecoderFilter.h
#OpusEncoderProperties.h
#resource.h
stdafx.h
)

SET(FLT_SRCS 
DLLSetup.cpp
OpusDecoderFilter.cpp
OpusDecoderFilter.def
#OpusDecoderFilter.rc
stdafx.cpp
)

ADD_LIBRARY(
OpusDecoderFilter SHARED ${FLT_SRCS} ${FLT_HDRS})

TARGET_LINK_LIBRARIES (
OpusDecoderFilter
OpusCodec
${vppLibs}
vpp
) 

INSTALL(
  TARGETS OpusDecoderFilter
  ARCHIVE DESTINATION bin
  LIBRARY DESTINATION bin
)

IF (REGISTER_DS_FILTERS)
ADD_CUSTOM_COMMAND(
TARGET OpusDecoderFilter 
POST_BUILD COMMAND 
regsvr32 /s \"$(TargetPath)\"
)
ENDIF(REGISTER_DS_FILTERS)
