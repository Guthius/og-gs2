#include "lexer.hpp"
#include "parser.hpp"

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
        println(cerr, "Error: {}", tokens.error().message);
        return 1;
    }

    auto unit = og::gs2::parse(*tokens);
    if (!unit) {
        println(cerr, "Error: {}", unit.error().message);
        return 1;
    }

    og::gs2::ast::print(cout, *unit);
    return 0;
}
