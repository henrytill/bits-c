const c = @cImport(@cInclude("bits.h"));
const std = @import("std");
const testing = std.testing;

fn roundtrip(comptime T: type, t: *c.struct_table, key: [*c]const u8, value: T) !T {
    const rc = c.table_put(t, key, @as(*anyopaque, @constCast(value)));
    try testing.expectEqual(rc, 0);
    const retPtr = c.table_get(t, key) orelse return error.Failure;
    return @ptrCast(@alignCast(retPtr));
}

test "roundtrip i32" {
    const t = c.table_create(8) orelse return error.Failure;
    defer c.table_destroy(t, null);

    const key = "key";
    const expected: i32 = 42;
    const actual = try roundtrip(@TypeOf(&expected), t, key, &expected);

    try testing.expectEqual(&expected, actual);
    try testing.expectEqual(expected, actual.*);
}

test "roundtrip string" {
    const t = c.table_create(8) orelse return error.Failure;
    defer c.table_destroy(t, null);

    const key = "key";
    const expected = "value";
    const actual = try roundtrip(@TypeOf(expected), t, key, expected);

    try testing.expectEqual(expected.ptr, actual);
    try testing.expectEqualStrings(expected, actual);
    try testing.expectEqual(expected[5], 0);
    try testing.expectEqual(actual[5], 0);
}
