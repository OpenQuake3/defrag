//:_________________________________________________________________
//  osdf  |  Copyright (C) Ivan Mar (sOkam!)  |  GPL-3.0-or-later  :
//:_________________________________________________________________
pub const flags = @This();
// @deps std
const std = @import("std");
// @deps buildsystem
const confy = @import("confy");


pub fn all (A :std.mem.Allocator) !confy.FlagList {
  var result = confy.FlagList.create_empty(A);
  try result.add_many(confy.flags.default(.c)); // confy is strict by default (`-Weverything -Werror`)
  try result.add_one("-std=c99");               // confy is c23 by default
  try result.add_many(&.{
    // Explicitly disable the warnings that the codebase does not respect.
    // Ideally this list should be completely empty, but that's a lot of work fixing old code.
    "-Wno-alloca",
    "-Wno-pedantic",
    "-Wno-strict-prototypes",
    "-Wno-shadow",
    "-Wno-bad-function-cast",
    "-Wno-undef",
    "-Wno-conditional-uninitialized",
    "-Wno-cast-qual",
    "-Wno-cast-align",
    "-Wno-cast-function-type-strict",
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
    "-Wno-unreachable-code-return",
    "-Wno-unreachable-code-break",
    "-Wno-invalid-noreturn",
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
  });
  try result.add_one("-fno-sanitize=all");  // This is really bad, but the code does not run with sanitizer active
  return result;
}

