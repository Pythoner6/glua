#pragma once

#include <stdexcept>
#include <tuple>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

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

/**
 * Create a new lua userdata of, allocating space for a specific
 * type of object and return a reference. NOTE: This does NOT call
 * the constructor for the object, so if you need to construct
 * the object, you must use the placement new operator
 */
template<typename T>
inline T& newUserdata(lua_State& l) {
    return *static_cast<T*>(lua_newuserdata(&l, sizeof(T)));
}

/**
 * If the object at index index is userdata, return a reference
 * to it. Otherwise throw an error.
 */
template<typename T>
inline T& getUserdata(lua_State& l, int index) {
    T* t = static_cast<T*>(lua_touserdata(&l, index));
    if(t == nullptr) throw std::runtime_error("Error: trying to get non-userdata as userdata");
    return *t;
}

namespace detail {

template<typename T>
struct registry {
    static constexpr bool exists = false;
    static constexpr char* name = nullptr;
};

/**
 * Template struct containing a single method
 * which pushes an arbitrary value onto the lua stack.
 * Default method of doing this is to create a new 
 * userdata and copy the value into the userdata.
 *
 * This set of methods is implemented using a struct
 * because partial specialization is required
 */
template<typename T>
struct _push_impl {
    inline static void push(lua_State& l, T val) {
        T* data = &newUserdata<T>(l);
        new (data) T(val);
    }
};

/**
 * Push implementaiton for arbitrary reference types.
 * Creates a new userdata of T* and assigns it the
 * value of &ref
 */
template<typename T>
struct _push_impl<T&> {
    inline static void push(lua_State& l, T& ref) {
        newUserdata<T*>(l) = &ref;
    }
};

/**
 * Push implementaiton for arbitrary pointer types.
 * Creates a new userdata of T* and assigns it the
 * value of ptr
 */
template<typename T>
struct _push_impl<T*> {
    inline static void push(lua_State& l, T* ptr) {
        newUserdata<T*>(l) = ptr;
    }
};

/** 
 * Push implementaiton for bool
 */
template<>
struct _push_impl<bool> {
    inline static void push(lua_State& l, bool val) {
        lua_pushboolean(&l, val);
    }
};

/** 
 * Push implementaiton for integers
 */
template<>
struct _push_impl<lua_Integer> {
    inline static void push(lua_State& l, lua_Integer val) {
        lua_pushinteger(&l, val);
    }
};

/** 
 * Push implementaiton for unsigned integers
 */
template<>
struct _push_impl<lua_Unsigned> {
    inline static void push(lua_State& l, lua_Unsigned val) {
        lua_pushunsigned(&l, val);
    }
};

/** 
 * Push implementaiton for the lua_Number type (usually double)
 */
template<>
struct _push_impl<lua_Number> {
    inline static void push(lua_State& l, lua_Number val) {
        lua_pushnumber(&l, val);
    }
};

/** 
 * Push implementaiton for c-style strings
 */
template<>
struct _push_impl<const char*> {
    inline static void push(lua_State& l, const char* val) {
        lua_pushstring(&l, val);
    }
};

/** 
 * Push implementaiton for strings
 */
template<>
struct _push_impl<std::string> {
    inline static void push(lua_State& l, std::string val) {
        lua_pushlstring(&l, val.c_str(), val.length());
    }
};

/** 
 * Push implementaiton for the nullptr type, which pushes nil 
 * to the lua stack.
 */
template<>
struct _push_impl<std::nullptr_t> {
    inline static void push(lua_State& l, std::nullptr_t) {
        lua_pushnil(&l);
    }
};

/**
 * Template struct supplying an implementaiton to 
 * push an arbitrary number of arbitrary values onto
 * the lua stack.
 *
 * This method is implemented using a struct because
 * partial specialization is required.
 */
template<typename... T>
struct _push_n_impl {};

/**
 * For more than one argument, push the first, then 
 * push the rest (instantiating a new copy of this struct)
 */
template<typename T1, typename... T>
struct _push_n_impl<T1, T...> {
    inline static void push(lua_State& l, T1&& val1, T&&... vals) {
        _push_impl<T1>::push(l, std::forward<T1>(val1));
        _push_n_impl<T...>::push(l, std::forward<T>(vals)...);
    }
};

/**
 * Specialization for pushing a single value.
 */
template<typename T>
struct _push_n_impl<T> {
    inline static void push(lua_State& l, T&& val) {
        _push_impl<T>::push(l, std::forward<T>(val));
    }
};
} // namespace detail

