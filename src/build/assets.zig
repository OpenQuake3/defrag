//:_________________________________________________________________
//  osdf  |  Copyright (C) Ivan Mar (sOkam!)  |  GPL-3.0-or-later  :
//:_________________________________________________________________
pub const Assets = @This();
// @deps std
const std = @import("std");
// @deps buildsystem
const confy = @import("confy");
const cfg = @import("./cfg.zig");


//______________________________________
// @section Object Fields
//____________________________
A     :std.heap.ArenaAllocator,
list  :confy.seq(confy.cstring),


//______________________________________
// @section Assets Manager: Create/Destroy
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
// @section Assets Manager: Packing
//____________________________
pub fn pack (
    src       : confy.cstring,
    trg       : confy.cstring,
    A         : std.mem.Allocator,
    arg       : struct{
      root    : confy.cstring = cfg.dir.assets,
      modname : confy.cstring = cfg.modname,
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
  confy.echo(cfg.modname++": Packing all assets into .pk3 files ...");
  for (res.list.data()) |asset| {
    confy.prnt(cfg.modname++": Packing {s} ...", .{asset});
    try Assets.pack(asset, "./bin/"++cfg.dir.assets, res.A.allocator(), .{});
  }
} //:: build.Assets.packAll

