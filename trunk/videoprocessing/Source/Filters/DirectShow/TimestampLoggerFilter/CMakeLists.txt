# CMakeLists.txt for <TimestampLoggerFilter> 

INCLUDE_DIRECTORIES(
${vppIncludes}
)

# Lib directories
LINK_DIRECTORIES(
${vppLink}
)

SET(FLT_HDRS
stdafx.h
TimestampLoggerFilter.h
)

SET(FLT_SRCS 
DLLSetup.cpp
stdafx.cpp
TimestampLoggerFilter.cpp
TimestampLoggerFilter.def
)

ADD_LIBRARY(
TimestampLoggerFilter SHARED ${FLT_SRCS} ${FLT_HDRS})

TARGET_LINK_LIBRARIES (
TimestampLoggerFilter
${vppLibs}
vpp
) 

INSTALL(
  TARGETS TimestampLoggerFilter
  ARCHIVE DESTINATION bin
  LIBRARY DESTINATION bin
)

IF (REGISTER_DS_FILTERS)
ADD_CUSTOM_COMMAND(
TARGET TimestampLoggerFilter 
POST_BUILD COMMAND 
regsvr32 /s \"$(TargetPath)\"
)
ENDIF(REGISTER_DS_FILTERS)
