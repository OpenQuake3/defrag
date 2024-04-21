# TL;DR
## Must have  
```md
- Keep track of your changes. ALWAYS.  
- Document. ALWAYS. Excessively.  
- Not everyone understands what you are doing, and/or doesn't remember  
```
## Styling  
```md
- Standard C99 
  Variables are defined where they are first used, not at the top.  
  Don't use one letter sentry variables outside of their loops. Find a way around them. 
- 1 letter words are extremely undesirable
  2 letter words are slightly better, but not by much.
- 3 letter words immediately become project wide language
  If you name something `trg`, referring to `target`, it must mean target **everywhere else** in the project. 
  This includes multiword names, such as `trgBuild`.
- Overall styling and naming:
  Respect original Q3 code standards as much as possible
- Don't hoard dead commented out code
```
```md
- Line size  
  A screen that can contain more than 150 or 160 characters easily with huge characters. Don't restrict it to 80.  
- Tab size  
  Indent with tabs, with a width of 4 spaces.
- Everything else:
  Use the `clang-format` file.
```

--- 
Details and reasoning:

# Must haves
## Keep track of your changes. ALWAYS.
In id-Tech3 engine, changing one thing usually requires modifying 3-4 different code files minimum.  
And the engine is very difficult to navigate, so changes can get lost really easily.  
As such, you should ALWAYS keep track of what files you have modified, and why.  

You might think "I know how this works, I don't need this"... but you will allways regret that.  
Or, whats worse... you will make the life of whoever comes behind you worse than it needs to be.  
That person won't just be dealing just with your code. They will be dealing with your code AND the engine's code.  
And yours might be simple, but the engine isn't. At all.  

## Document. ALWAYS. And Excessively.
Like the previous implies, this engine can be an absolute B to work with sometimes. Here is an example.  
Assuming that you wrote this code, it would take you literally two seconds to turn this:  
```c
// Code from:   quake3e/renderercommon/tr_font.c
buffer = ri.Malloc(width*height*4 + 18);
Com_Memset (buffer, 0, 18);
buffer[2] = 2;    // uncompressed type
buffer[12] = width&255;
buffer[13] = width>>8;
buffer[14] = height&255;
buffer[15] = height>>8;
buffer[16] = 32;  // pixel size
```
Into this instead:  
```c
// TGA header indexes
#define TGA_DATATYPECODE 2
#define TGA_WIDTH_B1 12
#define TGA_WIDTH_B2 13
#define TGA_HEIGHT_B1 14
#define TGA_HEIGHT_B2 15
#define TGA_BITSPERPIXEL 16
//.......................
#define TGA_HSIZE 18
//.......................
const int    colorChannels = RGBA;
const size_t bufsize       = width * height * colorChannels + TGA_HSIZE;
byte*        buffer        = malloc(bufsize);  // alloc enough memory for the tga image
memset(buffer, 0, TGA_HSIZE);                  // Set all bytes of the buffer header to 0
buffer[TGA_DATATYPECODE] = 2;                  // uncompressed type
buffer[TGA_WIDTH_B1]     = width & 255;        // First byte of width
buffer[TGA_WIDTH_B2]     = width >> 8;         // Second byte of width
buffer[TGA_HEIGHT_B1]    = height & 255;       // First byte of height
buffer[TGA_HEIGHT_B2]    = height >> 8;        // Second byte of height
buffer[TGA_BITSPERPIXEL] = 32;                 // pixel size
```
What changed in code behavior? ... Nothing. Semantics.  
But the code is now documented, instead of requiring everyone that works with it to do the whole process of analyzing the code step by step,
searching for the TGA header file specification online,
decrypting what the code was meant to do in the first place...
or instead just skip the whole thing entirely and having to wing it
because it just takes too much time to deal with such unnecessary complexity.  

id-Tech3 engine has enough of this BS for a couple hundred lifetimes. **Don't make it worse.**  
Document. ALWAYS. And do it excessively.  

## Not everyone understands what you are doing, and/or doesn't remember
Not everyone has such memory, or such good logical deduction skills, that they won't need comments ever.   
This engine is extremely guilty of this mentality.  

This project, instead, is built assuming that the person reading needs no memory of what the code does.  
Please respect that.

The engine has hundreds of thousands lines of code written without respecting this.  
Don't make it even worse than it already is.  
Explain what everything in your code does. It takes two minutes to do it in place, and hundreds of hours to do it "later"  
_(read as: never actually do it because its a massive PITA)_.  

The reader might have forgotten _(or be new, and really really need the explanation)_.  
And you will be that reader too in just a few months.  
Always document. Assume nothing.  


# Styling
---
### Standard C99 
Variables are defined where they are first used, not bunched up at the top.  

### Sentry variables
Don't use one letter sentry variables outside of their loops. They are unsafe and confusing. Find a way around them. 
If you require one of those, then you are not using a sentry variable anymore.
Therefore you MUST give it a unique and self-explanatory name, just like with any other variable.  

### Don't hoard dead commented out code
Avoid keeping duplicates in commented out code.  
Document your change properly, and rely on your explanation, patchnotes and git history to find the old outdated code.  
Duplicated commented out code everywhere, plus a complex codeflow, will turn any codebase into a mess. And this engine is already difficult enough to follow. We don't need any more of that.  

### Compact, but Intuitive Wording
- 1 letter words are extremely undesirable  
Unless it is painfully obvious what the variable is doing.  
_e.g:  r referring to result/return in a short function_  
2 letter words are slightly better, but not by much. Both of them should always be commented to avoid confusion.  
They could be project-wide language, depending on the use (like `nk` meaning `nuklearUI`). But 3letter words are preferred for that.  

- 3 letter words are immediately project wide language  
If you name something `trg`, referring to `target`, then that shortening must mean target **everywhere else** in the project.  
This includes words that are part of multi words _(such as trgBuild, where trg must also mean target)_  

### Overall styling and naming
The project's `clang-format` file can take care of most of the styling for you.  
#### Line size
You have a screen that can contain more than 150 or 160 characters easily with huge characters. Don't restrict it to 80.
#### Tab size
Indent with tabs, with a width of 4 spaces.

