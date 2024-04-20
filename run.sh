#!/bin/sh
set -eu
# Configure
S="testgpl"
G="/hdd/vg/defrag/$S"
B="./bin"
prev=$(pwd)
# Build
clear ; spry build
# Install
logFile=$prev/lib.log.txt
touch $logFile ; rm $logFile
rm $G/cgamex86_64.so  ; cp $B/cgamex86_64.so  $G/cgamex86_64.so
rm $G/qagamex86_64.so ; cp $B/qagamex86_64.so $G/qagamex86_64.so
rm $G/uix86_64.so     ; cp $B/uix86_64.so     $G/uix86_64.so
# Run
cd $G/..
export LD_DEBUG=all
./oDFe.vk.x64 +set fs_game $S +set vm_cgame 0 +set vm_game 0 +set vm_ui 0 +set sv_pure 0 2> $logFile
cd $prev

