const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const liballoc = b.addStaticLibrary(.{
        .name = "alloc",
        .root_source_file = .{ .path = "alloc.c" },
        .target = target,
        .optimize = optimize,
    });

    liballoc.linkLibC();
    b.installArtifact(liballoc);

    const libfnv = b.addStaticLibrary(.{
        .name = "fnv",
        .root_source_file = .{ .path = "fnv.c" },
        .target = target,
        .optimize = optimize,
    });

    libfnv.linkLibC();
    b.installArtifact(libfnv);

    const fnv_test_exe = b.addExecutable(.{
        .name = "fnv_test",
        .root_source_file = .{ .path = "fnv_test.c" },
        .target = target,
        .optimize = optimize,
    });

    fnv_test_exe.linkLibC();
    fnv_test_exe.addIncludePath(.{ .path = "." });
    fnv_test_exe.linkLibrary(libfnv);
    b.installArtifact(fnv_test_exe);

    const libhashtable = b.addStaticLibrary(.{
        .name = "hashtable",
        .root_source_file = .{ .path = "hashtable.c" },
        .target = target,
        .optimize = optimize,
    });

    libhashtable.linkLibC();
    b.installArtifact(libhashtable);

    const hashtable_test_exe = b.addExecutable(.{
        .name = "hashtable_test",
        .root_source_file = .{ .path = "hashtable_test.c" },
        .target = target,
        .optimize = optimize,
    });

    hashtable_test_exe.linkLibC();
    hashtable_test_exe.addIncludePath(.{ .path = "." });
    hashtable_test_exe.linkLibrary(liballoc);
    hashtable_test_exe.linkLibrary(libfnv);
    hashtable_test_exe.linkLibrary(libhashtable);
    b.installArtifact(hashtable_test_exe);

    const hashtable_tests = b.addTest(.{
        .root_source_file = .{ .path = "test.zig" },
        .target = target,
        .optimize = optimize,
    });

    hashtable_tests.addIncludePath(.{ .path = "." });
    hashtable_tests.linkLibrary(liballoc);
    hashtable_tests.linkLibrary(libfnv);
    hashtable_tests.linkLibrary(libhashtable);
    b.installArtifact(hashtable_tests);

    const run_hashtable_tests = b.addRunArtifact(hashtable_tests);

    const test_step = b.step("test", "Run library tests");
    test_step.dependOn(&run_hashtable_tests.step);
}
