//:_________________________________________________________________
//  osdf  |  Copyright (C) Ivan Mar (sOkam!)  |  GPL-3.0-or-later  :
//:_________________________________________________________________
pub const code = @This();
pub const src  = @This();
const This = @This();
// @deps buildsystem
const confy = @import("confy");
const Game  = struct {
  const dir  = @import("./cfg.zig").dir;
  const code = This;
};


//______________________________________
// @section Directories
//____________________________
const dirs = struct {
  const client = &[_]confy.Glob{
    .{.dir= Game.dir.client },
    .{.dir= Game.dir.phy    },
    .{.dir= Game.dir.hud    },
  };
  //__________________
  const server = &[_]confy.Glob{
    .{.dir= Game.dir.server },
    .{.dir= Game.dir.phy    },
  };
  //__________________
  const ui = &[_]confy.Glob{
    .{.dir= Game.dir.ui.base  },
    .{.dir= Game.dir.ui.core  },
    .{.dir= Game.dir.ui.color },
    .{.dir= Game.dir.ui.fwk   },
    .{.dir= Game.dir.ui.menu  },
  };
  //__________________
  const ui_q3 = &[_]confy.Glob{
    .{.dir= Game.dir.ui.q3 },
  };
};


//______________________________________
// @section Files
//____________________________
const files = struct {
  const lib = &[_]confy.cstring{
    Game.dir.common++"/q_shared.c",
    Game.dir.common++"/q_math.c",
  };
  //__________________
  const misc = &[_]confy.cstring{
    Game.dir.server++"/bg_misc.c",
  };
  //__________________
  const both = struct {
    const base = &[_]confy.cstring{
      Game.dir.server++"/bg_misc.c",
      Game.dir.server++"/bg_pmove.c",
      Game.dir.server++"/bg_slidemove.c",
    };
    const ents = &[_]confy.cstring{
      Game.dir.server++"/ent/velocity/trigger/both.c",
      Game.dir.server++"/ent/velocity/target/both.c",
    };
    const all = base ++ ents;
  };
  //__________________
  const server = struct {
    const ents = &[_]confy.cstring{
      Game.dir.server++"/ent/velocity/trigger/server.c",
      Game.dir.server++"/ent/velocity/target/server.c",
    };
    const all = ents;
  };
};


//______________________________________
// @section Game: Client
//____________________________
pub const client = struct {
  pub const dirs  :confy.Glob.List    = Game.code.dirs.client;
  pub const files :confy.cstring_List = Game.code.files.lib ++ Game.code.files.both.all;
};


//______________________________________
// @section Game: Server
//____________________________
pub const server = struct {
  pub const dirs  :confy.Glob.List    = Game.code.dirs.server;
  pub const files :confy.cstring_List = Game.code.files.lib ++ Game.code.files.both.all ++ Game.code.files.server.all;
};


//______________________________________
// @section Game: UI
//____________________________
pub const ui = struct {
  pub const dirs  :confy.Glob.List    = Game.code.dirs.ui;
  pub const files :confy.cstring_List = Game.code.files.lib ++ Game.code.files.misc;
};
//__________________
pub const ui_q3 = struct {
  pub const dirs  :confy.Glob.List    = Game.code.dirs.ui_q3;
  pub const files :confy.cstring_List = Game.code.files.lib ++ Game.code.files.misc;
};

