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
        Double
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
            default: return "Unknown";
        }
    }

    template <typename T> static Kind detectKind() {
        if constexpr (std::is_same_v<T, char>) {
            return Kind::Char;
        }
        else if constexpr (std::is_same_v<T, int> || std::is_same_v<T, long>) {
            return Kind::Int32
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
        if constexpr (std::is_fundamental_v<T>) {
            return NumericFactory<T>::Instance();
        } 
        else if constexpr (std::is_enum_v<T>) {
            return EnumFactory<T>::Instance();
        }
        else if constexpr (std::is_class_v<T>) {
            //
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