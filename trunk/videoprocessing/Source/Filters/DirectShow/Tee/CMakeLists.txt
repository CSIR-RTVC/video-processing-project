# CMakeLists.txt for <Tee> 

INCLUDE_DIRECTORIES(
${vppIncludes}
)

# Lib directories
LINK_DIRECTORIES(
${vppLink}
)

SET(FLT_HDRS
Tee.h
stdafx.h
)

SET(FLT_SRCS 
DLLSetup.cpp
Tee.cpp
Tee.def
stdafx.cpp
)

ADD_LIBRARY(
Tee SHARED ${FLT_SRCS} ${FLT_HDRS})

TARGET_LINK_LIBRARIES (
Tee
${vppLibs}
vpp
) 

INSTALL(
  TARGETS Tee
  ARCHIVE DESTINATION bin
  LIBRARY DESTINATION bin
)

IF (REGISTER_DS_FILTERS)
ADD_CUSTOM_COMMAND(
TARGET Tee 
POST_BUILD COMMAND 
regsvr32 /s \"$(TargetPath)\"
)
ENDIF(REGISTER_DS_FILTERS)
