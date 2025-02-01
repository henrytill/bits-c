#include <algorithm>
#include <iostream>
#include <memory>

#include "lambda.h"

auto main(void) -> int {
  auto show = ExprShow(std::cout);

  // (\x . x)
  auto xx = Abs{"x", Var{"x"}};

  show(xx);
  std::cout << std::endl;

  auto ft = std::make_unique<Abs>("f", Var{"t"});

  // (\t . (\f . t))
  auto tft = Abs{"t", std::move(ft)};

  show(tft);
  std::cout << std::endl;

  return 0;
}
