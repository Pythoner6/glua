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
        api::push<Ret>(*l, val);
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
        api::push<Ret>(*l, val);
        return function_traits<func_type>::nrets;
    }
};

template<typename Functor, bool = std::is_same<typename function_traits<Functor>::return_type, void>::value>
class cfunctor {};

template<typename Functor>
class cfunctor<Functor, false> {
public:
    using func_type      = Functor;
    using return_type    = typename function_traits<func_type>::return_type;
    using argument_types = typename function_traits<func_type>::argument_types;

    static inline void push(lua_State& l, Functor f) {
        func_type& func = api::newUserdata<func_type>(l, "functor");
        new (&func) func_type(f);
        lua_pushcclosure(&l, &wrapper, 1);
    }

    static inline int wrapper(lua_State* l) {
        func_type& func = api::getUserdata<func_type>(*l, api::upvalueIndex(1), "functor");
        return_type val = call_with_tuple(func, api::checkGet<argument_types>(*l));
        api::clearStack(*l);
        api::push<return_type>(*l, val);
        return function_traits<func_type>::nrets;
    }
};

template<typename Functor>
class cfunctor<Functor, true> {
public:
    using func_type      = Functor;
    using return_type    = typename function_traits<func_type>::return_type;
    using argument_types = typename function_traits<func_type>::argument_types;

    static inline void push(lua_State& l, Functor f) {
        func_type& func = api::newUserdata<func_type>(l, "functor");
        new (&func) func_type(f);
        lua_pushcclosure(&l, &wrapper, 1);
    }

    static inline int wrapper(lua_State* l) {
        func_type& func = api::getUserdata<func_type>(*l, api::upvalueIndex(1), "functor");
        call_with_tuple(func, api::checkGet<argument_types>(*l));
        api::clearStack(*l);
        return function_traits<func_type>::nrets;
    }
};

} // namespace glua
