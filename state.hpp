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

    /**
     * Access a global variable.
     */
    inline global operator[](const char* key) {
        return global(l, key);
    }

    /**
     * Load and run a lua file.
     */
    inline void load(const char* filename) {
        api::loadFile(l, filename);
    }

    /**
     * Load and run a lua file.
     */
    inline void load(std::string filename) {
        api::loadFile(l, filename);
    }

    /**
     * Load and run a c-string as a lua chunk.
     */
    inline void run(const char* chunk) {
        api::loadString(l, chunk);
    }

    /**
     * Load and run a string as a lua chunk.
     */
    inline void run(std::string chunk) {
        api::loadString(l, chunk);
    }

    template<typename FuncT, FuncT func>
    void registerFunction(const char* name) {
        lua_pushcclosure(&l, &cfunction<FuncT, func>::wrapper, 0);
        lua_setglobal(&l, name);
    }

    template<typename Functor>
    void registerFunction(const char* name, Functor f) {
        cfunctor<Functor>::push(l, f);
        lua_setglobal(&l, name);
    }
private:
    lua_State& l;
};

} // namespace glua
