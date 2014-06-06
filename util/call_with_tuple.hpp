#pragma once
/**
 * call_with_tuple.hpp
 */
#include "ppack.hpp"
#include <tuple>

namespace glua {
namespace detail{

template<
    typename Func, typename... Args, int... N, 
    // Enable this version only if Func is not a pointer to a member function,
    // since they require a special syntax to call
    typename std::enable_if<!std::is_member_function_pointer<Func>::value>::type* = nullptr
>
static auto _call_with_tuple(Func f, std::tuple<Args...> args, _index_list<N...>) 
#if __cplusplus <= 201103L
-> decltype(f(std::get<N>(args)...)) 
#endif
{
    return f(std::get<N>(args)...);
};

template<
    typename MFunc, typename... Args, int... N, 
    // Enable only if Func IS a pointer to a member function.
    typename std::enable_if<std::is_member_function_pointer<MFunc>::value>::type* = nullptr
>
static auto _call_with_tuple(MFunc mf, std::tuple<Args...> args, _index_list<N...>)
#if __cplusplus <= 201103L
-> decltype(_call_mfunc_with_tuple(mf, args, _index_list<N...>()))
#endif
{
    return _call_mfunc_with_tuple(mf, args, _index_list<N...>());
};

template<typename MFunc, typename... Args, int... N>
static auto _call_mfunc_with_tuple(MFunc mf, std::tuple<Args...> args, _index_list<N...>)
#if __cplusplus <= 201103L
-> decltype(_call_mfunc_with_tuple(mf, std::get<0>(args), args, typename _index_list_drop_first<N...>::build()))
#endif
{
    return _call_mfunc_with_tuple(mf, std::get<0>(args), args, typename _index_list_drop_first<N...>::build());
}

template<
    typename MFunc, typename Class, typename... Args, int... N,
    typename std::enable_if<std::is_pointer<Class>::value>::type* = nullptr
>
static auto _call_mfunc_with_tuple(MFunc mf, Class obj, std::tuple<Args...> args, _index_list<N...>) 
#if __cplusplus <= 201103L
-> decltype((obj->*mf)(std::get<N>(args)...))
#endif
{
    return (obj->*mf)(std::get<N>(args)...);
}

template<
    typename MFunc, typename Class, typename... Args, int... N,
    typename std::enable_if<!std::is_pointer<Class>::value>::type* = nullptr
>
static auto _call_mfunc_with_tuple(MFunc mf, Class obj, std::tuple<Args...> args, _index_list<N...>) 
#if __cplusplus <= 201103L
-> decltype((obj.*mf)(std::get<N>(args)...))
#endif
{
    return (obj.*mf)(std::get<N>(args)...);
}

} // namespace detail

template<typename Func, typename... Args>
static auto call_with_tuple(Func f, std::tuple<Args...> args)
#if __cplusplus <= 201103L
-> decltype(detail::_call_with_tuple(f, args, typename detail::_build_index_list<sizeof...(Args)>::build()))
#endif
{
    return detail::_call_with_tuple(f, args, typename detail::_build_index_list<sizeof...(Args)>::build());
}

template<typename MFunc, typename Class, typename... Args>
static auto call_mfunc_with_tuple(MFunc mf, Class obj, std::tuple<Args...> args)
#if __cplusplus <= 201103L
-> decltype(detail::_call_mfunc_with_tuple(mf, obj, args, typename detail::_build_index_list<sizeof...(Args)>::build()))
#endif
{
    return detail::_call_mfunc_with_tuple(mf, obj, args, typename detail::_build_index_list<sizeof...(Args)>::build());
}

} // namespace glua
