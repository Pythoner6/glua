#pragma once
#include <tuple>

namespace glua {

namespace detail {

template<typename... T>
class _tie { 
private:
    std::tuple<T&...> refs;
public:
    _tie(T&... args) : refs(args...) {}

    template<typename Sel>
    void operator=(Sel&& rhs)  {
        refs = rhs.template get<std::tuple<T...>>();
    }
};

} // namespace detail

template<typename... T>
inline detail::_tie<T...> tie(T&... t) {
    return detail::_tie<T...>(t...);
}

} // namespace glua
