#!/bin/sh
set -eu
# Configure
S="testgpl"
G="/hdd/vg/defrag/$S"
B="./bin"
# Build
clear ; spry build
# Install
rm ./lib.out.txt
rm $G/cgamex86_64.so  ; cp $B/cgamex86_64.so  $G/cgamex86_64.so
rm $G/qagamex86_64.so ; cp $B/qagamex86_64.so $G/qagamex86_64.so
rm $G/uix86_64.so     ; cp $B/uix86_64.so     $G/uix86_64.so
# Run
prev=$(pwd)
cd $G/..
export LD_DEBUG=all
./oDFe.vk.x64 +set fs_game $S +set vm_cgame 0 +set vm_game 0 +set vm_ui 0 +set sv_pure 0 2> $prev/lib.out.txt
cd $prev

