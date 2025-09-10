#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>

#include "defer.hpp"

namespace foo {

enum class message_kind : uint8_t {
	HELLO = 0,
	GOODBYE = 1,
};

#ifndef SAY_GOODBYE
constexpr message_kind
get_message_kind()
{
	return message_kind::HELLO;
}
#else
constexpr message_kind
get_message_kind()
{
	return message_kind::GOODBYE;
}
#endif

inline char *
make_hello(char const *name)
{
	size_t len = (size_t)std::snprintf(nullptr, 0, "Hello, %s", name);
	char *ret = (char *)std::calloc(++len, sizeof(*ret));
	if (ret == nullptr) {
		return nullptr;
	}
	(void)std::snprintf(ret, len, "Hello, %s", name);
	return ret;
}

inline char *
make_goodbye(char const *name)
{
	size_t len = (size_t)std::snprintf(nullptr, 0, "Goodbye, %s", name);
	char *ret = (char *)std::calloc(++len, sizeof(*ret));
	if (ret == nullptr) {
		return nullptr;
	}
	(void)std::snprintf(ret, len, "Goodbye, %s", name);
	return ret;
}

template <message_kind kind = get_message_kind()>
char *
message(char const *name)
{
	if constexpr (kind == message_kind::HELLO) {
		return make_hello(name);
	} else {
		return make_goodbye(name);
	}
}

} // namespace foo

int
main()
{
	char *msg = foo::message("world!");
	defer({
		std::free(msg);
		std::printf("msg freed\n");
	});

	std::printf("%s\n", msg);

	return EXIT_SUCCESS;
}
