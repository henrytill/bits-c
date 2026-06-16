const c = @cImport(@cInclude("bits.h"));
const std = @import("std");
const testing = std.testing;

fn roundtrip(t: *c.Table, key: [*c]const u8, value: anytype) !@TypeOf(value) {
    comptime std.debug.assert(@typeInfo(@TypeOf(value)) == .pointer);
    const rc = c.tableput(t, key, @as(*anyopaque, @constCast(value)));
    try testing.expectEqual(rc, 0);
    const retPtr = c.tableget(t, key) orelse return error.Failure;
    return @ptrCast(@alignCast(retPtr));
}

test "roundtrip i32" {
    const t = c.tablecreate(8) orelse return error.Failure;
    defer c.tabledestroy(t, null);

    const key = "key";
    const expected: i32 = 42;
    const actual = try roundtrip(t, key, &expected);

    try testing.expectEqual(&expected, actual);
    try testing.expectEqual(expected, actual.*);
}

test "roundtrip string" {
    const t = c.tablecreate(8) orelse return error.Failure;
    defer c.tabledestroy(t, null);

    const key = "key";
    const expected = "value";
    const actual = try roundtrip(t, key, expected);

    try testing.expectEqual(expected.ptr, actual);
    try testing.expectEqualStrings(expected, actual);
    try testing.expectEqual(expected[5], 0);
    try testing.expectEqual(actual[5], 0);
}
