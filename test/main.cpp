#include "../src/parser.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <print>

using namespace std;

auto test_file(const filesystem::path &path) {
    if (!filesystem::is_regular_file(path)) {
        return;
    }

    ifstream ifs(path, ios::in);
    if (!ifs.is_open()) {
        return;
    }

    auto tokens = og::gs2::tokenize(ifs);
    if (!tokens) {
        println(cerr, "{}: {} on line {}, column {}", path.string(),
                tokens.error().message,
                tokens.error().position.line,
                tokens.error().position.column);
        return;
    }

    auto stmt = og::gs2::parse(*tokens);
    if (!stmt) {
        println(cerr, "{}: {} on line {}, column {}", path.string(),
                stmt.error().message,
                stmt.error().position.line,
                stmt.error().position.column);
    }
}

auto main(int argc, char *argv[]) -> int {
    if (argc < 2) {
        return 0;
    }

    auto path = filesystem::path(argv[1]);
    if (!filesystem::exists(path) || !filesystem::is_directory(path)) {
        println(cerr, "Not a valid directory: {}", path.string());
    }

    for (const auto &entry : filesystem::recursive_directory_iterator(path)) {
        if (entry.is_regular_file()) {
            test_file(entry.path());
        }
    }

    return 0;
}
