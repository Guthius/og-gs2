#pragma once

#include "ast.hpp"
#include "scope.hpp"

namespace og::gs2 {
    auto eval(scope &scope, const ast::expr &expr) -> expected_value;
}