/**
 * Push any number of arbitrary values onto the lua stack.
 */
template<typename... T>
inline void push(lua_State& l, T&&... vals) {
    detail::_push_n_impl<T...>::push(l, std::forward<T>(vals)...);
}

namespace detail {
/**
 * Template struct supplying implementaiton of setTable.
 */
template<typename Key, typename Value>
struct _set_table_impl {
    inline static void set_table(lua_State& l, Key key, Value value) {
        push(l, key, value);
        lua_settable(&l, -3);
    }
};

/**
 * Sepcialization for c-string key type using the more efficient lua_setfield funciton.
 */
template<typename Value>
struct _set_table_impl<const char*, Value> {
    inline static void set_table(lua_State& l, const char* key, Value value) {
        push(l, value);
        lua_setfield(&l, -2, key);
    }
};

/**
 * Sepcialization for string key type using the more efficient lua_setfield funciton.
 */
template<typename Value>
struct _set_table_impl<std::string, Value> {
    inline static void set_table(lua_State& l, std::string key, Value value) {
        _set_table_impl<const char*, Value>::set_table(l, key.c_str(), value);
    }
};
} // namespace detail

/**
 * Sets t[key] = value,
 * where t is the table at the top of the stack.
 */
template<typename Key, typename Value>
inline void setTable(lua_State& l, Key key, Value value) {
    detail::_set_table_impl<Key, Value>::set_table(l, key, value);
}

/**
 * Sets the global named by key to the specified value
 */
template<typename Value>
inline void setGlobal(lua_State& l, const char* key, Value value) {
    push(l, value);
    lua_setglobal(&l, key);
}

/**
 * Pushes the global named by key onto the stack.
 */
inline void getGlobal(lua_State& l, const char* key) {
    lua_getglobal(&l, key);
}

/**
 * Pushes the global named by key onto the stack.
 */
inline void getGlobal(lua_State& l, std::string key) {
    lua_getglobal(&l, key.c_str());
}

/**
 * Pushes the value t[key] onto the stack, where 
 * t is the table at the top of the stack.
 */
template<typename Key>
inline void getTable(lua_State& l, Key key) {
    push(l, key);
    lua_gettable(&l, -2);
}

/**
 * Specialization of getTable for string key type.
 * Implemented using the more efficient lua_getfield funciton.
 */
template<>
inline void getTable(lua_State& l, std::string key) {
    lua_getfield(&l, -1, key.c_str());
}

/**
 * Specialization of getTable for c-string key type.
 * Implemented using the more efficient lua_getfield funciton.
 */
template<>
inline void getTable(lua_State& l, const char* key) {
    lua_getfield(&l, -1, key);
    //luaL_getsubtable(&l, -1, key);
}

namespace detail {

/**
 * Template struct supplying implementation of checkGet.
 *
 * Default implementation for arbitrary type T is to treat the
 * value on the stack as userdata of type T and return a copy
 * of the value.
 *
 * This method is implemented using a struct because partial
 * specialization is requied.
 */
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

/**
 * Partial specialization for reference types.
 * Treats the value on the top of the stack as 
 * userdata of a pointer to type T and returns 
 * a reference, as long as the pointer is not null.
 * If the pointer is null, throws an exception.
 */
template<typename T>
struct _check_get_impl<T&> {
    inline static T& get(lua_State& l, int index) 
    {
        T* ret = getUserdata<T*>(l, index);
        if(ret == nullptr) throw std::runtime_error("Lua returned null reference");
        return *ret;
    }
};

/**
 * Partial specialization for pointer types.
 * Treats the value on the top of the stack as 
 * userdata of pointer to type T and returns it.
 */
template<typename T>
struct _check_get_impl<T*> {
    inline static T* get(lua_State& l, int index) 
    {
        return getUserdata<T*>(l, index);
    }
};

/**
 * Partial specialization for booleans.
 */
template<>
struct _check_get_impl<bool> {
    inline static bool get(lua_State& l, int index) {
        return lua_toboolean(&l, index);
    }
};

/**
 * Partial specialization for integers.
 */
template<>
struct _check_get_impl<lua_Integer> {
    inline static lua_Integer get(lua_State& l, int index) {
        return luaL_checkinteger(&l, index);
    }
};

/**
 * Partial specialization for unsigned integers.
 */
template<>
struct _check_get_impl<lua_Unsigned> {
    inline static lua_Unsigned get(lua_State& l, int index) {
        return luaL_checkunsigned(&l, index);
    }
};

/**
 * Partial specialization for the lua_Number type (usually double).
 */
template<>
struct _check_get_impl<lua_Number> {
    inline static lua_Number get(lua_State& l, int index) {
        return luaL_checknumber(&l, index);
    }
};

/**
 * Partial specialization for strings.
 */
template<>
struct _check_get_impl<std::string> {
    inline static std::string get(lua_State& l, int index) {
        size_t      len = 0;
        const char* str = luaL_checklstring(&l, index, &len);
        return std::string(str, len);
    }
};

/**
 * Partial specialization for c-strings.
 */
template<>
struct _check_get_impl<const char*> {
    inline static const char* get(lua_State& l, int index) {
        return luaL_checkstring(&l, index);
    }
};

/**
 * Template struct providing an implementation for getting
 * an arbitrary number of arbitrary values from the lua stack.
 *
 * This method is implemented using a struct because it requires
 * partial specialization.
 */
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

/**
 * Partial specialization for tuple type, allowing the
 * retrieval of multiple values.
 */
template<typename... T>
struct _check_get_n_impl<std::tuple<T...>> {
    /**
     * Helper function which takes an _index_list to provide the
     * _check_get_n_impl instances with the required index parameter
     */
    template<int... I>
    inline static auto work(lua_State& l, ::glua::detail::_index_list<I...>) 
    -> decltype(std::make_tuple(_check_get_n_impl<T,sizeof...(I)>::get(l, I)...))
    {
        return std::make_tuple(_check_get_n_impl<T,sizeof...(I)>::get(l, I)...);
    }

