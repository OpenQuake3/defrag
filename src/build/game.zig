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
// @section Game: Project Configuration
//____________________________
const dir   = @import("./game/cfg.zig").dir;
const flags = @import("./flags.zig");
const code  = @import("./game/source.zig").code;


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
    ) !confy.Target {
    var flgs = try Game.flags.all(A);
    try flgs.add_one(try std.fmt.allocPrint(A, "-DGAME_NAME_SHORT=\"{s}\"", .{pkg.name.short}));
    try flgs.add_one(try std.fmt.allocPrint(A, "-DGAME_NAME_LONG=\"{s}\"",  .{pkg.name.long  orelse "Undefined_LongName"}));
    try flgs.add_one(try std.fmt.allocPrint(A, "-DGAME_NAME_HUMAN=\"{s}\"", .{pkg.name.human orelse "Undefined_HumanName"}));
    try flgs.add_one(try std.fmt.allocPrint(A, "-DGAME_VERSION=\"{f}\"",    .{pkg.version}));
    return confy.target(.dynamic, .{
      .trg          = name,
      .src          = src.files.data(),
      .src_absolute = true,
      .flags        = flgs.data(),
      .cfg          = cfg,
      // .version      = pkg.version,  // TODO: Is this needed??
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
    const src = try code.ui_q3(A);   // TODO: try code.ui(A);
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
  // cfg.verbose = true;
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
pub fn buildFor (G :*Game, systems :[]const confy.System, release :bool) !void { for (systems) |system| {
  _=release;
  var trg = G.*;
  // Fix `arm64` vs `aarch64` naming nonsense
  const cpu = if (system.cpu == .aarch64) "arm64" else @tagName(system.cpu);
  if (system.cpu == .aarch64) {
    trg.client.cfg.system.appendCpu = false;
    trg.server.cfg.system.appendCpu = false;
    trg.ui.cfg.system.appendCpu     = false;
    trg.client.trg = try trg_renamed(&trg.client, cpu);
    trg.server.trg = try trg_renamed(&trg.server, cpu);
    trg.ui.trg     = try trg_renamed(&trg.ui,     cpu);
  }
  // Add ARCH_STRING flag and compile
  const arch_string = try @import("std").fmt.allocPrint(G.A.allocator(), "-DARCH_STRING=\"{s}\"", .{cpu});
  try trg.client.flags.add_one(arch_string);
  try trg.server.flags.add_one(arch_string);
  try trg.ui.flags.add_one(arch_string);
  _= try trg.client.cross(system);
  _= try trg.server.cross(system);
  _= try trg.ui.cross(system);
  // Cleanup the windows output noise
  if (system.os == .windows) try confy.dir.remove_extensions(
    "./bin/x86_64-windows-gnu/", &.{".pdb", ".lib"}, G.A.allocator(), .{});
}}
//__________________
pub fn trg_renamed (trg :*confy.Target, cpu :confy.cstring) !confy.cstring {
  return try @import("std").fmt.allocPrint(trg.A.allocator(), "{s}{s}", .{trg.trg, cpu});
}

