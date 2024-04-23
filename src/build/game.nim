#:___________________________________________________________________
#  osdf  |  Copyright (C) Ivan Mar (sOkam!)  |  GNU GPLv2 or later  |
#:___________________________________________________________________
# @deps std
from std/sequtils import filterIt
# @deps ndk
import nstd/strings
import nstd/shell
import nstd/zip as z
import confy
# @deps build
import ./types


#_______________________________________
# @section Configuration
#_____________________________
nim.systemBin = off
cfg.libDir    = cfg.srcDir/"lib"
let gameDir   = cfg.srcDir/"game"
let cfgDir    = cfg.srcDir/"cfg"
let assetsDir = cfg.rootDir/"assets"

#_______________________________________
# @section Source code
#_____________________________
# Game: Common to all Libraries
let src_libAll = @[
  gameDir/"qcommon"/"q_shared.c",
  gameDir/"qcommon"/"q_math.c",
  ].toDirFile # << src_libAll = @[ ... ]
#___________________
# Game: Client
let src_phy   = glob(gameDir/"sgame"/"phy")
let src_hud   = glob(gameDir/"cgame"/"hud")
let src_cgame = glob(gameDir/"cgame").filterIt(
  # Excludes
  it.path.lastPathPart.string != "cg_newdraw.c"
  # Explicit Includes
  ) & src_phy & src_hud & @[
  gameDir/"sgame"/"bg_misc.c",
  gameDir/"sgame"/"bg_pmove.c",
  gameDir/"sgame"/"bg_slidemove.c",
  # Common to all Libraries
  ].toDirFile  & src_libAll # << src_cgame = ...
#___________________
# Game: Server
let src_sgame = glob(gameDir/"sgame").filterIt(
  #not it.path.lastPathPart.string.startsWith("ai_") and
  it.path.lastPathPart.string != "g_rankings.c"
  # Explicit Includes
  ) & src_phy & @[
  # Common to all Libraries
  ].toDirFile & src_libAll # << src_sgame = ...
#___________________
# Game: UI
let src_ui = glob(gameDir/"ui_q3").filterIt(
  it.path.lastPathPart.string != "ui_signup.c" and
  it.path.lastPathPart.string != "ui_specifyleague.c" and
  it.path.lastPathPart.string != "ui_rankings.c" and
  it.path.lastPathPart.string != "ui_rankstatus.c" and
  it.path.lastPathPart.string != "ui_login.c"
  # Explicit Includes
  ) & @[
  gameDir/"sgame"/"bg_misc.c",
  # Common to all Libraries
  ].toDirFile & src_libAll # << src_ui = ...


#_______________________________________
# @section Compile Flags
#_____________________________
let arch = "x86_64"
let q3_noErr = @[
  # Quake3 Requirements
  &"-DARCH_STRING=\\\"{arch}\\\"",
  # Recoverable warnings
  "-Wno-error=extra-semi",
  "-Wno-error=unused-function",
  "-Wno-error=unused-variable",
  "-Wno-error=strict-prototypes",
  "-Wno-unused-parameter",
  "-Wno-unused-but-set-variable",
  # Type Coercion
  "-Wno-sign-conversion",
  "-Wno-sign-compare",
  "-Wno-implicit-int-float-conversion",
  "-Wno-implicit-int-conversion",
  "-Wno-implicit-float-conversion",
  "-Wno-float-conversion",
  "-Wno-float-equal",
  "-Wno-double-promotion",
  "-Wno-shorten-64-to-32",
  # Strict Warnings
  "-Wno-missing-prototypes",
  "-Wno-missing-field-initializers",
  "-Wno-shadow",
  "-Wno-empty-translation-unit",
  "-Wno-error=macro-redefined",
  # Stop ZigCC from crashing the game due to UBSAN traps
  "-fno-sanitize=all",
  ].toCC # << args_libs = @[ ... ]


#_______________________________________
# @section Buildsystem Management
#_____________________________
# Configuration Generation
const DefaultConfigIgnores = @["bundle".Path, "bkp".Path, "defrag".Path]  ## Subfolders of the config folder that should be ignored by default
proc copyCfg *(
    trgDir     : Path;
    ignoreList : openArray[Path] = DefaultConfigIgnores;
  ) :void=
  # Ensure the target folder exists
  if dirExists(trgDir): md trgDir
  # Copy all files in the src/cfg folder into trgDir
  for it in cfgDir.walkDir:
    if it.path.lastPathPart in ignoreList: continue
    if   it.kind == pcFile : cp    it.path, trgDir/it.path.lastPathPart
    elif it.kind == pcdir  : cpDir it.path, trgDir/it.path.lastPathPart
#___________________
# Cross-Compilation
proc buildFor *(trg :confy.BuildTrg; systems :openArray[confy.System]) :void=
  ## @descr Compiles the {@arg trg} for all the given {@arg systems}
  var tmp = trg
  for sys in systems:
    let arch = case sys.cpu  # Architecture name that id-Tech3 understands
      of x86_64 : "x86_64"   # Rename `amd64` to id-Tech3's `x86_64`
      else      : $sys.cpu   # Use confy's name otherwise
    tmp.syst = sys           # The system we are building for
    tmp.trg  =               # Real target file we are building
      if arch notin tmp.trg.string : Path tmp.trg.string.replace("x86_64","") & arch
      else                         : tmp.trg
    tmp.sub = Path &"{sys.os}-{sys.cpu}"  # Subfolder of cfg.bindir where the resulting binaries will be output
    if sys.cpu == arm64: tmp.flags.cc = tmp.flags.cc.filterIt( "ARCH_STRING=" notin it )  # Fix ARCH_STRING bug when compiling for arm64
    # Build for the target
    if not dirExists(cfg.binDir/tmp.sub): md cfg.binDir/tmp.sub
    tmp.build()
    # Copy the mod's configuration files to the target folder
    copyCfg cfg.binDir/tmp.sub
