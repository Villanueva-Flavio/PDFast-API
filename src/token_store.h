#ifndef PDFAST_TOKEN_STORE_H
#define PDFAST_TOKEN_STORE_H

#include <string>

/**
 * Upload credentials handed to the client. Only their hashes are stored in Redis.
 **/
struct IssuedToken {
    std::string session_id;
    std::string csrf_token;
    long max_uses;
};

/**
 * @brief Issues a new session/CSRF pair valid for a number of uploads.
 * @param requested_uses Amount of uploads requested (clamped to the configured maximum)
 * @return The issued credentials
 **/
IssuedToken issue_token(long requested_uses);

/**
 * @brief Validates a session/CSRF pair and atomically consumes one use.
 * @return True when the pair is valid and still had uses left
 **/
bool consume_token(const std::string& session_id, const std::string& csrf_token);

#endif
