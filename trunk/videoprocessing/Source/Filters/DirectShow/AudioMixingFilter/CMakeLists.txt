# CMakeLists.txt for <AudioMixingFilter> 

INCLUDE_DIRECTORIES(
${vppIncludes}
${vppCodecIncludes}
)

# Lib directories
LINK_DIRECTORIES(
${vppLink}
)

SET(FLT_HDRS
AudioMixingFilter.h
AudioMixingProperties.h
resource.h
stdafx.h
)

SET(FLT_SRCS 
AudioMixingFilter.cpp
AudioMixingFilter.def
AudioMixingFilter.rc
DLLSetup.cpp
stdafx.cpp
)

ADD_LIBRARY(
AudioMixingFilter SHARED ${FLT_SRCS} ${FLT_HDRS})

TARGET_LINK_LIBRARIES (
AudioMixingFilter
${vppLibs}
vpp
) 

INSTALL(
  TARGETS AudioMixingFilter
  ARCHIVE DESTINATION bin
  LIBRARY DESTINATION bin
)


IF (REGISTER_DS_FILTERS)
ADD_CUSTOM_COMMAND(
TARGET AudioMixingFilter 
POST_BUILD COMMAND 
regsvr32 /s \"$(TargetPath)\"
)
ENDIF(REGISTER_DS_FILTERS)
