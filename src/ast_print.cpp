#include "ast.hpp"

#include <iostream>

namespace og::gs2::ast {
    using namespace std;

    namespace {
        class printer_impl {
            ostream &out_;
            string prefix_;

          public:
            explicit printer_impl(ostream &out) : out_(out) {
            }

            void print(const stmt &list) {
                print_stmt(list);
            }

            void operator()(const unique_ptr<number_expr> &ex) {
                node(format("Number ({})", ex->value));
            }

            void operator()(const unique_ptr<string_expr> &ex) {
                node(format("String ({})", ex->value));
            }

            void operator()(const unique_ptr<boolean_expr> &ex) {
                node(format("Boolean ({})", ex->value ? "true" : "false"));
            }

            void operator()(const unique_ptr<array_expr> &ex) {
                node(format("Array ({} elements)", ex->elements.size()));
            }

            void operator()(const unique_ptr<null_expr> &ex) {
                node("Null");
            }

            void operator()(const unique_ptr<identifier_expr> &ex) {
                node(format("Identifier ({})", ex->name));
            }

            void operator()(const unique_ptr<member_expr> &ex) {
                node("Member");
                property("object", ex->object);
                property("member", ex->member, true);
            }

            void operator()(const unique_ptr<index_expr> &ex) {
                node("Index");
                property("object", ex->object);
                property("index", ex->index, true);
            }

            void operator()(const unique_ptr<call_expr> &ex) {
                node(format("Call ({} args)", ex->args.size()));
                property("callee", ex->callee, ex->args.empty());
                property_set("arg", ex->args);
            }

            void operator()(const unique_ptr<unary_expr> &ex) {
                node(format("Unary ({}, {})", token_kind_string(ex->op), ex->prefix ? "prefix" : "postfix"));
                property("operand", ex->operand, true);
            }

            void operator()(const unique_ptr<binary_expr> &ex) {
                node(format("Binary ({})", token_kind_string(ex->op)));
                property("left", ex->left);
                property("right", ex->right, true);
            }

            void operator()(const unique_ptr<ternary_expr> &ex) {
                node(format("Ternary"));
                property("condition", ex->condition);
                property("then", ex->then_branch);
                property("else", ex->else_branch, true);
            }

            void operator()(const unique_ptr<assign_expr> &ex) {
                node(format("Assign ({})", token_kind_string(ex->op)));
                property("target", ex->target);
                property("value", ex->value, true);
            }

            void operator()(const unique_ptr<new_expr> &ex) {
                auto has_body = ex->body.size() > 0;

                node(format("New ({} args{})", ex->args.size(), has_body ? " with body" : ""));
                property("type", ex->object_name, ex->args.empty() && !has_body);
                property_set("arg", ex->args, has_body);

                if (has_body) {
                    property_set("body", ex->body);
                }
            }

            void operator()(const unique_ptr<range_expr> &ex) {
                node("Range");
                property("min", ex->min);
                property("max", ex->max, true);
            }

            void operator()(const unique_ptr<in_expr> &ex) {
                node("In");
                property("value", ex->value);
                property("range", ex->range, true);
            }

            void operator()(const unique_ptr<scope_resolution_expr> &ex) {
                node("ScopeMember");
                property("scope", ex->scope);
                property("member", ex->member, true);
            }

            void operator()(const unique_ptr<block_stmt> &st) {
                node(format("Block ({} stmts)", st->body.size()));
                print_stmt_list(st->body);
            }

            void operator()(const unique_ptr<expr_stmt> &st) {
                node("Expr");
                property("expr", st->expression, true);
            }

            void operator()(const unique_ptr<if_stmt> &st) {
                const auto has_else = st->else_branch.has_value();

                node("If");
                property("condition", st->condition);
                property("then", st->then_branch, has_else);

                if (has_else) {
                    property("else", *st->else_branch, true);
                }
            }

            void operator()(const unique_ptr<while_stmt> &st) {
                node("While");
                property("condition", st->condition);
                property("body", st->body, true);
            }

