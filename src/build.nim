#:___________________________________________________________________
#  osdf  |  Copyright (C) Ivan Mar (sOkam!)  |  GNU GPLv2 or later  |
#:___________________________________________________________________
# @deps ndk
import nstd/logger
import nstd/opts as cli
# @deps buildsystem
import ./build/types
import ./build/engine
import ./build/game
import ./build/git


#_______________________________________
# @section Buildsystem Control
#_____________________________
const buildGame   = on   ## Whether to build the gamecode or not
const buildEngine = on   ## Whether to build the engine or not
# CLI Control
let release    = cli.getOpt("r")                ## `./bin/build -r` to run the automatic GitHub release process
let distribute = cli.getOpt("d") or release     ## `./bin/build -d` to build the distributable version
let pack       = cli.getOpt("p") or distribute  ## `./bin/build -p` to pack everything


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
const repo = Repository(
  owner : "OpenQuake3",
  name  : "defrag"
  ) # << Repository( ... )


#_______________________________________
# @section Buildsystem Entry Point
#_____________________________
logger.init(name=name.short, threshold=
  when verbose: Log.All elif silent: Log.None else: Log.Err
  ) # << logger.init( ... )
#___________________
# Run each buildsystem module
when buildEngine : engine.build( name= name, cross= distribute, pack= pack )
when buildGame   :   game.build( name= name, cross= distribute, pack= pack )
if release       :  git.release( name= name, repo=  repo                   )

