
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

