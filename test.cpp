#include "variable_traits.hpp"
#include "function_traits.hpp"

#include <string>
#include <iostream>
#include <algorithm>
#include <tuple>
#include <type_traits>

struct Person final {
    std::string familyName;
    float height;
    bool isFemale;

    void Introduce() const { std::cout << "Hi" << std::endl; }
    bool IsFemale() const { return isFemale; }
    bool GetMarried(Person& other) {
        bool success = other.isFemale != isFemale;
        if (isFemale) familyName = "Mrs." + other.familyName;
        else familyName = "Mr." + other.familyName;
        return success;
    } 
};

void Foo() {}

template <typename T> struct TypeInfo;

template <typename Ret, typename... Args>
auto func_p_type(Ret (*)(Args...)) -> Ret (*)(Args...);

template <typename Ret, typename Class, typename... Args>
auto func_p_type(Ret (Class::*)(Args...)) -> Ret (Class::*)(Args...);

template <typename Ret, typename Class, typename... Args>
auto func_p_type(Ret (Class::*)(Args...) const) -> Ret (Class::*)(Args...) const;

template <auto F> 
using func_p_type_t = decltype(func_p_type(F));

template <auto F>
using function_traits_t = function_traits<func_p_type_t<F>>;

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
};

template <typename T> struct field_traits : public basic_field_traits<T, is_function_v<T>> {
    constexpr field_traits(T&& pointer, std::string_view name) : pointer { pointer }, 
    name { name.substr(name.find_last_of(":") + 1) } {}

    T pointer;
    std::string_view name;
};

#define BEGIN_CLASS(X) template <> struct TypeInfo<X> {

#define functions(...)\
    static constexpr auto functions = std::make_tuple(__VA_ARGS__);
#define func(F)\
    field_traits(F, #F)

#define END_CLASS()\
};

BEGIN_CLASS(Person)
    functions(
        func(&Person::GetMarried),
        func(&Person::IsFemale),
        func(&Person::Introduce)
    )
END_CLASS()

template <typename T>
constexpr auto type_info() {
    return TypeInfo<T>{};
};

template <size_t Idx, typename... Args, typename Class>
void VisitTuple(const std::tuple<Args...>& tuple, Class* instance) {
    using tuple_type = std::tuple<Args...>;
    if constexpr (std::tuple_size_v<tuple_type> <= Idx) {
        return;
    }
    else {
        if constexpr (auto elem = std::get<Idx>(tuple); elem.param_count() == 0) {
            (instance->*elem.pointer)();
        } 
        VisitTuple<Idx + 1>(tuple, instance);
    }
}

int main() {
    constexpr auto info = type_info<Person>();
    std::cout << std::get<0>(info.functions).name << std::endl;

    Person person;
    VisitTuple<0>(info.functions, &person);

    return 0;
}