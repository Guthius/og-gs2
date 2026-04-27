#include "eval.hpp"

#include <expected>
#include <limits>
#include <variant>

using namespace std;

namespace og::gs2 {
    namespace {
        template <class... Ts>
        struct overloaded : Ts... {
            using Ts::operator()...;
        };

        enum class flow_kind : uint8_t {
            none,
            break_,
            continue_,
            return_,
        };

        struct state {
            flow_kind flow = flow_kind::none;
            value return_value;

            auto consume(flow_kind check) -> bool {
                if (flow != check) {
                    return false;
                }
                flow = flow_kind::none;
                return true;
            }
        };

        auto eval(context &context, state &state, const ast::stmt &stmt) -> expected_void;

        auto eval_block(context &context, state &state, const unique_ptr<ast::block_stmt> &stmt) -> expected_void {
            for (const auto &child : stmt->body) {
                auto result = eval(context, child);
                if (!result) {
                    return result;
                }

                if (state.flow != flow_kind::none) {
                    return {};
                }
            }

            return {};
        };

        auto eval_expr(context &context, state &state, const unique_ptr<ast::expr_stmt> &stmt) -> expected_void {
            auto result = eval(context, stmt->expression);
            if (!result) {
                return unexpected(result.error());
            }

            return {};
        }

        auto eval_if(context &context, state &state, const unique_ptr<ast::if_stmt> &stmt) -> expected_void {
            auto condition = eval(context, stmt->condition);
            if (!condition) {
                return unexpected(condition.error());
            }

            if (to_bool(*condition)) {
                return eval(context, state, stmt->then_branch);
            }

            if (stmt->else_branch) {
                return eval(context, state, *stmt->else_branch);
            }

            return {};
        }

        auto eval_while(context &context, state &state, const unique_ptr<ast::while_stmt> &stmt) -> expected_void {
            while (true) {
                auto condition = eval(context, stmt->condition);
                if (!condition) {
                    return unexpected(condition.error());
                }

                if (!to_bool(*condition)) {
                    break;
                }

                auto result = eval(context, state, stmt->body);
                if (!result) {
                    return result;
                }

                if (state.consume(flow_kind::break_)) {
                    break;
                }

                if (state.consume(flow_kind::continue_)) {
                    continue;
                    ;
                }

                if (state.flow == flow_kind::return_) {
                    return {};
                }
            }

            return {};
        }

        auto eval_for(context &context, state &state, const unique_ptr<ast::for_stmt> &stmt) -> expected_void {
            if (stmt->initializer) {
                auto result = eval(context, *stmt->initializer);
                if (!result) {
                    return unexpected(result.error());
                }
            }

            while (true) {
                if (stmt->condition) {
                    auto condition = eval(context, *stmt->condition);
                    if (!condition) {
                        return unexpected(condition.error());
                    }

                    if (!to_bool(*condition)) {
                        break;
                    }
                }

                auto result = eval(context, state, stmt->body);
                if (!result) {
                    return result;
                }

                if (state.consume(flow_kind::break_)) {
                    break;
                }

                if (state.consume(flow_kind::continue_)) {
                    continue;
                }

                if (state.flow == flow_kind::return_) {
                    return {};
                }

                if (stmt->post) {
                    auto result = eval(context, *stmt->post);
                    if (!result) {
                        return unexpected(result.error());
                    }
                }
            }

            return {};
        }

        auto eval_foreach(context &context, state &state, const unique_ptr<ast::foreach_stmt> &stmt) -> expected_void {
            auto iterable = eval(context, stmt->iterable);
            if (!iterable) {
                return unexpected(iterable.error());
            }

            auto lvalue = resolve_lvalue(context, stmt->variable);
            if (!lvalue) {
                return unexpected(lvalue.error());
            }

            auto iterate = [&](const vector<value> &elements) -> expected_void {
                for (const auto &element : elements) {
                    set(*lvalue, element);

                    auto result = eval(context, state, stmt->body);
                    if (!result) {
                        return result;
                    }

                    if (state.consume(flow_kind::break_)) {
                        break;
                    }

                    if (state.consume(flow_kind::continue_)) {
                        continue;
                    }

                    if (state.flow == flow_kind::return_) {
                        return {};
                    }
                }

                return {};
            };

            if (auto *array = get_if<array_ptr>(&*iterable)) {
                return iterate((*array)->elements);
            }

            return {};
        }

        auto eval_with(context &context, state &state, const unique_ptr<ast::with_stmt> &stmt) -> expected_void {
            auto object = eval(context, stmt->object);
            if (!object) {
                return unexpected(object.error());
            }

            // TODO: Implement

            return {};
        }

        auto eval_return(context &context, state &state, const unique_ptr<ast::return_stmt> &stmt) -> expected_void {
            if (stmt->value) {
                auto value = eval(context, *stmt->value);
                if (!value) {
                    return unexpected(value.error());
                }

                state.return_value = std::move(*value);
            }

            state.flow = flow_kind::return_;

            return {};
        }

        auto eval_break(context &context, state &state, const unique_ptr<ast::break_stmt> &stmt) -> expected_void {
            state.flow = flow_kind::break_;

            return {};
        }

        auto eval_continue(context &context, state &state, const unique_ptr<ast::continue_stmt> &stmt) -> expected_void {
            state.flow = flow_kind::continue_;

            return {};
        }

