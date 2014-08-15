# CMakeLists.txt for <RotateFilter> 

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
RotateFilter.h
RotateProperties.h
stdafx.h
)

SET(FLT_SRCS 
DLLSetup.cpp
RotateFilter.cpp
RotateFilter.def
stdafx.cpp
)

ADD_LIBRARY(
RotateFilter SHARED ${FLT_SRCS} ${FLT_HDRS})

TARGET_LINK_LIBRARIES (
RotateFilter
${vppLibs}
vpp
) 

INSTALL(
  TARGETS RotateFilter
  ARCHIVE DESTINATION bin
  LIBRARY DESTINATION bin
)

IF (REGISTER_DS_FILTERS)
ADD_CUSTOM_COMMAND(
TARGET RotateFilter 
POST_BUILD COMMAND 
regsvr32 /s \"$(TargetPath)\"
)
ENDIF(REGISTER_DS_FILTERS)
