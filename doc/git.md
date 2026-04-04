## Submission Process overview
1. Pick something to work on from the [Current Milestone](https://github.com/OpensourceQuake3/defrag/milestones?direction=asc&sort=title&state=open)
2. Create a new branch and start doing your work
3. Squash+Merge workflow: The goal is to have very clean, self-contained commits.   
  - Make sure you name your commits appropriately, based on what you are changing.
  - When the code is ready, the code will merged using the `Squash+Merge` feature, and all your commit names will be part of the final commit message.
4. Open a PR from your branch
5. Your code will be reviewed. If it requires changes, fix them and commit again until the PR is approved.  
6. PR is approved:   
  - Someone with permissions will `Squash+Merge` your PR into mainline.  
  - Once that's done, the process is complete.  

## Review process overview
Code reviews consist of people:
- Reading over code.  
- Pointing out potential bugs.  
- Pointing out opportunities for things to be refactored.  
- Asking questions about things that are unclear.  
- Making "nitpick" comments on formatting and style. _(Make sure you follow and respect the style guide)_.  

## General rules
- Use the project's commit-message convention.  
- Follow the contributing guidelines for merging things into mainline.   
- Don't do work directly on the main branch, always make a new branch.  
- Access to the main branch is restricted, to make sure it doesn't break horribly.  
- Code review and automated testing (even if it's just "does this PR compile") are mandatory.  

## Mainline branches
This project maintains only one active mainline branch, called `master`.  
Releases will be published from mainline when the feature list can be playtested by players.  

Given the current alpha state of the whole project, code in the `master` branch might be broken for unknown reasons.  
And that is ok _(for now)_.  

When the project becomes stable _(near or at 1.0.0)_, PRs will need to reach `Release quality` before merging into `master`.
When that happens, a separate `next` branch will be created, that will contain the latest bleeding-edge set of features.  
From that point on, the `master` branch will only contain stable production code.  

Never submit PRs into mainline that have not been tested to work on all supported platforms _(windows+linux+mac)_.  
We are not stable yet. But this doesn't allow your code to be completely broken, and/or not compile at all.  

## Rebasing vs Merging 
This project uses the Squash+Merge workflow.  
Contributors are expected to maintain a clean list of commits in their their branch's history.  
The Squash+Merge process will only be executed when a PR is merged, by using the GitHub's `Squash and Merge` feature.  

The resulting commit message should appropriately reflect the list of your changes, and only contain information strictly necessary for understanding what the PR is trying to achieve.  

## Branching
Never branch off of a non-mainline branch.  
Git history might be rewritten on the branch you are using, and you will be responsible for merging your code back into that branch, and solving the problems caused by that.  
People working together on one branch need to cooperate as one, and will merge back into mainline together as a single unit of work.  

## Commit message convention
Follows a format similar to the patchnotes:  
```md 
key: Add category of fancy new features
^-^  ^---------------------------------^
|    |
|    +-> Summary of the change/changes
|
+------> Type: new, fix, rmv, doc, bld, etc

Valid Keywords are:

# User facing changes
new: New features
chg: Change in existing functionality  
fix: Bugfix  
rmv: Removed feature  

# Development changes
bld: Changes to the buildsystem.
doc: Changes to the documentation.
fmt: Formatting or refactoring of code. Naming, organizing, reflow, restructuring, etc; No behavior changes

# Not used:
tst: (not used) Changes to unit-tests code. No production code changed
...: (dont use) Part of the feature listed above it. Ok in patchnotes, but not on git history
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


