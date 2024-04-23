#:___________________________________________________________________
#  osdf  |  Copyright (C) Ivan Mar (sOkam!)  |  GNU GPLv2 or later  |
#:___________________________________________________________________
from std/osproc import execCmdEx
# @deps ndk
import nstd/logger
import nstd/shell
import nstd/strings
import confy except sh
# @deps buildsystem
import ./types


#_______________________________________
# @section Release Configuration
#_______________________________________
let rlsDir   = cfg.binDir/"releases"
let license  = "license.md"
let readme   = "readme.md"
#___________________
let docsFilter = [
  cfg.docDir/"notes",
  cfg.docDir/"+",
  cfg.docDir/"old",
  cfg.docDir/"build.md",
  cfg.docDir/"style.gpl.md"
  ] # << docsFilter = [ ... ]


#_______________________________________
# @section Release Helpers
#_____________________________
proc getRevNum *(
    version : Version;
    dir     : Path;
  ) :int=
  ## @descr Gets the release revision number for {@arg version}, based on the contents of folder [@arg dir}
  for it in dir.walkDir:
    if it.kind == pcFile: continue  # Ignore files, they don't influence the revision number
    let sub = it.path.relativePath(dir).string.split(DirSep)[0]  # Only count the root folder relative to {@arg dir}
    if $version notin sub: continue # Only increase the revision number for folders of the same (M,m,p) version
    result.inc


#_______________________________________
# @section Documentation Management
#_____________________________
proc copyDocs (dir :Path) :void=
  ## @descr Creates a copy of the documentation into {@arg dir}
  cp cfg.rootDir/license, dir/license   # Always add the license
  cp cfg.rootDir/readme,  dir/readme    # Always add the readme
  let trgDir = dir/cfg.docSub           # Subfolder of trg where the docs folder will be stored
  md trgDir
  cpDir cfg.docDir, trgDir, filter= docsFilter # Always add the doc folder to each release


#_______________________________________
# @section Local Release Entry Point
#_____________________________
proc local *(
    name    : Name;
    systems : openArray[confy.System];
    version : Version;
  ) :void=
  ## @descr Creates local release file for all of the given {@arg systems} using the {@arg name} of the mod on the resulting file
  let rev = &"-r{version.getRevNum(rlsDir)}"
  for sys in systems:                        # For every system we built for
    let sysSub = Path(&"{sys.os}-{sys.cpu}") # Name of the subfolder for this iteration's system
    let sub    = sysSub/name.short           # Name of the subfolder where everything was stored
    let dir    = cfg.binDir/sub              # Final name of the folder to create a release from
    copyDocs dir                             # Add the documentation to every release
    let zipName =                            # Final name of the resulting zip file
      if sys.os == Mac : &"{name.long}.{sys.os}-{sys.cpu}.{version}{rev}.zip"  # Only specify CPU for Mac
      else             : &"{name.long}.{sys.os}.{version}{rev}.zip"
    let versDir = rlsDir/($version&rev)
    if not dirExists(versDir): md versDir
    let zipFile = versDir/zipName
    zip dir, zipFile, rel=cfg.binDir/sysSub  # @note Very confusing. Relative to the system's sub, not `dir`. Otherwise we zip the mod's subdir, not the root


#_______________________________________
# @section GitHub Release Entry Point
#_____________________________
proc github *(
    name    : Name;
    systems : openArray[confy.System];
    repo    : Repository;
    version : Version;
  ) :void=
  if osproc.execCmdEx( "gh --version" ).exitCode != 0:
    err "Automated creation of a GitHub release build has been requested, but couldn't find the required GitHub's CLI application:  gh"
    return
  #for sys in systems:
  #  echo sys
  #echo name
  #echo repo

