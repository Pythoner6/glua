#pragma once
#include <stdexcept>

#include "api.hpp"
#include "global.hpp"
#include "ref.hpp"
#include "cfunction.hpp"

namespace glua {

class state {
public:
    state(bool openLibs = true) : l(api::open()) {
        if(openLibs) api::openLibs(l);
    }

    // No copying
    state(state&) = delete;
    state& operator=(state&) = delete;

    ~state() {
        api::close(l);
    }

    inline global operator[](const char* key) {
        return global(l, key);
    }

    inline void load(const char* filename) {
        api::loadFile(l, filename);
    }

    inline void load(std::string filename) {
        api::loadFile(l, filename);
    }

    inline void run(const char* chunk) {
        api::loadString(l, chunk);
    }

    inline void run(std::string chunk) {
        api::loadString(l, chunk);
    }

    template<typename FuncT, FuncT func>
    void registerFunction(const char* name) {
        lua_pushcclosure(&l, &cfunction<FuncT, func>::wrapper, 0);
        lua_setglobal(&l, name);
    }

    lua_State& l;
private:
};

} // namespace glua
