//:_________________________________________________________________
//  osdf  |  Copyright (C) Ivan Mar (sOkam!)  |  GPL-3.0-or-later  :
//:_________________________________________________________________
pub const src  = @This();
pub const code = @This();
const _This = @This();
// @deps std
const std = @import("std");
// @deps buildsystem
const confy = @import("confy");
const Game  = struct {
  const dir  = @import("./cfg.zig").dir;
  const code = _This;
};


//______________________________________
// @section Game Code: Common to all Libraries
//____________________________
const lib = &.{
  Game.dir.common++"/q_shared.c",
  Game.dir.common++"/q_math.c",
};
const misc = Game.dir.server++"/bg_misc.c";
const both = &.{
  Game.code.misc,
  Game.dir.server++"/bg_pmove.c",
  Game.dir.server++"/bg_slidemove.c",
};


//______________________________________
// @section Game Code: Client
//____________________________
pub fn client (A :std.mem.Allocator) !confy.CodeList {
  var result = confy.CodeList.create(A);
  try result.add_folder(Game.dir.client, .{});
  try result.add_folder(Game.dir.phy, .{});
  try result.add_folder(Game.dir.hud, .{});
  try result.add_many(Game.code.lib);
  try result.add_many(Game.code.both);
  return result;
}


//______________________________________
// @section Game Code: Server
//____________________________
pub fn server (A :std.mem.Allocator) !confy.CodeList {
  var result = confy.CodeList.create(A);
  try result.add_folder(Game.dir.server, .{});
  try result.add_folder(Game.dir.phy, .{});
  try result.add_many(Game.code.lib);
  try result.add_many(Game.code.both);
  return result;
}


//______________________________________
// @section Game Code: UI
//____________________________
pub fn ui (A :std.mem.Allocator) !confy.CodeList {
  var result = confy.CodeList.create(A);
  try result.add_folder(Game.dir.ui.base, .{});
  try result.add_folder(Game.dir.ui.core, .{});
  try result.add_folder(Game.dir.ui.color, .{});
  try result.add_folder(Game.dir.ui.fwk, .{});
  try result.add_folder(Game.dir.ui.menu, .{});
  try result.add_many(Game.code.lib);
  try result.add_one(Game.code.misc);
  return result;
}
//__________________
pub fn ui_q3 (A :std.mem.Allocator) !confy.CodeList {
  var result = confy.CodeList.create(A);
  try result.add_folder(Game.dir.ui.q3, .{});
  try result.add_many(Game.code.lib);
  try result.add_one(Game.code.misc);
  return result;
}

