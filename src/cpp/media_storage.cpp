#include "../media_storage.h"
#include "../config.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <vector>

namespace fs = std::filesystem;

namespace {

constexpr char PATH_SEPARATOR = '/';
constexpr char EXTENSION_SEPARATOR = '.';
constexpr char MIME_PARAMETERS_SEPARATOR = ';';
constexpr std::size_t MIN_PATH_DEPTH = 2;
constexpr std::size_t MAX_EXTENSION_SEPARATORS = 1;
constexpr const char* TEMPORARY_SUFFIX = ".uploading";
constexpr const char* WHITESPACE = " \t";

const std::unordered_map<std::string, std::string> UPLOAD_EXTENSIONS = {
    {"image/jpeg", "jpg"}, {"image/jpg", "jpg"}, {"image/png", "png"}, {"image/webp", "webp"},
    {"image/gif", "gif"}, {"image/bmp", "bmp"}, {"image/tiff", "tiff"}, {"image/avif", "avif"},
    {"video/mp4", "mp4"}, {"video/webm", "webm"}, {"application/pdf", "pdf"},
};

const std::vector<std::string> LOOKUP_EXTENSIONS = {
    "jpg", "jpeg", "png", "webp", "gif", "bmp", "tiff", "avif", "mp4", "webm", "pdf",
};

bool is_name_character(char character) {
    return std::isalnum(static_cast<unsigned char>(character)) || character == '-' || character == '_'
        || character == EXTENSION_SEPARATOR;
}

bool is_safe_segment(const std::string& segment) {
    if (segment.empty() || segment.front() == EXTENSION_SEPARATOR || segment.back() == EXTENSION_SEPARATOR) return false;
    auto separators = static_cast<std::size_t>(std::count(segment.begin(), segment.end(), EXTENSION_SEPARATOR));
    return separators <= MAX_EXTENSION_SEPARATORS && std::all_of(segment.begin(), segment.end(), is_name_character);
}

fs::path with_extension(const fs::path& base, const std::string& extension) {
    fs::path file = base;
    file += EXTENSION_SEPARATOR + extension;
    return file;
}

void remove_other_versions(const fs::path& base, const std::string& kept_extension) {
    std::error_code ignored;
    for (const auto& extension : LOOKUP_EXTENSIONS) {
        if (extension != kept_extension) fs::remove(with_extension(base, extension), ignored);
    }
}

bool write_file(const fs::path& path, const std::string& content) {
    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    file.write(content.data(), static_cast<std::streamsize>(content.size()));
    file.close();
    return file.good();
}

}

std::optional<MediaPath> parse_media_path(const std::string& raw_path) {
    std::vector<std::string> segments;
    std::stringstream stream(raw_path);
    for (std::string segment; std::getline(stream, segment, PATH_SEPARATOR);) {
        if (!is_safe_segment(segment)) return std::nullopt;
        segments.push_back(segment);
    }
    if (segments.size() < MIN_PATH_DEPTH || segments.size() > config().max_path_depth) return std::nullopt;
    fs::path relative;
    for (const auto& segment : segments) relative /= segment;
    bool has_extension = segments.back().find(EXTENSION_SEPARATOR) != std::string::npos;
    return MediaPath{segments.front(), relative, has_extension};
}

std::optional<fs::path> find_media_file(const MediaPath& media) {
    fs::path base = fs::path(config().media_root) / media.relative;
    std::error_code ignored;
    if (media.has_extension) {
        return fs::is_regular_file(base, ignored) ? std::optional(base) : std::nullopt;
    }
    for (const auto& extension : LOOKUP_EXTENSIONS) {
        fs::path candidate = with_extension(base, extension);
        if (fs::is_regular_file(candidate, ignored)) return candidate;
    }
    return std::nullopt;
}

std::optional<std::string> extension_for_mime(const std::string& content_type) {
    std::string mime = content_type.substr(0, content_type.find(MIME_PARAMETERS_SEPARATOR));
    mime.erase(0, mime.find_first_not_of(WHITESPACE));
    mime.erase(mime.find_last_not_of(WHITESPACE) + 1);
    std::transform(mime.begin(), mime.end(), mime.begin(), [](unsigned char c) { return std::tolower(c); });
    auto found = UPLOAD_EXTENSIONS.find(mime);
    return found != UPLOAD_EXTENSIONS.end() ? std::optional(found->second) : std::nullopt;
}

bool store_media(const MediaPath& media, const std::string& extension, const std::string& content) {
    fs::path base = fs::path(config().media_root) / media.relative;
    fs::path target = with_extension(base, extension);
    fs::path temporary = with_extension(base, extension + TEMPORARY_SUFFIX);
    std::error_code error;
    fs::create_directories(target.parent_path(), error);
    if (error || !write_file(temporary, content)) return false;
    fs::rename(temporary, target, error);
    if (error) {
        fs::remove(temporary, error);
        return false;
    }
    remove_other_versions(base, extension);
    return true;
}
