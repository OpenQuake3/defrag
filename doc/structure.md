# Default Folders
root    # This repository's root folder. Where all other dirs are stored  
bin     # Where the compiled binaries will be output  
doc     # Project documentation files  
src     # Source code of the project  
ref     # Reference code and other important information  
assets  # Project assets that will be packed into every distributed release  

---

# src Subfolders
cfg     # Default configuration files for the mod.  
game    # Contains a modified version of ioquake3's gamecode, used as a base for the mod.  
condump # For storing important quake console dumps  
pseudo  # Pseudocode/algorithms, for work in progress features  
lib     # Libraries required for the scripts stored in the root of this folder  

---

# gamecode Subfolders
botlib   # Ingame Bot/AI related code
cgame    # Client game code
client   # Code shared between gamecode and Client engine
dep      # Deprecated code (mostly ioq3 gamecode before forking)
qcommon  # Various engine utilities, common to engine/game
rendc    # Code shared between gamecode and Client engine renderer
sgame    # Server and shared gamecode
ui       # Contains the reworked UI code
ui_q3    # Original q3ui code (osdf main menu)
ui_ta    # Original teamarena code

---

# New Code folders
## src/game/sgame/phy
Contains all code related to the movement physics.  
All of this code happens inside a single `Pmove()` instance, and starts with `phy_PmoveSingle()` inside `bg_pmove.c`.  
This is very different to every other defrag mod, that has all of the physics integrated into one single code flow.  

This change started to preserve vq3 and cpm from being affected by bugs in the new physics, but turned out to be extremely useful for other physics too. They make the code much cleaner to navigate, and much easier to reason about.  
Instead of having gigantic functions full of edge cases that don't apply to what you are looking for, you follow the code into the correct function and read only whats in front of you.  

The goal of the `Pmove()` connector function is to move the code flow completely outside of the `bg_pmove.c` file, which was already 2500 lines before adding any defrag specific code, and really hard to navigate.   
_(for context: The file grew up to 5k lines at some point, and that was early in development with only cpm/vq3 physics)_

Each physics has its own separate flow, and must be kept separate. This ensures that bugs don't accidentally bleed from one physics into the other.  
Each of those flows are also kept in their own separate files, unless they use the exact same code from other files. In which case they just call for those functions instead.  
See `*/phy/vq1.c` or `*/phy/vq4.c` for codeflow examples that are completely different to q3a in almost every function  
See `*/phy/vjk.c` for a codeflow that follows q3a almost entirely, except for a couple of exceptions  

## src/game/cgame Subfolders
hud       # Code related to the ingame hud. It only has strafehud code, but eventually should contain everything else
qvm       # Deprecated qvm specific files. Disconnected from the buildsystem
teamarena # Deprecated teamarena specific code. Disconnected from the buildsystem

## src/game/ui Subfolders
Contains code related to the new ui framework.  

The structure aims to modularize the framework, within the limitations of C.  
C code files are stored in subfolders named `*/c/`,
while their roots contain a header file for each of the "submodules" contained within that `*/c/` subfolder.

This results in structures like:
```
Folder with two modules:   main and callbacks
*/ui/c/main.c         <--- UI Main implementation
*/ui/c/callbacks.c    <--- Callback implementation
*/ui/main.h           <--- Declarations of methods accesible from outside main.c
*/ui/callbacks.h      <--- Declarations of methods accessible from outside callbacks.c

Folder with one module:   elements
*/ui/framework/c/action.c      <-- Action element implementation code
*/ui/framework/c/cursor.c      <-- Cursor element implementation code
*/ui/framework/c/radiobtn.c    <-- RadioButton element implementation code
*/ui/framework/elements.h      <-- Declarations for all elements
```

