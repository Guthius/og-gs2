#pragma once

#include "lexer.hpp"
#include "value.hpp"

#include <filesystem>
#include <istream>
#include <memory>

namespace og::gs2 {
    struct script {
        virtual ~script() = default;

        [[nodiscard]]
        virtual auto has_function(std::string_view name) const -> bool = 0;

        [[nodiscard]]
        virtual auto call(std::string_view function_name, dictionary_ptr context, const values &args = {}) const -> expected_value = 0;
    };

    using script_ptr = std::shared_ptr<script>;
    using script_result = std::expected<script_ptr, error>;

    auto load_script(const tokens &tokens) -> script_result;
    auto load_script(std::istream &is) -> script_result;

    auto compile(const std::filesystem::path &path) -> script_result;
}
