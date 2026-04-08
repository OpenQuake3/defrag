//:_________________________________________________________________
//  osdf  |  Copyright (C) Ivan Mar (sOkam!)  |  GPL-3.0-or-later  :
//:_________________________________________________________________
pub const cfg = @This();


//______________________________________
// @section Game: Name
//____________________________
pub const name = struct {
  pub const short = "osdf";
  pub const long  = "opensource-defrag";
  pub const human = "OpenSource Defrag";
};


//______________________________________
// @section Game: Folders
//____________________________
pub const dir = struct {
  // Game Project folders
  pub const assets = "assets";
  pub const src    = "src";
  pub const lib    = dir.src++"/lib";
  pub const cfg    = dir.src++"/cfg";

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

