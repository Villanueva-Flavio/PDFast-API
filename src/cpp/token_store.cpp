#include "../token_store.h"
#include "../config.h"
#include "../security.h"

#include <algorithm>
#include <chrono>
#include <sw/redis++/redis++.h>

namespace {

constexpr const char* CSRF_KEY_PREFIX = "pdfast:csrf:";
constexpr const char* USES_KEY_PREFIX = "pdfast:uses:";
constexpr long MIN_TOKEN_USES = 1;
constexpr long long EXHAUSTED_USES = 0;

sw::redis::Redis& redis() {
    static sw::redis::Redis client(config().redis_url);
    return client;
}

void revoke(const std::string& session_hash) {
    redis().del({CSRF_KEY_PREFIX + session_hash, USES_KEY_PREFIX + session_hash});
}

}

IssuedToken issue_token(long requested_uses) {
    long uses = std::clamp(requested_uses, MIN_TOKEN_USES, config().max_token_uses);
    IssuedToken token{random_token(), random_token(), uses};
    std::string session_hash = hash_token(token.session_id);
    std::chrono::seconds ttl(config().token_ttl_seconds);
    redis().set(CSRF_KEY_PREFIX + session_hash, hash_token(token.csrf_token), ttl);
    redis().set(USES_KEY_PREFIX + session_hash, std::to_string(uses), ttl);
    return token;
}

bool consume_token(const std::string& session_id, const std::string& csrf_token) {
    if (session_id.empty() || csrf_token.empty()) return false;
    std::string session_hash = hash_token(session_id);
    auto stored_csrf = redis().get(CSRF_KEY_PREFIX + session_hash);
    if (!stored_csrf || !constant_time_equals(*stored_csrf, hash_token(csrf_token))) return false;
    long long remaining = redis().decr(USES_KEY_PREFIX + session_hash);
    if (remaining <= EXHAUSTED_USES) revoke(session_hash);
    return remaining >= EXHAUSTED_USES;
}
