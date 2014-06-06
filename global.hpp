#pragma once

#include "state.hpp"
#include "selector.hpp"

namespace glua {

class global : public selector<const char*> {
public:
    inline global(const global& g) : selector<const char*>(g.l, g.key) {}
    // On destruction of the first selector, clear the stack.
    // This works because the destructor is guaranteed to be
    // called after fully evaluating the expression in which
    // a temporary object was created, which in this case is
    // exactly where we want to clear the stack.
    inline virtual ~global() {
        api::clearStack(l);
    }

    inline virtual void push() {
        api::getGlobal(l, key);
    }

    template<typename T>
    void set(T val) {
        api::setGlobal(l, key, val);
    }
private:
    friend class state;
    inline global(lua_State& l, const char* key) : selector<const char*>(l, std::forward<const char*>(key)) {}
};

} // namespace glua
