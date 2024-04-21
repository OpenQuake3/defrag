# Notes
- Suppport for building on macosx not created yet: it needs to use ioq3 .sh scripts (ioq3: ROOT/*.sh)


# Folders Needed:
fileList.txt
- code/tools:   Everything
- code/cgame:   Everything
- code/game:    Everything
- code/qcommon: Everything
    ## *Might* get away with just:
    qcommon/  q_shared.h
    qcommon/  q_shared.c
    qcommon/  q_math.c
- code/ui:   Everything

# QVM tools only
QVM toolchain: lcc, rcc, q3asm

G: if you go in ioq3 and run make BUILD_CLIENT=0 BUILD_SERVER=0 BUILD_GAME_SO=0, this will build qvm tools + qvms only
sOkam!: could i get gamecode directly from gpl vanilla, and make it work by compiling in whatever engine?
G: yes. you don't compile in an engine
f: you don't need any engine to comile qvms. only qvm toolchain - lcc, rcc, q3asm. engine only runs qvms
G: you compile qvms first, then can run your .qvm files in any engine.  eg. ~/.q3a/baseq3/vm/cgame.qvm (edited)
by doing this. and when you run q3e/etc, the engine will load them