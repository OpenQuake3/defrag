//:_________________________________________________________________
//  osdf  |  Copyright (C) Ivan Mar (sOkam!)  |  GPL-3.0-or-later  :
//:_________________________________________________________________
pub const code = @This();
pub const src  = @This();
const This = @This();
// @deps std
const std = @import("std");
// @deps buildsystem
const confy  = @import("confy");
const Engine = struct {
  const dir  = @import("./cfg.zig").dir;
  const code = This;
};


//______________________________________
// @section Directories
//____________________________
const dirs = struct {
  const client = &[_]confy.Glob{
    .{.dir= Engine.dir.src++"/botlib" },
    .{.dir= Engine.dir.src++"/qcommon",        .opts= .{.filter= .{.excludes= &.{ "vm_x86", "vm_aarch64", "vm_armv7l" }}}},
    .{.dir= Engine.dir.src++"/client" },
    .{.dir= Engine.dir.src++"/server",         .opts= .{.filter= .{.excludes= &.{ "sv_rankings" }}}},
    .{.dir= Engine.dir.src++"/renderercommon" },
    .{.dir= Engine.dir.src++"/renderer" },
    .{.dir= Engine.dir.src++"/sdl" },
    .{.dir= Engine.dir.src++"/libjpeg" },
    .{.dir= Engine.dir.src++"/libogg/src" },
    .{.dir= Engine.dir.src++"/libvorbis/lib" },
  };
  //__________________
  const server = &[_]confy.Glob{
    .{.dir= Engine.dir.src++"/botlib" },
    .{.dir= Engine.dir.src++"/qcommon",  .opts= .{.filter= .{.excludes= &.{ "vm_x86", "vm_aarch64", "vm_armv7l" }}}},
    .{.dir= Engine.dir.src++"/server",   .opts= .{.filter= .{.excludes= &.{ "sv_rankings" }}}},
  };
  //__________________
  const unix = &[_]confy.Glob{.{ .dir= Engine.dir.src++"/unix", .opts= .{.filter= .{.excludes= &.{
    "linux_glimp", "linux_snd", "linux_joystick",
    "linux_qgl",   "linux_qvk",
    "x11_dga",     "x11_randr", "x11_vidmode",
  }}}}};
  //__________________
  const win32 = &[_]confy.Glob{.{ .dir= Engine.dir.src++"/win32", .opts= .{.filter= .{.excludes= &.{
    "win_gamma",    "win_glimp",   "win_input",
    "win_minimize", "win_snd",     "win_wndproc",
    "win_qgl",      "win_qvk",
  }}}}};
};


//______________________________________
// @section Directories
//____________________________
const files = struct {
  // Platform Specific
  const vm_x86     = &[_]confy.cstring{ Engine.dir.src++"/qcommon/vm_x86.c" };
  const vm_aarch64 = &[_]confy.cstring{ Engine.dir.src++"/qcommon/vm_aarch64.c" };
};


//______________________________________
// @section Engine: Client
//____________________________
pub const client = struct {
  const unix  = Engine.code.dirs.client ++ Engine.code.dirs.unix;
  const win32 = Engine.code.dirs.client ++ Engine.code.dirs.win32;
  //__________________
  pub fn dirs (system :confy.System) confy.Glob.List { return switch (system.os) {
    .windows => win32,
    else     => unix,
  };}
  //__________________
  pub fn files (system :confy.System) confy.cstring_List { return switch (system.cpu) {
    .x86_64  => Engine.code.files.vm_x86,
    .aarch64 => Engine.code.files.vm_aarch64,
    else     => &.{},
  };}
};


//______________________________________
// @section Engine: Server
//____________________________
pub const server = struct {
  const unix  = Engine.code.dirs.server ++ Engine.code.dirs.unix;
  const win32 = Engine.code.dirs.server ++ Engine.code.dirs.win32;
  //__________________
  pub fn dirs (system :confy.System) confy.Glob.List { return switch (system.os) {
    .windows => win32,
    else     => unix,
  };}
  //__________________
  pub fn files (system :confy.System) confy.cstring_List { return switch (system.cpu) {
    .x86_64  => Engine.code.files.vm_x86,
    .aarch64 => Engine.code.files.vm_aarch64,
    else     => &.{},
  };}
};

