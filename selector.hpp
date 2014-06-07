#pragma once

#include "api.hpp"

namespace glua {

class selector_base;

namespace detail {
template<typename Ret, typename... Args>
struct _call_impl {
    inline static Ret call(lua_State& l, Args&&... args) {
        api::push(l, std::forward<Args>(args)...);
        api::call(l, sizeof...(Args), 1);
        return api::checkGet<Ret>(l);
    }
};

template<typename... Rets, typename... Args>
struct _call_impl<std::tuple<Rets...>, Args...> {
    inline static std::tuple<Rets...> call(lua_State& l, Args&&... args) {
        api::push(l, std::forward<Args>(args)...);
        api::call(l, sizeof...(Args), sizeof...(Rets));
        return api::checkGet<std::tuple<Rets...>>(l);
    }
};

struct ret {
private:
    lua_State& l;

    friend class ::glua::selector_base;
    ret(lua_State& l) : l(l) {}
public:
    template<typename T>
    inline auto get() 
    -> decltype(api::checkGet<T>(l))
    {
        return api::checkGet<T>(l);
    }

    template<typename T>
    inline operator T() {
        return get<T>();
    }
};

} // namespace detail

class selector_base {
public:
    lua_State& l;

    virtual ~selector_base() {}
    virtual void push() = 0;

    selector_base(const selector_base&) = delete;
    selector_base& operator=(const selector_base&) = delete;
    //selector_base(selector_base&&) = delete;

    template<typename T>
    void set(T val);

    template<typename T>
    inline auto get() 
    -> decltype(api::checkGet<T>(l))
    {
        this->push();
        decltype(api::checkGet<T>(l)) val = api::checkGet<T>(l);
        return val;
    }

    template<typename T, typename = typename 
        std::enable_if<
            (std::is_pod<T>::value         ||
            std::is_fundamental<T>::value ||
            std::is_pointer<T>::value     ) &&
            !std::is_reference<T>::value
        >::type
    >
    inline operator T() {
        return get<T>();
    }

    template<typename... Args>
    inline detail::ret operator()(Args&&... args) {
        this->push();

        //return detail::_call_impl<Ret, Args...>::call(l, std::forward<Args>(args)...);
        api::push(l, std::forward<Args>(args)...);
        api::call(l, sizeof...(Args), LUA_MULTRET);
        return detail::ret(l);
    }

protected:
    selector_base(lua_State& l) : l(l) {}
};

template<typename Key>
class selector : public selector_base {
public:
    inline virtual void push() {
        api::getTable(l, key);
    }

    template<typename T>
    void set(T val) { api::setTable(l, key, val); }

    template<typename T>
    inline void operator=(T val) { set(val); }

    template<typename K>
    inline auto operator[](K&& nextKey) && -> selector<K> {
        this->push();
        return selector<K>(l, std::forward<K>(nextKey));
    }

protected:
    inline selector(lua_State& l, Key&& key) : selector_base(l), key(key) {}
    inline selector(lua_State& l, const Key& key) : selector_base(l), key(key) {}

    Key key;
};

} // namespace glua
