#pragma once
#include <type_traits>
#include "selector.hpp"
#include "global.hpp"

namespace glua {

class ref : public selector<int> {
public:
    inline ref(ref&& r) : selector<int>(r.l, r.key) {
        r.key = LUA_NOREF;
    }

    template<typename S, typename std::enable_if<std::is_base_of<selector_base, S>::value>::type* = nullptr>
    inline ref(S&& sel) : selector<int>(sel.l, LUA_NOREF) {
        sel.push();
        key = api::ref(l);
    }


    inline virtual ~ref() {
        api::unref(l, key); 
    }

    inline virtual void push() {
        api::getRef(l, key);
    }
};

} // namespace glua
