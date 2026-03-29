//:_________________________________________________________________
//  osdf  |  Copyright (C) Ivan Mar (sOkam!)  |  GPL-3.0-or-later  :
//:_________________________________________________________________
// @deps confy
const confy   = @import("confy");
const Name    = confy.Name;
const package = confy.package;
const git     = confy.git;
// @deps builder
const info   = @import("./info.zig");
const Game   = @import("./src/build/game.zig").Game;
const Engine = @import("./src/build/engine.zig").Engine;


//______________________________________
// @section Configuration Options
//____________________________
pub const release    = true;
pub const distribute = release and false;


//______________________________________
// @section Package Information
//____________________________
pub const P = package.info(.{
  .version = info.version,
  .name    = Name{ .short= info.name, .human= info.description },
  .author  = Name{ .short= info.author },
  .license = info.license,
  .git     = git.Info{ .owner= info.author, .repo= info.name, .baseURL= "https://github.com" },
});


//______________________________________
// @section Buildsystem Entry Point
//____________________________
pub fn main() !void {
  //______________________________________
  // @section Define Build Targets
  //____________________________
  var game   = try Game.create(P);
  var engine = try Engine.create(P);

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
  try game.buildFor(systems);
  try engine.buildFor(systems);
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

