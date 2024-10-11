#include <cstdint>
#include <cstdio>
#include <cstdlib>

#include "defer.h"

namespace foo {

enum class message_kind : uint8_t {
  HELLO = 0,
  GOODBYE = 1,
};

constexpr message_kind get_message_kind() {
#ifndef SAY_GOODBYE
  return message_kind::HELLO;
#else
  return message_kind::GOODBYE;
#endif
}

inline char *make_hello(const char *name) {
  size_t len = (size_t)snprintf(NULL, 0, "Hello, %s", name);
  char *ret = (char *)std::calloc(++len, sizeof(*ret));
  (void)snprintf(ret, len, "Hello, %s", name);
  return ret;
}

inline char *make_goodbye(const char *name) {
  size_t len = (size_t)snprintf(NULL, 0, "Goodbye, %s", name);
  char *ret = (char *)std::calloc(++len, sizeof(*ret));
  (void)snprintf(ret, len, "Goodbye, %s", name);
  return ret;
}

template <message_kind kind = get_message_kind()>
char *message(const char *name) {
  if constexpr (kind == message_kind::HELLO) {
    return make_hello(name);
  } else {
    return make_goodbye(name);
  }
}

} // namespace foo

int main() {
  char *msg = foo::message("world!");
  defer({
    std::free(msg);
    printf("msg freed\n");
  });

  printf("%s\n", msg);

  return EXIT_SUCCESS;
}
