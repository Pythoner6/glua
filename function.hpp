#pragma once
#include <tuple>
#include "api.hpp"
#include "ref.hpp"

namespace glua {

template<typename FuncType>
class function {};

template<typename Ret, typename... Args>
class function<Ret(Args...)> : public ref {
public:
    template<typename S, typename std::enable_if<std::is_base_of<selector_base, S>::value>::type* = nullptr>
    function(S&& sel) : ref(std::forward<S>(sel)) {}
    function(ref&& r) : ref(std::move(r)) {}
    function(function&& f) : ref(std::move(f)) {}

    virtual ~function() {}


    auto operator()(Args... args) 
    -> decltype(api::checkGet<Ret>(l))
    {
        this->ref::push();
        api::push(l, args...);
        api::call(l, sizeof...(Args), 1);
        decltype(api::checkGet<Ret>(l)) ret = api::checkGet<Ret>(l);
        api::clearStack(l);
        return ret;
        //return api::checkGet<Ret>(l);
    } 
private:
};

template<typename... Args>
class function<void(Args...)> : public ref {
public:
    template<typename S, typename std::enable_if<std::is_base_of<selector_base, S>::value>::type* = nullptr>
    function(S&& sel) : ref(std::forward<S>(sel)) {}
    function(ref&& r) : ref(std::move(r)) {}
    function(function&& f) : ref(std::move(f)) {}

    virtual ~function() {}

    void operator()(Args... args) {
        this->ref::push();
        api::push(l, args...);
        api::call(l, sizeof...(Args), 0);
    }
private:
};

template<typename... Rets, typename... Args>
class function<std::tuple<Rets...>(Args...)> : public ref {
public:
    template<typename S, typename std::enable_if<std::is_base_of<selector_base, S>::value>::type* = nullptr>
    function(S&& sel) : ref(std::forward<S>(sel)) {}
    function(ref&& r) : ref(std::move(r)) {}
    function(function&& f) : ref(std::move(f)) {}

    virtual ~function() {}

    std::tuple<Rets...> operator()(Args... args) {
        this->ref::push();
        api::push(l, args...);
        api::call(l, sizeof...(Args), sizeof...(Rets));
        std::tuple<Rets...> ret = api::checkGet<Rets...>(l);
        api::clearStack(l);
        return ret;
    }
};

} // namespace glua
