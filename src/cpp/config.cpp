#include "../config.h"

#include <cstdlib>
#include <fstream>
#include <sstream>
#include <unordered_map>

namespace {

using Values = std::unordered_map<std::string, std::string>;

constexpr const char* ENV_FILE = ".env";
constexpr char ASSIGNMENT = '=';
constexpr char COMMENT = '#';
constexpr char LIST_SEPARATOR = ',';
constexpr const char* WHITESPACE = " \t\r\n";

std::string trim(const std::string& value) {
    std::size_t start = value.find_first_not_of(WHITESPACE);
    if (start == std::string::npos) return "";
    std::size_t end = value.find_last_not_of(WHITESPACE);
    return value.substr(start, end - start + 1);
}

Values read_env_file() {
    Values values;
    std::ifstream file(ENV_FILE);
    for (std::string line; std::getline(file, line);) {
        line = trim(line);
        std::size_t separator = line.find(ASSIGNMENT);
        if (line.empty() || line.front() == COMMENT || separator == std::string::npos) continue;
        values[trim(line.substr(0, separator))] = trim(line.substr(separator + 1));
    }
    return values;
}

std::string read_text(const Values& file, const std::string& name, const std::string& fallback) {
    const char* environment_value = std::getenv(name.c_str());
    if (environment_value && *environment_value) return environment_value;
    auto found = file.find(name);
    return found != file.end() ? found->second : fallback;
}

long read_number(const Values& file, const std::string& name, long fallback) {
    std::string value = read_text(file, name, "");
    return value.empty() ? fallback : std::stol(value);
}

std::unordered_set<std::string> read_list(const Values& file, const std::string& name, const std::string& fallback) {
    std::unordered_set<std::string> items;
    std::stringstream stream(read_text(file, name, fallback));
    for (std::string item; std::getline(stream, item, LIST_SEPARATOR);) {
        if (!trim(item).empty()) items.insert(trim(item));
    }
    return items;
}

Config load_config() {
    const Values file = read_env_file();
    return Config{
        read_text(file, "CROW_HOST", "0.0.0.0"),
        static_cast<int>(read_number(file, "CROW_PORT", 8003)),
        static_cast<int>(read_number(file, "LOG_LEVEL", 1)),
        read_text(file, "REDIS_URL", "tcp://redis:6379"),
        read_text(file, "ENCRYPTION_KEY", ""),
        read_text(file, "CORS_ORIGIN", ""),
        read_list(file, "ALLOWED_HOSTS", ""),
        read_text(file, "MEDIA_ROOT", "Media"),
        read_list(file, "READABLE_COLLECTIONS", "Product,Website"),
        read_list(file, "WRITABLE_COLLECTIONS", "Product"),
        read_number(file, "TOKEN_TTL_SECONDS", 3600),
        read_number(file, "MAX_TOKEN_USES", 100),
        static_cast<std::size_t>(read_number(file, "MAX_UPLOAD_BYTES", 52428800)),
        static_cast<std::size_t>(read_number(file, "MAX_PATH_DEPTH", 6)),
    };
}

}

const Config& config() {
    static const Config instance = load_config();
    return instance;
}
