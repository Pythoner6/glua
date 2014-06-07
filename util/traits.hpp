#pragma once 

namespace glua {

template<typename Functor>
struct functor_traits : public functor_traits<decltype(&Functor::operator())> {};

/*
template<typename Class>
struct functor_traits<void(Class::*)() const> {
    static constexpr int nargs = 0;
    static constexpr int nrets = 0;

    using return_type    = void;
    using argument_types = std::tuple<>;
};
*/

/*
template<typename ReturnType, typename Class, typename... ArgumentTypes>
struct functor_traits<ReturnType(Class::* const)(ArgumentTypes...)> :
public functor_traits<ReturnType(ArgumentTypes...)> {};

template<typename ReturnType, typename Class>
struct functor_traits<ReturnType(Class::* const)()> :
public functor_traits<ReturnType()> {};

template<typename ReturnType, typename... ArgumentTypes>
struct functor_traits<ReturnType(ArgumentTypes...)> {
    static constexpr int nargs = sizeof...(ArgumentTypes);
    static constexpr int nrets = 1;

    using return_type    = ReturnType;
    using argument_types = std::tuple<ArgumentTypes...>;

    template<size_t n>
    struct arg {
        static_assert(n < nargs, "Error: invalid parameter index");
        using type = typename std::tuple_element<n, std::tuple<ArgumentTypes...>>::type;
    };
};

template<typename... ArgumentTypes>
struct functor_traits<void(ArgumentTypes...)> {
    static constexpr int nargs = sizeof...(ArgumentTypes);
    static constexpr int nrets = 0;

    using return_type    = void;
    using argument_types = std::tuple<ArgumentTypes...>;

    template<size_t n>
    struct arg {
        static_assert(n < nargs, "Error: invalid parameter index");
        using type = typename std::tuple_element<n, std::tuple<ArgumentTypes...>>::type;
    };
};

template<typename... ReturnTypes, typename... ArgumentTypes>
struct functor_traits<std::tuple<ReturnTypes...>(ArgumentTypes...)> {
    static constexpr int nargs = sizeof...(ArgumentTypes);
    static constexpr int nrets = sizeof...(ReturnTypes);

    using return_type    = std::tuple<ReturnTypes...>;
    using argument_types = std::tuple<ArgumentTypes...>;

    template<size_t n>
    struct arg {
        static_assert(n < nargs, "Error: invalid parameter index");
        using type = typename std::tuple_element<n, std::tuple<ArgumentTypes...>>::type;
    };
};
*/

template<typename Functor>
struct function_traits : public functor_traits<Functor> {
    /*
private:
    using functor_traits = detail::functor_traits<decltype(&Functor::operator())>;
public:
    // Have to subtract one because we don't want to include the
    // reference to a Functor object necessary to make the operator() call
    static constexpr int nargs = functor_traits::nargs - 1;
    static constexpr int nrets = functor_traits::nrets;

    using return_type    = typename functor_traits::return_type;
    using argument_types = typename functor_traits::argument_types;

    template<size_t n>
    struct arg {
        static_assert(n < nargs, "Error: invalid parameter index");
        using type = typename functor_traits::template arg<n+1>::type;
    };
    */
};

template<typename ReturnType, typename Class, typename... ArgumentTypes>
struct functor_traits<ReturnType(Class::*)(ArgumentTypes...) const> :
public function_traits<ReturnType(ArgumentTypes...)> {};

template<typename ReturnType, typename... ArgumentTypes>
struct function_traits<ReturnType(*)(ArgumentTypes...)> :
public function_traits<ReturnType(ArgumentTypes...)> {};

template<typename ReturnType, typename Class, typename... ArgumentTypes>
struct function_traits<ReturnType(Class::*)(ArgumentTypes...)> :
public function_traits<ReturnType(Class&, ArgumentTypes...)> {};

template<typename ReturnType, typename Class, typename... ArgumentTypes>
struct function_traits<ReturnType(Class::*)(ArgumentTypes...) const> :
public function_traits<ReturnType(const Class&, ArgumentTypes...)> {};

template<typename ReturnType, typename... ArgumentTypes>
struct function_traits<ReturnType(ArgumentTypes...)> {
    static constexpr int nargs = sizeof...(ArgumentTypes);
    static constexpr int nrets = 1;

    using return_type    = ReturnType;
    using argument_types = std::tuple<ArgumentTypes...>;

    template<size_t n>
    struct arg {
        static_assert(n < nargs, "Error: invalid parameter index");
        using type = typename std::tuple_element<n, std::tuple<ArgumentTypes...>>::type;
    };
};

template<typename... ReturnTypes, typename... ArgumentTypes>
struct function_traits<std::tuple<ReturnTypes...>(ArgumentTypes...)> {
    static constexpr int nargs = sizeof...(ArgumentTypes);
    static constexpr int nrets = sizeof...(ReturnTypes);

    using return_type    = std::tuple<ReturnTypes...>;
    using argument_types = std::tuple<ArgumentTypes...>;

    template<size_t n>
    struct arg {
        static_assert(n < nargs, "Error: invalid parameter index");
        using type = typename std::tuple_element<n, std::tuple<ArgumentTypes...>>::type;
    };
};

template<typename... ArgumentTypes>
struct function_traits<void(ArgumentTypes...)> {
    static constexpr int nargs = sizeof...(ArgumentTypes);
    static constexpr int nrets = 0;

    using return_type    = void;
    using argument_types = std::tuple<ArgumentTypes...>;

    template<size_t n>
    struct arg {
        static_assert(n < nargs, "Error: invalid parameter index");
        using type = typename std::tuple_element<n, std::tuple<ArgumentTypes...>>::type;
    };
};

namespace detail {

template<typename T>
struct type_traits {
};

} // namespace detail

} // namespace glua

#define GLUA_REGISTER(CLASS,NAME)  \
namespace glua {                    \
namespace detail {                   \
template<>                            \
struct type_traits<CLASS> {            \
    static constexpr const char* name = #NAME; \
};                                       \
}                                         \
}

#define GLUA_REG(CLASS) GLUA_REGISTER(CLASS,CLASS) GLUA_REGISTER(CLASS*,CLASS*)
