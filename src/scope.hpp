#pragma once

#include "value.hpp"

namespace og::gs2 {
    struct scope {
        virtual ~scope() = default;

        auto get(const std::string &name) -> expected_value;
        auto get_scope(const std::string &name) -> expected_dictionary;
    };
}
