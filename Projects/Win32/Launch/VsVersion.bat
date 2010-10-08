@echo Running VsVersion.bat commands

REM Set this to VC8 or VC9 depending on which VS version you want to use
@set VS_VERSION=VC9

IF %VS_VERSION% EQU VC8 (
@set VisualStudioRoot="C:\Program Files\Microsoft Visual Studio 8"
)

REM On a 64 bit OS -> set this to something like "C:\Program Files (x86)\Microsoft Visual Studio 9.0"
IF %VS_VERSION% EQU VC9 (
@set VisualStudioRoot="C:\Program Files\Microsoft Visual Studio 9.0"
)

@echo -- VisualStudioRoot set to %VisualStudioRoot%
@echo -- VS_SOL_EXT set to %VS_SOL_EXT%
