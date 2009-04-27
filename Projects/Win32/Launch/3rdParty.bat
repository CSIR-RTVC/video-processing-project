@echo Running 3rdParty.bat commands

@set LIVEMEDIAROOT=<<Insert LiveMedia live Directory>> 
@echo -- LIVEMEDIAROOT set to %LIVEMEDIAROOT%

@echo Setting up 3rd Party INCLUDE...
@set INCLUDE=%BOOSTROOT%;%INCLUDE%
@set INCLUDE=%LIVEMEDIAROOT%\groupsock\include;%INCLUDE%
@set INCLUDE=%LIVEMEDIAROOT%\BasicUsageEnvironment\include;%INCLUDE%
@set INCLUDE=%LIVEMEDIAROOT%\liveMedia\include;%INCLUDE%
@set INCLUDE=%LIVEMEDIAROOT%\UsageEnvironment\include;%INCLUDE%

@echo --------------------------------------------------------------------------------------------
@echo -- INCLUDEPATH set to %INCLUDE%
