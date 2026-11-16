#ifndef PDFAST_MEDIA_STORAGE_H
#define PDFAST_MEDIA_STORAGE_H

#include <filesystem>
#include <optional>
#include <string>

/**
 * A validated media location: <collection>/<segment>/.../<name>[.<extension>]
 **/
struct MediaPath {
    std::string collection;
    std::filesystem::path relative;
    bool has_extension;
};

/**
 * @brief Parses and validates a request path, rejecting traversal and unsafe characters.
 * @param raw_path The path captured after /Media/
 * @return The validated media path, or nullopt when invalid
 **/
std::optional<MediaPath> parse_media_path(const std::string& raw_path);

/**
 * @brief Finds the stored file of a media path, resolving the extension when omitted.
 * @return The absolute file path, or nullopt when it does not exist
 **/
std::optional<std::filesystem::path> find_media_file(const MediaPath& media);

/**
 * @brief Maps an upload Content-Type to the file extension used to store it.
 * @return The extension, or nullopt when the type is not accepted
 **/
std::optional<std::string> extension_for_mime(const std::string& content_type);

/**
 * @brief Atomically stores a file, replacing previous versions with any other extension.
 * @return True when the file was written completely
 **/
bool store_media(const MediaPath& media, const std::string& extension, const std::string& content);

#endif
