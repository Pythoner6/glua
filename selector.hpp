#pragma once

#include "api.hpp"

namespace glua {

namespace detail {
    template<int Nargs, typename T>
    struct _call_return {
        inline static auto call(lua_State& l) 
        -> decltype(api::checkGet<T>(l))
        {
            api::call(l, Nargs, 1);
            return api::checkGet<T>(l);
        }
    };

    template<int Nargs, typename... T>
    struct _call_return<Nargs, std::tuple<T...>> {
        inline static auto call(lua_State& l) 
        -> decltype(api::checkGet<std::tuple<T...>>(l))
        {
            api::call(l, Nargs, sizeof...(T));
            return api::checkGet<std::tuple<T...>>(l);
        }
    };

    template<int Nargs, typename T>
    inline auto callReturn(lua_State& l) 
    -> decltype(_call_return<Nargs, T>::call(l))
    {
        return _call_return<Nargs, T>::call(l);
    }

    template<int Nargs>
    class ret {
    private:
        lua_State& l;
    public:
        template<typename T>
        inline auto get() 
        -> decltype(detail::callReturn<Nargs, T>(l))
        {
            return detail::callReturn<Nargs, T>(l);
        }

        /*
        template<typename T, typename = typename 
            std::enable_if<
                //std::is_pod<T>::value         ||
                std::is_fundamental<T>::value ||
                std::is_pointer<T>::value     ||
                std::is_reference<T>::value   ||
                api::detail::registry<T>::exists
            >::type
        >
        */
        template<typename T>
        inline operator T() {
            return get<T>();
        }

        template<typename T>
        inline operator T&() {
            return get<T&>();
        }

        ret(lua_State& l) : l(l) {}
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

    //template<typename T, typename = typename std::enable_if<!std::is_base_of<selector_base, T>::value>::type>
    /*
    template<typename T, typename = typename 
        std::enable_if<
            std::is_pod<T>::value         ||
            std::is_fundamental<T>::value ||
            std::is_pointer<T>::value     ||
            std::is_reference<T>::value   ||
            api::detail::registry<T>::exists
        >::type
    >
    */
    template<typename T>
    inline auto get() 
    -> decltype(api::checkGet<T>(l))
    {
        this->push();
        auto val = api::checkGet<T>(l);
        return val;
    }

    template<typename T, typename = typename 
        std::enable_if<
            std::is_pod<T>::value         ||
            std::is_fundamental<T>::value ||
            std::is_pointer<T>::value     ||
            std::is_reference<T>::value   ||
            api::detail::registry<T>::exists
        >::type
    >
    inline operator T() {
        return get<T>();
    }

    template<typename T>
    inline void operator=(T&& val) {
        this->set(std::forward<T>(val));
    }

    template<typename... Args>
    inline detail::ret<sizeof...(Args)> operator()(Args&&... args) {
        this->push();
        api::push(l, std::forward<Args>(args)...);
        return detail::ret<sizeof...(Args)>(l);
    }

    inline detail::ret<0> operator()() {
        this->push();
        return detail::ret<0>(l);
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
    void set(T val) {
        api::setTable(l, key, val);
    }

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
