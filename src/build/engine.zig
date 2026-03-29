//:_________________________________________________________________
//  osdf  |  Copyright (C) Ivan Mar (sOkam!)  |  GPL-3.0-or-later  :
//:_________________________________________________________________
pub const Engine = @This();
// @deps buildsystem
const confy = @import("confy");


todo:bool=true,


//______________________________________
// @section Engine: Build Targets
//____________________________
pub fn create (pkg :confy.package.Info) !Engine {_=pkg; return .{}; }


//______________________________________
// @section Engine Builder: Entry Point
//____________________________
pub fn buildFor (E :*Engine, systems :[]const confy.System) !void {_=E;_=systems;
  confy.echo("TODO: Engine Builder");
  // _= try E.client.buildFor(systems);
  // _= try E.server.buildFor(systems);
}

