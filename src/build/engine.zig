//:_________________________________________________________________
//  osdf  |  Copyright (C) Ivan Mar (sOkam!)  |  GPL-3.0-or-later  :
//:_________________________________________________________________
pub const Engine = @This();
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


//______________________________________
// @section Engine: Project Configuration
//____________________________
const idtech3 = @import("./engine/cfg.zig").idtech3;
const flags   = @import("./engine/flags.zig");
const code    = @import("./engine/source.zig");


//______________________________________
// @section Engine: Build Targets
//____________________________
const target = struct {
  //__________________
  // Engine: Client
  fn client (
      pkg : confy.package.Info,
      cfg : confy.Config,
      A   : std.mem.Allocator,
    ) !confy.Target {_=pkg;
    const src  = try Engine.code.client(A);
    var   flgs = try Engine.flags.client(A);
    return confy.target(.program, .{
      .trg          = "oQ3.",  // TODO: Take from options  (todo: pkg)
      .src          = src.files.data(),
      .src_absolute = true,
      .flags        = flgs.data(),
      .cfg          = cfg,
      // .version      = pkg.version,  // TODO: Is this needed??
    });
  }
  //__________________
  // Engine: Server
  fn server (
      pkg : confy.package.Info,
      cfg : confy.Config,
      A   : std.mem.Allocator,
    ) !confy.Target {_=pkg;
    const src  = try Engine.code.server(A);
    var   flgs = try Engine.flags.server(A);
    return confy.target(.program, .{
      .trg          = "oQ3.dedicated.",  // TODO: Take from options  (todo: pkg)
      .src          = src.files.data(),
      .src_absolute = true,
      .flags        = flgs.data(),
      .cfg          = cfg,
      // .version      = pkg.version,  // TODO: Is this needed??
    });
  }
};
//__________________
pub fn create (pkg :confy.package.Info) !Engine {
  var arena = std.heap.ArenaAllocator.init(std.heap.page_allocator);
  const A = arena.allocator();
  var cfg :confy.Config= .default();
  cfg.system.subfolder = true;
  cfg.system.appendCpu = true;
  //__________________
  // Make sure the requirements are accessible
  try Engine.requirements(pkg, A);
  //__________________
  // Create the Targets
  return @This(){
    .client = try Engine.target.client(pkg, cfg, A),
    .server = try Engine.target.server(pkg, cfg, A),
    .A      = arena,
  };
}

//______________________________________
// @section Engine: Requirements
//____________________________
fn requirements (pkg :confy.package.Info, A :std.mem.Allocator) !void {_=pkg;
  if (!confy.dir.exists(Engine.idtech3.dir, .{})) {
    // 1. Clone the repository
    try confy.git.clone(Engine.idtech3.Remote, A, .{.trg= Engine.idtech3.dir });
    // 2. Revert Blacklisted commits
    for (Engine.idtech3.blacklist.commits) |commit| {
      var cmd = confy.Command.create(A); defer cmd.destroy();
      cmd.cwd = Engine.idtech3.dir;
      try cmd.add_many(&.{"git", "revert", "--mainline", "1", "--no-edit", commit});
      try cmd.run();
    }
    // 3. Apply custom patches
    //    ID3_SOUND_SIMD
    //    Automatic load osdf
    //    Default model to ranger
    //    ... other patches from engine repo
  }
}


//______________________________________
// @section Engine Builder: Entry Point
//____________________________
pub fn buildFor (E :*Engine, systems :[]const confy.System, release :bool) !void {_=systems;_=release;
  // _= try E.client.buildFor(systems);
  // _= try E.server.buildFor(systems);

  // Process
  try E.client.build();
}

