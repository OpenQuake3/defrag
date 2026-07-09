//:_________________________________________________________________
//  osdf  |  Copyright (C) Ivan Mar (sOkam!)  |  GPL-3.0-or-later  :
//:_________________________________________________________________
pub const flags = @This();
// @deps buildsystem
const confy = @import("confy");
// @deps config
const cfg = @import("./cfg.zig");


//______________________________________
// @section Common Flags: Optimization
//____________________________
const optimization = struct {
  const debug   = &[_]confy.cstring{ "-g", "-O0", "-DDEBUG", "-D_DEBUG" };
  const release = &[_]confy.cstring{ "-O2", "-DNDEBUG" };
};


//______________________________________
// @section Common Flags: (BASE_CFLAGS)
//____________________________
const base = confy.flags.default(.c) ++ &[_]confy.cstring{
  "-std=c99",
  "-fno-strict-aliasing",
  "-fvisibility=hidden",
  "-pipe",
};


//______________________________________
// @section Game Flags: Defines
//____________________________
const defines = &[_]confy.cstring{
  "-DGAME_NAME_SHORT=\"" ++ cfg.name.short ++ "\"",
  "-DGAME_NAME_LONG=\""  ++ cfg.name.long  ++ "\"",
  "-DGAME_NAME_HUMAN=\"" ++ cfg.name.human ++ "\"",
  "-DGAME_VERSION=\""    ++ cfg.info.version ++ "\"",
};


//______________________________________
// @section Game Flags: Precomputed
//____________________________
pub const game = struct {
  pub const dbg = flags.base ++ flags.unsafe ++ flags.defines ++ flags.optimization.debug;
  pub const rls = flags.base ++ flags.unsafe ++ flags.defines ++ flags.optimization.release;
  pub fn all (release :bool) confy.cstring_List {
    if (release) return flags.game.rls
    else         return flags.game.dbg;
  }
};


const unsafe = &[_]confy.cstring{
  // Explicitly disable the warnings that the codebase does not respect.
  // Ideally this list should be completely empty, but that's a lot of work fixing old code.
  "-Wno-alloca",
  "-Wno-pedantic",
  "-Wno-strict-prototypes",
  "-Wno-shadow",
  "-Wno-undef",
  "-Wno-conditional-uninitialized",
  "-Wno-cast-qual",
  "-Wno-cast-align",
  "-Wno-cast-function-type-strict",
  "-Wno-bad-function-cast",
  "-Wno-implicit-void-ptr-cast",
  "-Wno-implicit-int-enum-cast",  // C enums are ints
  "-Wno-double-promotion",
  "-Wno-float-conversion",
  "-Wno-enum-float-conversion",
  "-Wno-bitfield-enum-conversion",
  "-Wno-int-conversion",
  "-Wno-sign-conversion",
  "-Wno-sign-compare",
  "-Wno-implicit-float-conversion",
  "-Wno-implicit-int-conversion",
  "-Wno-implicit-int-float-conversion",
  "-Wno-implicit-function-declaration",
  "-Wno-shorten-64-to-32",
  "-Wno-shift-sign-overflow",
  "-Wno-float-equal",
  "-Wno-overlength-strings",
  "-Wno-tautological-unsigned-zero-compare",
  "-Wno-tautological-pointer-compare",
  "-Wno-missing-field-initializers",
  "-Wno-missing-variable-declarations",
  "-Wno-missing-noreturn",
  "-Wno-incompatible-pointer-types-discards-qualifiers",
  "-Wno-unreachable-code",
  "-Wno-unreachable-code-loop-increment",
  "-Wno-unreachable-code-break",
  "-Wno-unreachable-code-return",
  "-Wno-invalid-noreturn",
  "-Wno-nrvo",
  "-Wno-implicit-fallthrough",
  "-Wno-switch-default",
  "-Wno-switch-enum",
  "-Wno-assign-enum",
  "-Wno-unsafe-buffer-usage",
  "-Wno-padded",
  "-Wno-format",
  "-Wno-format-nonliteral",
  "-Wno-comma",
  "-Wno-extra-semi",
  "-Wno-extra-semi-stmt",
  "-Wno-ignored-qualifiers",
  "-Wno-pre-c11-compat",
  "-Wno-pre-c23-compat",
  "-Wno-c11-extensions",
  "-Wno-typedef-redefinition",
  "-Wno-missing-prototypes",
  "-Wno-unused-parameter",
  "-Wno-unused-variable",
  "-Wno-unused-macros",
  "-Wno-unused-function",
  "-Wno-used-but-marked-unused",
  "-Wno-unused-but-set-variable",
  "-Wno-unused-command-line-argument",
  "-Wno-disabled-macro-expansion",
  "-Wno-keyword-macro",
  "-Wno-macro-redefined",
  "-Wno-reserved-macro-identifier",
  "-Wno-reserved-identifier",
  "-Wno-redundant-parens",
  "-Wno-empty-translation-unit",
  "-Wno-date-time",
  "-Wno-documentation-unknown-command",
  "-Wno-documentation",
  "-fno-sanitize=all",
};
