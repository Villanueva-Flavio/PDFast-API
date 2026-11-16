#include "../security.h"
#include "../config.h"

#include <array>
#include <stdexcept>
#include <openssl/crypto.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/rand.h>

namespace {

constexpr std::size_t TOKEN_BYTES = 32;
constexpr char HEX_DIGITS[] = "0123456789abcdef";
constexpr unsigned int HIGH_NIBBLE_SHIFT = 4;
constexpr unsigned char NIBBLE_MASK = 0x0F;
constexpr char PORT_SEPARATOR = ':';
constexpr char IPV6_CLOSING_BRACKET = ']';
constexpr const char* HOST_HEADER = "Host";

std::string to_hex(const unsigned char* data, std::size_t size) {
    std::string hex;
    hex.reserve(size * 2);
    for (std::size_t i = 0; i < size; ++i) {
        hex += HEX_DIGITS[(data[i] >> HIGH_NIBBLE_SHIFT) & NIBBLE_MASK];
        hex += HEX_DIGITS[data[i] & NIBBLE_MASK];
    }
    return hex;
}

std::string strip_port(const std::string& host) {
    std::size_t colon = host.rfind(PORT_SEPARATOR);
    bool has_port = colon != std::string::npos && host.find(IPV6_CLOSING_BRACKET, colon) == std::string::npos;
    return has_port ? host.substr(0, colon) : host;
}

}

std::string random_token() {
    std::array<unsigned char, TOKEN_BYTES> bytes{};
    if (RAND_bytes(bytes.data(), static_cast<int>(bytes.size())) != 1) {
        throw std::runtime_error("RAND_bytes");
    }
    return to_hex(bytes.data(), bytes.size());
}

std::string hash_token(const std::string& token) {
    const std::string& secret = config().token_secret;
    std::array<unsigned char, EVP_MAX_MD_SIZE> digest{};
    unsigned int digest_size = 0;
    HMAC(EVP_sha256(), secret.data(), static_cast<int>(secret.size()),
         reinterpret_cast<const unsigned char*>(token.data()), token.size(), digest.data(), &digest_size);
    return to_hex(digest.data(), digest_size);
}

bool constant_time_equals(const std::string& left, const std::string& right) {
    return left.size() == right.size() && CRYPTO_memcmp(left.data(), right.data(), left.size()) == 0;
}

bool is_allowed_host(const crow::request& req) {
    const auto& allowed = config().allowed_hosts;
    return allowed.empty() || allowed.contains(strip_port(req.get_header_value(HOST_HEADER)));
}
