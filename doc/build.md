## Build Instructions
### Requirements
1. Install `git` for your operating system  
   Running `git --version` from any folder should give you git's version _(can be any version)_.
2. Download `confy` from [@heysokam/confy](https://codeberg.org/heysokam/confy/releases)
   Store the executable wherever you want. Its a standalone app with no dependencies.

### Overview
```bash
git clone --recurse-submodules https://github.com/OpenQuake3/defrag
cd defrag
confy       # `confy` and `confy build` are the same command
```

### How to build
#### Source Code
The source code for this project lives in a git repository and a series of **nested** submodules.  
The buildsystem requires the submodules to have been cloned, or nothing will work.  
Remember to add `--recurse-submodules` to your `git clone` command.  
```bash
git clone --recurse-submodules https://github.com/OpenQuake3/defrag
cd defrag
```
If you already cloned without submodules, or you are updating after `git pull`,
you'll need to run this command to update all submodules to their latest state:
```bash
git submodule update --init --recursive
```

#### Building
Building this project is as simple as running `confy` from its root.
```bash
cd defrag
confy
```

#### Build Output
The buildsystem will output all the files needed to run the game _(and engine)_ into `./bin/[host_cpu]-[host_os]`.  
eg: On windows 64bits, this will be `bin/x86_64-windows-gnu`
```
ROOT
|__ bin
    |
    |__ x86_64-windows-gnu
        |
        |__ osdf
        |   |__ uix86_64.dll
        |   |__ cgamex86_64.dll
        |   |__ qagamex86_64.dll
        |
        |__ oQ3.x86_64.exe
        |__ freetype.dll
        |__ SDL2.dll
```

#### Local Development
##### Compilation
The buildsystem is already setup for optimal compilation for local development of the game.  
You dont need to do anything special. Just run `confy` from root and done.  

##### Running the game
The folder output by the buildsystem is already setup for running the game.  
Copy _(or link/junction)_ your QuakeIIIArena's `baseq3` into `bin/[host]/baseq3` and launch the engine.  


#### Cross-compilation
Set `distribute = true` in `build.zig`,  
or change the value of `systems` to your desired list of desktop systems.  
The buildsystem will output the same folders described above, but for all systems listed in the `systems` variable.  


### Why confy
[Confy](https://github.com/heysokam/confy) is a complete make/cmake replacement,  
that was created **specifically** for id-Tech3.  
Confy is not just used by this project, **it exists** because of this project.  

You can read more about why/how we use it in the @[confy](https://github.com/OpenQuake3/engine/blob/master/doc/confy.md) doc file.  
 
