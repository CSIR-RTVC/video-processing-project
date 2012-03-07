@echo Running User.bat commands
@echo Setting DirectShow and SDK Paths

GOTO DETECT_SDK

REM try to detect windows SDK
:SET_WSDK_7_1
  @set DSHOWBASECLASSES=C:\Program Files\Microsoft SDKs\Windows\v7.1\Samples\Multimedia\DirectShow\BaseClasses
  @set WindowsSDKDir=C:\Program Files\Microsoft SDKs\Windows\v7.1
  @goto END

:SET_WSDK_7_0
  @set DSHOWBASECLASSES=C:\Program Files\Microsoft SDKs\Windows\v7.0\Samples\Multimedia\DirectShow\BaseClasses
  @set WindowsSDKDir=C:\Program Files\Microsoft SDKs\Windows\v7.0
  @goto END

:SET_WSDK_6_0A
  @set DSHOWBASECLASSES=C:\Program Files\Microsoft SDKs\Windows\v6.0A\Samples\Multimedia\DirectShow\BaseClasses
  @set WindowsSDKDir=C:\Program Files\Microsoft SDKs\Windows\v6.0A
  @goto END

:SET_WSDK_6_0
  @set DSHOWBASECLASSES=C:\Program Files\Microsoft SDKs\Windows\v6.0\Samples\Multimedia\DirectShow\BaseClasses
  @set WindowsSDKDir=C:\Program Files\Microsoft SDKs\Windows\v6.0
  @goto END
  
:DETECT_SDK
@IF exist "C:\Program Files\Microsoft SDKs\Windows\v7.1" GOTO SET_WSDK_7_1
@IF exist "C:\Program Files\Microsoft SDKs\Windows\v7.0" GOTO SET_WSDK_7_0
@IF exist "C:\Program Files\Microsoft SDKs\Windows\v6.0A" GOTO SET_WSDK_6_0A
@IF exist "C:\Program Files\Microsoft SDKs\Windows\v6.0" GOTO SET_WSDK_6_0

REM User can hard-code SDK version and path here
@set DSHOWBASECLASSES=C:\Program Files\Microsoft SDKs\Windows\v7.1\Samples\Multimedia\DirectShow\BaseClasses
@set WindowsSDKDir=C:\Program Files\Microsoft SDKs\Windows\v7.1
@goto END

:END
@echo -- WindowsSDKDir set to %WindowsSDKDir%
@echo -- DSHOWBASECLASSES set to %DSHOWBASECLASSES%