        auto eval_switch_at(context &context, state &state, const unique_ptr<ast::switch_stmt> &stmt, size_t index) -> expected_void {
            if (index >= stmt->items.size()) {
                return {};
            }

            for (size_t i = index; i < stmt->items.size(); ++i) {
                const auto &item = stmt->items[i];

                if (holds_alternative<ast::switch_stmt::label>(item)) {
                    continue;
                }

                auto result = eval(context, state, get<ast::stmt>(item));
                if (!result) {
                    return result;
                }

                if (state.consume(flow_kind::break_) || state.flow != flow_kind::none) {
                    return {};
                }
            }

            return {};
        }

        auto eval_switch(context &context, state &state, const unique_ptr<ast::switch_stmt> &stmt) -> expected_void {
            auto operand = eval(context, stmt->operand);
            if (!operand) {
                return unexpected(operand.error());
            }

            size_t default_index = numeric_limits<size_t>::max();

            for (size_t i = 0; i < stmt->items.size(); ++i) {
                const auto &item = stmt->items[i];

                if (const auto *label = get_if<ast::switch_stmt::label>(&item)) {
                    if (!label->value) {
                        default_index = i;
                        continue;
                    }

                    auto value = eval(context, *label->value);
                    if (!value) {
                        return unexpected(value.error());
                    }

                    if (values_equal(*operand, *value)) {
                        return eval_switch_at(context, state, stmt, i + 1);
                    }
                }
            }

            return eval_switch_at(context, state, stmt, default_index);
        }

        auto eval_enum(context &context, state &state, const unique_ptr<ast::enum_stmt> &stmt) -> expected_void {
            auto dictionary = [&]() -> dictionary_ptr {
                if (!stmt->name) {
                    return context.get_dictionary();
                }

                const auto new_dictionary = make_shared<basic_dictionary>();

                context.get_dictionary()->put(*stmt->name, new_dictionary);

                return new_dictionary;
            }();

            auto counter = 0.0;

            for (const auto &entry : stmt->entries) {
                value entry_value;

                if (entry.value) {
                    auto result = eval(context, *entry.value);
                    if (!result) {
                        return unexpected(result.error());
                    }

                    entry_value = *result;
                    if (auto *actual_value = get_if<double>(&entry_value)) {
                        counter = *actual_value + 1;
                    }
                } else {
                    entry_value = value{counter++};
                }

                dictionary->put(entry.name, entry_value);
            }

            return {};
        }

        auto eval_do_while(context &context, state &state, const unique_ptr<ast::do_while_stmt> &stmt) -> expected_void {
            do {
                auto result = eval(context, state, stmt->body);
                if (!result) {
                    return result;
                }

                if (state.consume(flow_kind::break_)) {
                    break;
                }

                if (state.consume(flow_kind::continue_)) {
                }

                if (state.flow == flow_kind::return_) {
                    return {};
                }

                auto condition = eval(context, stmt->condition);
                if (!condition) {
                    return unexpected(condition.error());
                }

                if (!to_bool(*condition)) {
                    break;
                }
            } while (true);

            return {};
        }

        auto eval(context &context, state &state, const ast::stmt &stmt) -> expected_void {
            return visit(
                overloaded{
                    [&](const unique_ptr<ast::block_stmt> &stmt) -> expected_void { return eval_block(context, state, stmt); },
                    [&](const unique_ptr<ast::expr_stmt> &stmt) -> expected_void { return eval_expr(context, state, stmt); },
                    [&](const unique_ptr<ast::if_stmt> &stmt) -> expected_void { return eval_if(context, state, stmt); },
                    [&](const unique_ptr<ast::while_stmt> &stmt) -> expected_void { return eval_while(context, state, stmt); },
                    [&](const unique_ptr<ast::for_stmt> &stmt) -> expected_void { return eval_for(context, state, stmt); },
                    [&](const unique_ptr<ast::foreach_stmt> &stmt) -> expected_void { return eval_foreach(context, state, stmt); },
                    [&](const unique_ptr<ast::with_stmt> &stmt) -> expected_void { return eval_with(context, state, stmt); },
                    [&](const unique_ptr<ast::return_stmt> &stmt) -> expected_void { return eval_return(context, state, stmt); },
                    [&](const unique_ptr<ast::break_stmt> &stmt) -> expected_void { return eval_break(context, state, stmt); },
                    [&](const unique_ptr<ast::continue_stmt> &stmt) -> expected_void { return eval_continue(context, state, stmt); },
                    [&](const unique_ptr<ast::function_stmt> &stmt) -> expected_void { return {}; },
                    [&](const unique_ptr<ast::switch_stmt> &stmt) -> expected_void { return eval_switch(context, state, stmt); },
                    [&](const unique_ptr<ast::enum_stmt> &stmt) -> expected_void { return eval_enum(context, state, stmt); },
                    [&](const unique_ptr<ast::do_while_stmt> &stmt) -> expected_void { return eval_do_while(context, state, stmt); },
                },
                stmt);
        }
    }

    auto eval(context &context, const ast::stmt &stmt) -> expected_void {
        state exec_state{};

        return eval(context, exec_state, stmt);
    }

    auto eval(context &context, const ast::function_stmt &fn, const values &args) -> expected_value {
        state exec_state{};

        auto temp = make_shared<basic_dictionary>();
        for (size_t i = 0; i < fn.params.size(); ++i) {
            temp->put(fn.params[i], i < args.size() ? args[i] : value{});
        }

        context.get_dictionary()->put("temp", temp);

        auto result = eval(context, exec_state, fn.body);

        context.get_dictionary()->erase("temp");

        if (!result) {
            return unexpected(result.error());
        }

        value return_value;
        if (exec_state.flow == flow_kind::return_) {
            return std::move(exec_state.return_value);
        }

        return {};
    }
}
