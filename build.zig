//:_________________________________________________________________
//  osdf  |  Copyright (C) Ivan Mar (sOkam!)  |  GPL-3.0-or-later  :
//:_________________________________________________________________
// @deps confy
const confy   = @import("confy");
// @deps builder
const cfg     = @import("./src/build/cfg.zig").cfg;
const Game    = @import("./src/build/game.zig").Game;
const Engine  = @import("./src/engine/src/build/engine.zig").Engine;
const Assets  = @import("./src/build/assets.zig").Assets;
const Config  = @import("./src/build/assets.zig").Config;
const Release = @import("./src/build/release.zig").Release;


//______________________________________
// @section Configuration Options
//____________________________
pub const verbose    = true;
pub const release    = true;                  // Whether we are building a release or debug version
pub const distribute = release    and false;  // Prepare the output for distribution (manual or automated) when true
pub const publish    = distribute and false;  // Publish to the relevant platforms when true


//______________________________________
// @section Buildsystem Entry Point
//____________________________
pub fn main (P :confy.Process) !void {
  //______________________________________
  // @section Define Build Targets
  //____________________________
  var engine = try Engine.create(P, .{
    .root    = "./src/engine",
    .release = release,
    .game    = cfg.name.full,
    .pkg     = cfg.package,
    .verbose = verbose,
  });
  var game   = try Game.create(P, cfg.package, release);
  var assets = try Assets.create(P, cfg.package);
  var config = try Config.create(P, cfg.package);
  var result = try Release.create(P, cfg.package, release, distribute);

  //______________________________________
  // @section Target System
  //____________________________
  const systems =
    if (distribute) confy.System.desktops()
    else            &.{confy.System.host()};
    // &.{confy.System.desktop.windows};
    // &.{confy.System.desktop.mac.x64, confy.System.desktop.mac.arm};

  //______________________________________
  // @section Order to Build
  //____________________________
  try assets.clean(); // Clean before running
  cfg.package.report();
  try game.buildFor(systems);
  try engine.buildFor(systems);
  try assets.packFor(systems);
  try config.packFor(systems);
  try result.packFor(systems);
  try result.publish(publish);

  //__________________
  confy.echo(cfg.name.short++": Done building.");
}


//______________________________________
// @section Zig Buildsystem: Error Message
//____________________________
pub fn build (b :*@import("std").Build) void {_=b; @import("std").debug.print(
  \\[ERROR] `zig build` is not supported.
  \\  Use `confy build` instead.
  \\  https://codeberg.org/heysokam/confy/releases {s}
  , .{"\n"});
}

