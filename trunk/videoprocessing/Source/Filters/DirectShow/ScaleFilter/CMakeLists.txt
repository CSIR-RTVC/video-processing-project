# CMakeLists.txt for <ScaleFilter> 

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_BUILD_FOR_SHORT")

INCLUDE_DIRECTORIES(
${vppIncludes}
)

# Lib directories
LINK_DIRECTORIES(
${vppLink}
)

SET(FLT_HDRS
resource.h
ScaleFilter.h
ScaleProperties.h
stdafx.h
)

SET(FLT_SRCS 
DLLSetup.cpp
ScaleFilter.cpp
ScaleFilter.def
ScaleFilter.rc
stdafx.cpp
)

ADD_LIBRARY(
ScaleFilter SHARED ${FLT_SRCS} ${FLT_HDRS})

TARGET_LINK_LIBRARIES (
ScaleFilter
${vppLibs}
vpp
) 

INSTALL(
  TARGETS ScaleFilter
  ARCHIVE DESTINATION bin
  LIBRARY DESTINATION bin
)

IF (REGISTER_DS_FILTERS)
ADD_CUSTOM_COMMAND(
TARGET ScaleFilter 
POST_BUILD COMMAND 
regsvr32 /s \"$(TargetPath)\"
)
ENDIF(REGISTER_DS_FILTERS)
