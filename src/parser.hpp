#pragma once

#include "ast.hpp"
#include "lexer.hpp"

#include <expected>

namespace og::gs2 {
    struct parse_error {
        std::string message;
        source_position position;
    };

    using parse_result = std::expected<ast::unit, parse_error>;

    auto parse(const tokens &toks) -> parse_result;
}
