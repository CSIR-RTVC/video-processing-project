# CMakeLists.txt for <FramerateDisplayFilter> 

INCLUDE_DIRECTORIES(
${vppIncludes}
)

# Lib directories
LINK_DIRECTORIES(
${vppLink}
)

SET(FLT_HDRS
FramerateDisplayFilter.h
FramerateDisplayProperties.h
resource.h
stdafx.h
)

SET(FLT_SRCS 
DLLSetup.cpp
FramerateDisplayFilter.cpp
FramerateDisplayFilter.def
FramerateDisplayFilter.rc
stdafx.cpp
)

ADD_LIBRARY(
FramerateDisplayFilter SHARED ${FLT_SRCS} ${FLT_HDRS})

TARGET_LINK_LIBRARIES (
FramerateDisplayFilter
${vppLibs}
vpp
gdiplus
) 

INSTALL(
  TARGETS FramerateDisplayFilter
  ARCHIVE DESTINATION bin
  LIBRARY DESTINATION bin
)

IF (REGISTER_DS_FILTERS)
ADD_CUSTOM_COMMAND(
TARGET FramerateDisplayFilter 
POST_BUILD COMMAND 
regsvr32 /s \"$(TargetPath)\"
)
ENDIF(REGISTER_DS_FILTERS)
