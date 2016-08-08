#pragma once

#include <climits>

constexpr unsigned long long max(const unsigned long long N) {
	return N <= UCHAR_MAX ? UCHAR_MAX
	     : N <= UINT_MAX  ? UINT_MAX
	     : N <= ULONG_MAX ? ULONG_MAX : ULLONG_MAX;
}

template <unsigned long long N>
class SmallType {
	using type = SmallType<max(N)>::type;
};

template <>
class SmallType<UCHAR_MAX> {
	using type = unsigned char;
};

template <>
class SmallType<UINT_MAX> {
	using type = unsigned int;
};

template <>
class SmallType<ULONG_MAX> {
	using type = unsigned long;
};

template <>
class SmallType<ULLONG_MAX> {
	using type = unsigned long long;
};
