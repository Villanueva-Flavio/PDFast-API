#ifndef PDFAST_SECURITY_H
#define PDFAST_SECURITY_H

#include <string>
#include "crow.h"

/**
 * @brief Generates a cryptographically secure random token (hex encoded).
 * @return The generated token
 **/
std::string random_token();

/**
 * @brief Hashes a token with HMAC-SHA256 using the configured secret.
 * @param token The token to hash
 * @return The hex encoded digest
 **/
std::string hash_token(const std::string& token);

/**
 * @brief Compares two strings in constant time.
 * @return True when both strings are equal
 **/
bool constant_time_equals(const std::string& left, const std::string& right);

/**
 * @brief Checks the Host header against the allowed hosts (port ignored).
 * @param req The incoming request
 * @return True when the host is allowed or no restriction is configured
 **/
bool is_allowed_host(const crow::request& req);

#endif
