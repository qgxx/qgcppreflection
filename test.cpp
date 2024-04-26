#include "variable_traits.hpp"
#include "function_traits.hpp"
#include "static_reflection.hpp"
#include "dynamic_reflection.hpp"
#include "type_list.hpp"

#include <string>
#include <iostream>
#include <algorithm>

struct Person final {
    std::string familyName;
    float height;
    bool isFemale;

    void Introduce() const { std::cout << "Hi" << std::endl; }
    bool IsFemale() const { 
        std::cout << "Test IsFemale()" << std::endl;
        return isFemale; 
    }
    bool GetMarried(Person& other) {
        bool success = other.isFemale != isFemale;
        if (isFemale) familyName = "Mrs." + other.familyName;
        else familyName = "Mr." + other.familyName;
        return success;
    } 
};

BEGIN_CLASS(Person)
    functions(
        func(&Person::GetMarried),
        func(&Person::IsFemale),
        func(&Person::Introduce)
    )
END_CLASS()

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

enum class TestEnum {
    val1 = 1,
    val2 = 2
};

template <typename T> struct is_integral {
    static constexpr bool value = std::is_integral_v<T>;
};

template <typename T> struct all_to_float {
    using type = float;
};

template <typename T> struct is_not_char {
    static constexpr bool value = !std::is_same_v<T, char>;
};

int main() {
    constexpr auto info = reflected_type<Person>();
    std::cout << std::get<0>(info.functions).name << std::endl;

    Person person;
    VisitTuple<0>(info.functions, &person);

    Registrar<TestEnum>().Regist("TestEnum").Add("val1", TestEnum::val1).Add("val2", TestEnum::val2);
    Registrar<Type::Kind>().Regist("Kind");
    const Type* typeinfo = GetType<TestEnum>();
    const Enum* enuminfo = typeinfo->AsEnum();
    typeinfo = GetType<Type::Kind>();
    std::cout << typeinfo->getName() << std::endl;
    std::cout << enuminfo->getName() << std::endl;
    for (auto& item : enuminfo->getItems()) {
        std::cout << item.name << ' ' << item.value << std::endl;
    }

    Registrar<Person>().Regist("Person").AddVariable<decltype(&Person::height)>("height").
    AddFunction<decltype(&Person::Introduce)>("Introduce").
    AddFunction<decltype(&Person::GetMarried)>("GetMarried");
    typeinfo = GetType<Person>();
    const Class* classinfo = typeinfo->AsClass();
    std::cout << classinfo->getName() << std::endl;
    for (auto var : classinfo->GetVariables()) {
        std::cout << var.name << '(' << var.type->getName() << ')' << std::endl;
    }
    for (auto func : classinfo->GetFunctions()) {
        std::cout << func.name << '(' << func.retType->getName() << ": ";
        bool firt_par = true;
        for (auto param : func.paramTypes) {
            if (!firt_par) std::cout << ", ";
            std::cout << param->getName();
            firt_par = false;
        }
        std::cout << ')' << std::endl;
    }

    using tl = type_list<void, bool, char, int, float, double>;
    using first_elem = head<tl>;
    using tail_elem = tail<tl>;
    using second_elem = ithType<tl, 1>;

    constexpr auto value = count<tl, is_integral>;
    
    using tl_map = map<tl, all_to_float>;
    using tl_cons = cons<tl, long long>; 
    using tl_concat = concat<tl_map, tl_cons>;
    using tl_init = init<tl>;
    using tl_filter = filter<tl, is_not_char>;

    return 0;
}