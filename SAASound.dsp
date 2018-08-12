# Microsoft Developer Studio Project File - Name="SAASound" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=SAASound - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "SAASound.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "SAASound.mak" CFG="SAASound - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "SAASound - Win32 Release" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "SAASound - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "SAASound - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /G6 /Gr /Zp1 /MD /W3 /vd0 /Ox /Ot /Oa /Og /Oi /Ob2 /Gf /Gy /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "WIN32_LEAN_AND_MEAN" /D "VC_EXTRALEAN" /D "NEW_RAND" /FAcs /FR /YX /FD /c
# SUBTRACT CPP /Os
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o NUL /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o NUL /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 /nologo /base:"0x178a0000" /version:3.1 /subsystem:windows /dll /machine:I386
# SUBTRACT LINK32 /profile /map /debug
# Begin Special Build Tool
SOURCE=$(InputPath)
PostBuild_Cmds=copy release\saasound.dll c:\windows\system
# End Special Build Tool

!ELSEIF  "$(CFG)" == "SAASound - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /G5 /Gr /Zp16 /MDd /W3 /Gm /vd0 /Zi /Od /Gf /Gy /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "NEW_RAND" /D "DEBUGSAA" /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# SUBTRACT LINK32 /map
# Begin Special Build Tool
SOURCE=$(InputPath)
PostBuild_Desc=copying dll
PostBuild_Cmds=copy "Debug\saasound.dll" "c:\windows\system"
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "SAASound - Win32 Release"
# Name "SAASound - Win32 Debug"
# Begin Group "C++ Source Code"

# PROP Default_Filter "cpp"
# Begin Source File

SOURCE=.\SAAAmp.cpp
# End Source File
# Begin Source File

SOURCE=.\SAAEnv.cpp
# End Source File
# Begin Source File

SOURCE=.\SAAFreq.cpp
# End Source File
# Begin Source File

SOURCE=.\SAANoise.cpp
# End Source File
# Begin Source File

SOURCE=.\SaaSndC.cpp
# End Source File
# Begin Source File

SOURCE=.\SAASoundImplementation.cpp
# End Source File
# End Group
# Begin Group "C++ Header Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\SAAAmp.h
# End Source File
# Begin Source File

SOURCE=.\SAAEnv.h
# End Source File
# Begin Source File

SOURCE=.\SAAFreq.h
# End Source File
# Begin Source File

SOURCE=.\SAANoise.h
# End Source File
# Begin Source File

SOURCE=.\SAASndC.h
# End Source File
# Begin Source File

SOURCE=.\SAASound.h
# End Source File
# Begin Source File

SOURCE=.\SAASoundImplementation.h
# End Source File
# Begin Source File

SOURCE=.\types.h
# End Source File
# End Group
# Begin Group "Nasty Inline Stuff"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\SAAFreqTable.dat
# End Source File
# End Group
# Begin Source File

SOURCE=.\Licence.txt
# End Source File
# Begin Source File

SOURCE=.\SAASound.DEF
# End Source File
# Begin Source File

SOURCE=.\SAASound.rc
# End Source File
# End Target
# End Project
