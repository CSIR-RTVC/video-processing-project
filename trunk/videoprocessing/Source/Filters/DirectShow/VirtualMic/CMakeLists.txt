# CMakeLists.txt for <VirtualMic> 

INCLUDE_DIRECTORIES(
${vppIncludes}
)

# Lib directories
LINK_DIRECTORIES(
${vppLink}
)

SET(FLT_HDRS
VirtualMic.h
VirtualMicSourceStream.h
)

SET(FLT_SRCS 
DllSetup.cpp
VirtualMic.cpp
VirtualMic.def
VirtualMicSourceStream.cpp
)

ADD_LIBRARY(
VirtualMic SHARED ${FLT_SRCS} ${FLT_HDRS})

TARGET_LINK_LIBRARIES (
VirtualMic
${vppLibs}
vpp
) 

INSTALL(
  TARGETS VirtualMic
  ARCHIVE DESTINATION bin
  LIBRARY DESTINATION bin
)

IF (REGISTER_DS_FILTERS)
ADD_CUSTOM_COMMAND(
TARGET VirtualMic 
POST_BUILD COMMAND 
regsvr32 /s \"$(TargetPath)\"
)
ENDIF(REGISTER_DS_FILTERS)
