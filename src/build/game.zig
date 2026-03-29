//:_________________________________________________________________
//  osdf  |  Copyright (C) Ivan Mar (sOkam!)  |  GPL-3.0-or-later  :
//:_________________________________________________________________
pub const Game = @This();
// @deps std
const std = @import("std");
// @deps buildsystem
const confy = @import("confy");


//______________________________________
// @section Object Fields
//____________________________
A       :std.heap.ArenaAllocator,
client  :confy.Target,
server  :confy.Target,
ui      :confy.Target,


//______________________________________
// @section Game: Project Folders
//____________________________
pub const dir = struct {
  // Game Project folders
  const assets = "assets";
  const src    = "src";
  const lib    = dir.src++"/lib";
  const cfg    = dir.src++"/cfg";

  // Game Code Folders
  const code   = dir.src++"/game";
  const common = dir.code++"/qcommon";
  const client = dir.code++"/cgame";
  const server = dir.code++"/sgame";
  const hud    = dir.client++"/hud";
  const phy    = dir.server++"/phy";
  const ui = struct {
    const q3    = dir.code++"/ui_q3";
    // New UI
    const base  = dir.code++"/ui";
    const core  = dir.ui.base++"/c";
    const color = dir.ui.base++"/color/c";
    const fwk   = dir.ui.base++"/framework/c";
    const menu  = dir.ui.base++"/menu/c";
  };
};


//______________________________________
// @section Game: Build Flags
//____________________________
const flags = struct {
  fn all (A :std.mem.Allocator) !confy.FlagList {
    var result = confy.FlagList.create_empty(A);
    try result.add_one("-DARCH_STRING=\"x86_64\"");  // TODO: Support the aarm64 case
    try result.add_many(confy.flags.default(.c)); // confy is strict by default (`-Weverything -Werror`)
    try result.add_many(&.{
      // Explicitly disable the warnings that the codebase does not respect.
      // Ideally this list should be completely empty, but that's a lot of work fixing old code.
      "-Wno-shadow",
      "-Wno-bad-function-cast",
      "-Wno-undef",
      "-Wno-conditional-uninitialized",
      "-Wno-cast-qual",
      "-Wno-cast-align",
      "-Wno-double-promotion",
      "-Wno-float-conversion",
      "-Wno-sign-conversion",
      "-Wno-sign-compare",
      "-Wno-implicit-float-conversion",
      "-Wno-implicit-int-conversion",
      "-Wno-implicit-int-float-conversion",
      "-Wno-shorten-64-to-32",
      "-Wno-shift-sign-overflow",
      "-Wno-float-equal",
      "-Wno-missing-field-initializers",
      "-Wno-missing-variable-declarations",
      "-Wno-incompatible-pointer-types-discards-qualifiers",
      "-Wno-unreachable-code",
      "-Wno-unreachable-code-loop-increment",
      "-Wno-unreachable-code-return",
      "-Wno-unreachable-code-break",
      "-Wno-invalid-noreturn",
      "-Wno-implicit-fallthrough",
      "-Wno-switch-default",
      "-Wno-switch-enum",
      "-Wno-assign-enum",
      "-Wno-unsafe-buffer-usage",
      "-Wno-padded",
      "-Wno-format",
      "-Wno-format-nonliteral",
      "-Wno-comma",
      "-Wno-extra-semi",
      "-Wno-extra-semi-stmt",
      "-Wno-pre-c11-compat",
      "-Wno-pre-c23-compat",
      "-Wno-missing-prototypes",
      "-Wno-unused-parameter",
      "-Wno-unused-variable",
      "-Wno-unused-macros",
      "-Wno-unused-function",
      "-Wno-used-but-marked-unused",
      "-Wno-disabled-macro-expansion",
      "-Wno-keyword-macro",
      "-Wno-macro-redefined",
      "-Wno-reserved-macro-identifier",
      "-Wno-reserved-identifier",
      "-Wno-empty-translation-unit",
      "-Wno-date-time",
    });
    return result;
  }
};


