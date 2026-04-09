//:_________________________________________________________________
//  osdf  |  Copyright (C) Ivan Mar (sOkam!)  |  GPL-3.0-or-later  :
//:_________________________________________________________________
pub const code = @This();
pub const src  = @This();
const _This = @This();
// @deps std
const std = @import("std");
// @deps buildsystem
const confy  = @import("confy");
const Engine = struct {
  const dir  = @import("./cfg.zig").dir;
  const code = _This;
};


//______________________________________
// @section Engine Code: Common to both
//____________________________
const lib = &.{
  Engine.dir.common++"/q_shared.c",
  Engine.dir.common++"/q_math.c",
};
const misc = Engine.dir.server++"/bg_misc.c";
const both = &.{
  Engine.code.misc,
  Engine.dir.server++"/bg_pmove.c",
  Engine.dir.server++"/bg_slidemove.c",
};


//______________________________________
// @section Engine Code: Client
//____________________________
pub fn client (A :std.mem.Allocator) !confy.CodeList {
  var result = confy.CodeList.create(A);
  inline for (Engine.code.hardcoded.client) |file| try result.add_one(Engine.code.hardcoded.root++file);
  return result;
}


//______________________________________
// @section Engine: Server
//____________________________
pub fn server (A :std.mem.Allocator) !confy.CodeList {
  var result = confy.CodeList.create(A);
  inline for (Engine.code.hardcoded.client) |file| try result.add_one(Engine.code.hardcoded.root++file);
  return result;
}