    inline static auto get(lua_State& l)
    -> decltype(work(l, typename ::glua::detail::_build_index_list<sizeof...(T)>::build()))
    {
        // Call the helper function to do all the work.
        return work(l, typename ::glua::detail::_build_index_list<sizeof...(T)>::build());
    }
};

} // namespace detail

/**
 * Get a value from the lua stack.
 * to get multiple values from the stack
 * use checkGet<std::tuple<...>>
 *
 * NOTE: does not pop any values from the stack.
 */
template<typename T>
inline auto checkGet(lua_State& l) 
#if __cplusplus <= 201103L
-> decltype(detail::_check_get_n_impl<T>::get(l))
#endif
{
    return detail::_check_get_n_impl<T>::get(l);
}

/**
 * Clear the stack.
 */
inline void clearStack(lua_State& l) {
    lua_settop(&l, 0);
}

/**
 * Create a new lua_State.
 */
inline lua_State& open() {
    lua_State* l = luaL_newstate();
    if(l == nullptr) throw std::runtime_error("Couldn't create new lua state!");
    return *l;
}

/**
 * Open the core lua libraries.
 */
inline void openLibs(lua_State& l) {
    luaL_openlibs(&l);
}

/**
 * Close a lus_State.
 */
inline void close(lua_State& l) {
    lua_close(&l);
}

/**
 * Execute a lua object on the stack.
 */
inline void call(lua_State& l, int nargs, int nret) {
    lua_call(&l, nargs, nret);
}

/**
 * Load a lua file and run it.
 */
inline void loadFile(lua_State& l, const char* filename) {
    luaL_dofile(&l, filename);
}

/**
 * Load a lua file and run it.
 */
inline void loadFile(lua_State& l, std::string filename) {
    luaL_dofile(&l, filename.c_str());
}

/**
 * Load and run a string as a lua chunk.
 */
inline void loadString(lua_State& l, const char* chunk) {
    luaL_dostring(&l, chunk);
}

/**
 * Load and run a string as a lua chunk.
 */
inline void loadString(lua_State& l, std::string chunk) {
    luaL_dostring(&l, chunk.c_str());
}

/**
 * Create a reference to the lua object on the top of
 * the stack.
 */
inline int ref(lua_State& l) {
    return luaL_ref(&l, LUA_REGISTRYINDEX);
}

/**
 * Delete a reference.
 */
inline void unref(lua_State& l, int ref) {
    luaL_unref(&l, LUA_REGISTRYINDEX, ref);
}

/**
 * Push the value associated with a particular
 * reference onto the stack.
 */
inline void getRef(lua_State& l, int ref) {
    lua_rawgeti(&l, LUA_REGISTRYINDEX, ref);
}

/**
 * Get an upvalue index.
 */
inline int upvalueIndex(int index) {
    return lua_upvalueindex(index);
};

} // namespace api
} // namespace glua
