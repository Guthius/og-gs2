#include "eval.hpp"

#include <cmath>

using namespace std;

namespace og::gs2 {
    namespace {
        constexpr value null_value = value{};
        constexpr value zero_value = value{0.0};
        constexpr value false_value = zero_value;
        constexpr value true_value = value{1.0};

        template <class... Ts>
        struct overloaded : Ts... {
            using Ts::operator()...;
        };

        struct member_ref {
            dictionary_ptr dictionary;
            string key;
        };

        struct array_ref {
            array_ptr array;
            size_t index;
        };

        using lvalue = variant<member_ref, array_ref>;

        using expected_lvalue = expected<lvalue, error>;

        auto resolve_identifier(context &context, const unique_ptr<ast::identifier_expr> &expr) -> expected_lvalue {
            const auto &name = expr->name;

            auto dictionary = context.get_scope(name);
            if (!dictionary) {
                return unexpected(dictionary.error());
            }

            return member_ref{
                .dictionary = *dictionary,
                .key = name,
            };
        }

        auto resolve_member_key(context &context, const ast::expr &expr) -> expected<string, error> {
            if (const auto *identifier = get_if<unique_ptr<ast::identifier_expr>>(&expr)) {
                return (*identifier)->name;
            }

            auto member = eval(context, expr);
            if (!member) {
                return unexpected(member.error());
            }

            return to_string(*member);
        }

        auto resolve_member(context &context, const unique_ptr<ast::member_expr> &expr) -> expected_lvalue {
            auto object = eval(context, expr->object);
            if (!object) {
                return unexpected(object.error());
            }

            auto key = resolve_member_key(context, expr->member);
            if (!key) {
                return unexpected(key.error());
            }

            if (auto *dictionary = get_if<dictionary_ptr>(&*object)) {
                return member_ref{
                    .dictionary = *dictionary,
                    .key = std::move(*key),
                };
            }

            return unexpected(error{
                .source = error_source::interpreter,
                .kind = error_kind::runtime_error,
                .message = "member assignment requires an object",
                .position = expr->position,
            });
        }

        auto resolve_index(context &context, const unique_ptr<ast::index_expr> &expr) -> expected_lvalue {
            auto object = eval(context, expr->object);
            if (!object) {
                return unexpected(object.error());
            }

            auto index = eval(context, expr->index);
            if (!index) {
                return unexpected(index.error());
            }

            if (auto *array = get_if<array_ptr>(&*object)) {
                auto index_num = static_cast<size_t>(to_number(*index));
                if (index_num >= (*array)->elements.size()) {
                    (*array)->elements.resize(index_num + 1);
                }

                return array_ref{
                    .array = *array,
                    .index = index_num,
                };
            }

            return unexpected(error{
                .source = error_source::interpreter,
                .kind = error_kind::runtime_error,
                .message = "index target is not an array",
                .position = expr->position,
            });
        }

        auto resolve_lvalue(context &context, const ast::expr &expr) -> expected_lvalue {
            return visit(
                overloaded{
                    [&](const unique_ptr<ast::identifier_expr> &expr) -> expected_lvalue {
                        return resolve_identifier(context, expr);
                    },
                    [&](const unique_ptr<ast::member_expr> &expr) -> expected_lvalue {
                        return resolve_member(context, expr);
                    },
                    [&](const unique_ptr<ast::index_expr> &expr) -> expected_lvalue {
                        return resolve_index(context, expr);
                    },
                    [](const auto &) -> expected_lvalue {
                        return unexpected(error{
                            .source = error_source::interpreter,
                            .kind = error_kind::runtime_error,
                            .message = "expression is not assignable",
                        });
                    },
                },
                expr);
        }

        auto get(const lvalue &lvalue) -> value {
            return visit(
                overloaded{
                    [](const member_ref &ref) -> value {
                        return ref.dictionary->get(ref.key);
                    },
                    [](const array_ref &ref) -> value {
                        if (ref.index < ref.array->elements.size()) {
                            return ref.array->elements[ref.index];
                        }

                        return null_value;
                    }},
                lvalue);
        }

