#pragma once

#pragma warning(push)     // disable for this header only
#pragma warning(disable:4312)
// DirectShow
#include <streams.h>
#pragma warning(pop)      // restore original warning level

#include <comdef.h>

#include <DirectShow/SettingsInterface.h>
#include <DirectShow/StatusInterface.h>

// DirectShow COM smart pointers
_COM_SMARTPTR_TYPEDEF(IGraphBuilder, IID_IGraphBuilder);
_COM_SMARTPTR_TYPEDEF(IMediaControl, IID_IMediaControl);
_COM_SMARTPTR_TYPEDEF(IMediaEvent, IID_IMediaEvent);
_COM_SMARTPTR_TYPEDEF(IMediaEventEx, IID_IMediaEventEx);
_COM_SMARTPTR_TYPEDEF(IMediaEventSink, IID_IMediaEventSink);
_COM_SMARTPTR_TYPEDEF(IMediaSeeking, IID_IMediaSeeking);
_COM_SMARTPTR_TYPEDEF(IBaseFilter, IID_IBaseFilter);
_COM_SMARTPTR_TYPEDEF(ICaptureGraphBuilder2, IID_ICaptureGraphBuilder2);
_COM_SMARTPTR_TYPEDEF(ICreateDevEnum, IID_ICreateDevEnum);
_COM_SMARTPTR_TYPEDEF(IPin, IID_IPin);
_COM_SMARTPTR_TYPEDEF(IFileSourceFilter, IID_IFileSourceFilter);

//Sample Grabber Interface
#include <QEdit.h>
_COM_SMARTPTR_TYPEDEF(ISampleGrabber, IID_ISampleGrabber);
_COM_SMARTPTR_TYPEDEF(ISampleGrabberCB, IID_ISampleGrabberCB);
// For compression
_COM_SMARTPTR_TYPEDEF(IAMStreamConfig, IID_IAMStreamConfig);
// Audio buffer latency configuration
_COM_SMARTPTR_TYPEDEF(IAMBufferNegotiation, IID_IAMBufferNegotiation);
// IReferenceClock
_COM_SMARTPTR_TYPEDEF(IReferenceClock, IID_IReferenceClock);
_COM_SMARTPTR_TYPEDEF(IMediaFilter, IID_IMediaFilter);
_COM_SMARTPTR_TYPEDEF(IAMCrossbar, IID_IAMCrossbar);

//Smart pointers for RTVC COM interfaces
_COM_SMARTPTR_TYPEDEF(ISettingsInterface, IID_ISettingsInterface);
_COM_SMARTPTR_TYPEDEF(IStatusInterface, IID_IStatusInterface);
