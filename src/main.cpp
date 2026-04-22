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

    og::gs2::print_tokens(ifs);
    return 0;
}
