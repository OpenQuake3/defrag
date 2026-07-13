# cairo
Can we run cairo in dynamic code, and send images to the renderer to load?  
Could potentially run all cairo generation on uiInit, and register .tga assets on menuCache.  
This should be slow, but its only run once on load.  
Better solution is desirable, but that might require engine wide support for cairo

# Nuklear
Can we connect it to the renderer, with existing callbacks?  
Does it need anything special that existing callbacks can't do?
Will use Nuklear, because Dear doesn't support skinning

---

# uiAdjustFrom640
_in ui/utils.c_
Resolution outdated scaling tech happens here  
Change this to normalized scaling instead  

# menuCache
_in ui/menu.c_
Initializes all menu assets.  
This is where the font is initially loaded (as tga).  

TODO:
<!-- - How to correctly load a font (if supported)   -->
<!-- - Reorganize menu asset folders: everything loaded from a common UI root folder   -->
<!-- - Erase the default menu sounds from existence. They make UI feedback feel really crappy -->
- Remake the default menu assets:  
  Their execution is 99% of the UI problem, from what I can see so far  
  I thought the framework was bad, but it might not be as bad as I thought  

# uiInit  
_in ui/core.c_
<!-- Initializes bias, xscale and yscale.   -->
<!-- The way this is done is probably wrong for actual widescreen UI support.   -->
<!-- Need to revisit these values once the code is explored a bit more. -->
bias, xscale and yscale are completely removed from the code
This framework doesn't scale anything, other than x,y coordinates for positioning
