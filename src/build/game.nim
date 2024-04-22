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
proc copyCfg (trgDir :Path) :void=
  # Ensure the target folder exists
  if dirExists(trgDir): md trgDir
  # Copy all files in the src/cfg folder into trgDir
  for it in cfgDir.walkDir:
    if it.path.string.startsWith("bundle", "bkp", "defrag"): continue
    if   it.kind == pcFile : cp    it.path, trgDir/it.path.lastPathPart
    elif it.kind == pcdir  : cpDir it.path, trgDir/it.path.lastPathPart
#___________________
# Cross-Compilation
proc buildFor (trg :confy.BuildTrg; args :openArray[confy.System]) :void=
  var tmp = trg
  for sys in args:
    let arch = case sys.cpu
      of x86_64: "x86_64"
      else: $sys.cpu
    tmp.syst = sys
    tmp.trg  =
      if arch notin tmp.trg.string : Path tmp.trg.string.replace("x86_64","") & arch
      else                         : tmp.trg
    tmp.sub = Path &"{sys.os}-{sys.cpu}"
    if sys.cpu == arm64: tmp.flags.cc = tmp.flags.cc.filterIt( "ARCH_STRING=" notin it )
    # Build for the target
    if not dirExists(cfg.binDir/tmp.sub): md cfg.binDir/tmp.sub
    tmp.build()
    # Copy the mod's configuration files to the target folder
    copyCfg cfg.binDir/tmp.sub
#___________________
# Automated Packing
proc packFor (trg :confy.BuildTrg; args :openArray[confy.System]) :void=
  var tmp = trg
  for sys in args:
    let sub   = &"{sys.os}-{sys.cpu}"
    let dir   = cfg.binDir/sub
    let files = trg.src.getFileList( cfg.srcDir/"game" )
    files.zip( dir/"tst.pk3", rel=cfg.srcDir )
#___________________
const DefaultAssetIgnores = @["src".Path]  ## Subfolders of the assets folder that should be ignored by default
proc packAssetsFor *(
    trg        : confy.BuildTrg;
    args       : openArray[confy.System];
    assetsDir  : Path;
    name       : Name;
    ignoreList : openArray[Path] = DefaultAssetIgnores;
  ) :void=
  var tmp = trg
  var dirs :seq[Path]
  for it in assetsDir.walkDir:
    if it.kind != pcDir      : continue
    if it.path in ignoreList : continue
    dirs.add it.path
  for sys in args:
    for dir in dirs:
      var files :seq[Path]
      for file in dir.walkDirRec: files.add file.Path
      let name = &"y.{name.short}-{dir.lastPathPart}.pk3"
      tmp.sub = Path &"{sys.os}-{sys.cpu}"
      files.zip( cfg.binDir/tmp.sub/name )


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
  for sys in systems:
    if not pack: break
    game.packAssetsFor(systems, assetsDir, name)


  # Build: Game Client
  #_____________________________
  var cgame = game
  cgame.src = src_cgame
  cgame.trg = ("cgame"&arch).Path
  #___________________
  cgame.buildFor(systems)
  if pack: cgame.packFor(systems)


  # Build: Game Server
  #_____________________________
  var sgame = game
  sgame.src = src_sgame
  sgame.trg = ("qagame"&arch).Path
  #___________________
  sgame.buildFor(systems)
  if pack: sgame.packFor(systems)


  # Build: Game UI
  #_____________________________
  var ui = game
  ui.src = src_ui
  ui.trg = ("ui"&arch).Path
  #___________________
  ui.buildFor(systems)
  if pack: ui.packFor(systems)






#___________________________________________________________________________________________________
# @section Old Buildsystem: References
##[
# SCons
scPlatforms    = ['posix', 'win32', 'cygwin', 'darwin', 'aix', 'hpux', 'irix', 'os2', 'sunos']
scArchs        = { 64:['x86_64','amd64'], 32:['x86','arm']}
def mkTruArch(bits,dic):  # Creates and returns a list of (64 or 32) platform aliases contained within the values of `dic`
  #explanation            [val.iterable (for sublist in list_of_lists:  for val in sublist: if     condition:                    )]
  if   bits == 64: return [val           for key,lst in dic.items()     for val in lst      if not any(ch.isdigit() for ch in val)]  # Dictionary value = list of 64bit platforms
  elif bits == 32: return [val           for key,lst in dic.items()     for val in lst      if     any(ch.isdigit() for ch in val)]  # Dictionary value = list of 32bit platforms
  else: sys.exit('::ERR Unsupported bits input for the function mkTruArch')
truPlatform    = {'win32':['w','win','w32','win32'],                   # Accepted win32 aliases in p=X, platform=X
                  'posix':['l','lnx','linux','l32','lnx32','linux32']} # Accepted posix aliases
truArch        = {'x86_64':mkTruArch(64,truPlatform), 'x86':mkTruArch(32,truPlatform)}        # Generated. Each value contains a list of valid platform aliases, with arch assigned as its dict key

# Supported Lists
validTargets = [  # make-like target selection
  # @todo Figure out what to do with these, and how (or if) they fit confy
  'release','debug',      'distribute',  'all',
  'engine', 'engine-dbg', 'engine-dist', 'engine-sdl',
  # 'server', 'server-dbg', 'server-dist',  #TODO: 'server',
  'game',   'game-dbg',   'game-dist',
  'q3ui',   'nui'  # New UI (wip)  #TODO: merge to game when done
  ]
# Aliases
  # Q3 renames  (not using them, keeping only as reference for future support implementation)
  # q3Platforms  = ['x86_64', 'x86', 'mingw32', 'mingw64','darwin', 'aarch64']
  # remaps:        'arm64':'aarch64',  'mingw32'+'i386':'x86',   'cygwin':'mingw32',   'arm':'aarch64'
  # q3Archs      = ['i86pc','x86','x86_64','x64']
  # remaps:        'i86pc':'x86',   'x86_64'or'x64':'x86_64'
# Other
vmArchs = ['x86_64', 'x86', 'arm', 'aarch64'] # List of architectures compatible with vm compiling  #TEMP: Q3 names. fix this
]##

