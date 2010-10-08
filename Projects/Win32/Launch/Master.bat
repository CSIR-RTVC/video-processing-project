@call VsVersion.bat

@call User.bat

@echo Running Master.bat commands
@echo Setting up System Specific Variables...

@echo Setting up Visual Studio Paths
@set VisualStudioPath=%VisualStudioRoot%\Common7\IDE
@echo -- VisualStudioPath set to %VisualStudioPath%
@set VCInstallDir=%VisualStudioRoot%\VC
@echo -- VCInstallDir set to %VCInstallDir%

@echo Setting RTVC Roots relative
@set RTVCRootDir=%cd%\..\..\..
@echo -- RTVCRootDir set to %RTVCRootDir%

@echo Setting Project Directories
@set SolutionDir=%RTVCRootDir%\Projects\Win32\%VS_VERSION%
@echo -- SolutionDir set to %SolutionDir%
@echo Setting Source Directories
@set SourceDir=%RTVCRootDir%\Source
@echo -- SourceDir set to %SourceDir%
@set RTVCLibDir=%SourceDir%\RtvcLib
@echo -- RTVCLibDir set to %RTVCLibDir%

@set BuildDir=%RTVCRootDir%\Build\%VS_VERSION%
@echo -- BuildDir set to %BuildDir%
@set ReleaseBuildDir=%BuildDir%\Release
@echo -- ReleaseBuildDir set to %ReleaseBuildDir%
@set DebugBuildDir=%BuildDir%\Debug
@echo -- DebugBuildDir set to %DebugBuildDir%
@set LibDir=%RTVCRootDir%\Lib\%VS_VERSION%
@echo -- LibDir set to %LibDir%

@echo Setting up 3rd party directories

@set LIVEMEDIAROOT=%RTVCRootDir%\Source\3rdParty\LiveMedia\live
@echo -- LIVEMEDIAROOT set to %LIVEMEDIAROOT%

@echo Setting up 3rd Party INCLUDE...
@set INCLUDE=%LIVEMEDIAROOT%\groupsock\include;%INCLUDE%
@set INCLUDE=%LIVEMEDIAROOT%\BasicUsageEnvironment\include;%INCLUDE%
@set INCLUDE=%LIVEMEDIAROOT%\liveMedia\include;%INCLUDE%
@set INCLUDE=%LIVEMEDIAROOT%\UsageEnvironment\include;%INCLUDE%

@set DevEnvDir=%VisualStudioRoot%\Common7\IDE

@echo Setting up System PATH...
@set PATH=%VisualStudioRoot%\Common7\Tools\Bin;%PATH%
@set PATH=%VisualStudioRoot%\Common7\IDE;%PATH%
@set PATH=%VisualStudioRoot%\Common7\Tools;%PATH%
@set PATH=%VisualStudioRoot%\VC\bin;%PATH%
@set PATH=%WindowsSDKDir%\Bin;%PATH%

@echo Setting up System INCLUDE...
@set INCLUDE=$(VCInstallDir)PlatformSDK\include;%INCLUDE%
@set INCLUDE=%WindowsSDKDir%\Include;%INCLUDE%
@set INCLUDE=$(VCInstallDir)include;%INCLUDE%
@set INCLUDE=$(FrameworkSDKDir)include;%INCLUDE%
@set INCLUDE=$(VCInstallDir)atlmfc\include;%INCLUDE%

@set INCLUDE=%DSHOWBASECLASSES%;%INCLUDE%
@set INCLUDE=%SourceDir%;%INCLUDE%
@set INCLUDE=%RTVCLibDir%;%INCLUDE%

@echo Setting up System LIB...
@set LIB=$(VCInstallDir)lib;
@set LIB=$(VCInstallDir)atlmfc\lib;%LIB%
@set LIB=$(VSInstallDir);%LIB%
@set LIB=%DXSDKDir%\lib;%LIB%
@set LIB=%WindowsSDKDir%\lib;%LIB%
@set LIB=%LibDir%;%LIB%

