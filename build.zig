//:_________________________________________________________________
//  osdf  |  Copyright (C) Ivan Mar (sOkam!)  |  GPL-3.0-or-later  :
//:_________________________________________________________________
// const confy   = @import("./bin/.cache/confy/lib/confy/src/confy.zig");
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
  const game   = try Game.create(P);
  const engine = try Engine.create(P);
  _=game;
  _=engine;

  //______________________________________
  // @section Target System
  //____________________________
  const systems = &.{confy.System.host()};
  // const systems = confy.System.desktops(); // TODO: Cross-Compilation  (Missing arm64)
  _=systems;

  //______________________________________
  // @section Order to Build
  //____________________________
  P.report();
  // try game.buildFor(systems);
  // try engine.buildFor(systems);

  var osdf = try confy.Program("hello.c", .{});
  try osdf.build();
}