//______________________________________
// @section Game: Source Code
//____________________________
const code = struct {
  //__________________
  // Game: Common to all Libraries
  const lib = &.{
    Game.dir.common++"/q_shared.c",
    Game.dir.common++"/q_math.c",
  };
  const both = &.{
    Game.dir.server++"/bg_misc.c",
    Game.dir.server++"/bg_pmove.c",
    Game.dir.server++"/bg_slidemove.c",
  };
  //__________________
  // Game: Client
  fn client (A :std.mem.Allocator) !confy.CodeList {
    var result = confy.CodeList.create(A);
    try result.add_folder(Game.dir.client, .{});
    try result.add_folder(Game.dir.phy, .{});
    try result.add_folder(Game.dir.hud, .{});
    try result.add_many(Game.code.lib);
    try result.add_many(Game.code.both);
    return result;
  }
  //__________________
  // Game: Server
  fn server (A :std.mem.Allocator) !confy.CodeList {
    var result = confy.CodeList.create(A);
    try result.add_folder(Game.dir.server, .{});
    try result.add_folder(Game.dir.phy, .{});
    try result.add_many(Game.code.lib);
    try result.add_many(Game.code.both);
    return result;
  }
  //__________________
  // Game: UI
  fn ui (A :std.mem.Allocator) !confy.CodeList {
    var result = confy.CodeList.create(A);
    try result.add_folder(Game.dir.ui.base, .{});
    try result.add_folder(Game.dir.ui.core, .{});
    try result.add_folder(Game.dir.ui.color, .{});
    try result.add_folder(Game.dir.ui.fwk, .{});
    try result.add_folder(Game.dir.ui.menu, .{});
    try result.add_many(Game.code.lib);
    return result;
  }
};


//______________________________________
// @section Game: Build Targets
//____________________________
const Part = enum { client, server, ui };
//__________________
const target = struct {
  //__________________
  // Game: Any
  fn any (
      name : confy.cstring,
      pkg  : confy.package.Info,
      cfg  : confy.Config,
      src  : confy.CodeList,
      A    : std.mem.Allocator,
    ) !confy.Target {_=pkg;
    const flgs = try Game.flags.all(A); defer flgs.destroy();
    return confy.target(.dynamic, .{
      .trg          = name,
      .src          = src.files.data(),
      .src_absolute = true,
      .flags        = flgs.data(),
      .cfg          = cfg,
      // .version = pkg.version,  TODO: Is this needed??
    });
  }
  //__________________
  // Game: Client
  fn client (pkg :confy.package.Info, cfg :confy.Config, A :std.mem.Allocator) !confy.Target {
    const src = try code.client(A);
    return Game.target.any("cgame", pkg, cfg, src, A);
  }
  //__________________
  // Game: Server
  fn server (pkg :confy.package.Info, cfg :confy.Config, A :std.mem.Allocator) !confy.Target {
    const src = try code.server(A);
    return Game.target.any("qagame", pkg, cfg, src, A);
  }
  //__________________
  // Game: UI
  fn ui (pkg :confy.package.Info, cfg :confy.Config, A :std.mem.Allocator) !confy.Target {
    const src = try code.ui(A);
    return Game.target.any("ui", pkg, cfg, src, A);
  }
};
//__________________
pub fn create (pkg :confy.package.Info) !Game {
  var arena = std.heap.ArenaAllocator.init(std.heap.page_allocator);
  const A = arena.allocator();
  var cfg :confy.Config= .default();
  cfg.system.subfolder = true;
  cfg.system.appendCpu = true;
  return @This(){
    .client = try Game.target.client(pkg, cfg, A),
    .server = try Game.target.server(pkg, cfg, A),
    .ui     = try Game.target.ui(pkg, cfg, A),
    .A      = arena,
  };
}


//______________________________________
// @section Game Builder: Entry Point
//____________________________
pub fn buildFor (G :*Game, systems :[]const confy.System) !void {
  _= try G.client.buildFor(systems);
  _= try G.server.buildFor(systems);
  _= try G.ui.buildFor(systems);
}

