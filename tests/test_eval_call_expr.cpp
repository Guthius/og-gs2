#include <catch2/catch_test_macros.hpp>

#include "ast.hpp"
#include "utils.hpp"

using namespace std;
using namespace og::gs2;

namespace {
    auto evaluate(const ast::expr &expr) -> expected_value {
        auto self = make_shared<basic_dictionary>();

        auto test_environment = environment();
        auto text_context = context(test_environment, self);

        return eval(text_context, expr);
    }
}

TEST_CASE("Call expressions evaluate as error when callee is not callable", "[eval]") {
    SECTION("Identifier that does not resolve to a callable") {
        auto expr = make_expr(ast::call_expr{
            .callee = make_expr(ast::identifier_expr{
                .name = "test_func",
            }),
        });

        auto result = evaluate(expr);

        REQUIRE(!result.has_value());
    }

    SECTION("Number literal") {
        auto expr = make_expr(ast::call_expr{
            .callee = make_expr(ast::number_expr{
                .value = 1.0,
            }),
        });

        auto result = evaluate(expr);

        REQUIRE(!result.has_value());
    }

    SECTION("String literal") {
        auto expr = make_expr(ast::call_expr{
            .callee = make_expr(ast::string_expr{
                .value = "test_func",
            }),
        });

        auto result = evaluate(expr);

        REQUIRE(!result.has_value());
    }
}
