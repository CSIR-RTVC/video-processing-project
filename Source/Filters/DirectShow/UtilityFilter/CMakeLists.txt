# CMakeLists.txt for <UtilityFilter> 

INCLUDE_DIRECTORIES(
${vppIncludes}
${vppCodecIncludes}
${vppUtilIncludes}
)

# Lib directories
LINK_DIRECTORIES(
${vppLink}
)

SET(FLT_HDRS
resource.h
stdafx.h
UtilityFilter.h
UtilityProperties.h
)

SET(FLT_SRCS 
DLLSetup.cpp
stdafx.cpp
UtilityFilter.cpp
UtilityFilter.def
UtilityFilter.rc
)

ADD_LIBRARY(
UtilityFilter SHARED ${FLT_SRCS} ${FLT_HDRS})

TARGET_LINK_LIBRARIES (
UtilityFilter
${vppLibs}
vpp
gdiplus
) 

INSTALL(
  TARGETS UtilityFilter
  ARCHIVE DESTINATION bin
  LIBRARY DESTINATION bin
)

IF (REGISTER_DS_FILTERS)
ADD_CUSTOM_COMMAND(
TARGET UtilityFilter 
POST_BUILD COMMAND 
regsvr32 /s \"$(TargetPath)\"
)
ENDIF(REGISTER_DS_FILTERS)
