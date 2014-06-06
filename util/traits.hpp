#pragma once 

template<typename Functor>
struct function_traits {
private:
    using functor_traits = function_traits<decltype(&Functor::operator())>;
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
};


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
