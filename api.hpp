#pragma once

#include <stdexcept>
#include <tuple>
#include "util.hpp"

/**
 * api.hpp
 * Contains free functions in the glua::api (and glua::api::detail)
 * namespace which provide a simple wrapper for the lua c api. Other
 * than the use of a *reference* to lua_State instead of a pointer,
 * only a few functions have been modified, such as push; the version
 * of push here takes a variable number of arguments of arbitrary types
 * and pushes them all, in order onto the stack. All functions are 
 * inline, since most simply forward their arguments onto the
 * corresponding call in the lua c api.
 */

namespace glua {
namespace api {

template<typename T>
inline T& newUserdata(lua_State& l) {
    return *static_cast<T*>(lua_newuserdata(&l, sizeof(T)));
}

template<typename T>
inline T& getUserdata(lua_State& l, int index) {
    return *static_cast<T*>(lua_touserdata(&l, index));
}

namespace detail {

template<typename T>
struct registry {
    static constexpr bool exists = false;
    static constexpr char* name = nullptr;
};

/**
 * _push() functions
 * This set of functions is responsible
 * for actually pushing values of arbitrary
 * types onto the stack.
 */
template<typename T>
struct _push_impl {
    inline static void push(lua_State& l, T val) {
        T* data = &newUserdata<T>(l);
        new (data) T(val);
    }
};

template<typename T>
struct _push_impl<T&> {
    inline static void push(lua_State& l, T& ref) {
        newUserdata<T*>(l) = &ref;
    }
};

template<typename T>
struct _push_impl<T*> {
    inline static void push(lua_State& l, T* ref) {
        newUserdata<T*>(l) = ref;
    }
};

template<>
struct _push_impl<bool> {
    inline static void push(lua_State& l, bool val) {
        lua_pushboolean(&l, val);
    }
};

template<>
struct _push_impl<lua_Integer> {
    inline static void push(lua_State& l, lua_Integer val) {
        lua_pushinteger(&l, val);
    }
};

template<>
struct _push_impl<lua_Unsigned> {
    inline static void push(lua_State& l, lua_Unsigned val) {
        lua_pushunsigned(&l, val);
    }
};

template<>
struct _push_impl<lua_Number> {
    inline static void push(lua_State& l, lua_Number val) {
        lua_pushnumber(&l, val);
    }
};

template<>
struct _push_impl<const char*> {
    inline static void push(lua_State& l, const char* val) {
        lua_pushstring(&l, val);
    }
};

template<>
struct _push_impl<std::string> {
    inline static void push(lua_State& l, std::string val) {
        lua_pushlstring(&l, val.c_str(), val.length());
    }
};

template<>
struct _push_impl<std::nullptr_t> {
    inline static void push(lua_State& l, std::nullptr_t) {
        lua_pushnil(&l);
    }
};

template<typename... T>
struct _push_n_impl {};

template<typename T1, typename... T>
struct _push_n_impl<T1, T...> {
    inline static void push(lua_State& l, T1 val1, T... vals) {
        _push_impl<T1>::push(l, std::forward<T1>(val1));
        _push_n_impl<T...>::push(l, std::forward<T>(vals)...);
    }
};

template<typename T>
struct _push_n_impl<T> {
    inline static void push(lua_State& l, T&& val) {
        _push_impl<T>::push(l, std::forward<T>(val));
    }
};
} // namespace detail

template<typename... T>
inline void push(lua_State& l, T&&... vals) {
    detail::_push_n_impl<T...>::push(l, std::forward<T>(vals)...);
}

namespace detail {
template<typename Key, typename Value>
struct _set_table_impl {
    inline static void set_table(lua_State& l, Key key, Value value) {
        push(l, key);
        push(l, value);
        lua_settable(&l, -3);
    }
};

template<typename Value>
struct _set_table_impl<const char*, Value> {
    inline static void set_table(lua_State& l, const char* key, Value value) {
        push(l, value);
        lua_setfield(&l, -2, key);
    }
};

template<typename Value>
struct _set_table_impl<std::string, Value> {
    inline static void set_table(lua_State& l, std::string key, Value value) {
        _set_table_impl<const char*, Value>::set_table(l, key.c_str(), value);
    }
};
} // namespace detail

template<typename Key, typename Value>
inline void setTable(lua_State& l, Key key, Value value) {
    detail::_set_table_impl<Key, Value>::set_table(l, key, value);
}

template<typename Value>
inline void setGlobal(lua_State& l, const char* key, Value value) {
    push(l, value);
    lua_setglobal(&l, key);
}

inline void getGlobal(lua_State& l, const char* key) {
    lua_getglobal(&l, key);
}

inline void getGlobal(lua_State& l, std::string key) {
    lua_getglobal(&l, key.c_str());
}

template<typename Key>
inline void getTable(lua_State& l, Key key) {
    push(l, key);
    lua_gettable(&l, -2);
}

template<>
inline void getTable(lua_State& l, std::string key) {
    push(l, key.c_str());
    lua_gettable(&l, -2);
}

template<>
inline void getTable(lua_State& l, const char* key) {
    lua_getfield(&l, -1, key);
    //luaL_getsubtable(&l, -1, key);
}

namespace detail {

template<typename T>
struct _check_get_impl {
    /*
    inline static auto get(lua_State& l, int index) 
    -> decltype(T::get(l, index))
    {
        return T::get(l, index);
    }
    */

