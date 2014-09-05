# CMakeLists.txt

SET(DS_SRCS 
DirectShow/CSettingsInterface.cpp
DirectShow/CStatusInterface.cpp
DirectShow/CustomBaseFilter.cpp
DirectShow/DirectShowHelper.cpp
DirectShow/MultiIOBaseFilter.cpp
DirectShow/MultiIOInputPin.cpp
DirectShow/MultiIOOutputPin.cpp
DirectShow/stdafx.cpp
DirectShow/VideoMixingBase.cpp
)
SET(GU_SRCS
GeneralUtils/MeasurementTable.cpp
)
SET(IU_SRCS
Image/FastFixedPointRGB24toYUV420Converter.cpp
Image/FastGpuRGB24toYUV420Converter.cpp
Image/FastLookupTableRGB24toYUV420Converter.cpp
Image/FastRGB24toYUV420Converter.cpp
Image/FastSimdRGB24toYUV420Converter.cpp
Image/ImageHandlerV2.cpp
Image/MtRGB24toYUV420Converter.cpp
Image/OverlayExtMem2Dv2.cpp
Image/OverlayMem2Dv2.cpp
Image/PicConcatRGB24Impl.cpp
Image/PicConcatRGB32Impl.cpp
Image/PicCropperRGB24Impl.cpp
Image/PicCropperRGB32Impl.cpp
Image/PicInPicRGB24Impl.cpp
Image/PicInPicRGB32Impl.cpp
Image/PicRotateRGB24Impl.cpp
Image/PicRotateRGB32Impl.cpp
Image/PicRotateRGBBase.cpp
Image/PicScalerRGB24Impl.cpp
Image/PicScalerYUV420PImpl.cpp
Image/RealRGB24toYUV420CCIR601Converter.cpp
Image/RealRGB24toYUV420CCIR601ConverterVer16.cpp
Image/RealRGB24toYUV420Converter.cpp
Image/RealRGB24toYUV420ConverterImpl2.cpp
Image/RealRGB24toYUV420ConverterImpl2Ver16.cpp
Image/RealRGB32toYUV420Converter.cpp
Image/RealYUV420toRGB24CCIR601Converter.cpp
Image/RealYUV420toRGB24CCIR601ConverterVer16.cpp
Image/RealYUV420toRGB24Converter.cpp
Image/RealYUV420toRGB24ConverterImpl2.cpp
Image/RealYUV420toRGB24ConverterImpl2Ver16.cpp
)
SET(S_SRCS
Shared/MediaSample.cpp
)
SET(DS_HEADERS
DirectShow/CodecControlInterface.h
DirectShow/CommonDefs.h
DirectShow/ComTypeDefs.h
DirectShow/CSettingsInterface.h
DirectShow/CStatusInterface.h
DirectShow/CustomBaseFilter.h
DirectShow/CustomMediaTypes.h
DirectShow/DirectShowHelper.h
DirectShow/FilterPropertiesBase.h
DirectShow/MultiIOBaseFilter.h
DirectShow/MultiIOInputPin.h
DirectShow/MultiIOOutputPin.h
DirectShow/NotifyCodes.h
DirectShow/SettingsInterface.h
DirectShow/StatusInterface.h
DirectShow/stdafx.h
DirectShow/VideoMixingBase.h
)
SET(GU_HEADERS
GeneralUtils/MeasurementTable.h
)
SET(IU_HEADERS
Image/FastFixedPointRGB24toYUV420Converter.h
Image/FastGpuRGB24toYUV420Converter.h
Image/FastLookupTableRGB24toYUV420Converter.h
Image/FastRGB24toYUV420Converter.h
Image/FastSimdRGB24toYUV420Converter.h
Image/ImageHandlerV2.h
Image/MtRGB24toYUV420Converter.h
Image/OverlayExtMem2Dv2.h
Image/OverlayMem2Dv2.h
Image/PicConcatBase.h
Image/PicConcatRGB24Impl.h
Image/PicConcatRGB32Impl.h
Image/PicCropperBase.h
Image/PicCropperRGB24Impl.h
Image/PicCropperRGB32Impl.h
Image/PicInPicBase.h
Image/PicInPicRGB24Impl.h
Image/PicInPicRGB32Impl.h
Image/PicRotateBase.h
Image/PicRotateRGB24Impl.h
Image/PicRotateRGB32Impl.h
Image/PicRotateRGBBase.h
Image/PicScalerBase.h
Image/PicScalerRGB24Impl.h
Image/PicScalerYUV420PImpl.h
Image/RealRGB24toYUV420CCIR601Converter.h
Image/RealRGB24toYUV420CCIR601ConverterVer16.h
Image/RealRGB24toYUV420Converter.h
Image/RealRGB24toYUV420ConverterImpl2.h
Image/RealRGB24toYUV420ConverterImpl2Ver16.h
Image/RealRGB32toYUV420Converter.h
Image/RealYUV420toRGB24CCIR601Converter.h
Image/RealYUV420toRGB24CCIR601ConverterVer16.h
Image/RealYUV420toRGB24Converter.h
Image/RealYUV420toRGB24ConverterImpl2.h
Image/RealYUV420toRGB24ConverterImpl2Ver16.h
Image/RGBtoRGBConverter.h
Image/RGBtoYUV420Converter.h
Image/YUV420toRGBConverter.h
)
SET(S_HEADERS
Shared/CommonDefs.h
Shared/Conversion.h
Shared/MediaSample.h
Shared/StringUtil.h
Shared/TimerUtil.h
)

SET(VPP_SRCS
${DS_SRCS} 
${GU_SRCS}
${IU_SRCS}
${S_SRCS}
)
SET(VPP_HEADERS 
${DS_HEADERS}
${GU_HEADERS}
${IU_HEADERS}
${S_HEADERS}
)

INCLUDE_DIRECTORIES(
${vppIncludes}
)

SOURCE_GROUP("Source Files\\DirectShow" FILES ${DS_SRCS})
SOURCE_GROUP("Source Files\\GeneralUtils" FILES ${GU_SRCS})
SOURCE_GROUP("Source Files\\Image" FILES ${IU_SRCS})
SOURCE_GROUP("Source Files\\Shared" FILES ${S_SRCS})

SOURCE_GROUP("Header Files\\DirectShow" FILES ${DS_HEADERS})
SOURCE_GROUP("Header Files\\GeneralUtils" FILES ${GU_HEADERS})
SOURCE_GROUP("Header Files\\Image" FILES ${IU_HEADERS})
SOURCE_GROUP("Header Files\\Shared" FILES ${S_HEADERS})

ADD_LIBRARY( vpp STATIC ${VPP_SRCS} ${VPP_HEADERS})

# adding precompiled header support on windows
#if (MSVC)
#   set_target_properties(Core PROPERTIES COMPILE_FLAGS "/YuCorePch.h")
#   set_source_files_properties(CorePch.cpp PROPERTIES COMPILE_FLAGS "/YcCorePch.h")
#endif(MSVC)
