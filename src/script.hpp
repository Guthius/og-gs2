#pragma once

#include "lexer.hpp"

#include <filesystem>
#include <istream>
#include <memory>

namespace og::gs2 {
    struct script_error {
        std::string message;
        source_position position;
    };

    struct script {
        virtual ~script() = default;

        [[nodiscard]]
        auto has_function(std::string_view name) const -> bool;
    };

    using script_ptr = std::shared_ptr<script>;
    using script_result = std::expected<script_ptr, script_error>;

    auto load_script(const tokens &tokens) -> script_result;
    auto load_script(std::istream &is) -> script_result;

    auto compile(const std::filesystem::path &path) -> script_result;
}
