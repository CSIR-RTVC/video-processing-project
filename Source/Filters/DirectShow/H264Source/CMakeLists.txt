# CMakeLists.txt for <H264Source> 

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
H264OutputPin.h
H264Source.h
H264Properties.h
resource.h
stdafx.h
)

SET(FLT_SRCS 
DLLSetup.cpp
H264OutputPin.cpp
H264Source.cpp
H264Source.def
H264Source.rc
stdafx.cpp
)

ADD_LIBRARY(
H264Source SHARED ${FLT_SRCS} ${FLT_HDRS})

TARGET_LINK_LIBRARIES (
H264Source
H264v2
${vppLibs}
vpp
) 

INSTALL(
  TARGETS H264Source
  ARCHIVE DESTINATION bin
  LIBRARY DESTINATION bin
)

IF (REGISTER_DS_FILTERS)
ADD_CUSTOM_COMMAND(
TARGET H264Source 
POST_BUILD COMMAND 
regsvr32 /s \"$(TargetPath)\"
)
ENDIF(REGISTER_DS_FILTERS)
