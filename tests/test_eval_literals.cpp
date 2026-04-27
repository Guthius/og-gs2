#include <catch2/catch_test_macros.hpp>

#include "ast.hpp"
#include "eval.hpp"

using namespace std;
using namespace og::gs2;

namespace {
    template <typename T>
    constexpr auto make_expr(T value) -> ast::expr {
        return make_unique<T>(std::move(value));
    }

    auto evaluate(const ast::expr &expr) -> expected_value {
        auto self = make_shared<basic_dictionary>();

        auto test_environment = environment();
        auto text_context = context(test_environment, self);

        return eval(text_context, expr);
    }

    template <typename T>
    auto has_value(const expected_value &result, const T &expected_value) -> bool {
        if (!result.has_value()) {
            return false;
        }

        if (auto *value = get_if<T>(&*result)) {
            return (*value) == expected_value;
        }

        return false;
    }
}

TEST_CASE("Number expression evaluates to correct value", "[eval]") {
    static constexpr auto test_value = 12.5;

    auto expr = make_expr(ast::number_expr{
        .value = test_value,
    });

    REQUIRE(has_value(evaluate(expr), test_value));
}

TEST_CASE("String expression evaluates to correct value", "[eval]") {
    static constexpr string test_value = "Hello World";

    auto expr = make_expr(ast::string_expr{
        .value = test_value,
    });

    REQUIRE(has_value(evaluate(expr), test_value));
}

TEST_CASE("True expression evaluates to correct value", "[eval]") {
    auto expr = make_expr(ast::boolean_expr{
        .value = true,
    });

    REQUIRE(has_value(evaluate(expr), 1.0));
}

TEST_CASE("False expression evaluates to correct value", "[eval]") {
    auto expr = make_expr(ast::boolean_expr{
        .value = false,
    });

    REQUIRE(has_value(evaluate(expr), 0.0));
}
