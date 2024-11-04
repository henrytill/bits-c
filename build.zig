const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const flags = &.{ "-std=gnu11", "-Wall", "-Wextra", "-Wconversion", "-Wsign-conversion" };

    const includePath = b.path("include");

    const fnvObj = b.addObject(.{
        .name = "fnv",
        .target = target,
        .optimize = optimize,
    });

    fnvObj.addCSourceFile(.{ .file = b.path("lib/fnv.c"), .flags = flags });
    fnvObj.linkLibC();
    fnvObj.addIncludePath(includePath);

    const fnvTestExe = b.addExecutable(.{
        .name = "fnv_test",
        .target = target,
        .optimize = optimize,
    });

    fnvTestExe.addCSourceFile(.{ .file = b.path("test/fnv_test.c"), .flags = flags });
    fnvTestExe.addObject(fnvObj);
    fnvTestExe.linkLibC();
    fnvTestExe.addIncludePath(includePath);
    b.installArtifact(fnvTestExe);

    const hashtableObj = b.addObject(.{
        .name = "hashtable",
        .target = target,
        .optimize = optimize,
    });

    hashtableObj.addCSourceFile(.{ .file = b.path("lib/hashtable.c"), .flags = flags });
    hashtableObj.linkLibC();
    hashtableObj.addIncludePath(includePath);

    const hashtableTestExe = b.addExecutable(.{
        .name = "hashtable_test",
        .target = target,
        .optimize = optimize,
    });

    hashtableTestExe.addCSourceFile(.{ .file = b.path("test/hashtable_test.c"), .flags = flags });
    hashtableTestExe.addObject(fnvObj);
    hashtableTestExe.addObject(hashtableObj);
    hashtableTestExe.linkLibC();
    hashtableTestExe.addIncludePath(includePath);
    b.installArtifact(hashtableTestExe);

    const messageQueueObj = b.addObject(.{
        .name = "message_queue",
        .target = target,
        .optimize = optimize,
    });

    messageQueueObj.addCSourceFile(.{ .file = b.path("lib/message_queue.c"), .flags = flags });
    messageQueueObj.linkLibC();
    messageQueueObj.addIncludePath(includePath);

    const messageQueueBasicExe = b.addExecutable(.{
        .name = "message_queue_basic",
        .target = target,
        .optimize = optimize,
    });

    messageQueueBasicExe.addCSourceFile(.{ .file = b.path("test/message_queue_basic.c"), .flags = flags });
    messageQueueBasicExe.addObject(messageQueueObj);
    messageQueueBasicExe.linkLibC();
    messageQueueBasicExe.addIncludePath(includePath);
    b.installArtifact(messageQueueBasicExe);

    const messageQueueExpectedObj = b.addObject(.{
        .name = "message_queue_expected",
        .target = target,
        .optimize = optimize,
    });

    messageQueueExpectedObj.addCSourceFile(.{ .file = b.path("test/message_queue_expected.c"), .flags = flags });
    messageQueueExpectedObj.linkLibC();
    messageQueueExpectedObj.addIncludePath(includePath);

    const messageQueueBlockExe = b.addExecutable(.{
        .name = "message_queue_block",
        .target = target,
        .optimize = optimize,
    });

    messageQueueBlockExe.addCSourceFile(.{ .file = b.path("test/message_queue_block.c"), .flags = flags });
    messageQueueBlockExe.addObject(messageQueueObj);
    messageQueueBlockExe.addObject(messageQueueExpectedObj);
    messageQueueBlockExe.linkLibC();
    messageQueueBlockExe.addIncludePath(includePath);
    b.installArtifact(messageQueueBlockExe);

    const hashtableTests = b.addTest(.{
        .root_source_file = b.path("test.zig"),
        .target = target,
        .optimize = optimize,
    });

    hashtableTests.addIncludePath(includePath);
    hashtableTests.addObject(fnvObj);
    hashtableTests.addObject(hashtableObj);
    b.installArtifact(hashtableTests);

    const runHashtableTests = b.addRunArtifact(hashtableTests);

    const testStep = b.step("test", "Run library tests");
    testStep.dependOn(&runHashtableTests.step);
}
