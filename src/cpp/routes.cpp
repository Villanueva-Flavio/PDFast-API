#include "../routes.h"
#include "../config.h"
#include "../media_storage.h"
#include "../security.h"
#include "../token_store.h"

namespace {

namespace Status {
constexpr int CREATED = 201;
constexpr int BAD_REQUEST = 400;
constexpr int UNAUTHORIZED = 401;
constexpr int FORBIDDEN = 403;
constexpr int NOT_FOUND = 404;
constexpr int PAYLOAD_TOO_LARGE = 413;
constexpr int UNSUPPORTED_MEDIA_TYPE = 415;
constexpr int INTERNAL_ERROR = 500;
}

constexpr const char* CONTENT_TYPE_HEADER = "Content-Type";
constexpr const char* SESSION_HEADER = "X-Session-ID";
constexpr const char* CSRF_HEADER = "X-CSRF-Token";
constexpr const char* JSON_MIME = "application/json";
constexpr const char* PING_RESPONSE = "boop";

void reply_json(crow::response& res, int code, const crow::json::wvalue& body) {
    res.code = code;
    res.set_header(CONTENT_TYPE_HEADER, JSON_MIME);
    res.write(body.dump());
    res.end();
}

void reply_status(crow::response& res, int code, const char* status) {
    reply_json(res, code, crow::json::wvalue({{"status", status}}));
}

void handle_token(const crow::request& req, crow::response& res, int requested_uses) {
    if (!is_allowed_host(req)) return reply_status(res, Status::FORBIDDEN, "forbidden");
    IssuedToken token = issue_token(requested_uses);
    reply_json(res, crow::status::OK, crow::json::wvalue({
        {"session_id", token.session_id}, {"csrf_token", token.csrf_token}, {"max_uses", token.max_uses},
    }));
}

void handle_media_get(const crow::request& req, crow::response& res, const std::string& raw_path) {
    if (!is_allowed_host(req)) return reply_status(res, Status::FORBIDDEN, "forbidden");
    auto media = parse_media_path(raw_path);
    if (!media || !config().readable_collections.contains(media->collection)) {
        return reply_status(res, Status::NOT_FOUND, "not_found");
    }
    auto file = find_media_file(*media);
    if (!file) return reply_status(res, Status::NOT_FOUND, "not_found");
    res.set_static_file_info_unsafe(file->string());
    res.end();
}

void handle_media_post(const crow::request& req, crow::response& res, const std::string& raw_path) {
    if (!is_allowed_host(req)) return reply_status(res, Status::FORBIDDEN, "forbidden");
    auto media = parse_media_path(raw_path);
    if (!media || media->has_extension) return reply_status(res, Status::BAD_REQUEST, "invalid_path");
    if (!config().writable_collections.contains(media->collection)) {
        return reply_status(res, Status::FORBIDDEN, "read_only_collection");
    }
    if (req.body.size() > config().max_upload_bytes) return reply_status(res, Status::PAYLOAD_TOO_LARGE, "too_large");
    auto extension = extension_for_mime(req.get_header_value(CONTENT_TYPE_HEADER));
    if (!extension) return reply_status(res, Status::UNSUPPORTED_MEDIA_TYPE, "unsupported_type");
    if (!consume_token(req.get_header_value(SESSION_HEADER), req.get_header_value(CSRF_HEADER))) {
        return reply_status(res, Status::UNAUTHORIZED, "invalid_token");
    }
    if (!store_media(*media, *extension, req.body)) return reply_status(res, Status::INTERNAL_ERROR, "write_failed");
    reply_status(res, Status::CREATED, "created");
}

}

void configure_cors(PDFastApp& app) {
    auto& rules = app.get_middleware<crow::CORSHandler>().global()
        .methods("GET"_method, "POST"_method, "OPTIONS"_method)
        .headers(CONTENT_TYPE_HEADER, SESSION_HEADER, CSRF_HEADER);
    if (!config().cors_origin.empty()) rules.origin(config().cors_origin);
}

void setup_routes(PDFastApp& app) {
    CROW_ROUTE(app, "/token/<int>").methods("GET"_method)(handle_token);
    CROW_ROUTE(app, "/Media/<path>").methods("GET"_method)(handle_media_get);
    CROW_ROUTE(app, "/Media/<path>").methods("POST"_method)(handle_media_post);
    CROW_ROUTE(app, "/beep").methods("GET"_method)([] { return PING_RESPONSE; });
}
