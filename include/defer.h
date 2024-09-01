// -*- mode: c++; -*-

#pragma once

template <typename F>
struct deferred {
  F f;
  deferred(F f) : f(f) {}
  ~deferred() { f(); }
};

template <typename F>
inline deferred<F> make_deferred(F f) {
  return deferred<F>(f);
}

#define DO_JOINSTRING2(x, y) x##y
#define JOINSTRING2(x, y)    DO_JOINSTRING2(x, y)
#define defer(stmt)          auto JOINSTRING2(defer_, __LINE__) = make_deferred([&]() { stmt; })
