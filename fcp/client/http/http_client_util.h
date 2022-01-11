/*
 * Copyright 2021 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef FCP_CLIENT_HTTP_HTTP_CLIENT_UTIL_H_
#define FCP_CLIENT_HTTP_HTTP_CLIENT_UTIL_H_

#include <optional>
#include <string>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "fcp/client/http/http_client.h"

namespace fcp {
namespace client {
namespace http {

inline static constexpr char kHttpsScheme[] = "https://";
inline static constexpr char kAcceptEncodingHdr[] = "Accept-Encoding";
inline static constexpr char kContentLengthHdr[] = "Content-Length";
inline static constexpr char kContentEncodingHdr[] = "Content-Encoding";
inline static constexpr char kTransferEncodingHdr[] = "Transfer-Encoding";
// The "Transfer-Encoding" header value when the header is present but indicates
// that no encoding was actually applied.
inline static constexpr char kIdentityEncodingHdrValue[] = "identity";

// A non-exhaustive enumeration of common HTTP response codes.
// Note this is purposely *not* an "enum class", to allow easy comparisons
// against the int codes returned by `HttpResponse`.
enum HttpResponseCode {
  kHttpOk = 200,
  kHttpBadRequest = 400,
  kHttpUnauthorized = 401,
  kHttpForbidden = 403,
  kHttpNotFound = 404,
  kHttpConflict = 409,
  kHttpTooManyRequests = 429,
  kHttpClientClosedRequest = 499,
  kHttpInternalServerError = 500,
  kHttpNotImplemented = 501,
  kHttpServiceUnavailable = 503,
  kHttpGatewayTimeout = 504,
};

// Converts an HTTP response code into an `absl::Status` (incl. an error message
// with the original HTTP code).
absl::Status ConvertHttpCodeToStatus(int code);

// Finds the header value for header with name `needle` in a list of headers
// (incl. normalizing the header names to lowercase before doing any
// comparisons). Note that this returns the first matching header value (rather
// than coalescing repeated header values as per RFC2616 section 4.2), so it
// must only be used for headers for which only a single value is expected.
// Returns an empty optional if no header value was found.
std::optional<std::string> FindHeader(const HeaderList& headers,
                                      absl::string_view needle);

// Creates a URI out of a base URI and a suffix.
//
// The `base_uri` argument is expected to be a valid fully qualified URI on its
// own (i.e. having non-empty scheme and authority/host segments, and possibly a
// path segment as well), although this function does not validate this. It may
// or may not end with a trailing '/'. may or may not
//
// The `uri_suffix` argument must always either be empty or start with a
// leading '/'.
//
// Returns a URI formed by joining the two arguments, ensuring there is
// always a single '/' in between the two parts. E.g. if both `base_uri` ends
// with a '/' and `uri_suffix` start with a '/', then the two '/' characters
// will be normalized into a single one. If `uri_suffix` is empty, then the
// resulting URI will always end in a '/'.
absl::StatusOr<std::string> JoinBaseUriWithSuffix(absl::string_view base_uri,
                                                  absl::string_view uri_suffix);

}  // namespace http
}  // namespace client
}  // namespace fcp

#endif  // FCP_CLIENT_HTTP_HTTP_CLIENT_UTIL_H_
