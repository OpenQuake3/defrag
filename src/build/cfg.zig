//:_________________________________________________________________
//  osdf  |  Copyright (C) Ivan Mar (sOkam!)  |  GPL-3.0-or-later  :
//:_________________________________________________________________
pub const cfg = @This();

pub const modname = @import("./game/cfg.zig").name;

pub const dir = struct {
  pub const assets = "assets";
  pub const config = "./src/cfg";
};

