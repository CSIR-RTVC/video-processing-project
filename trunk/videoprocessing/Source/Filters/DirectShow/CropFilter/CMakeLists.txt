# CMakeLists.txt for <CropFilter> 

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_BUILD_FOR_SHORT")

INCLUDE_DIRECTORIES(
${vppIncludes}
)

# Lib directories
LINK_DIRECTORIES(
${vppLink}
)

SET(FLT_HDRS
CropFilter.h
CropProperties.h
resource.h
stdafx.h
)

SET(FLT_SRCS 
CropFilter.cpp
CropFilter.def
CropFilter.rc
DLLSetup.cpp
stdafx.cpp
)

ADD_LIBRARY(
CropFilter SHARED ${FLT_SRCS} ${FLT_HDRS})

TARGET_LINK_LIBRARIES (
CropFilter
${vppLibs}
vpp
) 

INSTALL(
  TARGETS CropFilter
  ARCHIVE DESTINATION bin
  LIBRARY DESTINATION bin
)


IF (REGISTER_DS_FILTERS)
ADD_CUSTOM_COMMAND(
TARGET CropFilter 
POST_BUILD COMMAND 
regsvr32 /s \"$(TargetPath)\"
)
ENDIF(REGISTER_DS_FILTERS)