    inline static T get(lua_State& l, int index) 
    {
        return getUserdata<T>(l, index);
    }
};

template<typename T>
struct _check_get_impl<T&> {
    inline static T& get(lua_State& l, int index) 
    {
        T* ret = getUserdata<T*>(l, index);
        if(ret == nullptr) throw std::runtime_error("Lua returned null reference");
        return *ret;
    }
};

template<typename T>
struct _check_get_impl<T*> {
    inline static T* get(lua_State& l, int index) 
    {
        return getUserdata<T*>(l, index);
    }
};

template<>
struct _check_get_impl<bool> {
    inline static bool get(lua_State& l, int index) {
        return lua_toboolean(&l, index);
    }
};

template<>
struct _check_get_impl<lua_Integer> {
    inline static lua_Integer get(lua_State& l, int index) {
        return luaL_checkinteger(&l, index);
    }
};

template<>
struct _check_get_impl<lua_Unsigned> {
    inline static lua_Unsigned get(lua_State& l, int index) {
        return luaL_checkunsigned(&l, index);
    }
};

template<>
struct _check_get_impl<lua_Number> {
    inline static lua_Number get(lua_State& l, int index) {
        return luaL_checknumber(&l, index);
    }
};

template<>
struct _check_get_impl<std::string> {
    inline static std::string get(lua_State& l, int index) {
        size_t      len = 0;
        const char* str = luaL_checklstring(&l, index, &len);
        return std::string(str, len);
    }
};

template<>
struct _check_get_impl<const char*> {
    inline static const char* get(lua_State& l, int index) {
        return luaL_checkstring(&l, index);
    }
};

template<typename T, int N = 1>
struct _check_get_n_impl {
    inline static auto get(lua_State& l, int index) 
    -> decltype(_check_get_impl<T>::get(l, -(N-index)))
    {
        return _check_get_impl<T>::get(l, -(N-index));
    }

    inline static auto get(lua_State& l) 
    -> decltype( _check_get_impl<T>::get(l, -N))
    {
        return _check_get_impl<T>::get(l, -N);
    }
};

template<typename... T>
struct _check_get_n_impl<std::tuple<T...>> {
    template<int... I>
    inline static auto work(lua_State& l, ::glua::detail::_index_list<I...>) 
    -> decltype(std::make_tuple(_check_get_n_impl<T,sizeof...(I)>::get(l, I)...))
    {
        return std::make_tuple(_check_get_n_impl<T,sizeof...(I)>::get(l, I)...);
    }

    inline static auto get(lua_State& l)
    -> decltype(work(l, typename ::glua::detail::_build_index_list<sizeof...(T)>::build()))
    {
        return work(l, typename ::glua::detail::_build_index_list<sizeof...(T)>::build());
    }
};

} // namespace detail

template<typename T>
inline auto checkGet(lua_State& l) 
#if __cplusplus <= 201103L
-> decltype(detail::_check_get_n_impl<T>::get(l))
#endif
{
    return detail::_check_get_n_impl<T>::get(l);
}

template<typename T>
inline auto pop(lua_State& l)
#if __cplusplus <= 201103L
-> decltype(checkGet<T>(l))
#endif
{
    auto vals = checkGet<T>(l);
    //lua_pop(&l, sizeof...(T));
    return vals;
}

inline void clearStack(lua_State& l) {
    lua_settop(&l, 0);
}

inline lua_State& open() {
    lua_State* l = luaL_newstate();
    if(l == nullptr) throw std::runtime_error("Couldn't create new lua state!");
    return *l;
}

inline void openLibs(lua_State& l) {
    luaL_openlibs(&l);
}

inline void close(lua_State& l) {
    lua_close(&l);
}

inline void call(lua_State& l, int nargs, int nret) {
    lua_call(&l, nargs, nret);
}

inline void load(lua_State& l, const char* filename) {
    luaL_dofile(&l, filename);
}

inline int ref(lua_State& l) {
    return luaL_ref(&l, LUA_REGISTRYINDEX);
}

inline void unref(lua_State& l, int ref) {
    luaL_unref(&l, LUA_REGISTRYINDEX, ref);
}

inline void getRef(lua_State& l, int ref) {
    lua_rawgeti(&l, LUA_REGISTRYINDEX, ref);
}

inline int upvalueIndex(int index) {
    return lua_upvalueindex(index);
};

} // namespace api
} // namespace glua
