//:_________________________________________________________________
//  osdf  |  Copyright (C) Ivan Mar (sOkam!)  |  GPL-3.0-or-later  :
//:_________________________________________________________________
pub const Game = @This();
// @deps buildsystem
const confy = @import("confy");


//______________________________________
// @section Object Fields
//____________________________
client  :confy.Target,
server  :confy.Target,
ui      :confy.Target,
pkg     :confy.package.Info,


//______________________________________
// @section Game: Project Configuration
//____________________________
const dir     = @import("./game/cfg.zig").dir;
const name    = @import("./game/cfg.zig").name;
const flags   = @import("./flags.zig");
const code    = @import("./game/source.zig").code;
const version = @import("../../info.zig").version;


//______________________________________
// @section Game: Flags
//____________________________
const defines = [_]confy.cstring{
  "-DGAME_NAME_SHORT=\"" ++ name.short ++ "\"",
  "-DGAME_NAME_LONG=\""  ++ name.long  ++ "\"",
  "-DGAME_NAME_HUMAN=\"" ++ name.human ++ "\"",
  "-DGAME_VERSION=\""    ++ version    ++ "\"",
};


//______________________________________
// @section Game: Build Targets
//____________________________
const target = struct {
  //__________________
  // Game: Client
  fn client (P :confy.Process, cfg :confy.Config) !confy.Target {
    return try confy.target(.dynamic, .{
      .trg          = "cgame",
      .src          = Game.code.client.files,
      .globs        = Game.code.client.dirs,
      .src_absolute = true,
      .flags        = flags.all ++ Game.defines,
      .cfg          = cfg,
      .P            = P,
      // .version      = pkg.version,  // TODO: Is this needed??
    });
  }
  //__________________
  // Game: Server
  fn server (P :confy.Process, cfg :confy.Config) !confy.Target {
    return try confy.target(.dynamic, .{
      .trg          = "qagame",
      .src          = Game.code.server.files,
      .globs        = Game.code.server.dirs,
      .src_absolute = true,
      .flags        = flags.all ++ Game.defines,
      .cfg          = cfg,
      .P            = P,
      // .version      = pkg.version,  // TODO: Is this needed??
    });
  }
  //__________________
  // Game: UI
  fn ui (P :confy.Process, cfg :confy.Config) !confy.Target {
    return try confy.target(.dynamic, .{
      .trg          = "ui",
      .src          = Game.code.ui_q3.files, // TODO: Game.code.ui.files
      .globs        = Game.code.ui_q3.dirs,  // TODO: Game.code.ui.dirs
      .src_absolute = true,
      .flags        = flags.all ++ Game.defines,
      .cfg          = cfg,
      .P            = P,
      // .version      = pkg.version,  // TODO: Is this needed??
    });
  }
};
//__________________
pub fn create (P :confy.Process, pkg :confy.package.Info, release :bool) !Game {_=release;
  var cfg :confy.Config= .default();
  cfg.system.subfolder = true;
  cfg.system.appendCpu = true;
  cfg.verbose = true;
  return Game{
    .client = try Game.target.client(P, cfg),
    .server = try Game.target.server(P, cfg),
    .ui     = try Game.target.ui(P, cfg),
    .pkg    = pkg,
  };
}


//______________________________________
// @section Game Builder: Entry Point
//____________________________
pub fn buildFor (G :*Game, systems :[]const confy.System) !void {
  for (systems) |system| {
    // Fix `arm64` vs `aarch64` naming nonsense
    const cpu = if (system.cpu == .aarch64) "arm64" else @tagName(system.cpu);
    if (system.cpu == .aarch64) {
      G.client.cfg.system.appendCpu = false;
      G.server.cfg.system.appendCpu = false;
      G.ui.cfg.system.appendCpu     = false;
      G.client.trg                  = try trg_renamed(&G.client, cpu);
      G.server.trg                  = try trg_renamed(&G.server, cpu);
      G.ui.trg                      = try trg_renamed(&G.ui,     cpu);
    }
    // Add ARCH_STRING flag and compile
    const arch_string = try confy.string.create_format("-DARCH_STRING=\"{s}\"", .{cpu}, G.client.A.allocator());
    try G.client.flags.add_one(arch_string.data());
    try G.server.flags.add_one(arch_string.data());
    try G.ui.flags.add_one(arch_string.data());
    _= try G.client.cross(system);
    _= try G.server.cross(system);
    _= try G.ui.cross(system);
    // Cleanup the windows output noise
    if (system.os == .windows) try confy.dir.remove_extensions(
      "./bin/x86_64-windows-gnu/", &.{".pdb", ".lib"}, G.client.io.io(),  G.client.A.allocator(), .{});
  }
}
//__________________
pub fn trg_renamed (trg :*confy.Target, cpu :confy.cstring) !confy.cstring {
  var result = try confy.string.create_format("{s}{s}", .{trg.trg, cpu}, trg.A.allocator());
  return try result.toOwned();
}

