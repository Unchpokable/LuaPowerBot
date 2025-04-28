#include "configs.hxx"

#include <fstream>
#include <mutex>

#include "error.hxx"
#include "expected.hxx"
#include "logdef.hxx"

#include "thirdparty/base64/base64.hpp"
#include "thirdparty/json/json.hpp"

using namespace nlohmann;

namespace configs::internal::data {

std::unordered_map<std::size_t, TypeInfo> g_type_registry;
std::unordered_map<std::string, ConfigValue> g_config_values;
std::mutex g_config_mutex;

}

void conf_value_to_json(json& j, const configs::ConfigValue& config) {
    const configs::ConfigValueBase& value = config.getValue();

    if(std::holds_alternative<std::string>(value)) {
        j = json { {"type", "string"}, {"value", std::get<std::string>(value)} };
    } else if(std::holds_alternative<int>(value)) {
        j = json { {"type", "int"}, {"value", std::get<int>(value)} };
    } else if(std::holds_alternative<double>(value)) {
        j = json { {"type", "double"}, {"value", std::get<double>(value)} };
    } else if(std::holds_alternative<bool>(value)) {
        j = json { {"type", "bool"}, {"value", std::get<bool>(value)} };
    } else if(std::holds_alternative<configs::Vec2Int>(value)) {
        const auto& vec = std::get<configs::Vec2Int>(value);
        j = json { {"type", "vec2int"}, {"x", vec.x}, {"y", vec.y} };
    } else if(std::holds_alternative<std::vector<std::uint8_t>>(value)) {
        const auto& binary = std::get<std::vector<std::uint8_t>>(value);
        std::string encoded = base64::to_base64(std::string_view(reinterpret_cast<const char*>(binary.data()), binary.size()));
        j = json { {"type", "binary"}, {"value", encoded} };
    }
}

void conf_value_from_json(const json& j, configs::ConfigValue& config) {
    std::string type = j["type"];

    if(type == "string") {
        config.setValue(j["value"].get<std::string>());
    } else if(type == "int") {
        config.setValue(j["value"].get<int>());
    } else if(type == "double") {
        config.setValue(j["value"].get<double>());
    } else if(type == "bool") {
        config.setValue(j["value"].get<bool>());
    } else if(type == "vec2int") {
        configs::Vec2Int vec;
        vec.x = j["x"];
        vec.y = j["y"];
        config.setValue(vec);
    } else if(type == "binary") {
        std::string encoded = j["value"].get<std::string>();
        std::string decoded = base64::from_base64(encoded);
        std::vector<std::uint8_t> binary(decoded.begin(), decoded.end());
        config.setValue(binary);
    }
}

std::unordered_map<std::size_t, configs::internal::TypeInfo>& configs::internal::get_type_registry()
{
    return data::g_type_registry;
}

std::unordered_map<std::string, configs::ConfigValue>& configs::internal::get_config_values()
{
    std::unique_lock lock(data::g_config_mutex);
    return data::g_config_values;
}

Expected<bool, errors::Error> configs::load_from_file(const fs::path& path)
{
    try {
        if(!exists(path)) {
            luabot_logWarn("Config file does not exist: {}", path.string());
            return errors::Error("Config file does not exists!");
        }

        std::ifstream file(path);
        if(!file.is_open()) {
            return errors::Error("Failed to open config file: " + path.string());
        }

        json j;
        file >> j;

        auto& values = internal::get_config_values();
        values.clear();
        for(auto& [key, value] : j.items()) {
            ConfigValue configValue;
            conf_value_from_json(value, configValue);
            values[key] = configValue;
        }

        luabot_logInfo("Config loaded from {}", path.string());
        return true;

    } catch(const json::exception& e) {
        return errors::Error("JSON error while loading config: " + std::string(e.what()));
    } catch(const std::exception& e) {
        return errors::Error("Error loading config: " + std::string(e.what()));
    }
}

Expected<bool, errors::Error> configs::save_to_file(const fs::path& path)
{
    try {
        create_directories(path.parent_path());

        std::ofstream file(path);
        if(!file.is_open()) {
            return errors::Error("Failed to open config file for writing: " + path.string());
        }

        json j;
        auto& values = internal::get_config_values();
        for(const auto& [key, value] : values) {
            json json_value;
            conf_value_to_json(json_value, value);
            j[key] = json_value;
        }

        file << j.dump(4);
        luabot_logInfo("Config saved to {}", path.string());
        return true;
    } catch(const json::exception& e) {
        return errors::Error("JSON error while saving config: " + std::string(e.what()));
    } catch(const std::exception& e) {
        return errors::Error("Error saving config: " + std::string(e.what()));
    }
}