        void set(const lvalue &lvalue, value new_value) {
            visit(
                overloaded{
                    [&new_value](const member_ref &ref) -> void {
                        ref.dictionary->put(ref.key, std::move(new_value));
                    },
                    [&new_value](const array_ref &ref) -> void {
                        if (ref.index >= ref.array->elements.size()) {
                            ref.array->elements.resize(ref.index + 1);
                        }

                        ref.array->elements[ref.index] = std::move(new_value);
                    },
                },
                lvalue);
        }
    }

    namespace {
        auto eval_array(context &context, const unique_ptr<ast::array_expr> &expr) -> expected_value {
            auto arr = make_shared<array>();
            for (const auto &element : expr->elements) {
                auto element_value = eval(context, element);
                if (!element_value) {
                    return element_value;
                }

                arr->elements.push_back(std::move(*element_value));
            }

            return value{arr};
        }

        auto eval_identifier(context &context, const unique_ptr<ast::identifier_expr> &expr) -> expected_value {
            return context.get(expr->name);
        }

        auto eval_member(context &context, const unique_ptr<ast::member_expr> &expr) -> expected_value {
            auto object = eval(context, expr->object);
            if (!object) {
                return object;
            }

            auto key = resolve_member_key(context, expr->member);
            if (!key) {
                return unexpected(key.error());
            }

            if (auto *dict = get_if<dictionary_ptr>(&*object)) {
                if ((*dict)->contains(*key)) {
                    return (*dict)->get(*key);
                }

                return null_value;
            }

            return *object;
        }
    }

    auto eval_index(context &context, const unique_ptr<ast::index_expr> &expr) -> expected_value {
        auto object = eval(context, expr->object);
        if (!object) {
            return object;
        }

        auto index = eval(context, expr->index);
        if (!index) {
            return index;
        }

        if (auto *arr = get_if<array_ptr>(&*object)) {
            auto idx = static_cast<size_t>(to_number(*index));
            if (idx < (*arr)->elements.size()) {
                return (*arr)->elements[idx];
            }

            return null_value;
        }

        if (auto *dict = get_if<dictionary_ptr>(&*object)) {
            auto key = to_string(*index);
            if ((*dict)->contains(key)) {
                return (*dict)->get(key);
            }

            return null_value;
        }

        return null_value;
    }

    auto eval_call(context &context, const unique_ptr<ast::call_expr> &expr) -> expected_value {
        auto callee = eval(context, expr->callee);
        if (!callee) {
            return callee;
        }

        values args;
        for (const auto &val : expr->args) {
            auto arg = eval(context, val);
            if (!arg) {
                return arg;
            }

            args.push_back(std::move(*arg));
        }

        if (auto *callable = get_if<callable_ptr>(&*callee)) {
            return (*callable)->invoke(args);
        }

        return unexpected(error{
            .source = error_source::interpreter,
            .kind = error_kind::runtime_error,
            .message = format("expression '{}' is not callable", to_string(expr->callee)),
            .position = expr->position,
        });
    }

    auto eval_unary(context &context, const unique_ptr<ast::unary_expr> &expr) -> expected_value {
        if (expr->prefix) {
            if (expr->op == token_kind::op_increment ||
                expr->op == token_kind::op_decrement) {
                auto lvalue = resolve_lvalue(context, expr->operand);
                if (!lvalue) {
                    return unexpected(lvalue.error());
                }

                auto old_value = to_number(get(*lvalue));
                auto new_value = expr->op == token_kind::op_increment ? old_value + 1 : old_value - 1;

                set(*lvalue, value{new_value});

                return value{new_value};
            }

            auto operand = eval(context, expr->operand);
            switch (expr->op) {
            case token_kind::op_not:
                return to_bool(*operand) ? false_value : true_value;

            case token_kind::op_subtract:
                return value{-to_number(*operand)};

            default:
                return null_value;
            }
        }

        auto lvalue = resolve_lvalue(context, expr->operand);
        if (!lvalue) {
            return unexpected(lvalue.error());
        }

        auto old_value = to_number(get(*lvalue));
        auto new_value = expr->op == token_kind::op_increment ? old_value + 1 : old_value - 1;

        set(*lvalue, value{new_value});

        return value{old_value};
    }

