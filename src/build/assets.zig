//:_________________________________________________________________
//  osdf  |  Copyright (C) Ivan Mar (sOkam!)  |  GPL-3.0-or-later  :
//:_________________________________________________________________
// @deps std
const std = @import("std");
// @deps buildsystem
const confy = @import("confy");
const cfg = @import("./cfg.zig");

pub const Assets = struct {
  //______________________________________
  // @section Object Fields
  //____________________________
  io    :std.Io,
  A     :*confy.Allocator,
  list  :confy.seq(confy.cstring),
  info  :confy.package.Info,


  //______________________________________
  // @section Configuration
  //____________________________
  pub const dir_out = "./bin/"++cfg.dir.assets;


  //______________________________________
  // @section Create/Destroy
  //____________________________
  pub fn create (P :confy.Process, pkg :confy.package.Info) !Assets {
    var result = Assets{
      .A    = P.arena,
      .io   = P.io,
      .info = pkg,
      .list = undefined,
    };
    result.list = try confy.dir.list(cfg.dir.assets, result.io, result.A.allocator(), .{
      .kind    = .folders,
      .exclude = &.{"src", "dep", "skel"},
    });
    return result;
  } //:: build.Assets.create


  //______________________________________
  // @section Cleanup
  //____________________________
  pub fn clean (res :*Assets) !void {
    if (!confy.dir.exists(Assets.dir_out, res.io, .{})) return;
    try confy.dir.remove(Assets.dir_out,  res.io, .{});
  }

  //______________________________________
  // @section Assets Manager: Packing
  //____________________________
  pub fn pack (
      src       : confy.cstring,
      trg       : confy.cstring,
      io        : std.Io,
      A         : std.mem.Allocator,
      arg       : struct{
        root    : confy.cstring = cfg.dir.assets,
        modname : confy.cstring = cfg.name.short,
    }) !void {
    // Change dir to ./root/src for the zip command
    const cwd = try confy.path.join(A, &.{arg.root, src});
    // Compress all into y.MODNAME.src.pk3
    const filename = try confy.string.create_format("y.{s}.{s}.pk3", .{arg.modname, src}, A);
    confy.prnt(cfg.name.short++": Compressing {s} into {s}...", .{cwd, filename.data()});
    try confy.shell.zip(cwd, filename.data(), io, A, .{});
    // Write resulting zip into trg
    const zip = try confy.path.join(A, &.{arg.root, filename.data()});
    const out = try confy.path.join(A, &.{trg,      filename.data()});
    try confy.dir.create(trg, io, .{});
    try confy.file.move(zip, out, io, .{});
  } //:: build.Assets.pack
  //__________________
  pub fn packAll (res :*Assets) !void {
    confy.echo(cfg.name.short++": Packing all assets into .pk3 files ...");
    for (res.list.data()) |asset| {
      confy.prnt(cfg.name.short++": Packing {s} ...", .{asset});
      try Assets.pack(asset, "./bin/"++cfg.dir.assets, res.io, res.A.allocator(), .{});
    }
  } //:: build.Assets.packAll
  //__________________
  pub fn packFor (
      res     : *Assets,
      systems : []const confy.System,
    ) !void {
    const root    = "./bin/"++cfg.dir.assets;
    const modname = cfg.name.short;
    const A       = res.A.allocator();
    try confy.dir.create(root, res.io, .{});
    try res.packAll();
    for (res.list.data()) |asset| {
      var filename = confy.string.create_empty(A);
      try filename.write("y.{s}.{s}.pk3", .{modname, asset});
      for (systems) |system| {
        const sub = try system.zig_triple(A);
        const dir = try confy.path.join(A, &.{"./bin", sub, cfg.name.short});
        try confy.dir.create(dir, res.io, .{});
        const src = try confy.path.join(A, &.{root, filename.data()});
        const trg = try confy.path.join(A, &.{dir, filename.data()});
        try confy.file.copy(src, trg, res.io, .{});
      }
    }
  } //:: build.Assets.packFor
}; //:: build.Assets


//______________________________________
// @section Config Manager
//____________________________
pub const Config = struct {
  //______________________________________
  // @section Object Fields
  //____________________________
  io    :std.Io,
  A     :*confy.Allocator,
  list  :confy.seq(confy.cstring),
  info  :confy.package.Info,


  //______________________________________
  // @section Create/Destroy
  //____________________________
  pub fn create (P :confy.Process, pkg :confy.package.Info) !Config {
    var result = Config{
      .info = pkg,
      .list = undefined,
      .A    = P.arena,
      .io   = P.io,
    };
    result.list = try confy.dir.list(cfg.dir.config, result.io, result.A.allocator(), .{.kind= .files});
    try result.list.add_one("phy");
    return result;
  } //:: build.Config.create


  //______________________________________
  // @section Config Manager: Packing
  //____________________________
  pub fn packAll (C :*Config) !void {
    confy.echo(cfg.name.short++": Copying all config files into the assets output folder ...");
    //__________________
    // Copy all `.cfg` individual files
    for (C.list.data()) |file| {
      if (std.mem.eql(u8, file, "phy")) continue;
      const src = try confy.path.join(C.A.allocator(), &.{cfg.dir.config, file});
      const trg = try confy.path.join(C.A.allocator(), &.{"bin", cfg.dir.assets, file});
      try confy.file.copy(src, trg, C.io, .{});
    }
    //__________________
    // Explicitly copy the ./src/cfg/phy folder
    const phy_src = try confy.path.join(C.A.allocator(), &.{cfg.dir.config, "phy"});
    const phy_trg = try confy.path.join(C.A.allocator(), &.{"bin", cfg.dir.assets, "phy"});
    // TODO: Should be:   try confy.dir.copy(phy_src, phy_trg, .{});
    var phy_cmd = confy.Command.create(C.io, C.A.allocator());
    try phy_cmd.add_many(&.{"cp", "-r", phy_src, phy_trg});
    try phy_cmd.run();
    //__________________
    // Modify the `description.txt` file with the correct values
    const description_trg  = try confy.path.join(C.A.allocator(), &.{"bin", cfg.dir.assets, "description.txt"});
    var   description_code = confy.string.fromOwned(@constCast(try confy.file.read(description_trg, C.io, C.A.allocator(), .{})),  C.A.allocator());
    var   version          = confy.string.create_empty(C.A.allocator());
    try version.write("{f}", .{C.info.version});
    try description_code.replace("[MOD_HUMAN_NAME]", cfg.name.human);
    try description_code.replace("[SEP]", " ");
    try description_code.replace("[MOD_VERSION]", version.data());
    try confy.file.write(description_trg, description_code.data(), C.io, .{});
  } //:: build.Config.packAll
  //__________________
  pub fn packFor (
      C       : *Config,
      systems : []const confy.System,
    ) !void {
    const root = "./bin/"++cfg.dir.assets;
    const A    = C.A.allocator();
    try confy.dir.create(root, C.io, .{});
    try C.packAll();
    for (systems) |system| {
      const system_sub = try system.zig_triple(A);
      const trg        = try confy.path.join(A, &.{"./bin", system_sub, cfg.name.short});
      try confy.dir.create(trg, C.io, .{});
      try confy.dir.copy_contents(root, trg, C.io, A, .{});
    }
  } //:: build.Config.packFor
}; //:: build.Config

