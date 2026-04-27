#pragma once

#include "ast.hpp"
#include "context.hpp"

namespace og::gs2 {
    auto eval(context &context, const ast::expr &expr) -> expected_value;
}
