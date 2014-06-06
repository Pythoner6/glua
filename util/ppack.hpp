#pragma once
/**
 * ppack.hpp
 * Contains helper structs for dealing with parameter packs.
 */

namespace glua {
namespace detail {

/**
 * Sturct _first
 * template struct used to get the first type
 * in a parameter pack (or other list) of types
 *
 * typical usage is:
 * typename _first<Types...>::type
 */

template<typename... T>
struct _first {};

template<typename T1, typename... T>
struct _first<T1, T...> {
    using type = T1;
};

template<typename T>
struct _first<T> {
    using type = T;
};

/**
 * Struct _index_list
 * template struct used to pass around a list
 * of index_list at compile time. Useful for 
 * unpacking in a function like this:
 *
 * template<typename... Types, int... N>
 * void unpack(std::tuple<Types...> tuple, _index_list<N...>) {
 *     std::get<N>(tuple)... // do something with unpacked values here
 * }
 */
template<int... N>
struct _index_list {};

/**
 * Struct _build_index_list
 * helper template to obtain an _index_list class 
 * more easily.
 *
 * typename _build_index_list<N>::get 
 * is equivalent to
 * _index_list<0, 1, 2, . . . N-1>
 */
template<int... N>
struct _build_index_list {};

template<int N>
struct _build_index_list<N> : 
public _build_index_list<N-2, N-1> {};

template<int N1, int... N>
struct _build_index_list<N1, N...> : 
public _build_index_list<N1-1, N1, N...> {};

template<int... N>
struct _build_index_list<0, N...> {
    using build = _index_list<0, N...>;
};

template<>
struct _build_index_list<1> {
    using build = _index_list<0>;
};

template<>
struct _build_index_list<0> {
    using build = _index_list<>;
};

/**
 * Struct _index_list_drop_first
 * Helper template to get an _index_list
 * type containing all but the first N.
 *
 * typename _index_list_drop_first<0, 1, 2, . . . N>::get 
 * is equivalent to
 * _index_list<1, 2, . . . N>
 */
template<int N1, int... N>
struct _index_list_drop_first {
    using build = _index_list<N...>;
};

} // namespace detail
} // namespace glua
