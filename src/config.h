#ifndef PDFAST_CONFIG_H
#define PDFAST_CONFIG_H

#include <cstddef>
#include <string>
#include <unordered_set>

/**
 * Runtime configuration. Values come from environment variables,
 * falling back to a local .env file and finally to built-in defaults.
 **/
struct Config {
    std::string bind_host;
    int port;
    int log_level;
    std::string redis_url;
    std::string token_secret;
    std::string cors_origin;
    std::unordered_set<std::string> allowed_hosts;
    std::string media_root;
    std::unordered_set<std::string> readable_collections;
    std::unordered_set<std::string> writable_collections;
    long token_ttl_seconds;
    long max_token_uses;
    std::size_t max_upload_bytes;
    std::size_t max_path_depth;
};

/**
 * @brief Returns the configuration, loading it on first use.
 * @return The immutable application configuration
 **/
const Config& config();

#endif
