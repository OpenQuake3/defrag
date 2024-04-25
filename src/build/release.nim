#:___________________________________________________________________
#  osdf  |  Copyright (C) Ivan Mar (sOkam!)  |  GNU GPLv2 or later  |
#:___________________________________________________________________
# @deps std
from std/osproc import execCmdEx
from std/sequtils import filterIt
# @deps ndk
import nstd/logger
import nstd/shell except git
import nstd/strings
import nstd/git
import confy except sh, git
# @deps buildsystem
import ./types


#_______________________________________
# @section Release Configuration
#_______________________________________
let rlsDir   = cfg.binDir/"releases"
let license  = "license.md"
let readme   = "readme.md"
let notes    = "patchnotes.md"
#___________________
let docsFilter = [
  cfg.docDir/"notes",
  cfg.docDir/"+",
  cfg.docDir/"old",
  cfg.docDir/"build.md",
  cfg.docDir/"style.gpl.md"
  ] # << docsFilter = [ ... ]
#___________________
const FirstCommit {.strdefine.}= "0d4ad80e"  ## First commmit of this repository. Used as fallback when a version tag cannot be found


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
  # Find the data we need for error cheching
  let tag          = $version
  let previousVers = git.getPreviousTag( fallback= FirstCommit )
  # Error check
  if osproc.execCmdEx( "gh --version" ).exitCode != 0:
    err "Requested automated creation of a GitHub release, but couldn't find GitHub's CLI application:  gh"
    return
  if tag == previousVers:
    err &"Requested automated creation of a GitHub release, but the requested version is equal to the last tag on this repository:  {tag} .. {previousVers}"
    return
  # Find the rest of the data we need
  let patchnotes = git.getChangesSince(previousVers)
  let rev        = &"-r{version.getRevNum(rlsDir)-1}" # Get the `-rNUM` value of the latest local release
  let versDir    = rlsDir/(tag&rev)
  let notesFile  = versDir/notes
  # Move the local files to their final release names
  for file in versDir.walkDir:
    if file.kind != pcFile: continue # Ignore non-files
    let trg = Path file.path.string.split("-")[0..^2].join("-") & ".zip"  # Extract the `-rNUM` from the filename
    mv file.path, trg
  # Release Process
  git "tag", tag                   # Create the tag for current commit
  git "push", "origin", tag        # Push it to git
  notesFile.writeFile(patchnotes)  # Write the patchnotes into the releases folder
  gh "release create", tag,        # Create the Release and upload all of the `.zip` files generated inside `versDir`
    &"-F {notesFile}", versDir/"*.zip", "--verify-tag"

