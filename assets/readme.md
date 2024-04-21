Each subfolder is meant to store separate asset groups/categories.
Each `.pk3` file that will be created will have its own set of subfolders.

- The name of the folder must match the sub-name for the output file.
- The folder will be considered the root for the resulting `pk3` file
- The structure of the subfolder must be the one that will be understood by the code.
- Empty folders will be skipped

## Example:
Distributing a file called `y.osdf.example.pk3`:
- Create a folder called `example` 
- `assets/example/` will now be used as the base root of the `pk3`
- The code expects a file called `ui/bg.jpg`, that will be accessed from `INSTALLDIR/ourmod/ui/bg.jpg`
- Create a file at `assets/example/ui/bg.jpg`
- The resulting `.pk3` file will contain a folder and sub-file named `ui/bg.jpg` at its root

**Automation:**  
The Buildsystem executes this process automatically for all existing folders.  

