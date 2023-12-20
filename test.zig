const c = @cImport(@cInclude("hashtable.h"));
const std = @import("std");
const testing = std.testing;

test "roundtrip i32" {
    const t = c.table_create(8) orelse return error.Failure;
    defer c.table_destroy(t);

    const key = "key";
    const expected: i32 = 42;
    const expected_ptr = @as(*anyopaque, @constCast(&expected));

    const rc = c.table_put(t, key, expected_ptr);
    try testing.expectEqual(rc, 0);

    const val_ptr: *anyopaque = c.table_get(t, key) orelse return error.Failure;
    const actual_ptr: *i32 = @ptrCast(@alignCast(val_ptr));
    const actual: i32 = actual_ptr.*;

    try testing.expectEqual(expected, actual);
}

test "roundtrip string" {
    const t = c.table_create(8) orelse return error.Failure;
    defer c.table_destroy(t);

    const key = "key";
    const expected = "value";
    const expected_ptr = @as(*anyopaque, @constCast(expected.ptr));

    const rc = c.table_put(t, key, expected_ptr);
    try testing.expectEqual(rc, 0);

    const val_ptr: *anyopaque = c.table_get(t, key) orelse return error.Failure;
    const actual: @TypeOf(expected) = @ptrCast(@alignCast(val_ptr));

    try testing.expectEqual(expected, actual);
}