#___________________
# Automated Packing
proc packCodeFor *(trg :confy.BuildTrg; systems :openArray[confy.System]) :void=
  ## @descr Packs the source code for {@arg trg} into the subfolders of all given {@arg systems}
  for sys in systems: # For every system we compile for
    let sub     = &"{sys.os}-{sys.cpu}"                     # Name of the subfolder where the .pk3 will be output
    let dir     = cfg.binDir/sub                            # Path of the folder where the .zip file will be output
    let files   = trg.src.getFileList( cfg.srcDir/"game" )  # Find the list of files from the BuildTrg object
    let trgFile = dir/trg.trg.addFileExt(".code.zip")       # Full Path of the final .zip file
    if not dirExists(dir): md dir
    files.zip( trgFile, rel=cfg.srcDir/"game" )
#___________________
const DefaultAssetIgnores = @["src".Path]  ## Subfolders of the assets folder that should be ignored by default
proc packAssetsFor *(
    assetsDir  : Path;
    systems    : openArray[confy.System];
    name       : Name;
    ignoreList : openArray[Path] = DefaultAssetIgnores;
  ) :void=
  ## @descr
  ##  Packs all asset folders contained in {@arg assetsDir} into the subfolders of all given {@arg systems}
  ##  The resulting file will contain the {@arg name}'s short name
  ##  Accepts an optional {@arg ignoreList} that specifies the list of filenames contained in {@arg assetsDir} that should be skipped from the resulting .pk3
  # Find the list of folders we should be packing
  var dirs :seq[Path]
  for it in assetsDir.walkDir:
    if it.kind != pcDir      : continue  # Skip all files at the root of assetsDir
    if it.path in ignoreList : continue  # Skip all paths that are part of the ignoreList
    dirs.add it.path
  # Pack for every system + assets/subfolder
  for sys in systems:  # For every system we compile for
    for dir in dirs:   # For every folder in assetsDir
      var files :seq[Path]
      for file in dir.walkDirRec: files.add file.Path      # Add every file in the assets subfolder to the list
      let name = &"y.{name.short}-{dir.lastPathPart}.pk3"  # Name of the final .pk3 that the user will see
      let sub  = Path &"{sys.os}-{sys.cpu}"                # Name of the subfolder where the .pk3 will be output
      let trgDir = cfg.binDir/sub
      if not dirExists(trgDir): md trgDir
      files.zip( trgDir/name )


#_______________________________________
# @section Entry Point: Game Buildsystem
#_____________________________
proc build *(
    name       : Name;
    cross      : bool = off;
    pack       : bool = off;
    assetsDir  : Path = assetsDir;
  ) :void=
  # Find the systems we compile/pack for
  let systems = if cross: @[
    System(os: Linux,   cpu: x86_64),
    System(os: Windows, cpu: x86_64),
    System(os: Mac,     cpu: x86_64),
    System(os: Mac,     cpu: arm64),
    ] else: @[confy.getHost()]
  #_____________________________
  # Define the Game Base BuildTarget
  let game = SharedLibrary.new(
    src   = cfg.srcDir/"tst.c", # Dummy path. Just for init
    flags = confy.flags(C) & q3_noErr,
    ) # << SharedLibrary.new( ... )


  # Build: Game Client
  #_____________________________
  var cgame = game
  cgame.src = src_cgame
  cgame.trg = ("cgame"&arch).Path
  #___________________
  cgame.buildFor(systems)


  # Build: Game Server
  #_____________________________
  var sgame = game
  sgame.src = src_sgame
  sgame.trg = ("qagame"&arch).Path
  #___________________
  sgame.buildFor(systems)


  # Build: Game UI
  #_____________________________
  var ui = game
  ui.src = src_ui
  ui.trg = ("ui"&arch).Path
  #___________________
  ui.buildFor(systems)


  # Pack the assets
  #_____________________________
  if pack:
    assetsDir.packAssetsFor(systems, name)  # rootDir/assets/*
    cgame.packCodeFor(systems)              # cgameARCH.code.zip
    sgame.packCodeFor(systems)              # sgameARCH.code.zip
    ui.packCodeFor(systems)                 #    uiARCH.code.zip






#___________________________________________________________________________________________________
# @section Old Buildsystem: References
##[
# Supported Lists
validTargets = [  # make-like target selection
  # @todo Figure out what to do with these, and how (or if) they fit confy
  'release','debug',      'distribute',  'all',
  'engine', 'engine-dbg', 'engine-dist', 'engine-sdl',
  # 'server', 'server-dbg', 'server-dist',  #TODO: 'server',
  'game',   'game-dbg',   'game-dist',
  'q3ui',   'nui'  # New UI (wip)  #TODO: merge to game when done
  ]
# Q3 os+cpu renames    (not using them, keeping only as reference for future support implementation)
# q3Platforms.remaps:  'arm64':'aarch64', 'mingw32'+'i386':'x86',   'cygwin':'mingw32',   'arm':'aarch64'
#     q3Archs.remaps:  'i86pc':'x86',      'x86_64'or'x64':'x86_64'
#            vmArchs:  ['x86_64', 'x86', 'arm', 'aarch64'] # List of architectures compatible with vm compiling  #TEMP: Q3 names
]##

