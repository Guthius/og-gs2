#include "script.hpp"

#include "parser.hpp"
#include "registry.hpp"

#include <filesystem>
#include <format>
#include <fstream>

using namespace std;

namespace og::gs2 {
    namespace {
        struct script_impl : script {
            using function_stmt_ptr = const ast::function_stmt *;

            ast::stmt root;
            registry<function_stmt_ptr> functions;

            script_impl(ast::stmt stmt) : root(std::move(stmt)) {
                detect_functions(root);
            }

            auto has_function(string_view name) const -> bool {
                return functions.contains(name);
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

    auto load_script(const tokens &tokens) -> script_result {
        auto stmt = parse(tokens);
        if (!stmt) {
            return unexpected(script_error{
                .message = stmt.error().message,
                .position = stmt.error().position,
            });
        }

        return make_shared<script_impl>(std::move(*stmt));
    }

    auto load_script(istream &is) -> script_result {
        auto tokens = tokenize(is);
        if (!tokens) {
            return unexpected(script_error{
                .message = tokens.error().message,
                .position = tokens.error().position,
            });
        }

        return load_script(*tokens);
    }

    auto compile(const filesystem::path &path) -> script_result {
        if (!filesystem::exists(path)) {
            return unexpected(script_error{
                .message = format("file '{}' does not exist", path.string()),
            });
        }

        if (!filesystem::is_regular_file(path)) {
            return unexpected(script_error{
                .message = format("'{}' is not a file", path.string()),
            });
        }

        ifstream ifs(path, ios::in);
        if (!ifs.is_open()) {
            return unexpected(script_error{
                .message = format("error opening '{}'", path.string()),
            });
        }

        return load_script(ifs);
    }
}
