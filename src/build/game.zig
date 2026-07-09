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
const cfg   = @import("./cfg.zig");
const dir   = @import("./cfg.zig").dir;
const name  = @import("./cfg.zig").name;
const flags = @import("./flags.zig").game;
const code  = @import("./source.zig").code;


//______________________________________
// @section Game: Flags
//____________________________
const defines = [_]confy.cstring{
  "-DGAME_NAME_SHORT=\"" ++ cfg.name.short ++ "\"",
  "-DGAME_NAME_LONG=\""  ++ cfg.name.long  ++ "\"",
  "-DGAME_NAME_HUMAN=\"" ++ cfg.name.human ++ "\"",
  "-DGAME_VERSION=\""    ++ cfg.info.version ++ "\"",
};


//______________________________________
// @section Game: Build Targets
//____________________________
const target = struct {
  //__________________
  // Game: Client
  fn client (P :confy.Process, C :confy.Config, release :bool) !confy.Target {
    return try confy.target(.dynamic, .{
      .trg          = "cgame",
      .src          = Game.code.client.files,
      .globs        = Game.code.client.dirs,
      .src_absolute = true,
      .flags        = Game.flags.all(release),
      .cfg          = C,
      .P            = P,
      // .version      = pkg.version,  // TODO: Is this needed??
    });
  }
  //__________________
  // Game: Server
  fn server (P :confy.Process, C :confy.Config, release :bool) !confy.Target {
    return try confy.target(.dynamic, .{
      .trg          = "qagame",
      .src          = Game.code.server.files,
      .globs        = Game.code.server.dirs,
      .src_absolute = true,
      .flags        = Game.flags.all(release),
      .cfg          = C,
      .P            = P,
      // .version      = pkg.version,  // TODO: Is this needed??
    });
  }
  //__________________
  // Game: UI
  fn ui (P :confy.Process, C :confy.Config, release :bool) !confy.Target {
    return try confy.target(.dynamic, .{
      .trg          = "ui",
      .src          = Game.code.ui_q3.files, // TODO: Game.code.ui.files
      .globs        = Game.code.ui_q3.dirs,  // TODO: Game.code.ui.dirs
      .src_absolute = true,
      .flags        = Game.flags.all(release),
      .cfg          = C,
      .P            = P,
      // .version      = pkg.version,  // TODO: Is this needed??
    });
  }
};
//__________________
pub fn create (P :confy.Process, pkg :confy.package.Info, release :bool) !Game {
  var config :confy.Config= .default();
  config.system.subfolder = true;
  config.system.appendCpu = true;
  config.verbose          = true;
  return Game{
    .client = try Game.target.client(P, config, release),
    .server = try Game.target.server(P, config, release),
    .ui     = try Game.target.ui(P, config, release),
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
    var client = try G.client.clone();
    var server = try G.server.clone();
    var ui     = try G.ui.clone();
    if (system.cpu == .aarch64) {
      client.cfg.system.appendCpu = false;
      server.cfg.system.appendCpu = false;
      ui.cfg.system.appendCpu     = false;
      client.trg                  = try trg_renamed(&client, cpu);
      server.trg                  = try trg_renamed(&server, cpu);
      ui.trg                      = try trg_renamed(&ui,     cpu);
    }
    // Add ARCH_STRING flag and compile
    const arch_string = try confy.string.create_format("-DARCH_STRING=\"{s}\"", .{cpu}, client.A.allocator());
    try client.flags.add_one(arch_string.data());
    try server.flags.add_one(arch_string.data());
    try ui.flags.add_one(arch_string.data());
    client.system = system;
    server.system = system;
    ui.system     = system;
    try client.build();
    try server.build();
    try ui.build();
    // Cleanup the windows output noise
    if (system.os == .windows) try confy.dir.remove_extensions(
      try client.out_dir(), &.{".pdb", ".lib"}, client.io.io(), client.A.allocator(), .{});
  }
}
//__________________
pub fn trg_renamed (trg :*confy.Target, cpu :confy.cstring) !confy.cstring {
  var result = try confy.string.create_format("{s}{s}", .{trg.trg, cpu}, trg.A.allocator());
  return try result.toOwned();
}

