#:___________________________________________________________________
#  osdf  |  Copyright (C) Ivan Mar (sOkam!)  |  GNU GPLv2 or later  |
#:___________________________________________________________________
# @deps ndk
import nstd/logger
import confy
# @deps build
import ./types

#___________________________________________________________________________________________________
from std/os import parentDir, `/`, dirExists
static:  # Ensure the engine's buildsystem folder exists at compile time
  const thisDir        = currentSourcePath.parentDir()
  const buildsystemDir = thisDir/"system"
  if not dirExists(buildsystemDir):
    let output = gorge "git clone https://github.com/OpenQuake3/buildsystem "&buildsystemDir
    echo output
#___________________________________________________________________________________________________
# @deps build.engine
import ./system/engine/src
import ./system/engine/dir


#_______________________________________
# @section Entry Point: Order to build
#_____________________________
proc build *(
    name       : Name;
    distribute : bool = off;
  ) :void=
  info "This will build id-Tech3 with name:  " & $name
  # Clone the repo   or   Pull latest changes
  # Apply custom patches
  #   -> patches/NAME/code/dir/file.c.patch
  # Move files/folders into the new structure
  # Create a local commit with `thisRepo/commitHash`
  # Build it with confy





#___________________________________________________________________________________________________
# @section Old Buildsystem: References
##[
# Set Defaults
# :::::::::::::::: 
# Output filename
cName             = 'osdf'
dName             = cName+'.ded'
render_prefix     = cName
# Select what to build
build_default     = ['game'] # ['release','debug',]   # List of targets to build by default. Will use debug if empty
# Select what systems to compile
use_local_jpeg    = True   # Links to local jpeg (windows only). Binaries are hard to find
use_local_pcre    = True   # Links to local pcre (windows only). Binaries are hard to find
#use_sdl           = True  # Disabled. Always True
#use_curl          = True  # Disabled. Always True
#use_local_headers = False # Disabled. Always False
#use_ccache        = True  # Deprecated. Scons has a compiler cache system
# Render compile settings
render_default    = 'opengl' # valid options: opengl, vulkan, opengl2
use_opengl        = True
use_opengl2       = False
use_vulkan        = True
use_vulkan_api    = use_vulkan #Default: True   #TODO: What is this used for?
use_opengl= use_opengl2= use_vulkan= use_vulkan_api= False
if render_default == 'opengl':  use_opengl  = True
if render_default == 'opengl2': use_opengl2 = True
if render_default == 'vulkan':  use_vulkan  = use_vulkan_api = True
]##

