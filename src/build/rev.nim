#:___________________________________________________________________
#  osdf  |  Copyright (C) Ivan Mar (sOkam!)  |  GNU GPLv2 or later  |
#:___________________________________________________________________
# @fileoverview Revision Number management helpers

#_____________________________
# Revision Number management
#___________________
proc set *(v :int; f:string) :void=
  if not dirExists(splitFile(f).dir): sh.md splitFile(f).dir
  if not fileExists(f): exec &"touch {f}"; return
  writeFile f, $v
  echo &":: curr Revision is {v}, set in file {f}"
#___________________
proc get *(f :string) :int=
  if fileExists(f):
    let rev = int(parseInt(readFile(f)))
    result = rev
  else:
    echo &":: File {f} doesn't exist. Creating"
    setRevNum(-1, f)
    result = -1
#___________________
proc update *(file :string) :void=
  var ID = rev.get(file)
  echo &":: prev Revision is {ID}, stored in file {file}"
  rev.set(ID+1, file)

