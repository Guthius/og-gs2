#pragma once

#include "ast.hpp"
#include "context.hpp"

namespace og::gs2 {
    struct member_ref {
        dictionary_ptr dictionary;
        std::string key;
    };

    struct array_ref {
        array_ptr array;
        size_t index;
    };

    using lvalue = std::variant<member_ref, array_ref>;

    using expected_lvalue = std::expected<lvalue, error>;

    auto eval(context &context, const ast::expr &expr) -> expected_value;
    auto eval(context &context, const ast::stmt &stmt) -> expected_void;
    auto eval(context &context, const ast::function_stmt &fn, const values &args) -> expected_value;

    auto resolve_lvalue(context &context, const ast::expr &expr) -> expected_lvalue;

    auto get(const lvalue &lvalue) -> value;
    void set(const lvalue &lvalue, value new_value);
}