    auto eval_logical_and(context &context, const unique_ptr<ast::binary_expr> &expr) -> expected_value {
        auto left = eval(context, expr->left);
        if (!left) {
            return left;
        }

        if (!to_bool(*left)) {
            return false_value;
        }

        auto right = eval(context, expr->right);
        if (!right) {
            return right;
        }

        return to_bool(*right) ? true_value : false_value;
    }

    auto eval_logical_or(context &context, const unique_ptr<ast::binary_expr> &expr) -> expected_value {
        auto left = eval(context, expr->left);
        if (!left) {
            return left;
        }

        if (to_bool(*left)) {
            return true_value;
        }

        auto right = eval(context, expr->right);
        if (!right) {
            return right;
        }

        return to_bool(*right) ? true_value : false_value;
    }

    auto eval_binary(context &context, const unique_ptr<ast::binary_expr> &expr) -> expected_value {
        if (expr->op == token_kind::op_and) {
            return eval_logical_and(context, expr);
        }

        if (expr->op == token_kind::op_or) {
            return eval_logical_or(context, expr);
        }

        auto left = eval(context, expr->left);
        if (!left) {
            return left;
        }

        auto right = eval(context, expr->right);
        if (!right) {
            return right;
        }

        switch (expr->op) {
        case token_kind::op_add:        return value{to_number(*left) + to_number(*right)};
        case token_kind::op_subtract:   return value{to_number(*left) - to_number(*right)};
        case token_kind::op_multiply:   return value{to_number(*left) * to_number(*right)};
        case token_kind::op_exponent:   return value(pow(to_number(*left), to_number(*right)));
        case token_kind::op_concat:     return value(to_string(*left) + to_string(*right));
        case token_kind::op_concat_spc: return value(to_string(*left) + " " + to_string(*right));
        case token_kind::op_concat_nl:  return value(to_string(*left) + "\n" + to_string(*right));
        case token_kind::op_eq:         return values_equal(*left, *right) ? true_value : false_value;
        case token_kind::op_neq:        return values_equal(*left, *right) ? false_value : true_value;
        case token_kind::op_lt:         return to_number(*left) < to_number(*right) ? true_value : false_value;
        case token_kind::op_lte:        return to_number(*left) <= to_number(*right) ? true_value : false_value;
        case token_kind::op_gt:         return to_number(*left) > to_number(*right) ? true_value : false_value;
        case token_kind::op_gte:        return to_number(*left) >= to_number(*right) ? true_value : false_value;

        case token_kind::op_divide:
            {
                auto divisor = to_number(*right);
                if (divisor == 0.0) {
                    return zero_value;
                }

                return value{to_number(*left) / divisor};
            }

        case token_kind::op_modulo:
            {
                auto divisor = to_number(*right);
                if (divisor == 0.0) {
                    return zero_value;
                }

                return value{fmod(to_number(*left), divisor)};
            }

        default: return null_value;
        }
    }

    auto eval_ternary(context &context, const unique_ptr<ast::ternary_expr> &expr) -> expected_value {
        auto condition = eval(context, expr->condition);
        if (!condition) {
            return condition;
        }

        return to_bool(*condition)
                   ? eval(context, expr->then_branch)
                   : eval(context, expr->else_branch);
    }

