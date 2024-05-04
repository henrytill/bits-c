const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const flags = &.{ "-std=gnu11", "-Wall", "-Wextra", "-Wconversion", "-Wsign-conversion" };

    const fnvLib = b.addStaticLibrary(.{
        .name = "fnv",
        .target = target,
        .optimize = optimize,
    });

    fnvLib.addCSourceFile(.{ .file = .{ .path = "fnv.c" }, .flags = flags });
    fnvLib.linkLibC();
    fnvLib.addIncludePath(.{ .path = "." });
    b.installArtifact(fnvLib);

    const fnvTestExe = b.addExecutable(.{
        .name = "fnv_test",
        .target = target,
        .optimize = optimize,
    });

    fnvTestExe.addCSourceFile(.{ .file = .{ .path = "fnv_test.c" }, .flags = flags });
    fnvTestExe.linkLibC();
    fnvTestExe.addIncludePath(.{ .path = "." });
    fnvTestExe.linkLibrary(fnvLib);
    b.installArtifact(fnvTestExe);

    const hashtableLib = b.addStaticLibrary(.{
        .name = "hashtable",
        .target = target,
        .optimize = optimize,
    });

    hashtableLib.addCSourceFile(.{ .file = .{ .path = "hashtable.c" }, .flags = flags });
    hashtableLib.linkLibC();
    b.installArtifact(hashtableLib);

    const hashtableTestExe = b.addExecutable(.{
        .name = "hashtable_test",
        .target = target,
        .optimize = optimize,
    });

    hashtableTestExe.addCSourceFile(.{ .file = .{ .path = "hashtable_test.c" }, .flags = flags });
    hashtableTestExe.linkLibC();
    hashtableTestExe.addIncludePath(.{ .path = "." });
    hashtableTestExe.linkLibrary(fnvLib);
    hashtableTestExe.linkLibrary(hashtableLib);
    b.installArtifact(hashtableTestExe);

    const hashtableTests = b.addTest(.{
        .root_source_file = .{ .path = "test.zig" },
        .target = target,
        .optimize = optimize,
    });

    hashtableTests.addIncludePath(.{ .path = "." });
    hashtableTests.linkLibrary(fnvLib);
    hashtableTests.linkLibrary(hashtableLib);
    b.installArtifact(hashtableTests);

    const run_hashtableTests = b.addRunArtifact(hashtableTests);

    const test_step = b.step("test", "Run library tests");
    test_step.dependOn(&run_hashtableTests.step);
}
