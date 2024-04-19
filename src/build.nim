# @deps std
from std/sequtils import filterIt
# @deps ndk
import confy


#_______________________________________
# @section Configuration
#_____________________________
cfg.nim.systemBin = off
cfg.libDir  = cfg.srcDir/"lib"
let ioq3Dir = cfg.libDir/"ioq3"/"code"


#_______________________________________
# @section Source code
#_____________________________
# Game: Common to all Libraries
let src_libAll = @[
  ioq3Dir/"qcommon"/"q_shared.c",
  ioq3Dir/"qcommon"/"q_math.c",
  #ioq3Dir/"qcommon"/"common.c",
  ].toDirFile # << src_libAll = @[ ... ]
#___________________
# Game: Client
let src_cgame = glob(ioq3Dir/"cgame").filterIt(
  # Excludes
  it.path.lastPathPart.string != "cg_newdraw.c"
  ) & @[
  # Explicit Includes
  ioq3Dir/"game"/"bg_misc.c",
  ioq3Dir/"game"/"bg_pmove.c",
  ioq3Dir/"game"/"bg_slidemove.c",
  # Common to all Libraries
  ].toDirFile  & src_libAll # << src_cgame = ...
#___________________
# Game: Server
let src_sgame = glob(ioq3Dir/"game").filterIt(
  #not it.path.lastPathPart.string.startsWith("ai_") and
  it.path.lastPathPart.string != "g_rankings.c"
  # Explicit Includes
  ) & @[
  # Common to all Libraries
  ].toDirFile & src_libAll # << src_sgame = ...
#___________________
# Game: UI
let src_ui = glob(ioq3Dir/"q3_ui").filterIt(
  it.path.lastPathPart.string != "ui_signup.c" and
  it.path.lastPathPart.string != "ui_specifyleague.c" and
  it.path.lastPathPart.string != "ui_rankings.c" and
  it.path.lastPathPart.string != "ui_rankstatus.c" and
  it.path.lastPathPart.string != "ui_login.c"
  # Explicit Includes
  ) & @[
  ioq3Dir/"game"/"bg_misc.c",
  ioq3Dir/"ui"/"ui_syscalls.c",
  # Common to all Libraries
  ].toDirFile & src_libAll # << src_ui = ...


#_______________________________________
# @section Compile Flags
#_____________________________
let arch = "x86_64"
let q3_noErr = @[
  # Quake3 Requirements
  &"-DARCH_STRING=\\\"{arch}\\\"",
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
  "-Wno-unused-parameter",
  "-Wno-unused-but-set-variable",
  "-Wno-shadow",
  "-Wno-empty-translation-unit",
  # Stop ZigCC from crashing the game due to UBSAN traps
  "-fno-sanitize-trap=all",
  "-lubsan",
  ].toCC # << args_libs = @[ ... ]


#_______________________________________
# @section Buildsystem Management
#_____________________________
# Submodules
let ioq3 = submodule("ioq3", "https://github.com/ioquake/ioq3", "code")
# Cross-Compilation
proc buildFor (trg :confy.BuildTrg; args :varargs[confy.System]) :void=
  var tmp = trg
  for sys in args:
    tmp.syst = sys
    echo $sys.os
    tmp.sub = Path &"{sys.os}-{sys.cpu}"
    tmp.build()


#_______________________________________
# Build: Game Client
#_____________________________
let cgame = SharedLibrary.new(
  src   = src_cgame,
  trg   = "cgame"&arch,
  deps  = Dependencies.new(ioq3),
  flags = confy.flags(C) & q3_noErr,
  syst  = confy.getHost(),
  ) # << Program.new( ... )
# cgame.buildFor( confy.getHost() )
cgame.buildFor(
  System(os: OS.Linux,   cpu: CPU.x86_64),
  System(os: OS.Windows, cpu: CPU.x86_64),
  System(os: OS.Mac,     cpu: CPU.x86_64),
  System(os: OS.Mac,     cpu: CPU.arm64),
  ) # << cgame.buildFor( ... )


#_______________________________________
# Build: Game Server
#_____________________________
let sgame = SharedLibrary.new(
  src   = src_sgame,
  trg   = "qagame"&arch,
  deps  = Dependencies.new(ioq3),
  flags = confy.flags(C) & q3_noErr,
  syst  = confy.getHost(),
  ) # << Program.new( ... )
# sgame.buildFor( confy.getHost() )
sgame.buildFor(
  System(os: OS.Linux,   cpu: CPU.x86_64),
  System(os: OS.Windows, cpu: CPU.x86_64),
  System(os: OS.Mac,     cpu: CPU.x86_64),
  System(os: OS.Mac,     cpu: CPU.arm64),
  ) # << sgame.buildFor( ... )


#_______________________________________
# Build: Game UI
#_____________________________
let ui = SharedLibrary.new(
  src   = src_ui,
  trg   = "ui"&arch,
  deps  = Dependencies.new(ioq3),
  flags = confy.flags(C) & q3_noErr,
  syst  = confy.getHost(),
  ) # << Program.new( ... )
# ui.buildFor( confy.getHost() )
ui.buildFor(
  System(os: OS.Linux,   cpu: CPU.x86_64),
  System(os: OS.Windows, cpu: CPU.x86_64),
  System(os: OS.Mac,     cpu: CPU.x86_64),
  System(os: OS.Mac,     cpu: CPU.arm64),
  ) # << ui.buildFor( ... )


