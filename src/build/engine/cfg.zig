//:_________________________________________________________________
//  osdf  |  Copyright (C) Ivan Mar (sOkam!)  |  GPL-3.0-or-later  :
//:_________________________________________________________________
pub const cfg = @This();


pub const idtech3 = struct {
  pub const Remote = "https://github.com/jbustos22/oDFe";  // WARN: Redirect. Transfered to the Defrag-Racing org
  pub const dir    = "./bin/.lib/idtech3";
  pub const blacklist = struct {
    pub const commits = [_][]const u8{
      "8f2c9e323b2cb76612e7346699893e22c8f05811", // Strict No AI policy. See the Code of Conduct of this project.
    };
  };
};


pub const name = struct {
  pub const short = "oQ3";
  pub const long  = "openquake3";
  pub const human = "OpenQuake3";
};


pub const dir = struct {
  const root   = idtech3.dir;
  const src    = dir.root++"/code";
  const common = dir.src++"/qcommon";
  const client = dir.src++"/client";
  const server = dir.src++"/server";
};