//______________________________________
// @section Engine: Hardcoded File Lists
//____________________________
// FIX: Remove these lists. They should be globbed instead.
pub const hardcoded = struct {
  pub const root = "./bin/.lib/idtech3/code/";
  //______________________________________
  // @section Engine Client: Linux
  //____________________________
  pub const client = &.{
    // cc -DELF -x assembler-with-cpp -o build/debug-linux-x86_64/client/snd_mix_x86_64.o -c code/asm/snd_mix_x86_64.s

    // -DBOTLIB
    "botlib/be_aas_bspq3.c",
    "botlib/be_aas_cluster.c",
    "botlib/be_aas_debug.c",
    "botlib/be_aas_entity.c",
    "botlib/be_aas_file.c",
    "botlib/be_aas_main.c",
    "botlib/be_aas_move.c",
    "botlib/be_aas_optimize.c",
    "botlib/be_aas_reach.c",
    "botlib/be_aas_route.c",
    "botlib/be_aas_routealt.c",
    "botlib/be_aas_sample.c",
    "botlib/be_ai_char.c",
    "botlib/be_ai_chat.c",
    "botlib/be_ai_gen.c",
    "botlib/be_ai_goal.c",
    "botlib/be_ai_move.c",
    "botlib/be_ai_weap.c",
    "botlib/be_ai_weight.c",
    "botlib/be_ea.c",
    "botlib/be_interface.c",
    "botlib/l_crc.c",
    "botlib/l_libvar.c",
    "botlib/l_log.c",
    "botlib/l_memory.c",
    "botlib/l_precomp.c",
    "botlib/l_script.c",
    "botlib/l_struct.c",

    // -fno-fast-math
    "qcommon/vm.c",
    "qcommon/vm_interpreted.c",
    "qcommon/vm_x86.c",

    // Everything else
    "qcommon/cm_load.c",
    "qcommon/cm_patch.c",
    "qcommon/cm_polylib.c",
    "qcommon/cm_test.c",
    "qcommon/cm_trace.c",
    "qcommon/cmd.c",
    "qcommon/common.c",
    "qcommon/cvar.c",
    "qcommon/files.c",
    "qcommon/history.c",
    "qcommon/keys.c",
    "qcommon/md4.c",
    "qcommon/md5.c",
    "qcommon/msg.c",
    "qcommon/net_chan.c",
    "qcommon/net_ip.c",
    "qcommon/huffman.c",
    "qcommon/huffman_static.c",
    "qcommon/q_math.c",
    "qcommon/q_shared.c",
    "qcommon/unzip.c",
    "qcommon/puff.c",

    "client/cl_cgame.c",
    "client/cl_cin.c",
    "client/cl_console.c",
    "client/cl_input.c",
    "client/cl_keys.c",
    "client/cl_main.c",
    "client/cl_net_chan.c",
    "client/cl_parse.c",
    "client/cl_scrn.c",
    "client/cl_ui.c",
    "client/cl_avi.c",
    "client/cl_tc_vis.c",
    "client/cl_jpeg.c",
    "client/cl_curl.c",
    "client/snd_adpcm.c",
    "client/snd_dma.c",
    "client/snd_mem.c",
    "client/snd_mix.c",
    "client/snd_wavelet.c",
    "client/snd_main.c",
    "client/snd_codec.c",
    "client/snd_codec_wav.c",
    "client/snd_codec_ogg.c",

    "server/sv_bot.c",
    "server/sv_ccmds.c",
    "server/sv_client.c",
    "server/sv_filter.c",
    "server/sv_game.c",
    "server/sv_init.c",
    "server/sv_main.c",
    "server/sv_net_chan.c",
    "server/sv_snapshot.c",
    "server/sv_world.c",

    "renderercommon/tr_font.c",
    "renderercommon/tr_image_png.c",
    "renderercommon/tr_image_jpg.c",
    "renderercommon/tr_image_bmp.c",
    "renderercommon/tr_image_tga.c",
    "renderercommon/tr_image_pcx.c",
    "renderercommon/tr_noise.c",

    "renderer/tr_animation.c",
    "renderer/tr_arb.c",
    "renderer/tr_backend.c",
    "renderer/tr_bsp.c",
    "renderer/tr_cmds.c",
    "renderer/tr_curve.c",
    "renderer/tr_flares.c",
    "renderer/tr_image.c",
    "renderer/tr_init.c",
    "renderer/tr_light.c",
    "renderer/tr_main.c",
    "renderer/tr_marks.c",
    "renderer/tr_mesh.c",
    "renderer/tr_model.c",
    "renderer/tr_model_iqm.c",
    "renderer/tr_scene.c",
    "renderer/tr_shade.c",
    "renderer/tr_shade_calc.c",
    "renderer/tr_shader.c",
    "renderer/tr_shadows.c",
    "renderer/tr_sky.c",
    "renderer/tr_surface.c",
    "renderer/tr_vbo.c",
    "renderer/tr_world.c",

    "unix/unix_main.c",
    "unix/unix_shared.c",
    "unix/linux_signals.c",

    "sdl/sdl_glimp.c",
    "sdl/sdl_gamma.c",
    "sdl/sdl_input.c",
    "sdl/sdl_snd.c",

    "libjpeg/jaricom.c",
    "libjpeg/jcapimin.c",
    "libjpeg/jcapistd.c",
    "libjpeg/jcarith.c",
    "libjpeg/jccoefct.c",
    "libjpeg/jccolor.c",
    "libjpeg/jcdctmgr.c",
    "libjpeg/jchuff.c",
    "libjpeg/jcinit.c",
    "libjpeg/jcmainct.c",
    "libjpeg/jcmarker.c",
    "libjpeg/jcmaster.c",
    "libjpeg/jcomapi.c",
    "libjpeg/jcparam.c",
    "libjpeg/jcprepct.c",
    "libjpeg/jcsample.c",
    "libjpeg/jctrans.c",
    "libjpeg/jdapimin.c",
    "libjpeg/jdapistd.c",
    "libjpeg/jdarith.c",
    "libjpeg/jdatadst.c",
    "libjpeg/jdatasrc.c",
    "libjpeg/jdcoefct.c",
    "libjpeg/jdcolor.c",
    "libjpeg/jddctmgr.c",
    "libjpeg/jdhuff.c",
    "libjpeg/jdinput.c",
    "libjpeg/jdmainct.c",
    "libjpeg/jdmarker.c",
    "libjpeg/jdmaster.c",
    "libjpeg/jdmerge.c",
    "libjpeg/jdpostct.c",
    "libjpeg/jdsample.c",
    "libjpeg/jdtrans.c",
    "libjpeg/jerror.c",
    "libjpeg/jfdctflt.c",
    "libjpeg/jfdctfst.c",
    "libjpeg/jfdctint.c",
    "libjpeg/jidctflt.c",
    "libjpeg/jidctfst.c",
    "libjpeg/jidctint.c",
    "libjpeg/jmemmgr.c",
    "libjpeg/jmemnobs.c",
    "libjpeg/jquant1.c",
    "libjpeg/jquant2.c",
    "libjpeg/jutils.c",

    "libogg/src/bitwise.c",
    "libogg/src/framing.c",

    "libvorbis/lib/analysis.c",
    "libvorbis/lib/bitrate.c",
    "libvorbis/lib/block.c",
    "libvorbis/lib/codebook.c",
    "libvorbis/lib/envelope.c",
    "libvorbis/lib/floor0.c",
    "libvorbis/lib/floor1.c",
    "libvorbis/lib/info.c",
    "libvorbis/lib/lookup.c",
    "libvorbis/lib/lpc.c",
    "libvorbis/lib/lsp.c",
    "libvorbis/lib/mapping0.c",
    "libvorbis/lib/mdct.c",
    "libvorbis/lib/psy.c",
    "libvorbis/lib/registry.c",
    "libvorbis/lib/res0.c",
    "libvorbis/lib/smallft.c",
    "libvorbis/lib/sharedbook.c",
    "libvorbis/lib/synthesis.c",
    "libvorbis/lib/vorbisfile.c",
    "libvorbis/lib/window.c",
  };

  //______________________________________
  // @section Engine Server: Linux
  //____________________________
  pub const server = &.{
  };
};

