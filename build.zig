//:_________________________________________________________________
//  osdf  |  Copyright (C) Ivan Mar (sOkam!)  |  GPL-3.0-or-later  :
//:_________________________________________________________________
// @deps confy
const confy   = @import("confy");
const Name    = confy.Name;
const package = confy.package;
const git     = confy.git;
// @deps builder
const info    = @import("./info.zig");
const cfg     = @import("./src/build/cfg.zig").cfg;
const Game    = @import("./src/build/game.zig").Game;
const Engine  = @import("./src/build/engine.zig").Engine;
const Assets  = @import("./src/build/assets.zig").Assets;
const Config  = @import("./src/build/assets.zig").Config;
const Release = @import("./src/build/release.zig").Release;


//______________________________________
// @section Configuration Options
//____________________________
pub const release    = true;                  // Whether we are building a release or debug version
pub const distribute = release and true;     // Prepare the output for distribution (manual or automated) when true
pub const publish    = distribute and true;  // Publish to the relevant platforms when true


//______________________________________
// @section Package Information
//____________________________
pub const P = package.info(.{
  .version = info.version,
  .name    = Name{ .short= cfg.modname.short, .long= cfg.modname.long, .human= cfg.modname.human },
  .author  = Name{ .short= info.author },
  .license = info.license,
  .git     = git.Info{ .owner= info.author, .repo= info.name, .baseURL= "https://github.com" },
});


//______________________________________
// @section Buildsystem Entry Point
//____________________________
pub fn main() !void {
  //______________________________________
  // @section Clean before running
  //____________________________
  try Assets.clean();

  //______________________________________
  // @section Define Build Targets
  //____________________________
  var game   = try Game.create(P);
  var engine = try Engine.create(P);
  var assets = try Assets.create(P);
  var config = try Config.create(P);
  var result = try Release.create(P);

  //______________________________________
  // @section Target System
  //____________________________
  const systems =
    if (distribute) confy.System.desktops()
    else            &.{confy.System.host()};

  //______________________________________
  // @section Order to Build
  //____________________________
  P.report();
  try game.buildFor(systems, release);
  try engine.buildFor(systems, release);
  try assets.packFor(systems);
  try config.packFor(systems);
  try result.packFor(systems, release);
  try result.publish(publish);

  //__________________
  confy.echo(cfg.modname.short++": Done building.");
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

