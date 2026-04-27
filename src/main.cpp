#include "ast.hpp"
#include "parser.hpp"
#include "script.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <print>

using namespace std;

auto main(int argc, char *argv[]) -> int {
    if (argc != 2) {
        println(cerr, "No input file specified");
        return 1;
    }

    auto path = filesystem::path(argv[1]);
    if (!filesystem::exists(path)) {
        println(cerr, "Input file not found: {}", path.string());
        return 1;
    }

    ifstream ifs(path, ios::in);
    if (!ifs.is_open()) {
        println(cerr, "Failed to open: {}", path.string());
        return 1;
    }

    auto tokens = og::gs2::tokenize(ifs);
    if (!tokens) {
        println(cerr, "Error: {} on line {}, column {}",
                tokens.error().message,
                tokens.error().position.line,
                tokens.error().position.column);
        return 1;
    }

    auto stmt = og::gs2::parse(*tokens);
    if (!stmt) {
        println(cerr, "Error: {} on line {}, column {}",
                stmt.error().message,
                stmt.error().position.line,
                stmt.error().position.column);
        return 1;
    }

    og::gs2::ast::print(cout, *stmt);

    auto env = og::gs2::environment();
    auto script = og::gs2::compile(env, path);
    if (!script) {
        println(cerr, "Error: {} on line {}, column {}",
                stmt.error().message,
                stmt.error().position.line,
                stmt.error().position.column);
        return 1;
    }

    return 0;
}
