const std = @import("std");

const Build = std.Build;

const Params = struct {
    name: []const u8,
    file: Build.LazyPath,
    target: Build.ResolvedTarget,
    optimize: std.builtin.OptimizeMode,
    includePath: Build.LazyPath,
    flags: []const []const u8 = &.{ "-std=gnu11", "-Wall", "-Wextra", "-Wconversion", "-Wsign-conversion" },
};

fn createCObj(
    b: *Build,
    ps: Params,
) *Build.Step.Compile {
    const root_module = b.createModule(.{ .target = ps.target, .optimize = ps.optimize, .link_libc = true });
    root_module.addCSourceFile(.{ .file = ps.file, .flags = ps.flags });
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
    const root_module = b.createModule(.{ .target = ps.target, .optimize = ps.optimize, .link_libc = true });
    root_module.addCSourceFile(.{ .file = ps.file, .flags = ps.flags });
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

    const fnvObj = createCObj(b, .{
        .name = "fnv",
        .file = b.path("lib/fnv.c"),
        .target = target,
        .optimize = optimize,
        .includePath = includePath,
    });

    const fnvTestExe = createCExecutable(b, .{
        .name = "fnv_test",
        .file = b.path("test/fnv_test.c"),
        .target = target,
        .optimize = optimize,
        .includePath = includePath,
    }, &.{fnvObj});

    b.installArtifact(fnvTestExe);

    const hashtableObj = createCObj(b, .{
        .name = "hashtable",
        .file = b.path("lib/hashtable.c"),
        .target = target,
        .optimize = optimize,
        .includePath = includePath,
    });

    const hashtableTestExe = createCExecutable(b, .{
        .name = "hashtable_test",
        .file = b.path("test/hashtable_test.c"),
        .target = target,
        .optimize = optimize,
        .includePath = includePath,
    }, &.{ fnvObj, hashtableObj });

    b.installArtifact(hashtableTestExe);

    const messageQueueObj = createCObj(b, .{
        .name = "message_queue",
        .file = b.path("lib/message_queue.c"),
        .target = target,
        .optimize = optimize,
        .includePath = includePath,
    });

    const messageQueueBasicExe = createCExecutable(b, .{
        .name = "message_queue_basic",
        .file = b.path("test/message_queue_basic.c"),
        .target = target,
        .optimize = optimize,
        .includePath = includePath,
    }, &.{messageQueueObj});

    b.installArtifact(messageQueueBasicExe);

    const messageQueueExpectedObj = createCObj(b, .{
        .name = "message_queue_expected",
        .file = b.path("test/message_queue_expected.c"),
        .target = target,
        .optimize = optimize,
        .includePath = includePath,
    });

    const messageQueueBlockExe = createCExecutable(b, .{
        .name = "message_queue_block",
        .file = b.path("test/message_queue_block.c"),
        .target = target,
        .optimize = optimize,
        .includePath = includePath,
    }, &.{ messageQueueObj, messageQueueExpectedObj });

    b.installArtifact(messageQueueBlockExe);

    const hashtableTestRoot = b.createModule(.{
        .root_source_file = b.path("test/hashtable_test.zig"),
        .target = target,
        .optimize = optimize,
    });
    hashtableTestRoot.addIncludePath(includePath);
    hashtableTestRoot.addObject(fnvObj);
    hashtableTestRoot.addObject(hashtableObj);

    const hashtableTests = b.addTest(.{
        .root_module = hashtableTestRoot,
    });

    b.installArtifact(hashtableTests);

    const runHashtableTests = b.addRunArtifact(hashtableTests);

    const testStep = b.step("test", "Run library tests");
    testStep.dependOn(&runHashtableTests.step);
}
