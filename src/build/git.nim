#:___________________________________________________________________
#  osdf  |  Copyright (C) Ivan Mar (sOkam!)  |  GNU GPLv2 or later  |
#:___________________________________________________________________
from std/osproc import execCmdEx
# @deps ndk
import nstd/logger
import nstd/shell
import nstd/strings
# @deps buildsystem
import ./types


#_______________________________________
# @section GitHub Helpers
#_______________________________________
proc gh (args :varargs[string, `$`]) :void=  sh "gh "&args.join(" ")


#_______________________________________
# @section GitHub Release Entry Point
#_____________________________
proc release *(
    name : Name;
    repo : Repository
  ) :void=
  if osproc.execCmdEx( "gh --version" ).exitCode != 0:
    err "Automated creation of a release build has been requested, but couldn't find the required GitHub's CLI application:  gh"
    return
  echo name
  echo repo

