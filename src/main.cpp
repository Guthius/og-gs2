#include "lexer.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <print>

auto main(int argc, char *argv[]) -> int {
    if (argc != 2) {
        std::println(std::cerr, "No input file specified");
        return 1;
    }

    auto path = std::filesystem::path(argv[1]);
    if (!std::filesystem::exists(path)) {
        std::println(std::cerr, "Input file not found: {}", path.string());
        return 1;
    }

    std::ifstream ifs(path, std::ios::in);
    if (!ifs.is_open()) {
        return 1;
    }

    auto result = og::gs2::tokenize(ifs);
    if (result.has_value()) {
        std::println("{:>5} |{:>5} | {:<20}| {}", "Line", "Col", "Type", "Lexeme");
        std::println("------+------+---------------------+---------------------");

        for (auto &tok : *result) {
            std::println(
                "{:>5} |{:>5} | {:<20}| {}",
                tok.position.line, tok.position.column,
                og::gs2::token_kind_string(tok.kind),
                tok.lexeme);
        }
    } else {
        std::println(std::cerr, "Error: {}", result.error().message);
    }

    return 0;
}
