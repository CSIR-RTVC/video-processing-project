# CMakeLists.txt in Opus dir

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_BUILD_FOR_SHORT")

INCLUDE_DIRECTORIES(
${vppIncludes}
${vppCodecIncludes}
$ENV{OPUS_ROOT}/include
$ENV{OPUS_ROOT}/celt
$ENV{OPUS_ROOT}/silk
$ENV{OPUS_ROOT}/silk/fixed
$ENV{OPUS_ROOT}/silk/float
)

# Lib directories
LINK_DIRECTORIES(
${vppLink}
)

SET(OPUS_LIB_HDRS
OpusCodec.h
OpusFactory.h
StdAfx.h
targetver.h
)

SET(OPUS_LIB_SRCS 
OpusCodec.cpp
OpusFactory.cpp
dllmain.cpp
StdAfx.cpp
)

ADD_LIBRARY( OpusCodec SHARED ${OPUS_LIB_SRCS} ${OPUS_LIB_HDRS})

TARGET_LINK_LIBRARIES(
OpusCodec
opus
celt
silk_common
silk_fixed
silk_float
) 
