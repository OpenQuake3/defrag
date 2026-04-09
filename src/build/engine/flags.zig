//:_________________________________________________________________
//  osdf  |  Copyright (C) Ivan Mar (sOkam!)  |  GPL-3.0-or-later  :
//:_________________________________________________________________
pub const flags = @This();
// @deps std
const std = @import("std");
// @deps buildsystem
const confy = @import("confy");


//______________________________________
// @section Engine Flags: Server
//____________________________
pub fn server (A :std.mem.Allocator) !confy.FlagList {
  var result = try @import("../flags.zig").all(A);
  // TODO: Warnings/Errors
  try result.add_one("-Wall"); // TODO: Remove. Change to list.
  // TODO: Optimization
  // if (!release) try result.add_many(&.{
  //   "-pipe",
  //   "-g",
  //   "-O0",
  //   "-DDEBUG",
  //   "-D_DEBUG",
  // });
  // TODO: Defines
  // TODO: Includes
  // TODO: Exceptions
  return result;
} //:: build.engine.flags.server


//______________________________________
// @section Engine Flags: Client
//____________________________
pub fn client (A :std.mem.Allocator) !confy.FlagList {
  var result = try @import("../flags.zig").all(A);
  // Warnings/Errors
  try result.add_many(&.{
    // "-MMD",
    // "-Wall",
    // "-Wimplicit",
    // "-Wstrict-prototypes",
    // "-Wno-unused-result",
  });
  // TODO: Optimization
  try result.add_many(&.{
    "-pipe",
    "-g",
    "-O0",
    "-DDEBUG",
    "-D_DEBUG",
  });
  // Defines
  try result.add_many(&.{
    "-DUSE_CURL",
    "-DUSE_CURL_DLOPEN",
    "-DUSE_PCRE2",
    "-DUSE_OPENGL_API",
    "-DUSE_ICON",
    "-DUSE_OGG_VORBIS",
  });
  // Includes
  try result.add_many(&.{
    // "-I/usr/include ",
    "-I/usr/local/include",
    "-I/usr/include/SDL2",
    "-I./bin/.lib/idtech3/code/libogg/include",
    "-I./bin/.lib/idtech3/code/libvorbis/include",
    "-I./bin/.lib/idtech3/code/libvorbis/lib",
  });
  // Exceptions
  try result.add_one("-fno-fast-math");  // NOTE: Only for vm_* files, but we add it for all
  try result.add_one("-DBOTLIB");        // NOTE: Only for botlib/* files, but we add it for all
  try result.add_one("-D_GNU_SOURCE");   //  FIX: Only for unix
  // Linker Flags
  try result.add_many(&.{
    "-lSDL2",
    "-lpcre2-8",
    "-lm",
    "-Wl,--gc-sections",
    "-fvisibility=hidden",
    "-ldl",
    "-Wl,--hash-style=both",
    "-rdynamic",
  });
  return result;
} //:: build.engine.flags.client