            void operator()(const unique_ptr<for_stmt> &st) {
                node("For");

                if (st->initializer) {
                    property("initializer", *st->initializer);
                }

                if (st->condition) {
                    property("condition", *st->condition);
                }

                if (st->post) {
                    property("post", *st->post);
                }

                property("body", st->body, true);
            }

            void operator()(const unique_ptr<foreach_stmt> &st) {
                node("Foreach");
                property("variable", st->variable);
                property("iterable", st->iterable);
                property("body", st->body, true);
            }

            void operator()(const unique_ptr<with_stmt> &st) {
                node("With");
                property("object", st->object);
                property("body", st->body);
            }

            void operator()(const unique_ptr<return_stmt> &st) {
                node("Return");

                if (st->value) {
                    property("value", *st->value, true);
                }
            }

            void operator()(const unique_ptr<break_stmt> &st) {
                node("Break");
            }

            void operator()(const unique_ptr<continue_stmt> &st) {
                node("Continue");
            }

            void operator()(const unique_ptr<function_stmt> &st) {
                string params;
                for (size_t i = 0; i < st->params.size(); ++i) {
                    if (i > 0) {
                        params += ", ";
                    }

                    params += st->params[i];
                }

                node(format("Function {}({})", st->name, params));
                property("body", st->body);
            }

            void operator()(const unique_ptr<switch_stmt> &st) {
                node("Switch");
                property("operand", st->operand, true);
            }

            void operator()(const unique_ptr<enum_stmt> &st) {
                node(format("Enum ({} entries)", st->entries.size()));

                for (size_t i = 0; i < st->entries.size(); ++i) {
                    auto last = (i == st->entries.size() - 1);
                    auto ctx = child(format("{}", st->entries[i].name), last);
                    if (st->entries[i].value) {
                        out_ << " = ";
                        print_expr(*st->entries[i].value);
                    } else {
                        out_ << '\n';
                    }
                }
            }

            void operator()(const unique_ptr<do_while_stmt> &st) {
                node("DoWhile");
                property("condition", st->condition);
                property("body", st->body, true);
            }

          private:
            class child_scope {
                printer_impl &printer_;
                string saved_prefix_;

              public:
                child_scope(printer_impl &print, string_view label, bool last) : printer_(print), saved_prefix_(print.prefix_) {
                    printer_.out_
                        << printer_.prefix_
                        << (last ? "└── " : "├── ")
                        << label;

                    printer_.prefix_ += last ? "    " : "│   ";
                }

                ~child_scope() {
                    printer_.prefix_ = saved_prefix_;
                }
            };

            auto child(string_view label, bool last) -> child_scope {
                return {*this, label, last};
            }

            void node(string_view label) {
                out_ << label << "\n";
            }

            void property(string_view label, const expr &expr, const bool last = false) {
                auto ctx = child(format("{}: ", label), last);
                print_expr(expr);
            }

            void property(string_view label, const stmt &stmt, const bool last = false) {
                auto ctx = child(format("{}: ", label), last);
                print_stmt(stmt);
            }

            void property_set(string_view label, const expr_list &list, bool after_last = false) {
                for (size_t i = 0; i < list.size(); ++i) {
                    auto last = (i == list.size() - 1) && !after_last;
                    auto ctx = child(format("{}{} ", label, i), last);

                    print_expr(list[i]);
                }
            }

            void body(string_view label, const expr_list &list) {
                auto ctx = child(format("{} ({} exprs)", label, list.size()), true);
                out_ << '\n';
                property_set("expr", list);
            }

            void body(string_view label, const block_stmt &list) {
                if (list.body.empty()) {
                    return;
                }

                auto ctx = child(format("{} ({} stmts)", label, list.body.size()), true);
                out_ << '\n';
                print_stmt_list(list.body);
            }

            void print_expr(const expr &ex) {
                visit(*this, ex);
            }

            void print_stmt(const stmt &st) {
                visit(*this, st);
            }

            void print_stmt_list(const stmt_list &list) {
                for (size_t i = 0; i < list.size(); ++i) {
                    auto ctx = child("", i == list.size() - 1);

                    print_stmt(list[i]);
                }
            }
        };
    }

    void print(std::ostream &os, const stmt &stmt) {
        printer_impl printer(os);

        printer.print(stmt);
    }
}
