//:_________________________________________________________________
//  osdf  |  Copyright (C) Ivan Mar (sOkam!)  |  GPL-3.0-or-later  :
//:_________________________________________________________________
pub const cfg = @This();
// @deps buildsystem
const confy = @import("confy");
const Name  = confy.Name;
const git   = confy.git;


//______________________________________
// @section Package Information
//____________________________
pub const info    = @import("../../build.zig.zon");
pub const package = confy.package.info(.{
  .version = cfg.info.version,
  .name    = cfg.name.full,
  .author  = Name{ .short= "sOkam" },
  .license = cfg.info.license,
  .git     = git.Info{ .owner= "OpenQuake3", .repo= "defrag", .host= "https://github.com" },
});


//______________________________________
// @section Game: Name
//____________________________
pub const name = struct {
  pub const short = @tagName(cfg.info.name);
  pub const long  = "opensource-defrag";
  pub const human = "OpenSource Defrag";
  pub const full  = confy.Name{.short= cfg.name.short, .long= cfg.name.long, .human= cfg.name.human };
};


//______________________________________
// @section Game: Folders
//____________________________
pub const dir = struct {
  // Game Project folders
  pub const assets = "assets";
  pub const src    = "src";
  pub const lib    = dir.src++"/lib";
  pub const config = dir.src++"/cfg";
  // Game Code Folders
  pub const code   = dir.src++"/game";
  pub const common = dir.code++"/qcommon";
  pub const client = dir.code++"/cgame";
  pub const server = dir.code++"/sgame";
  pub const hud    = dir.client++"/hud";
  pub const phy    = dir.server++"/phy";
  pub const ui = struct {
    pub const q3    = dir.code++"/ui_q3";
    // New UI
    pub const base  = dir.code++"/ui";
    pub const core  = dir.ui.base++"/c";
    pub const color = dir.ui.base++"/color/c";
    pub const fwk   = dir.ui.base++"/framework/c";
    pub const menu  = dir.ui.base++"/menu/c";
  };
};

