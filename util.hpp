#pragma once
#define COMMON_HEADER_ONLY

#ifdef COMMON_HEADER_ONLY
#include <ostream>
#include <functional>
#include <tuple>
#include <variant>

class UTIL {
public:
};

#define defer1(a, b) a##b
#define defer2(a, b) defer1(a, b)
#define defer(expr) Defer defer2(__Defer, __COUNTER__)([&]() { expr; })

class Defer {
public:
    Defer(std::function<void()> fn) : fn(fn) {}
    ~Defer() noexcept {
        if (fn) {
            fn();
        }
    }

private:
    std::function<void()> fn;
};

//
template <unsigned int n, typename... T>
std::variant<T...> _tuple_index(std::tuple<T...> t, unsigned int index) {
    if (index >= sizeof...(T)) {
        throw std::out_of_range("tuple 下标越界");
    }
    if (index == n) {
        return std::variant<T...>{std::in_place_index<n>, get<n>(t)};
    }
    return _tuple_index < n < sizeof...(T) - 1 ? n + 1 : 0 > (t, index);
}

template <typename... T>
std::variant<T...> tuple_index(std::tuple<T...> t, unsigned int index) {
    return _tuple_index<0>(t, index);
}

template <typename T0, typename... T>
std::ostream &operator<<(std::ostream &s, std::variant<T0, T...> const &v) {
    visit([&](auto x) {
        s << x;
    },
          v);
    return s;
}

#ifdef LINUX
#endif

#endif
