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
  A     :std.heap.ArenaAllocator,
  list  :confy.seq(confy.cstring),


  //______________________________________
  // @section Configuration
  //____________________________
  pub const dir_out = "./bin/"++cfg.dir.assets;


  //______________________________________
  // @section Create/Destroy
  //____________________________
  pub fn create (pkg :confy.package.Info) !@This() {_=pkg;
    var arena = std.heap.ArenaAllocator.init(std.heap.page_allocator);
    const A = arena.allocator();
    var result = @This(){
      .list = undefined,
      .A    = arena,
    };
    result.list = try confy.dir.list(cfg.dir.assets, A, .{.kind= .folders});
    return result;
  } //:: build.Assets.create


  //______________________________________
  // @section Cleanup
  //____________________________
  pub fn clean () !void {
    if (!confy.dir.exists(Assets.dir_out, .{})) return;
    try confy.dir.remove(Assets.dir_out, .{});
  }

  //______________________________________
  // @section Assets Manager: Packing
  //____________________________
  pub fn pack (
      src       : confy.cstring,
      trg       : confy.cstring,
      A         : std.mem.Allocator,
      arg       : struct{
        root    : confy.cstring = cfg.dir.assets,
        modname : confy.cstring = cfg.modname.short,
    }) !void {
    // Change dir to ./root/src for the zip command
    const cwd = try confy.path.join(A, &.{arg.root, src}); defer A.free(cwd);
    // Compress all into y.MODNAME.src.pk3
    var filename = confy.string.create_empty(A); defer filename.destroy();
    try filename.write("y.{s}.{s}.pk3", .{arg.modname, src});
    try confy.shell.zip(cwd, filename.data(), A);
    // Write resulting zip into trg
    const zip = try confy.path.join(A, &.{arg.root, filename.data()}); defer A.free(zip);
    const out = try confy.path.join(A, &.{trg,      filename.data()}); defer A.free(out);
    try confy.dir.create(trg, .{});
    try confy.file.move(zip, out, .{});
  } //:: build.Assets.pack
  //__________________
  pub fn packAll (res :*@This()) !void {
    confy.echo(cfg.modname.short++": Packing all assets into .pk3 files ...");
    for (res.list.data()) |asset| {
      confy.prnt(cfg.modname.short++": Packing {s} ...", .{asset});
      try Assets.pack(asset, "./bin/"++cfg.dir.assets, res.A.allocator(), .{});
    }
  } //:: build.Assets.packAll
}; //:: build.Assets


//______________________________________
// @section Config Manager
//____________________________
pub const Config = struct {
  //______________________________________
  // @section Object Fields
  //____________________________
  A     :std.heap.ArenaAllocator,
  list  :confy.seq(confy.cstring),
  info  :confy.package.Info,


  //______________________________________
  // @section Create/Destroy
  //____________________________
  pub fn create (pkg :confy.package.Info) !@This() {
    var arena = std.heap.ArenaAllocator.init(std.heap.page_allocator);
    const A = arena.allocator();
    var result = @This(){
      .info = pkg,
      .list = undefined,
      .A    = arena,
    };
    result.list = try confy.dir.list(cfg.dir.config, A, .{.kind= .files});
    try result.list.add_one("phy");
    return result;
  } //:: build.Config.create


  //______________________________________
  // @section Config Manager: Packing
  //____________________________
  pub fn packAll (C :*@This()) !void {
    confy.echo(cfg.modname.short++": Copying all config files into the assets output folder ...");
    //__________________
    // Copy all `.cfg` individual files
    for (C.list.data()) |file| {
      if (std.mem.eql(u8, file, "phy")) continue;
      const src = try confy.path.join(C.A.allocator(), &.{cfg.dir.config, file}); defer C.A.allocator().free(src);
      const trg = try confy.path.join(C.A.allocator(), &.{"bin", cfg.dir.assets, file}); defer C.A.allocator().free(trg);
      try confy.file.copy(src, trg, .{});
    }
    //__________________
    // Explicitly copy the ./src/cfg/phy folder
    const phy_src = try confy.path.join(C.A.allocator(), &.{cfg.dir.config, "phy"}); defer C.A.allocator().free(phy_src);
    const phy_trg = try confy.path.join(C.A.allocator(), &.{"bin", cfg.dir.assets, "phy"}); defer C.A.allocator().free(phy_trg);
    // TODO: Should be:   try confy.dir.copy(phy_src, phy_trg, .{});
    var phy_cmd = confy.Command.create(C.A.allocator()); defer phy_cmd.destroy();
    try phy_cmd.add_many(&.{"cp", "-r", phy_src, phy_trg});
    try phy_cmd.run();
    //__________________
    // Modify the `description.txt` file with the correct values
    const description_trg = try confy.path.join(C.A.allocator(), &.{"bin", cfg.dir.assets, "description.txt"}); defer C.A.allocator().free(description_trg);
    var description_code = confy.string.fromOwned(@constCast(try confy.file.read(description_trg, C.A.allocator(), .{})),  C.A.allocator()); defer description_code.destroy();
    var version = confy.string.create_empty(C.A.allocator()); defer version.destroy();
    try version.write("{f}", .{C.info.version});
    try description_code.replace("[MOD_HUMAN_NAME]", cfg.modname.human);
    try description_code.replace("[SEP]", " ");
    try description_code.replace("[MOD_VERSION]", version.data());
    try confy.file.write(description_trg, description_code.data(), .{});
  } //:: build.Config.packAll
}; //:: build.Config

