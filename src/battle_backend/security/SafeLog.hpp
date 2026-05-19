#pragma once

#include <string>
#include <string_view>

namespace battle::backend::security {

// Placeholder for a future centralized redaction helper.
// Real logging code should pass sensitive values through redaction before output.
inline std::string redactForLog(std::string_view value) {
    if (value.empty()) {
        return {};
    }
    return "<redacted>";
}

} // namespace battle::backend::security
