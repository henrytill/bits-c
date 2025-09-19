#pragma once

extern "C" {
#include "bits.h"
}

template <typename F>
struct Deferred {
	F f;
	Deferred(F f)
		: f(f)
	{
	}
	~Deferred() { f(); }
};

template <typename F>
inline Deferred<F>
mkdeferred(F f)
{
	return Deferred<F>(f);
}

#define DO_JOINSTRING2(x, y) x##y
#define JOINSTRING2(x, y)    DO_JOINSTRING2(x, y)
#define defer(stmt)          auto JOINSTRING2(defer_, __LINE__) = mkdeferred([&]() { stmt; })
