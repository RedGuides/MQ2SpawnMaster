!include "../global.mak"

ALL : "$(OUTDIR)\MQ2Spawnmaster.dll"

CLEAN :
	-@erase "$(INTDIR)\MQ2Spawnmaster.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\MQ2Spawnmaster.dll"
	-@erase "$(OUTDIR)\MQ2Spawnmaster.exp"
	-@erase "$(OUTDIR)\MQ2Spawnmaster.lib"
	-@erase "$(OUTDIR)\MQ2Spawnmaster.pdb"


LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib $(DETLIB) ..\Release\MQ2Main.lib /nologo /dll /incremental:no /pdb:"$(OUTDIR)\MQ2Spawnmaster.pdb" /debug /machine:I386 /out:"$(OUTDIR)\MQ2Spawnmaster.dll" /implib:"$(OUTDIR)\MQ2Spawnmaster.lib" /OPT:NOICF /OPT:NOREF 
LINK32_OBJS= \
	"$(INTDIR)\MQ2Spawnmaster.obj" \
	"$(OUTDIR)\MQ2Main.lib"

"$(OUTDIR)\MQ2Spawnmaster.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) $(LINK32_FLAGS) $(LINK32_OBJS)


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("MQ2Spawnmaster.dep")
!INCLUDE "MQ2Spawnmaster.dep"
!ELSE 
!MESSAGE Warning: cannot find "MQ2Spawnmaster.dep"
!ENDIF 
!ENDIF 


SOURCE=.\MQ2Spawnmaster.cpp

"$(INTDIR)\MQ2Spawnmaster.obj" : $(SOURCE) "$(INTDIR)"

