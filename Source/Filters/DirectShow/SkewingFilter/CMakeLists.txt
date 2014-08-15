# CMakeLists.txt for <SkewingFilter> 

INCLUDE_DIRECTORIES(
${vppIncludes}
)

# Lib directories
LINK_DIRECTORIES(
${vppLink}
)

SET(FLT_HDRS
resource.h
SkewingFilter.h
SkewingProperties.h
stdafx.h
)

SET(FLT_SRCS 
DLLSetup.cpp
SkewingFilter.cpp
SkewingFilter.def
SkewingFilter.rc
stdafx.cpp
)

ADD_LIBRARY(
SkewingFilter SHARED ${FLT_SRCS} ${FLT_HDRS})

TARGET_LINK_LIBRARIES (
SkewingFilter
${vppLibs}
vpp
) 

INSTALL(
  TARGETS SkewingFilter
  ARCHIVE DESTINATION bin
  LIBRARY DESTINATION bin
)

IF (REGISTER_DS_FILTERS)
ADD_CUSTOM_COMMAND(
TARGET SkewingFilter 
POST_BUILD COMMAND 
regsvr32 /s \"$(TargetPath)\"
)
ENDIF(REGISTER_DS_FILTERS)
