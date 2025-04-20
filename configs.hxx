#pragma once

#include <any>
#include <filesystem>
#include <functional>
#include <optional>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <variant>
#include <vector>

#include "error.hxx"
#include "expected.hxx"

namespace fs = std::filesystem;

namespace configs {

struct Vec2Int
{
    int x;
    int y;

    bool operator==(const Vec2Int& other) const {
        return x == other.x && y == other.y;
    }
};

using ConfigValueBase = std::variant<
    std::string,
    int,
    double,
    bool,
    Vec2Int,
    std::vector<std::uint8_t>
>;

using SerializeFunc = std::function<std::vector<std::uint8_t>(const std::any&)>;
using DeserializeFunc = std::function<std::any(const std::vector<std::uint8_t>&)>;

class ConfigValue
{
public:
    ConfigValue() : _value(std::string {}) {}
    ConfigValue(const std::string& value) : _value(value) {}
    ConfigValue(int value) : _value(value) {}
    ConfigValue(double value) : _value(value) {}
    ConfigValue(bool value) : _value(value) {}
    ConfigValue(const Vec2Int& value) : _value(value) {}

    template<typename T>
    ConfigValue(const T& value)
    {
        if constexpr(std::is_same_v<T, std::string> ||
            std::is_same_v<T, int> ||
            std::is_same_v<T, double> ||
            std::is_same_v<T, bool> ||
            std::is_same_v<T, Vec2Int>) {
            _value = value;
        } else {
            _value = serialize(value);
        }
    }

    template<typename T>
    std::optional<T> as() const;


    const ConfigValueBase& getValue() const { return _value; }

    template<typename T>
    void setValue(T&& value);

private:
    ConfigValueBase _value;
};

namespace internal {

struct TypeInfo
{
    SerializeFunc serialize;
    DeserializeFunc deserialize;
};

std::unordered_map<std::size_t, TypeInfo>& get_type_registry();
std::unordered_map<std::string, ConfigValue>& get_config_values();

}

template<typename T>
std::vector<std::uint8_t> serialize(const T& obj)
{
    auto typeId = typeid(T).hash_code();
    auto& registry = internal::get_type_registry();
    auto it = registry.find(typeId);
    if(it == registry.end()) {
        throw std::runtime_error("Type not registered for serialization: " + std::string(typeid(T).name()));
    }

    return it->second.serialize(std::any(obj));
}

template<typename T>
std::optional<T> deserialize(const std::vector<std::uint8_t>& data)
{
    auto typeId = typeid(T).hash_code();
    auto& registry = internal::get_type_registry();
    auto it = registry.find(typeId);
    if(it == registry.end()) {
        return std::nullopt;
    }

    std::any result = it->second.deserialize(data);
    try {
        return std::any_cast<T>(result);
    } catch(const std::bad_any_cast&) {
        return std::nullopt;
    }
}

template<typename T>
void register_type(
    std::function<std::vector<std::uint8_t>(const T&)> serializeFunc,
    std::function<std::optional<T>(const std::vector<std::uint8_t>&)> deserializeFunc)
{
    auto typeId = typeid(T).hash_code();
    auto& registry = internal::get_type_registry();

    internal::TypeInfo info;
    info.serialize = [serializeFunc](const std::any& value) -> std::vector<std::uint8_t> {
        try {
            const T& typedValue = std::any_cast<const T&>(value);
            return serializeFunc(typedValue);
        } catch(const std::bad_any_cast&) {
            return {};
        }
    };

    info.deserialize = [deserializeFunc](const std::vector<std::uint8_t>& data) -> std::any {
        auto result = deserializeFunc(data);
        if(result) {
            return std::any(*result);
        }
        return {};
    };

    registry[typeId] = std::move(info);
}

template<typename T>
std::optional<T> ConfigValue::as() const {
    if constexpr(std::is_same_v<T, std::string> ||
        std::is_same_v<T, int> ||
        std::is_same_v<T, double> ||
        std::is_same_v<T, bool> ||
        std::is_same_v<T, Vec2Int>) {
        if(std::holds_alternative<T>(_value)) {
            return std::get<T>(_value);
        }
    } else {
        if(std::holds_alternative<std::vector<std::uint8_t>>(_value)) {
            const auto& data = std::get<std::vector<std::uint8_t>>(_value);
            return deserialize<T>(data);
        }
    }
    return std::nullopt;
}

template <typename T>
void ConfigValue::setValue(T&& value)
{
    if constexpr(std::is_same_v<std::decay_t<T>, std::string> ||
        std::is_same_v<std::decay_t<T>, int> ||
        std::is_same_v<std::decay_t<T>, double> ||
        std::is_same_v<std::decay_t<T>, bool> ||
        std::is_same_v<std::decay_t<T>, Vec2Int>) {
        _value = std::forward<T>(value);
    } else {
        _value = serialize(std::forward<T>(value));
    }
}

template<>
inline std::optional<std::string> ConfigValue::as<std::string>() const {
    if(std::holds_alternative<std::string>(_value)) {
        return std::get<std::string>(_value);
    }
    return std::nullopt;
}

template<typename T>
const T* get(const std::string& key) {
    auto& values = internal::get_config_values();
    auto it = values.find(key);
    if(it == values.end()) {
        return nullptr;
    }

    auto value = it->second.as<T>();
    if(!value.has_value()) {
        return nullptr;
    }

    static T result;
    result = value.value();
    return &result;
}

template<typename T>
void set(const std::string& key, T&& value) {
    auto& values = internal::get_config_values();
    values[key].setValue(std::forward<T>(value));
}

Expected<bool, errors::Error> load_from_file(const fs::path& path);
Expected<bool, errors::Error> save_to_file(const fs::path& path);

}
