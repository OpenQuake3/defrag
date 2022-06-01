# Opensource Defrag: Gamecode
This repository contains only the necessary files for the gamecode to be compiled, -without- any of its engine dependencies.  
It won't compile independently, since some required engine header files are missing.  

The buildsystem is stored in the root `osdf` repository, inside the folder: `osdf/src/*`
It gets the missing files from the `osdf-engine/*` repository.  

#### dep folder
Stores deprecated files from the original ioquake3 repository.

A copy of the required files to build this code is stored inside `./dep/code/*`, just in case. But they are deprecated, might be removed and won't be maintained from this repository.
