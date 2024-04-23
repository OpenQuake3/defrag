#:___________________________________________________________________
#  osdf  |  Copyright (C) Ivan Mar (sOkam!)  |  GNU GPLv2 or later  |
#:___________________________________________________________________
# @deps ndk
import nstd/logger
import nstd/opts as cli
import confy
# @deps buildsystem
import ./build/types
import ./build/engine
import ./build/game
import ./build/release as rls


#_______________________________________
# @section Buildsystem Control
#_____________________________
const buildGame   = on   ## Whether to build the gamecode or not
const buildEngine = on   ## Whether to build the engine or not
const versGame    = version(0, 2,99)
const versEngine  = version(0, 0, 0)  # @todo Should be managed by the engine repo/buildsystem, not from here
# CLI Control
let publish    = cli.getOpt("publish")          ## `./bin/build --publish` to publish the result to GitHub
let release    = cli.getOpt("r") or publish     ## `./bin/build -r` to run the automatic release generation process
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
# Find the systems we compile/pack for
let systems = if distribute: @[
  System(os: Linux,   cpu: x86_64),
  System(os: Windows, cpu: x86_64),
  System(os: Mac,     cpu: x86_64),
  System(os: Mac,     cpu: arm64),
  ] else: @[confy.getHost()]
# Run each buildsystem module
if buildEngine : engine.build( name= name, systems= systems, pack= pack, version= versEngine )
if buildGame   :   game.build( name= name, systems= systems, pack= pack, version= versGame   )
if release     :    rls.local( name= name, systems= systems,             version= versGame   )
if publish     :   rls.github( name= name, systems= systems, repo= repo, version= versGame   )

