#:___________________________________________________________________
#  osdf  |  Copyright (C) Ivan Mar (sOkam!)  |  GNU GPLv2 or later  |
#:___________________________________________________________________

#___________________________________________________________________________________________________
# @section Old Buildsystem: References
##[
#_____________________________
# Project Directories
#___________________
engineDir = srcDir.Dir('engine')          # Repository for engine code
gameDir   = srcDir.Dir('game')            # Repository for gamecode
binDir    = rootDir.Dir('bin')            # Output root folder where binaries will be compiled to
instDir   = rootDir.Dir('install-'+cName) # Default linux:  '/usr/local/games/quake3'
baseDir   = None                          # TODO: what is this DEFAULT_BASEDIR value used for?

#_____________________________
# Source code directories
#___________________
## Engine src folders      #  relative to srcDir, we need multiple links (virtual copies) for SCons. absolute = srcDir/'folder'
clDir  = 'client'
svDir  = 'server'
rcDir  = 'rendc'
r1Dir  = 'rend1'
r2Dir  = 'rend2'
rvDir  = 'rendv'
sdlDir = 'sdl'
qcmDir = 'qcommon'
unxDir = 'unix'
winDir = 'win32'
botDir = 'botlib'
#___________________
# Libraries
libDir  = 'lib'
jpgDir  = os.path.join(libDir,'jpeg')
pcreDir = os.path.join(libDir,'pcre')
#___________________
# Gamecode src folders
cgDir  = 'cgame'
sgDir  = 'sgame'
uiDir  = 'ui_q3'
phyDir = os.path.join(sgDir,'phy')
hudDir = os.path.join(cgDir,'hud')
#___________________
# LCC Compiler tools folder     #todo: Port from ioq3 Makefile 
# toolDir= lnkDir +s+ 'tools'
# lccDir = lnkDir+toolDir +s+ 'lcc'
#___________________
# Not used
asmDir = 'asm'
tuiDir = 'ui_ta'
#___________________
# New UI folders (WIP)
nuiDir       = 'ui'
nuiDir_menu  = os.path.join(nuiDir,'menu')
nuiDir_fwork = os.path.join(nuiDir,'framework')
nuiDir_color = os.path.join(nuiDir,'color')
]##

