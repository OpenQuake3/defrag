## TL;DR
### Must have  
```md
- Keep track of your changes. ALWAYS.  
- Document. ALWAYS. Excessively.  
- Not everyone understands what you are doing, and/or doesn't remember  
- Old code modifications  
  Keep track of old code modifications with `//::OSDF yourname->key : description`.  
- New code  
  Can be tracked with git history, patchnotes, etc.  
```
### Styling  
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
# Code Formatting
- Overall:  
  Use the project's `.clang-format` file for new code.  
  Preserve the original code's styling as it was.  
- Line breaks  
  Keep the code into one line where possible and it makes sense  (eg: Early returns under a condition).  
- Line size  
  A screen that can contain more than 150 or 160 characters easily with huge characters. Don't restrict it to 80.  
```
```md
# Naming Convention
- This project follows SOV word order -strictly-
  Subject -> Object -> Verb -> Differentiating Exception   
  Names such as: `id3_target_create()` and `id3_target_Build`         (categorical SOV)  
  Instead of:    `create_build_target()` and `update_build_target()`  (idiomatic SVO)  
- Case styling: New code  
  snake_case for symbol namespacing  
  camelCase for symbols  
  PascalCase for types and constants  (preceding namespaces will stay snake_case)  
  SCREAMCASE is forbidden. Use namespaced PascalCase with good names
- Case styling: Old code  
  Respect original Q3 code formatting when editing in-place.    
  Apply the style rules for any new code.  
```
```md
# Folder Structure
- Categorize with folders
  The "One Gigantic File/Folder to Rule them All" pattern is strictly forbidden.  
  A complex system needs a complex folder structure for intuitive navigation.  
  Hoarding, or trying to hide, complexity only makes problems worse.  
```

--- 
Details and reasoning:

## Must haves
### Keep track of your changes. ALWAYS.
In id-Tech3 engine, changing one thing usually requires modifying 3-4 different code files minimum.  
And the engine is very difficult to navigate, so changes can get lost really easily.  
As such, you should ALWAYS keep track of what files you have modified, and why.  

You might think "I know how this works, I don't need this"... but, on this engine, you will allways regret that. Always.  
Or, worse... you will make the life of whoever comes behind you miserable.  
That person won't be dealing with your code. They will be dealing with your code AND the engine's code.  
And yours might be simple, but the engine isn't. At all.  

### Document. ALWAYS. And Excessively.
Like the previous point implies, this engine can be an absolute B to work with. Here is an example.  
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
What changed in behavior? ... Absolutely Nothing.  
But the code is now documented...
instead of requiring every single reader to do the whole process of analyzing the code step by step,
searching for the TGA header file specification online,
and decrypting what the code was meant to do in the first place...
or... instead... just skip the whole thing entirely and having to use the code blindly,
because it just takes too much time to deal with all the unnecessary complexity.  

id-Tech3 engine has enough of this BS for a couple hundred lifetimes.
**Don't make it worse.**  


### Not everyone understands what you are doing, and/or doesn't remember
No one has such memory, or such good logical deduction skills, that they won't need comments ever.   
This engine is extremely guilty of this mentality.  

This project, instead, is built assuming that the person reading needs no memory of what the code does.  
Always respect that.

The engine has hundreds of thousands lines of code written without respecting this.  
Don't make it worse than it already is.  
Explain what everything in your code does.  
It takes two minutes to do it in place, and hundreds of hours to do it "later"  
_(as in: never actually do it because its a massive PITA)_.  

The reader might have forgotten _(or be new, and really really need the explanation)_.  
and you will be that forgetful reader too in just a few months anyway.  
Always document. Assume nothing.  


## Old code modifications
To keep track of old code modifications, the project uses `//::OSDF keyword`.
If you search for `//::OSDF` you will find all code modifications that the mod has done to the original q3a code
For multiline changes, these blocks always end with `//::OSDF end`
All other changes always start with `//::OSDF keyword`. 
- `modded` or `change` means something was modified, but overall behavior will be similar
- `added` means it is new code that didn't exist before in q3a
- `removed` this is not needed much, but self explanatory. Only used in cases where we want to keep track of removed code, but still keep using parts of the original. Very undesirable, so please avoid. If a change is big enough, it is preferable to hardwire the old code into a new codeflow, than to modify the original code in place _(see the hook towards `phy/*` code inside `bg_pmove.c` `Pmove()` for an example)._

Please avoid old code modifications where possible.  
But if you must modify, then add your name to the changes somewhere, in the format: `//::OSDF name->key : explanation`.  
_Valid keys are the same as the patchnotes format. `chg`, `fix`, `add`, etc. See the patchnotes file for a list of options._   
Example:   
```c
//::OSDF sokam->add  : Added thing and otherThing, because they were missing
thing();       // Has new super important code. Read documentation inside the function
otherThing();  // Also has some new code that is needed to make X and Y work
//::OSDF end
```
```c
someOtherCode();  //::OSDF sokam->chg  :  Switched oldCode with someOtherCode
```
Also, avoid keeping duplicates in commented out code.  
Document your change properly, and rely on your explanation, patchnotes and git history to find the old outdated code.  
Duplicated commented out code everywhere, plus a complex codeflow, will turn any codebase into a mess. And this engine is already difficult enough to follow. We don't need any more of that.  

### Reasoning
Keeping track of whats new and what was originally there can be extremely difficult.  
And the code is already hard to work with as it is.  
Any way to make this better will be very impactful.  

## New code
New code doesn't have such strict tracking requirements, **as long as the code is contained in non-q3a files**.  
New code (that uses separate files) can be tracked with other methods (git history, patchnotes, etc).
Examples of the new code system: `src/game/sgame/phy/*`, `src/ui/*`, `src/sgame/ent/*`


## Styling
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
Using the project's `.clang-format` is mandatory (with exceptions), and will take care of all the styling for you.  
Formatting of **pre-existing** QuakeIIIArena files **is forbidden**. It would make a giant mess.  
Formatting of **new files** with this style is **mandatory**, and code won't be accepted without it.  


## Naming Convention
### Casing
Regular symbols use `camelCase`.  
Typedefs and Constants use `PascalCase`.  
Namespaces are separated by `_`.  
All non-local names must be namespaced with their parent categories.  
### Name Structure
This project follows [SOV](https://en.wikipedia.org/wiki/Subject%E2%80%93object%E2%80%93verb_word_order) word order **strictly**.
```md
- Subject : The owner of the function.  `eg: cg`
- Object  : The target of an action.    `eg: entity`
- Verb    : The action being executed.  `eg: create`
- SVO     : `cg_CreateEntity`
- SOV     : `cg_entity_create`
```
> @note  
> This does NOT respect english syntactical/gramattical rules, and this is **by design**.  
> _SVO causes tremendous amount of inconsistencies when creating a systematic naming convention._  

