#include "script.hpp"

#include "context.hpp"
#include "parser.hpp"

#include <filesystem>
#include <format>
#include <fstream>

using namespace std;

namespace og::gs2 {
    namespace {
        struct script_impl : script {
            using function_stmt_ptr = const ast::function_stmt *;

            environment &env;
            ast::stmt root;
            registry<function_stmt_ptr> functions;

            script_impl(environment &env, ast::stmt stmt) : env(env), root(std::move(stmt)) {
                detect_functions(root);
            }

            auto has_function(string_view name) const -> bool override {
                return functions.contains(name);
            }

            auto call(string_view function_name, dictionary_ptr self, const values &args = {}) const -> expected_value override {
                auto ctx = context(env, std::move(self));

                // TODO: Implement

                return {};
            }

            void detect_functions(const ast::stmt &stmt) {
                visit(
                    [this](const auto &node_ptr) -> auto {
                        using T = decay_t<decltype(node_ptr)>;

                        if constexpr (is_same_v<T, unique_ptr<ast::block_stmt>>) {
                            for (const auto &child : node_ptr->body) {
                                detect_functions(child);
                            }
                        }

                        if constexpr (is_same_v<T, unique_ptr<ast::function_stmt>>) {
                            auto key = node_ptr->name;

                            functions[key] = node_ptr.get();
                        }
                    },
                    stmt);
            }
        };
    }

    auto load_script(environment &env, const tokens &tokens) -> script_result {
        auto stmt = parse(tokens);
        if (!stmt) {
            return unexpected(stmt.error());
        }

        return make_shared<script_impl>(env, std::move(*stmt));
    }

    auto load_script(environment &env, istream &is) -> script_result {
        auto tokens = tokenize(is);
        if (!tokens) {
            return unexpected(tokens.error());
        }

        return load_script(env, *tokens);
    }

    auto compile(environment &env, const filesystem::path &path) -> script_result {
        if (!filesystem::exists(path)) {
            return unexpected(error{
                .source = error_source::interpreter,
                .kind = error_kind::file_error,
                .message = format("file '{}' does not exist", path.string()),
            });
        }

        if (!filesystem::is_regular_file(path)) {
            return unexpected(error{
                .source = error_source::interpreter,
                .kind = error_kind::file_error,
                .message = format("'{}' is not a file", path.string()),
            });
        }

        ifstream ifs(path, ios::in);
        if (!ifs.is_open()) {
            return unexpected(error{
                .source = error_source::interpreter,
                .kind = error_kind::file_error,
                .message = format("error opening '{}'", path.string()),
            });
        }

        return load_script(env, ifs);
    }
}
