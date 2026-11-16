# PDFast-API

A fast and secure C++ ([Crow](https://github.com/CrowCpp/Crow)) file server for anything that can be rendered over HTTP: images, videos, PDFs, CSS, JS and fonts.

It is meant to sit **behind another API**: that API authenticates its users, asks PDFast-API for a short-lived upload token and uploads the files. Reads are served directly, streamed from disk.

## 1. Quick start

Requirements: `docker` with the compose plugin.

```bash
cp .env.example .env      # set ENCRYPTION_KEY and ALLOWED_HOSTS at least
docker compose up -d --build
curl http://localhost:8003/beep   # -> boop
```

## 2. Configuration

Every value is read from the environment, then from a local `.env` file, then from the default.

| Variable | Default | Description |
|---|---|---|
| `CROW_HOST` | `0.0.0.0` | Bind address |
| `CROW_PORT` | `8003` | Port |
| `LOG_LEVEL` | `1` | Crow log level (0 debug, 1 info, 2 warning, 3 error, 4 critical) |
| `ENCRYPTION_KEY` | – | Secret used to HMAC-SHA256 the tokens stored in Redis |
| `ALLOWED_HOSTS` | *(any)* | Comma separated `Host` header values accepted (port ignored) |
| `CORS_ORIGIN` | `*` | Browser origin allowed by CORS |
| `REDIS_URL` | `tcp://redis:6379` | Redis connection |
| `TOKEN_TTL_SECONDS` | `3600` | Upload token lifetime |
| `MAX_TOKEN_USES` | `100` | Upper bound for the uses of a single token |
| `MEDIA_ROOT` | `Media` | Storage directory |
| `READABLE_COLLECTIONS` | `Product,Website` | Collections that can be downloaded |
| `WRITABLE_COLLECTIONS` | `Product` | Collections that accept uploads |
| `MAX_UPLOAD_BYTES` | `52428800` | Maximum upload size (50 MB) |
| `MAX_PATH_DEPTH` | `6` | Maximum segments after `/Media/` |

## 3. Endpoints

| Method | Path | Description |
|---|---|---|
| `GET` | `/beep` | Health check, answers `boop` |
| `GET` | `/token/<uses>` | Issues `{session_id, csrf_token, max_uses}` valid for `<uses>` uploads (clamped to `MAX_TOKEN_USES`) |
| `GET` | `/Media/<collection>/<...>/<name>[.<ext>]` | Streams a file. Without extension the stored one is resolved automatically |
| `POST` | `/Media/<collection>/<...>/<name>` | Stores the raw body. Requires `X-Session-ID` and `X-CSRF-Token`; the extension comes from `Content-Type` |

Examples: `/Media/Product/whey-protein/banner`, `/Media/Product/whey-protein/1`, `/Media/Website/CSS/site.css`.

Accepted upload types: JPEG, PNG, WebP, GIF, BMP, TIFF, AVIF, MP4, WebM and PDF. Uploading a file replaces any previous version stored with another extension.

Errors are returned as JSON: `{"status": "not_found"}`, `invalid_token`, `unsupported_type`, `too_large`, `read_only_collection`, `invalid_path`, `forbidden`, `write_failed`.

## 4. Security model

- Only requests whose `Host` is in `ALLOWED_HOSTS` are served.
- Tokens are 256-bit values from OpenSSL's CSPRNG. Redis only stores their HMAC-SHA256, so a Redis dump cannot be used to upload.
- Every upload atomically consumes one use (`DECR`); exhausted tokens are deleted.
- Path segments only accept `[A-Za-z0-9_-]` plus a single inner `.`, which blocks traversal (`..`), hidden files and absolute paths.
- Files are written to a temporary name and renamed, so readers never see partial uploads.

## 5. Code layout

| File | Responsibility |
|---|---|
| `main.cpp` | Entry point |
| `src/config` | Environment / `.env` configuration |
| `src/security` | Random tokens, HMAC hashing, host validation |
| `src/token_store` | Token issuing and consumption in Redis |
| `src/media_storage` | Path validation, file lookup and atomic writes |
| `src/routes` | HTTP routes and CORS |

Build locally (requires Crow, redis-plus-plus, hiredis and OpenSSL): `./compile.sh`.

## 6. License

Free to use, modify and sell. Please keep a reference to this project and its author.
