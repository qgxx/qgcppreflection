#ifndef __DYNAMIC_REFLECTION_H__
#define __DYNAMIC_REFLECTION_H__

#include "variable_traits.hpp"
#include "function_traits.hpp"

#include <string>
#include <vector>

class Numeric;
class Enum;
class Class;

class Type {
public:
    template <typename T> friend class EnumFactory;
    template <typename T> friend class ClassFactory;

    enum class Kind {
        Numeric,
        Enum,
        Class
    }; 

    virtual ~Type() = default;

    Type(const std::string& name, Kind kind) : name_ { name }, kind_ { kind } {}

    auto& getName() const { return name_; }
    auto& getKind() const { return kind_; }

    const Numeric* AsNumeric() const {
        if (kind_ == Kind::Numeric) {
            return (const Numeric*)this;
        }
        else {
            return nullptr;
        }
    }

    const Enum* AsEnum() const {
        if (kind_ == Kind::Enum) {
            return (const Enum*)this;
        }
        else {
            return nullptr;
        }
    }

    const Class* AsClass() const {
        if (kind_ == Kind::Class) {
            return (const Class*)this;
        }
        else {
            return nullptr;
        }
    }

private:
    std::string name_;
    Kind kind_;
};

class Numeric : public Type {
public:
    enum class Kind {
        Unknown,
        Char,
        Int32,
        Int64,
        Float,
        Double,
        Void,
        Bool
    };

    Numeric(Kind kind, bool isSigned) : Type { getName(kind), Type::Kind::Numeric }, 
    kind_ { kind }, isSigned_ { isSigned } {}

    template <typename T> static Numeric Create() {
        return Numeric{ detectKind<T>(), std::is_signed_v<T> };
    }

private:
    Kind kind_;
    bool isSigned_;

    static std::string getName(Kind kind) {
        switch (kind) {
            case Kind::Char: return "Char";
            case Kind::Int32: return "Int32";
            case Kind::Int64: return "Int64";
            case Kind::Float: return "Float";
            case Kind::Double: return "Double";
            case Kind::Void: return "Void";
            case Kind::Bool: return "Bool";
            default: return "Unknown";
        }
    }

    template <typename T> static Kind detectKind() {
        if constexpr (std::is_same_v<T, char>) {
            return Kind::Char;
        }
        else if constexpr (std::is_same_v<T, int> || std::is_same_v<T, long>) {
            return Kind::Int32;
        }
        else if constexpr (std::is_same_v<T, long long>) {
            return Kind::Int64;
        }
        else if constexpr (std::is_same_v<T, float>) {
            return Kind::Float;
        }
        else if constexpr (std::is_same_v<T, double>) {
            return Kind::Double;
        }
        else if constexpr (std::is_same_v<T, void>) {
            return Kind::Void;
        }
        else if constexpr (std::is_same_v<T, bool>) {
            return Kind::Bool;
        }
        else {
            return Kind::Unknown;
        }
    }
};

class Enum : public Type {
public:
    struct Item {
        using value_type = long long;

        std::string name;
        value_type value;
    };

    Enum() : Type { "Unkonwn-Enum", Type::Kind::Enum } {}
    Enum(const std::string name) : Type { name, Type::Kind::Enum } {}

    template <typename T> void Add(const std::string& name, T value) {
        items_.emplace_back(Item{name, static_cast<typename Item::value_type>(value)});
    }

    auto& getItems() const { return items_; }

private:
    std::vector<Item> items_;
};

class Class : public Type {
public:
    struct MemberVariable {
        std::string name;
        const Type* type;

        template <typename T>
        static MemberVariable Create(const std::string& name) {
            using type = typename variable_traits<T>::type; 
            return MemberVariable { name, GetType<type>() };
        }
    };

    struct MemberFunction {
        std::string name;
        const Type* retType;
        std::vector<const Type*> paramTypes;

        template <typename T>
        static MemberFunction Create(const std::string& name) {
            using traits = function_traits<T>;
            using args = typename traits::args;
            return MemberFunction { name, GetType<typename traits::ret_type>(), 
            cvtTypeList2Vector<args>(std::make_index_sequence<std::tuple_size_v<args>>()) };
        }

        private:
            template<typename Params, size_t... Idx>
            static std::vector<const Type*> cvtTypeList2Vector(std::index_sequence<Idx...>) {
                return { GetType<std::tuple_element_t<Idx, Params>>() ... };
            }
    };

public:
    Class() : Type { "Unknown-Class", Type::Kind::Class } {}
    Class(const std::string& name) : Type { name, Type::Kind::Class } {}

    void AddVar(MemberVariable&& var) {
        vars_.emplace_back(std::move(var));
    } 

    void AddFunc(MemberFunction&& func) {
        funcs_.emplace_back(std::move(func));
    }

    auto& GetVariables() const { return vars_; }
    auto& GetFunctions() const { return funcs_; }

private:
    std::vector<MemberVariable> vars_;
    std::vector<MemberFunction> funcs_;
};

template <typename T>
class NumericFactory final {
public:
    static NumericFactory& Instance() {
        static NumericFactory instance { Numeric::Create<T>() };
        return instance;
    }

    auto& Info() const { return info_; }

private:
    Numeric info_;

    NumericFactory(Numeric&& info) : info_ { std::move(info) } {}
};

template <typename T>
class EnumFactory final {
public:
    static EnumFactory& Instance() {
        static EnumFactory instacne;
        return instacne;
    }

    auto& Info() const { return info_; }

    EnumFactory& Regist(const std::string& name) {
        info_.name_ = name;
        return *this;
    }

    template<typename U>
    EnumFactory& Add(const std::string& name, U value) {
        info_.Add(name, value);
        return *this;
    }

private:
    Enum info_;
};

template <typename T>
class ClassFactory final {
public:
    static ClassFactory& Instance() {
        static ClassFactory instance;
        return instance;
    }

    auto& Info() const { return info_; }

    ClassFactory& Regist(const std::string name) {
        info_.name_ = name;
        return *this;
    }

    template <typename U>
    ClassFactory& AddVariable(const std::string& name) {
        info_.AddVar(Class::MemberVariable::Create<U>(name));
        return *this;
    }

    template <typename U>
    ClassFactory& AddFunction(const std::string& name) {
        info_.AddFunc(Class::MemberFunction::Create<U>(name));
        return *this;
    }

    void Unregist() {
        info_ = Class{};
    }

private:
    Class info_;

};

class TrivalFactory {
public:
    static TrivalFactory& Instance() {
        static TrivalFactory instance;
        return instance;
    }

private:
    TrivalFactory() {}
};

template <typename T>
class Factory final {
public:
    static auto& GetFactory() {
        using type = std::remove_reference_t<T>;
        if constexpr (std::is_fundamental_v<type>) {
            return NumericFactory<type>::Instance();
        } 
        else if constexpr (std::is_enum_v<type>) {
            return EnumFactory<type>::Instance();
        }
        else if constexpr (std::is_class_v<type>) {
            return ClassFactory<type>::Instance();
        }
        else {
            return TrivalFactory::Instance();
        }
    }
};

template <typename T> auto& Registrar() {
    return Factory<T>::GetFactory();
}

template <typename T> const Type* GetType() {
    return &Factory<T>::GetFactory().Info();
}

#endif // !__DYNAMIC_REFLECTION_H__