    auto eval_assign(context &context, const unique_ptr<ast::assign_expr> &expr) -> expected_value {
        auto lvalue = resolve_lvalue(context, expr->target);
        if (!lvalue) {
            return unexpected(lvalue.error());
        }

        auto right = eval(context, expr->value);
        if (!right) {
            return right;
        }

        value result;
        if (expr->op == token_kind::op_assign) {
            result = *right;
        } else {
            auto current_value = to_number(get(*lvalue));
            auto new_value = to_number(*right);

            switch (expr->op) {
            case token_kind::op_assign_add:      result = value{current_value + new_value}; break;
            case token_kind::op_assign_subtract: result = value{current_value - new_value}; break;
            case token_kind::op_assign_multiply: result = value{current_value + new_value}; break;
            case token_kind::op_assign_divide:   result = value{new_value != 0 ? current_value / new_value : 0}; break;
            case token_kind::op_assign_modulo:   result = value{new_value != 0 ? fmod(current_value, new_value) : 0}; break;

            case token_kind::op_assign_concat:
                result = value{to_string(get(*lvalue)) + to_string(*right)};
                break;

            default:
                result = *right;
            }
        }

        set(*lvalue, result);

        return result;
    }

    auto eval_new(context &context, const unique_ptr<ast::new_expr> &expr) -> expected_value {
        // TODO: Implement

        return null_value;
    }

    auto eval_in(context &context, const unique_ptr<ast::in_expr> &expr) -> expected_value {
        auto value = eval(context, expr->value);
        if (!value) {
            return value;
        }

        if (auto *range = get_if<unique_ptr<ast::range_expr>>(&expr->range)) {
            auto min = eval(context, (*range)->min);
            if (!min) {
                return min;
            }

            auto max = eval(context, (*range)->max);
            if (!max) {
                return max;
            }

            auto num = to_number(*value);

            return num >= to_number(*min) && num <= to_number(*max) ? true_value : false_value;
        }

        auto right = eval(context, expr->range);
        if (!right) {
            return right;
        }

        if (auto *array = get_if<array_ptr>(&*right)) {
            for (const auto &element : (*array)->elements) {
                if (values_equal(*value, element)) {
                    return true_value;
                }
            }
        }

        return false_value;
    }

    auto eval_scope_resolution(context &context, const unique_ptr<ast::scope_resolution_expr> &expr) -> expected_value {
        // TODO: Implement

        return null_value;
    }

    auto eval(context &context, const ast::expr &expr) -> expected_value {
        return visit(
            overloaded{
                [](const unique_ptr<ast::number_expr> &expr) -> expected_value { return value{expr->value}; },
                [](const unique_ptr<ast::string_expr> &expr) -> expected_value { return value{expr->value}; },
                [](const unique_ptr<ast::boolean_expr> &expr) -> expected_value { return value{expr->value ? 1.0 : 0.0}; },
                [](const unique_ptr<ast::null_expr> &expr) -> expected_value { return null_value; },

                [&](const unique_ptr<ast::array_expr> &expr) -> expected_value { return eval_array(context, expr); },
                [&](const unique_ptr<ast::identifier_expr> &expr) -> expected_value { return eval_identifier(context, expr); },
                [&](const unique_ptr<ast::member_expr> &expr) -> expected_value { return eval_member(context, expr); },
                [&](const unique_ptr<ast::index_expr> &expr) -> expected_value { return eval_index(context, expr); },
                [&](const unique_ptr<ast::call_expr> &expr) -> expected_value { return eval_call(context, expr); },
                [&](const unique_ptr<ast::unary_expr> &expr) -> expected_value { return eval_unary(context, expr); },
                [&](const unique_ptr<ast::binary_expr> &expr) -> expected_value { return eval_binary(context, expr); },
                [&](const unique_ptr<ast::ternary_expr> &expr) -> expected_value { return eval_ternary(context, expr); },
                [&](const unique_ptr<ast::assign_expr> &expr) -> expected_value { return eval_assign(context, expr); },
                [&](const unique_ptr<ast::new_expr> &expr) -> expected_value { return eval_new(context, expr); },
                [&](const unique_ptr<ast::range_expr> &expr) -> expected_value { return null_value; },
                [&](const unique_ptr<ast::in_expr> &expr) -> expected_value { return eval_in(context, expr); },
                [&](const unique_ptr<ast::scope_resolution_expr> &expr) -> expected_value { return eval_scope_resolution(context, expr); },
            },
            expr);
    }
}
