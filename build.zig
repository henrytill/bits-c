const std = @import("std");
const Translator = @import("translate_c").Translator;

const Build = std.Build;

const flags: []const []const u8 = &.{
    "-std=c89",
    "-Wall",
    "-Wextra",
    "-Wpedantic",
    "-Wconversion",
    "-Wsign-conversion",
    "-D_DEFAULT_SOURCE",
};

const Ctx = struct {
    b: *Build,
    target: Build.ResolvedTarget,
    optimize: std.builtin.OptimizeMode,
    include_path: Build.LazyPath,

    fn cModule(ctx: Ctx, files: []const Build.LazyPath) *Build.Module {
        const m = ctx.b.createModule(.{
            .target = ctx.target,
            .optimize = ctx.optimize,
            .link_libc = true,
        });

        m.addIncludePath(ctx.include_path);

        for (files) |file| {
            m.addCSourceFile(.{ .file = file, .flags = flags });
        }

        return m;
    }

    fn cObj(ctx: Ctx, name: []const u8, files: []const Build.LazyPath) *Build.Step.Compile {
        return ctx.b.addObject(.{
            .name = name,
            .root_module = ctx.cModule(files),
        });
    }

    fn cExe(
        ctx: Ctx,
        name: []const u8,
        files: []const Build.LazyPath,
        objs: []const *Build.Step.Compile,
    ) *Build.Step.Compile {
        const m = ctx.cModule(files);

        for (objs) |o| {
            m.addObject(o);
        }

        return ctx.b.addExecutable(.{
            .name = name,
            .root_module = m,
        });
    }

    fn zigTest(
        ctx: Ctx,
        root_source_file: Build.LazyPath,
        bits: *Build.Module,
        objs: []const *Build.Step.Compile,
    ) *Build.Step.Compile {
        const m = ctx.b.createModule(.{
            .root_source_file = root_source_file,
            .target = ctx.target,
            .optimize = ctx.optimize,
            .link_libc = true,
        });

        m.addIncludePath(ctx.include_path);
        m.addImport("bits", bits);

        for (objs) |o| {
            m.addObject(o);
        }

        return ctx.b.addTest(.{ .root_module = m });
    }
};

pub fn build(b: *Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});
    const include_path = b.path("include");

    const ctx = Ctx{
        .b = b,
        .target = target,
        .optimize = optimize,
        .include_path = include_path,
    };

    const translate_c = ctx.b.dependency("translate_c", .{});

    const t: Translator = .init(translate_c, .{
        .c_source_file = ctx.b.path("include/bits.h"),
        .target = ctx.target,
        .optimize = ctx.optimize,
    });

    const bits_lib_obj = ctx.cObj("bits", &.{
        b.path("src/libbits/arena.c"),
        b.path("src/libbits/channel.c"),
        b.path("src/libbits/fnv.c"),
        b.path("src/libbits/hashtable.c"),
    });

    const arena_test_exe = ctx.cExe(
        "arena_test",
        &.{b.path("src/cmd/arena_test.c")},
        &.{bits_lib_obj},
    );

    const base64_exe = blk: {
        const exe = ctx.cExe(
            "base64",
            &.{b.path("src/cmd/base64.c")},
            &.{bits_lib_obj},
        );
        exe.root_module.linkSystemLibrary("ssl", .{});
        exe.root_module.linkSystemLibrary("crypto", .{});
        break :blk exe;
    };

    const demo_oop_exe = ctx.cExe(
        "demo_oop",
        &.{b.path("src/cmd/demo_oop.c")},
        &.{},
    );

    const fnv_test_exe = ctx.cExe(
        "fnv_test",
        &.{b.path("src/cmd/fnv_test.c")},
        &.{bits_lib_obj},
    );

    const hashtable_test_exe = ctx.cExe(
        "hashtable_test",
        &.{b.path("src/cmd/hashtable_test.c")},
        &.{bits_lib_obj},
    );

    const hashtable_compact_test_exe = ctx.cExe(
        "hashtable_compact_test",
        &.{b.path("src/cmd/hashtable_compact_test.c")},
        &.{bits_lib_obj},
    );

    const hashtable_test_zig = ctx.zigTest(
        b.path("src/cmd/hashtable_test.zig"),
        t.mod,
        &.{bits_lib_obj},
    );

    const lambda_exe = ctx.cExe(
        "lambda",
        &.{b.path("src/cmd/lambda.c")},
        &.{bits_lib_obj},
    );

    const mkavatar_exe = blk: {
        const exe = ctx.cExe(
            "mkavatar",
            &.{b.path("src/cmd/mkavatar.c")},
            &.{},
        );
        exe.root_module.linkSystemLibrary("png", .{});
        break :blk exe;
    };

    const channel_basic_test_exe = ctx.cExe(
        "channel_basic_test",
        &.{b.path("src/cmd/channel_basic.c")},
        &.{bits_lib_obj},
    );

    const channel_block_test_exe = ctx.cExe(
        "channel_block_test",
        &.{
            b.path("src/cmd/channel_block.c"),
            b.path("src/cmd/channel_expected.c"),
        },
        &.{bits_lib_obj},
    );

    const executables = [_]struct { exe: *Build.Step.Compile, run: bool }{
        .{ .exe = arena_test_exe, .run = true },
        .{ .exe = base64_exe, .run = true },
        .{ .exe = demo_oop_exe, .run = false },
        .{ .exe = fnv_test_exe, .run = true },
        .{ .exe = hashtable_test_exe, .run = true },
        .{ .exe = hashtable_compact_test_exe, .run = true },
        .{ .exe = hashtable_test_zig, .run = true },
        .{ .exe = lambda_exe, .run = true },
        .{ .exe = mkavatar_exe, .run = false },
        .{ .exe = channel_basic_test_exe, .run = true },
        .{ .exe = channel_block_test_exe, .run = true },
    };

    const test_step = b.step("test", "Run tests");

    for (executables) |item| {
        b.installArtifact(item.exe);
        if (item.run) {
            const run = b.addRunArtifact(item.exe);
            test_step.dependOn(&run.step);
        }
    }
}
