#pragma once

#include "environment.hpp"
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
        virtual auto call(std::string_view function_name, dictionary_ptr self, const values &args = {}) -> expected_value = 0;
    };

    using script_ptr = std::shared_ptr<script>;
    using script_result = std::expected<script_ptr, error>;

    auto load_script(environment &env, const tokens &tokens) -> script_result;
    auto load_script(environment &env, std::istream &is) -> script_result;

    auto compile(environment &env, const std::filesystem::path &path) -> script_result;
}
