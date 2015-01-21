# CMakeLists.txt for <VirtualCam> 

INCLUDE_DIRECTORIES(
${vppIncludes}
)

# Lib directories
LINK_DIRECTORIES(
${vppLink}
)

SET(FLT_HDRS
VirtualCam.h
)

SET(FLT_SRCS 
DllSetup.cpp
VirtualCam.cpp
VirtualCam.def
)

ADD_LIBRARY(
VirtualCam SHARED ${FLT_SRCS} ${FLT_HDRS})

TARGET_LINK_LIBRARIES (
VirtualCam
${vppLibs}
vpp
) 

INSTALL(
  TARGETS VirtualCam
  ARCHIVE DESTINATION bin
  LIBRARY DESTINATION bin
)

IF (REGISTER_DS_FILTERS)
ADD_CUSTOM_COMMAND(
TARGET VirtualCam 
POST_BUILD COMMAND 
regsvr32 /s \"$(TargetPath)\"
)
ENDIF(REGISTER_DS_FILTERS)
