const std = @import("std");

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
    includePath: Build.LazyPath,

    fn module(ctx: Ctx, files: []const Build.LazyPath) *Build.Module {
        const m = ctx.b.createModule(.{
            .target = ctx.target,
            .optimize = ctx.optimize,
            .link_libc = true,
        });

        m.addIncludePath(ctx.includePath);

        for (files) |file| {
            m.addCSourceFile(.{ .file = file, .flags = flags });
        }

        return m;
    }

    fn cObj(ctx: Ctx, name: []const u8, files: []const Build.LazyPath) *Build.Step.Compile {
        return ctx.b.addObject(.{
            .name = name,
            .root_module = ctx.module(files),
        });
    }

    fn cExe(
        ctx: Ctx,
        name: []const u8,
        files: []const Build.LazyPath,
        objs: []const *Build.Step.Compile,
    ) *Build.Step.Compile {
        const m = ctx.module(files);

        for (objs) |o| {
            m.addObject(o);
        }

        return ctx.b.addExecutable(.{
            .name = name,
            .root_module = m,
        });
    }
};

pub fn build(b: *Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});
    const includePath = b.path("include");

    const ctx = Ctx{
        .b = b,
        .target = target,
        .optimize = optimize,
        .includePath = includePath,
    };

    const bitsLibObj = ctx.cObj("bits", &.{
        b.path("src/libbits/arena.c"),
        b.path("src/libbits/channel.c"),
        b.path("src/libbits/fnv.c"),
        b.path("src/libbits/hashtable.c"),
    });

    const arenaTestExe = ctx.cExe("arena_test", &.{b.path("src/cmd/arena_test.c")}, &.{bitsLibObj});

    const base64Exe = blk: {
        const exe = ctx.cExe("base64", &.{b.path("src/cmd/base64.c")}, &.{bitsLibObj});
        exe.root_module.linkSystemLibrary("ssl", .{});
        exe.root_module.linkSystemLibrary("crypto", .{});
        break :blk exe;
    };

    const demoOopExe = ctx.cExe("demo_oop", &.{b.path("src/cmd/demo_oop.c")}, &.{});

    const fnvTestExe = ctx.cExe("fnv_test", &.{b.path("src/cmd/fnv_test.c")}, &.{bitsLibObj});

    const hashtableTestExe = ctx.cExe("hashtable_test", &.{b.path("src/cmd/hashtable_test.c")}, &.{bitsLibObj});

    const hashtableCompactTestExe = ctx.cExe("hashtable_compact_test", &.{b.path("src/cmd/hashtable_compact_test.c")}, &.{bitsLibObj});

    const hashtableZigTests = blk: {
        const root = b.createModule(.{
            .root_source_file = b.path("src/cmd/hashtable_test.zig"),
            .target = target,
            .optimize = optimize,
        });
        root.addIncludePath(includePath);
        root.addObject(bitsLibObj);
        const exe = b.addTest(.{ .root_module = root });
        break :blk exe;
    };

    const lambdaExe = ctx.cExe("lambda", &.{b.path("src/cmd/lambda.c")}, &.{bitsLibObj});

    const messageQueueBasicTestExe = ctx.cExe("channel_basic_test", &.{b.path("src/cmd/channel_basic.c")}, &.{bitsLibObj});

    const messageQueueBlockTestExe = ctx.cExe("channel_block_test", &.{
        b.path("src/cmd/channel_block.c"),
        b.path("src/cmd/channel_expected.c"),
    }, &.{bitsLibObj});

    const executables = [_]struct { exe: *Build.Step.Compile, run: bool }{
        .{ .exe = arenaTestExe, .run = true },
        .{ .exe = base64Exe, .run = true },
        .{ .exe = demoOopExe, .run = false },
        .{ .exe = fnvTestExe, .run = true },
        .{ .exe = hashtableTestExe, .run = true },
        .{ .exe = hashtableCompactTestExe, .run = true },
        .{ .exe = hashtableZigTests, .run = true },
        .{ .exe = lambdaExe, .run = true },
        .{ .exe = messageQueueBasicTestExe, .run = true },
        .{ .exe = messageQueueBlockTestExe, .run = true },
    };

    const testStep = b.step("test", "Run tests");

    for (executables) |item| {
        b.installArtifact(item.exe);
        if (item.run) {
            const run = b.addRunArtifact(item.exe);
            testStep.dependOn(&run.step);
        }
    }
}
