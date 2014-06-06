#pragma once

#include "util.hpp"

namespace glua {

template<typename FunctionT, FunctionT func>
class cfunction {};

template<typename Ret, typename... Args, Ret(*func)(Args...)>
class cfunction<Ret(*)(Args...), func> {
public:
    using func_type      = decltype(func);
    using return_type    = typename function_traits<func_type>::return_type;
    using argument_types = typename function_traits<func_type>::argument_types;

    static inline int wrapper(lua_State* l) {
        return_type val = call_with_tuple(func, api::checkGet<argument_types>(*l));
        api::clearStack(*l);
        api::push(*l, val);
        return function_traits<func_type>::nrets;
    }
};

template<typename Ret, typename C, typename... Args, Ret(C::*func)(Args...)>
class cfunction<Ret(C::*)(Args...), func> {
public:
    using func_type      = decltype(func);
    using return_type    = typename function_traits<func_type>::return_type;
    using argument_types = typename function_traits<func_type>::argument_types;

    static inline int wrapper(lua_State* l) {
        return_type val = call_with_tuple(func, api::checkGet<argument_types>(*l));
        api::clearStack(*l);
        api::push(*l, val);
        return function_traits<func_type>::nrets;
    }
};

/*
template<typename Functor>
class cfunctor {
    using func_type      = Functor;
    using return_type    = typename function_traits<func_type>::return_type;
    using argument_types = typename function_traits<func_type>::argument_types;

    static inline int wrapper(lua_State* l) {
        func_type& func = *api::getUserdata<func_type*>(*l, api::upvalueIndex(1));
        return_type val = call_with_tuple(func, api::checkGet<argument_types>(*l));
        api::clearStack(*l);
        api::push(*l, val);
        return function_traits<func_type>::nrets;
    }
};
*/

} // namespace glua
