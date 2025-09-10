#include <iostream>

#include "lambda.h"

auto
main(void) -> int
{
	auto show = expr_show(std::cout);

	// (\x . x)
	auto id = lam("x", var("x"));

	show(id);
	std::cout << std::endl;

	// (\t . (\f . t))
	auto k = lam("t", lam("f", var("t")));

	show(k);
	std::cout << std::endl;

	// ((\x . x) y)
	auto app1 = app(std::move(id), var("y"));

	show(app1);
	std::cout << std::endl;

	return 0;
}
