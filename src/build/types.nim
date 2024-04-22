#:___________________________________________________________________
#  osdf  |  Copyright (C) Ivan Mar (sOkam!)  |  GNU GPLv2 or later  |
#:___________________________________________________________________

type Name * = object
  short  *:string
  long   *:string
  human  *:string

type Repository * = object
  owner  *:string   ## GitHub's User or Organization name
  name   *:string   ## Name of the repository
