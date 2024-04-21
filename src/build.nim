#:___________________________________________________________________
#  osdf  |  Copyright (C) Ivan Mar (sOkam!)  |  GNU GPLv2 or later  |
#:___________________________________________________________________
# @deps buildsystem
import ./build/types
import ./build/engine
import ./build/game
import nstd/logger


#_______________________________________
# @section Buildsystem Control
#_____________________________
const distribute  = on                  ## `on` will build+pack everything for all platforms
const buildGame   = on                  ## Whether to build the gamecode or not
const buildEngine = on                  ## Whether to build the gamecode or not


#_______________________________________
# @section Buildsystem Configuration
#_____________________________
const verbose = off                 ## Log debug messages from the buildsystem to CLI
const silent  = off and not verbose ## Silence every CLI message
const name = Name(
  short : "osdf",
  long  : "opensource-defrag",
  human : "Opensource Defrag"
  ) # << name = (... )


#_______________________________________
# @section Buildsystem Entry Point
#_____________________________
logger.init(name=name.short, threshold=
  when verbose: Log.All elif silent: Log.None else: Log.Err
  ) # << logger.init( ... )
#___________________
# Run each buildsystem module
when buildEngine : engine.build( name= name, distribute= distribute )
when buildGame   :   game.build( name= name, distribute= distribute )

