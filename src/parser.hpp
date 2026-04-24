#pragma once

#include "ast.hpp"

namespace og::gs2 {
    struct parse_error {
        std::string message;
        source_position position;
    };

    using parse_result = std::expected<ast::stmt, parse_error>;

    auto parse(const tokens &toks) -> parse_result;
}
