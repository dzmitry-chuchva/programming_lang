# Microsoft Developer Studio Project File - Name="parser" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=parser - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "parser.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "parser.mak" CFG="parser - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "parser - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "parser - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "parser - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\Release"
# PROP BASE Intermediate_Dir ".\Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\Release"
# PROP Intermediate_Dir ".\Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /ML /W3 /O2 /Ob1 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /GF /Gy /YX /Fp".\Release/parser.pch" /Fo".\Release/" /Fd".\Release/" /c /GX 
# ADD CPP /nologo /ML /W3 /O2 /Ob1 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /GF /Gy /YX /Fp".\Release/parser.pch" /Fo".\Release/" /Fd".\Release/" /c /GX 
# ADD BASE MTL /nologo /tlb".\Release\parser.tlb" /win32 
# ADD MTL /nologo /tlb".\Release\parser.tlb" /win32 
# ADD BASE RSC /l 1049 /d "NDEBUG" 
# ADD RSC /l 1049 /d "NDEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo 
# ADD BSC32 /nologo 
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /out:".\Release\compiler.exe" /incremental:no /pdb:".\Release\parser.pdb" /pdbtype:sept /subsystem:console 
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /out:".\Release\compiler.exe" /incremental:no /pdb:".\Release\parser.pdb" /pdbtype:sept /subsystem:console 

!ELSEIF  "$(CFG)" == "parser - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\Debug"
# PROP BASE Intermediate_Dir ".\Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\Debug"
# PROP Intermediate_Dir ".\Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MLd /ZI /W3 /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" PRECOMP_VC7_TOBEREMOVED /Fp".\Debug/parser.pch" /Fo".\Debug/" /Fd".\Debug/" /GZ /c /GX 
# ADD CPP /nologo /MLd /ZI /W3 /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" PRECOMP_VC7_TOBEREMOVED /Fp".\Debug/parser.pch" /Fo".\Debug/" /Fd".\Debug/" /GZ /c /GX 
# ADD BASE MTL /nologo /tlb".\Debug\parser.tlb" /win32 
# ADD MTL /nologo /tlb".\Debug\parser.tlb" /win32 
# ADD BASE RSC /l 1049 /d "_DEBUG" 
# ADD RSC /l 1049 /d "_DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo 
# ADD BSC32 /nologo 
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /out:".\Debug\compiler.exe" /incremental:no /debug /pdb:".\Debug\parser.pdb" /pdbtype:sept /subsystem:console 
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /out:".\Debug\compiler.exe" /incremental:no /debug /pdb:".\Debug\parser.pdb" /pdbtype:sept /subsystem:console 

!ENDIF

# Begin Target

# Name "parser - Win32 Release"
# Name "parser - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\asm_code.c
# End Source File
# Begin Source File

SOURCE=.\code3.c
# End Source File
# Begin Source File

SOURCE=.\codetable.c
# End Source File
# Begin Source File

SOURCE=lex.c

!IF  "$(CFG)" == "parser - Win32 Release"

# ADD CPP /nologo /O2 /GX 
!ELSEIF  "$(CFG)" == "parser - Win32 Debug"

# ADD CPP /nologo /Od /GZ /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=.\main.c
# End Source File
# Begin Source File

SOURCE=parse.c

!IF  "$(CFG)" == "parser - Win32 Release"

# ADD CPP /nologo /O2 /GX 
!ELSEIF  "$(CFG)" == "parser - Win32 Debug"

# ADD CPP /nologo /Od /GZ /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=symbol.c

!IF  "$(CFG)" == "parser - Win32 Release"

# ADD CPP /nologo /O2 /GX 
!ELSEIF  "$(CFG)" == "parser - Win32 Debug"

# ADD CPP /nologo /Od /GZ /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=.\unistack.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\asm_code.h
# End Source File
# Begin Source File

SOURCE=.\code3.h
# End Source File
# Begin Source File

SOURCE=.\codetable.h
# End Source File
# Begin Source File

SOURCE=.\main.h
# End Source File
# Begin Source File

SOURCE=parse.h
# End Source File
# Begin Source File

SOURCE=symbol.h
# End Source File
# Begin Source File

SOURCE=.\unistack.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Source File

SOURCE=lex.l

!IF  "$(CFG)" == "parser - Win32 Debug"

# PROP Ignore_Default_Tool 1
USERDEP_FILE="parse.h"	
# Begin Custom Build - Compiling lex.l
SOURCE="$(InputPath)"

BuildCmds= \
	flex -l -olex.c lex.l \


"lex.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF

# End Source File
# Begin Source File

SOURCE=parse.y

!IF  "$(CFG)" == "parser - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Compiling parse.y
SOURCE="$(InputPath)"

BuildCmds= \
	bison -dtv -oparse.c parse.y \


"parse.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"parse.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF

# End Source File
# End Target
# End Project

