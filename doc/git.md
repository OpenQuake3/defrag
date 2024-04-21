## Submission Process overview
1. Pick something to work on from the issue list of the [Current Milestone](https://github.com/OpensourceQuake3/defrag/milestones?direction=asc&sort=title&state=open)
2. Create a new branch and start doing your work
3. Rebase workflow: goal is to have very clean, self-contained commits.   
  - Do a bunch of work, maybe make some WIP commits   
  - When the code is ready, Rebase+Squash your branch until your commit history is clean   
4. Open a PR for your branch into the base mainline branch you originally forked from  
5. Your code will be reviewed. If your code needs changes, fix them and squash again until its approved.  
6. PR is approved:   
  - Someone with permissions has to do the Rebasing into mainline  
  - Once that's done, the process is complete.  

## Review process overview
Code reviews consist of people:
- Reading over code 
- Pointing out potential bugs 
- Pointing out opportunities for things to be refactored
- Asking questions about things that are unclear
- Making small nitpicks on formatting and style

## General rules
- Use the project's commit-style  
- Follow the contributing standards for merging things into mainline.   
- Don't do work on the main branch, always make a new branch so that you don't accidentally break something  
- Access to the main branch is restricted, to make sure it doesn't break horribly  
- Code review and automated testing (even if it's just "does this PR compile") are really important  

## Mainline branches
This projects currently maintains only one active mainline branch, called `master`.  
Releases will be published from code in that branch when the feature list needs to be playtested by players.  

Given the current alpha state of the whole project, code in the `master` branch might end up broken due to unknown reasons.  
And that _(for now)_ is ok.  

When the project becomes stable _(near or at 1.0.0)_, PRs will need to reach `Release quality` for merging into `master`.
At that point, a separate `next` branch will be created, that will contain the latest bleeding-edge set of features.  
And the `master` branch will then contain only stable / production code.  

We are not at that stage yet. But this doesn't mean that your code can be completely broken, and/or not compile at all.  
Do not submit PRs into mainline that have not been tested to work on all supported platforms _(windows+linux+mac)_.  

## Rebasing vs Merging 
This project uses the [Rebase workflow](https://www.youtube.com/watch?v=7Mh259hfxJg).  
Contributors are expected to condense their branch history as much as possible with the aid of [Rebase Squashing](https://www.youtube.com/watch?v=RwvTrSm7zEY).  
This process only needs to be done right before submitting a PR, or by using the GitHub's `Squash and Merge` feature on merge.  

The resulting commit should reflect the big picture perspective of your changes, and only contain information strictly necessary for understanding what was changed.  
When possible, this history shouldn't resemble the look of a very detailed patchnotes list.   
Rather, it should reflect the list of overarching categories that the changes include, and let the diff list and `docs/patchnotes.md` file contain a full list of individual/atomic changes.  
```md 
# Undesirable Git history
fix : Code in line 1505 of file asdf.c missing a symbol
chg : Commit for the weekend, nothing changed
new : Function for doing new thing in file asdf9i.c
fix : File in folder fjgo/ renamed from 0sdf0.c to i0g.c
new : Initial branching from master
 ^
 |.. Nothing in them expresses what category these changes belong to.
 |.. The changes are too atomic for git history, but would be fine for docs/patchnotes.md
```
```
# Better
new : Simple early implementation of feature X
new : Full implementation of feature Y
 ^
 |.. Concise and big picture perspective, but understandable
 |.. Whoever needs more data will rely on git, diff tools and/or the docs/patchnotes.md file, to find specific changes, 
```

## Branching
Never branch off of a non-mainline branch.  
Git history might be rewritten on the branch you are using, and you will be responsible for merging your code back into that branch, and solving the problems that this created.  
If you do, you need to communicate with its creator, so that your branch doesn't end up broken due to Squashing/Cleaning rewriting the history.  
At that point, people working on that branch will need to cooperate as one, to merge back into mainline together as a single Rebased and Squashed branch.  

## Commit message style
Follows a format similar to that of the patchnotes:  
```md 
key : Add category of fancy new features
^--^  ^---------------------------------^
|     |
|     +-> Summary of the change/changes
|
+-------> Type: new, fix, rmv, doc, bld, etc

Valid Keywords are:

# User facing changes
new : New features
chg : Change in existing functionality  
fix : Bugfix
rmv : Removed feature  
dep : Soon-to-be removed feature  
sec : Security. Fix for vulnerabilities.  
rls : Last commit before a release. Includes version

# Development changes
bld : Changes to the buildsystem. New and fixes
doc : Changes to the documentation. New and fixes
org : Project organization / structuring. Not related to code (use ref for code organization)
ref : Refactor of code. Naming, organizing, reflow, etc. No behavior changes
sty : Formatting, styling, etc; No behavior changes

# Not used:
tst : (not used) Changes to tests code. No production code changed
... : (dont use) Part of the feature listed above it. Ok in patchnotes, but not on git history
```

---
```md
# TODO
## Buildsystem
- [ ] write documentation about contributing 
  - [x] style guidelines for code, commits, and general practices
  - [ ] a process for submitting work to be included 
    - [x] PRs
    - [x] Code review 
    - [x] Merging/rebasing
    - [x] Testing 
    - [ ] CI

## Github systems
- [ ] automate formatting with the .clang-format file (hooks or github actions)
- [ ] Multi-user way to track progress, todo and task assignment (issue tracker, maybe other systems)
```

