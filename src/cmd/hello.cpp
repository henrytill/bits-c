#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>

#include "bits.hpp"

namespace foo {

enum class Message : uint8_t {
    Hello = 0,
    Goodbye = 1,
};

#ifndef SAY_GOODBYE
constexpr Message getmessage()
{
    return Message::Hello;
}
#else
constexpr Message getmessage()
{
    return Message::Goodbye;
}
#endif

inline char *mkhello(char const *name)
{
    size_t len = (size_t)std::snprintf(nullptr, 0, "Hello, %s", name);
    char *ret = (char *)std::calloc(++len, sizeof(*ret));
    if (ret == nullptr)
        return nullptr;

    (void)std::snprintf(ret, len, "Hello, %s", name);
    return ret;
}

inline char *mkgoodbye(char const *name)
{
    size_t len = (size_t)std::snprintf(nullptr, 0, "Goodbye, %s", name);
    char *ret = (char *)std::calloc(++len, sizeof(*ret));
    if (ret == nullptr)
        return nullptr;

    (void)std::snprintf(ret, len, "Goodbye, %s", name);
    return ret;
}

template <Message kind = getmessage()>
char *message(char const *name)
{
    if constexpr (kind == Message::Hello)
        return mkhello(name);
    else
        return mkgoodbye(name);
}

} // namespace foo

int main()
{
    char *msg = foo::message("world!");
    defer({
        std::free(msg);
        std::printf("msg freed\n");
    });

    std::printf("%s\n", msg);

    return EXIT_SUCCESS;
}
