//:_________________________________________________________________
//  osdf  |  Copyright (C) Ivan Mar (sOkam!)  |  GPL-3.0-or-later  :
//:_________________________________________________________________
pub const Release = @This();
// @deps std
const std = @import("std");
// @deps buildsystem
const confy = @import("confy");
const cfg   = @import("./cfg.zig");


//______________________________________
// @section Release Config Options
//____________________________
const root = "./bin/releases";


//______________________________________
// @section Object Fields
//____________________________
A      :std.heap.ArenaAllocator,
info   :confy.package.Info,
rev    :usize= 0,
files  :confy.seq(confy.Path),


//______________________________________
// @section Release: Create/Destroy
//____________________________
pub fn create (pkg :confy.package.Info) !Release {
  var   arena = std.heap.ArenaAllocator.init(std.heap.page_allocator);
  const A     = arena.allocator();
  const vers  = try std.fmt.allocPrint(A, "{f}", .{pkg.version});
  const dir   = try confy.path.join(A, &.{Release.root, vers});
  try confy.dir.create(Release.root, .{});
  try confy.dir.create(dir, .{});
  const revs  = try confy.dir.list(dir, A, .{});
  const rev   = revs.len(); // With 10 existing revisions, first is 0, last is 9, and next will be `.len()` (ie: 10)
  return @This(){
    .A     = arena,
    .info  = pkg,
    .rev   = rev,
    .files = undefined,
  };
}


//______________________________________
// @section Release: Packing Manager
//____________________________
pub fn packFor (
    R       : *@This(),
    systems : []const confy.System,
    release : bool,
  ) !void {
  if (!release) return;
  const A    = R.A.allocator();
  const vers = try std.fmt.allocPrint(A, "{f}", .{R.info.version});
  R.files    = .create_empty(A);
  for (systems) |system| {
    const sys_name = try system.zig_triple(A);
    //__________________
    // Create the target dir & file names
    const revision = try std.fmt.allocPrint(A, "r{d}", .{R.rev});
    var   trg_name = confy.string.create_empty(A);
    try trg_name.write("{s}-{s}-{s}{s}{s}.zip", .{
      R.info.name.long.?, sys_name, vers, if (R.rev != 0) "-" else "", if (R.rev != 0) revision else ""});
    const trg_dir = try confy.path.join(A, &.{Release.root, vers, revision});
    const trg_zip = try confy.path.join(A, &.{trg_dir, trg_name.data()});
    //__________________
    // Create the src dir & file names
    var src_name = confy.string.create_empty(A);
    try src_name.write("{s}.zip", .{sys_name});
    const src_dir = try confy.path.join(A, &.{"./bin", sys_name});
    const src_zip = try confy.path.join(A, &.{"./bin", src_name.data()});
    //__________________
    // Run the process
    confy.prnt(cfg.modname.short++": Packing release files for `{s}` ...\n  target: {s}", .{sys_name, trg_zip});
    try R.files.add_one(trg_zip);
    try confy.shell.zip(src_dir, src_name.data(), A);
    try confy.dir.create(trg_dir, .{});
    try confy.file.move(src_zip, trg_zip, .{});
  }
}


//______________________________________
// @section Release: Publishing Manager
//____________________________
pub fn publish (
    R       : *@This(),
    run     : bool,
  ) !void {
  if (!run) return;
  for (R.files.data()) |trg| {
    confy.echo(trg);
  }
}

