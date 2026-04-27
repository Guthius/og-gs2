#include "ast.hpp"

#include <sstream>

using namespace std;

namespace og::gs2::ast {
    namespace {
        template <class... Ts>
        struct overloaded : Ts... {
            using Ts::operator()...;
        };

        auto operator<<(ostream &os, const token_kind kind) -> ostream & {
            switch (kind) {
            case token_kind::op_add:             os << '+'; break;
            case token_kind::op_subtract:        os << '-'; break;
            case token_kind::op_multiply:        os << '*'; break;
            case token_kind::op_divide:          os << '/'; break;
            case token_kind::op_modulo:          os << '%'; break;
            case token_kind::op_exponent:        os << '^'; break;
            case token_kind::op_concat:          os << '@'; break;
            case token_kind::op_concat_spc:      os << "SPC"; break;
            case token_kind::op_concat_nl:       os << "NL"; break;
            case token_kind::op_lt:              os << '<'; break;
            case token_kind::op_lte:             os << "<="; break;
            case token_kind::op_gt:              os << '>'; break;
            case token_kind::op_gte:             os << ">="; break;
            case token_kind::op_eq:              os << "=="; break;
            case token_kind::op_neq:             os << "!="; break;
            case token_kind::op_and:             os << "&&"; break;
            case token_kind::op_or:              os << "||"; break;
            case token_kind::op_not:             os << '!'; break;
            case token_kind::op_assign:          os << '='; break;
            case token_kind::op_assign_add:      os << "+="; break;
            case token_kind::op_assign_subtract: os << "-="; break;
            case token_kind::op_assign_multiply: os << "*="; break;
            case token_kind::op_assign_divide:   os << "/="; break;
            case token_kind::op_assign_modulo:   os << "%="; break;
            case token_kind::op_assign_concat:   os << "@="; break;
            case token_kind::op_increment:       os << "++"; break;
            case token_kind::op_decrement:       os << "--"; break;
            default:                             break;
            }

            return os;
        }

        void write(ostream &os, const unique_ptr<ast::string_expr> &expr) {
            os << '"';
            for (const char ch : expr->value) {
                switch (ch) {
                case '\t': os << "\\t"; break;
                case '\n': os << "\\n"; break;
                case '\r': os << "\\r"; break;
                case '\\': os << "\\"; break;
                case '"':  os << "\\\""; break;
                default:   os << ch; break;
                }
            }
            os << '"';
        }

        void write(ostream &os, const unique_ptr<ast::array_expr> &expr) {
            os << '{';
            for (size_t i = 0; i < expr->elements.size(); ++i) {
                if (i > 0) {
                    os << ", ";
                }

                os << expr->elements[i];
            }
            os << '}';
        }

        void write(ostream &os, const unique_ptr<ast::member_expr> &expr) {
            os << expr->object << '.' << expr->member;
        }

        void write(ostream &os, const unique_ptr<ast::index_expr> &expr) {
            os << expr->object << '[' << expr->index << ']';
        }

        void write(ostream &os, const unique_ptr<ast::call_expr> &expr) {
            os << expr->callee << '(';
            for (size_t i = 0; i < expr->args.size(); ++i) {
                if (i > 0) {
                    os << ", ";
                }
                os << expr->args[i];
            }
            os << ')';
        }

        void write(ostream &os, const unique_ptr<ast::unary_expr> &expr) {
            if (expr->prefix) {
                os << expr->op;
                os << expr->operand;
                return;
            }

            os << expr->operand;
            os << expr->op;
        }

        void write(ostream &os, const unique_ptr<ast::binary_expr> &expr) {
            os << expr->left << ' ' << expr->op << ' ' << expr->right;
        }

        void write(ostream &os, const unique_ptr<ast::ternary_expr> &expr) {
            os << expr->condition << " ? " << expr->then_branch << " : " << expr->else_branch;
        }

        void write(ostream &os, const unique_ptr<ast::assign_expr> &expr) {
            os << expr->target << expr->op << expr->value;
        }

        void write(ostream &os, const unique_ptr<ast::new_expr> &expr) {
            os << "new " << expr->object_name << '(';
            for (size_t i = 0; i < expr->args.size(); ++i) {
                if (i > 0) {
                    os << ", ";
                }
                os << to_string(expr->args[i]);
            }
            os << ')';

            if (!expr->body.empty()) {
                os << '{';
                for (size_t i = 0; i < expr->body.size(); ++i) {
                    if (i > 0) {
                        os << "; ";
                    }
                    os << expr->body[i];
                }
                os << '}';
            }
        }

        void write(ostream &os, const unique_ptr<ast::range_expr> &expr) {
            os << '|' << expr->min << ", " << expr->max << '|';
        }

        void write(ostream &os, const unique_ptr<ast::in_expr> &expr) {
            os << expr->value << " in " << expr->range;
        }

        void write(ostream &os, const unique_ptr<ast::scope_resolution_expr> &expr) {
            os << expr->scope << "::" << expr->member;
        }
    }

    auto to_string(const ast::expr &expr) -> string {
        ostringstream os;

        os << expr;

        return os.str();
    }

    auto operator<<(ostream &os, const ast::expr &expr) -> ostream & {
        visit(
            overloaded{
                [&](const unique_ptr<ast::number_expr> &expr) -> void { os << std::to_string(expr->value); },
                [&](const unique_ptr<ast::string_expr> &expr) -> void { write(os, expr); },
                [&](const unique_ptr<ast::boolean_expr> &expr) -> void { os << (expr->value ? "true" : "false"); },
                [&](const unique_ptr<ast::array_expr> &expr) -> void { write(os, expr); },
                [&](const unique_ptr<ast::null_expr> &expr) -> void { os << "null"; },
                [&](const unique_ptr<ast::identifier_expr> &expr) -> void { os << expr->name; },
                [&](const unique_ptr<ast::member_expr> &expr) -> void { write(os, expr); },
                [&](const unique_ptr<ast::index_expr> &expr) -> void { write(os, expr); },
                [&](const unique_ptr<ast::call_expr> &expr) -> void { write(os, expr); },
                [&](const unique_ptr<ast::unary_expr> &expr) -> void { write(os, expr); },
                [&](const unique_ptr<ast::binary_expr> &expr) -> void { write(os, expr); },
                [&](const unique_ptr<ast::ternary_expr> &expr) -> void { write(os, expr); },
                [&](const unique_ptr<ast::assign_expr> &expr) -> void { write(os, expr); },
                [&](const unique_ptr<ast::new_expr> &expr) -> void { write(os, expr); },
                [&](const unique_ptr<ast::range_expr> &expr) -> void { write(os, expr); },
                [&](const unique_ptr<ast::in_expr> &expr) -> void { write(os, expr); },
                [&](const unique_ptr<ast::scope_resolution_expr> &expr) -> void { write(os, expr); },
            },
            expr);

        return os;
    }
}
