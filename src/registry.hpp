#pragma once

#include <string>
#include <unordered_map>

namespace og::gs2 {
    struct registry_hash {
        using is_transparent = void;

        auto operator()(std::string_view sv) const {
            return std::hash<std::string_view>{}(sv);
        }

        auto operator()(const std::string &str) const {
            return std::hash<std::string>{}(str);
        }
    };

    template <typename TValue>
    using registry = std::unordered_map<std::string, TValue, registry_hash, std::equal_to<>>;
}
