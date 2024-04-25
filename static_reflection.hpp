#ifndef __STATIC_REFLECTION_H__
#define __STATIC_REFLECTION_H__

#include "variable_traits.hpp"
#include "function_traits.hpp"

#include <type_traits>
#include <string_view>

template <typename T> struct TypeInfo;

template <typename Ret, typename... Args>
auto func_p_type(Ret (*)(Args...)) -> Ret (*)(Args...);

template <typename Ret, typename Class, typename... Args>
auto func_p_type(Ret (Class::*)(Args...)) -> Ret (Class::*)(Args...);

template <typename Ret, typename Class, typename... Args>
auto func_p_type(Ret (Class::*)(Args...) const) -> Ret (Class::*)(Args...) const;

template <auto F> using func_p_type_t = decltype(func_p_type(F));

template <typename T> struct is_function {
    static constexpr bool value = std::is_function_v<std::remove_pointer_t<T>> || std::is_member_function_pointer_v<T>;
};

template <typename T>
constexpr bool is_function_v = is_function<T>::value;

template <typename T, bool isFunc> struct basic_field_traits;

template <typename T> struct basic_field_traits<T, true> : public function_traits<T> {
    using traits = function_traits<T>;
    
    constexpr bool is_member() const {
        return traits::is_member;
    }

    constexpr bool is_const() const {
        return traits::is_const;
    }

    constexpr bool is_function() const {
        return true;
    }

    constexpr bool is_variable() const {
        return false;
    }

    constexpr size_t param_count() const {
        return std::tuple_size_v<typename traits::args>;
    }
};

template <typename T> struct basic_field_traits<T, false> : public variable_traits<T> {
    using traits = variable_traits<T>;
    
    constexpr bool is_member() const {
        return traits::is_member;
    }

    constexpr bool is_const() const {
        return traits::is_const;
    }

    constexpr bool is_function() const {
        return false;
    }

    constexpr bool is_variable() const {
        return true;
    }
};

template <typename T> struct field_traits : public basic_field_traits<T, is_function_v<T>> {
    constexpr field_traits(T&& pointer, std::string_view name) : pointer { pointer }, 
    name { name.substr(name.find_last_of(":") + 1) } {}

    T pointer;
    std::string_view name;
};

#define BEGIN_CLASS(X) template <> struct TypeInfo<X> {

#define functions(...)\
    static constexpr auto functions = std::tuple{__VA_ARGS__};
#define func(F)\
    field_traits(F, #F)

#define END_CLASS()\
};

template <typename T>
constexpr auto reflected_type() {
    return TypeInfo<T>{};
};

#endif // !__STATIC_REFLECTION_H__