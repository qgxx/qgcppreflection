#include "variable_traits.hpp"
#include "function_traits.hpp"
#include "static_reflection.hpp"

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

int main() {
    constexpr auto info = reflected_type<Person>();
    std::cout << std::get<0>(info.functions).name << std::endl;

    Person person;
    VisitTuple<0>(info.functions, &person);

    return 0;
}