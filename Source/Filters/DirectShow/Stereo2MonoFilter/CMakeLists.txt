# CMakeLists.txt for <Stereo2MonoFilter> 

INCLUDE_DIRECTORIES(
${vppIncludes}
)

# Lib directories
LINK_DIRECTORIES(
${vppLink}
)

SET(FLT_HDRS
resource.h
Stereo2MonoFilter.h
Stereo2MonoProperties.h
stdafx.h
)

SET(FLT_SRCS 
DLLSetup.cpp
Stereo2MonoFilter.cpp
Stereo2MonoFilter.def
Stereo2MonoFilter.rc
stdafx.cpp
)

ADD_LIBRARY(
Stereo2MonoFilter SHARED ${FLT_SRCS} ${FLT_HDRS})

TARGET_LINK_LIBRARIES (
Stereo2MonoFilter
${vppLibs}
vpp
) 

INSTALL(
  TARGETS Stereo2MonoFilter
  ARCHIVE DESTINATION bin
  LIBRARY DESTINATION bin
)

IF (REGISTER_DS_FILTERS)
ADD_CUSTOM_COMMAND(
TARGET Stereo2MonoFilter 
POST_BUILD COMMAND 
regsvr32 /s \"$(TargetPath)\"
)
ENDIF(REGISTER_DS_FILTERS)
