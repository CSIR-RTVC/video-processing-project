# CMakeLists.txt for <PicInPicFilter> 

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_BUILD_FOR_SHORT")

INCLUDE_DIRECTORIES(
${vppIncludes}
)

# Lib directories
LINK_DIRECTORIES(
${vppLink}
)

SET(FLT_HDRS
PicInPicFilter.h
PicInPicProperties.h
resource.h
stdafx.h
)

SET(FLT_SRCS 
DLLSetup.cpp
PicInPicFilter.cpp
PicInPicFilter.def
PicInPicFilter.rc
PicInPicProperties.cpp
stdafx.cpp
)

ADD_LIBRARY(
PicInPicFilter SHARED ${FLT_SRCS} ${FLT_HDRS})

TARGET_LINK_LIBRARIES (
PicInPicFilter
${vppLibs}
vpp
) 

INSTALL(
  TARGETS PicInPicFilter
  ARCHIVE DESTINATION bin
  LIBRARY DESTINATION bin
)

IF (REGISTER_DS_FILTERS)
ADD_CUSTOM_COMMAND(
TARGET PicInPicFilter 
POST_BUILD COMMAND 
regsvr32 /s \"$(TargetPath)\"
)
ENDIF(REGISTER_DS_FILTERS)
