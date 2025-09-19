const std = @import("std");

const Build = std.Build;

const Params = struct {
    name: []const u8,
    files: []const Build.LazyPath,
    target: Build.ResolvedTarget,
    optimize: std.builtin.OptimizeMode,
    includePath: Build.LazyPath,
    flags: []const []const u8 = &.{
        "-std=c89",
        "-Wall",
        "-Wextra",
        "-Wpedantic",
        "-Wconversion",
        "-Wsign-conversion",
        "-D_DEFAULT_SOURCE",
    },
};

fn createCObj(
    b: *Build,
    ps: Params,
) *Build.Step.Compile {
    const root_module = b.createModule(.{
        .target = ps.target,
        .optimize = ps.optimize,
        .link_libc = true,
    });
    for (ps.files) |file| {
        root_module.addCSourceFile(.{ .file = file, .flags = ps.flags });
    }
    root_module.addIncludePath(ps.includePath);

    const ret = b.addObject(.{
        .name = ps.name,
        .root_module = root_module,
    });

    return ret;
}

fn createCExecutable(
    b: *Build,
    ps: Params,
    os: []const *Build.Step.Compile,
) *Build.Step.Compile {
    const root_module = b.createModule(.{
        .target = ps.target,
        .optimize = ps.optimize,
        .link_libc = true,
    });
    for (ps.files) |file| {
        root_module.addCSourceFile(.{ .file = file, .flags = ps.flags });
    }
    root_module.addIncludePath(ps.includePath);

    for (os) |o| {
        root_module.addObject(o);
    }

    const ret = b.addExecutable(.{
        .name = ps.name,
        .root_module = root_module,
    });

    return ret;
}

pub fn build(b: *Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});
    const includePath = b.path("include");

    const bitsLibObj = createCObj(b, .{
        .name = "bits",
        .files = &.{
            b.path("src/libbits/arena.c"),
            b.path("src/libbits/channel.c"),
            b.path("src/libbits/fnv.c"),
            b.path("src/libbits/hashtable.c"),
        },
        .target = target,
        .optimize = optimize,
        .includePath = includePath,
    });

    const arenaTestExe = createCExecutable(b, .{
        .name = "arena_test",
        .files = &.{b.path("src/cmd/arena_test.c")},
        .target = target,
        .optimize = optimize,
        .includePath = includePath,
    }, &.{bitsLibObj});

    const base64Exe = blk: {
        const exe = createCExecutable(b, .{
            .name = "base64",
            .files = &.{b.path("src/cmd/base64.c")},
            .target = target,
            .optimize = optimize,
            .includePath = includePath,
        }, &.{bitsLibObj});
        exe.linkSystemLibrary("ssl");
        exe.linkSystemLibrary("crypto");

        break :blk exe;
    };

    const demoOopExe = createCExecutable(b, .{
        .name = "demo_oop",
        .files = &.{b.path("src/cmd/demo_oop.c")},
        .target = target,
        .optimize = optimize,
        .includePath = includePath,
    }, &.{});

    const fnvTestExe = createCExecutable(b, .{
        .name = "fnv_test",
        .files = &.{b.path("src/cmd/fnv_test.c")},
        .target = target,
        .optimize = optimize,
        .includePath = includePath,
    }, &.{bitsLibObj});

    const hashtableTestExe = createCExecutable(b, .{
        .name = "hashtable_test",
        .files = &.{b.path("src/cmd/hashtable_test.c")},
        .target = target,
        .optimize = optimize,
        .includePath = includePath,
    }, &.{bitsLibObj});

    const hashtableCompactTestExe = createCExecutable(b, .{
        .name = "hashtable_compact_test",
        .files = &.{b.path("src/cmd/hashtable_compact_test.c")},
        .target = target,
        .optimize = optimize,
        .includePath = includePath,
    }, &.{bitsLibObj});

    const hashtableZigTests = blk: {
        const root = b.createModule(.{
            .root_source_file = b.path("src/cmd/hashtable_test.zig"),
            .target = target,
            .optimize = optimize,
        });
        root.addIncludePath(includePath);
        root.addObject(bitsLibObj);

        break :blk b.addTest(.{
            .root_module = root,
        });
    };

    const messageQueueBasicTestExe = createCExecutable(b, .{
        .name = "channel_basic_test",
        .files = &.{b.path("src/cmd/channel_basic.c")},
        .target = target,
        .optimize = optimize,
        .includePath = includePath,
    }, &.{bitsLibObj});

    const messageQueueBlockTestExe = createCExecutable(b, .{
        .name = "channel_block_test",
        .files = &.{
            b.path("src/cmd/channel_block.c"),
            b.path("src/cmd/channel_expected.c"),
        },
        .target = target,
        .optimize = optimize,
        .includePath = includePath,
    }, &.{bitsLibObj});

    const executables = [_]struct { exe: *Build.Step.Compile, run: bool }{
        .{ .exe = arenaTestExe, .run = true },
        .{ .exe = base64Exe, .run = true },
        .{ .exe = demoOopExe, .run = false },
        .{ .exe = fnvTestExe, .run = true },
        .{ .exe = hashtableTestExe, .run = true },
        .{ .exe = hashtableCompactTestExe, .run = true },
        .{ .exe = hashtableZigTests, .run = true },
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
