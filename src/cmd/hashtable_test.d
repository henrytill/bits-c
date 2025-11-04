import core.stdc.string;

extern (C)
{
    struct Table;

    Table* tablecreate(size_t columns_len);
    void tabledestroy(Table* t, void function(void*) finalize);
    int tableput(Table* t, const char* key, void* value);
    void* tableget(Table* t, const char* key);
}

T roundtrip(T)(Table* t, const char* key, T value)
{
    int rc = t.tableput(key, cast(void*) value);
    assert(rc == 0, "tableput failed");

    void* retPtr = t.tableget(key);
    assert(retPtr !is null, "tableget returned null");

    return cast(T) retPtr;
}

unittest
{
    Table* t = tablecreate(8);
    assert(t !is null, "tablecreate failed");
    scope (exit)
        t.tabledestroy(null);

    const char* key = "key";
    int expected = 42;
    int* actual = t.roundtrip(key, &expected);

    assert(actual == &expected, "pointer mismatch");
    assert(*actual == expected, "value mismatch");
}

unittest
{
    Table* t = tablecreate(8);
    assert(t !is null, "tablecreate failed");
    scope (exit)
        t.tabledestroy(null);

    const char* key = "key";
    const char* expected = "value";
    const char* actual = t.roundtrip(key, cast(char*) expected);

    assert(actual == expected, "pointer mismatch");
    assert(actual.strcmp(expected) == 0, "string content mismatch");
    assert(expected[5] == 0, "expected string not null-terminated");
    assert(actual[5] == 0, "actual string not null-terminated");
}

void main()
{
}
