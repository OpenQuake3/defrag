#:___________________________________________________________________
#  osdf  |  Copyright (C) Ivan Mar (sOkam!)  |  GNU GPLv2 or later  |
#:___________________________________________________________________

#___________________________________________________________________________________________________
# @section Old Buildsystem: References
##[
# Versioning
verFile   = engineDir.Dir('qcommon').File('q_shared.h')  # File where the version will be searched for. If changed, version won't be searched for in the right file
verMacro  = 'Q3_VERSION'                     # Macro name of the version value. If changed, version won't be found in the file
version   = getVersion(verFile,verMacro)     # If this is changed, we overwrite the project's version defined in the source code

# Version
import re
def getVersion(file,macro):
  for line in open(file.abspath).readlines():   # For every line in file
    parts = line.split()                        # Convert line into a list
    lineIsEmpty = not bool(parts)               # true if parts is None
    lineHasOneWord = not (0 < 1 < len(parts))   # bool( index `1` exists in `parts` )  :Python thinks: (0 < 1) and (1 < len(parts)) 
    if lineIsEmpty or lineHasOneWord: continue
    lineContainsMacro = bool(parts[1] == macro)  # If the second word is the macro name
    if lineContainsMacro:
      # Search for Pattern in this line
      exp = '\\\".* ([^ ]*)\\\"'#   from first `"` until last ` ` before a group of anything with no spaces, that's right before the last `"`
      obj = re.compile(exp)     # Compile the expression
      m   = obj.search(line)    # Compare exp with line
      vers = m.group(1)         # Version is content of group #1 (between parenthesis)
      return vers               # Return string matched  # Stops at first line matched. Macro redefinitions are not standard ISO
]##
