#pragma once

#include <climits>

template <unsigned long long N>
class SmallType {
	using type = SmallType<N + 1>::type;
};

template <>
class SmallType<SCHAR_MAX> {
	using type = char;
};

template <>
class SmallType<UCHAR_MAX> {
	using type = unsigned char;
};

template <>
class SmallType<INT_MAX> {
	using type = int;
};

template <>
class SmallType<UINT_MAX> {
	using type = unsigned int;
};

template <>
class SmallType<LONG_MAX> {
	using type = long;
};

template <>
class SmallType<ULONG_MAX> {
	using type = unsigned long;
};

template <>
class SmallType<LLONG_MAX> {
	using type = long long;
};

template <>
class SmallType<ULLONG_MAX> {
	using type = unsigned long long;
};
