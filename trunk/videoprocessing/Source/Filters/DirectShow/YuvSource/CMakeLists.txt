# CMakeLists.txt for <YuvSource> 

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_BUILD_FOR_SHORT")

INCLUDE_DIRECTORIES(
${vppIncludes}
)

# Lib directories
LINK_DIRECTORIES(
${vppLink}
)

SET(FLT_HDRS
YuvProperties.h
YuvSource.h
)

SET(FLT_SRCS 
DLLSetup.cpp
YuvSource.cpp
YuvSource.def
)

ADD_LIBRARY(
YuvSource SHARED ${FLT_SRCS} ${FLT_HDRS})

TARGET_LINK_LIBRARIES (
YuvSource
${vppLibs}
vpp
) 

INSTALL(
  TARGETS YuvSource
  ARCHIVE DESTINATION bin
  LIBRARY DESTINATION bin
)

IF (REGISTER_DS_FILTERS)
ADD_CUSTOM_COMMAND(
TARGET YuvSource 
POST_BUILD COMMAND 
regsvr32 /s \"$(TargetPath)\"
)
ENDIF(REGISTER_DS_FILTERS)
