#include "value.hpp"

#include <format>

using namespace std;

namespace og::gs2 {
    namespace {
        constexpr string generic_array_name = "Array";
        constexpr string generic_object_name = "Object";
    }

    auto to_number(const value &value) -> double {
        return visit(
            [](const auto &current) -> double {
                using T = decay_t<decltype(current)>;

                if constexpr (is_same_v<T, double>) {
                    return current;
                }

                if constexpr (is_same_v<T, string>) {
                    return strtod(current.c_str(), nullptr);
                }

                return 0.0;
            },
            value);
    }

    auto to_string(const value &value) -> string {
        return visit(
            [](const auto &current) -> string {
                using T = decay_t<decltype(current)>;

                if constexpr (is_same_v<T, monostate>) {
                    return "";
                }

                if constexpr (is_same_v<T, double>) {
                    return format("{}", current);
                }

                if constexpr (is_same_v<T, string>) {
                    return current;
                }

                if constexpr (is_same_v<T, array_ptr>) {
                    return generic_array_name;
                }

                if constexpr (is_same_v<T, dictionary_ptr>) {
                    return generic_object_name;
                }

                return "";
            },
            value);
    }

    auto to_bool(const value &value) -> bool {
        return visit(
            [](const auto &current) -> bool {
                using T = decay_t<decltype(current)>;

                if constexpr (is_same_v<T, monostate>) {
                    return false;
                }

                if constexpr (is_same_v<T, double>) {
                    return current != 0.0;
                }

                if constexpr (is_same_v<T, string>) {
                    return !current.empty();
                }

                if constexpr (is_same_v<T, array_ptr>) {
                    return current != nullptr;
                }

                if constexpr (is_same_v<T, dictionary_ptr>) {
                    return current != nullptr;
                }

                return false;
            },
            value);
    }

    auto values_equal(const value &left, const value &right) -> bool {
        // null == null
        if (holds_alternative<monostate>(left) &&
            holds_alternative<monostate>(right)) {
            return true;
        }

        // null == 0 or null == ""
        if (holds_alternative<monostate>(left) ||
            holds_alternative<monostate>(right)) {
            return to_number(left) == to_number(right);
        }

        // Both numeric
        if (holds_alternative<double>(left) &&
            holds_alternative<double>(right)) {
            return get<double>(left) == get<double>(right);
        }

        // Both string
        if (holds_alternative<string>(left) &&
            holds_alternative<string>(right)) {
            return get<string>(left) == get<string>(right);
        }

        // Mixed number/string — compare as numbers
        if ((holds_alternative<double>(left) || holds_alternative<string>(left)) &&
            (holds_alternative<double>(right) || holds_alternative<string>(right))) {
            return to_number(left) == to_number(right);
        }

        // Object identity
        if (holds_alternative<dictionary_ptr>(left) && holds_alternative<dictionary_ptr>(right)) {
            return get<dictionary_ptr>(left) == get<dictionary_ptr>(right);
        }

        if (holds_alternative<array_ptr>(left) && holds_alternative<array_ptr>(right)) {
            return get<array_ptr>(left) == get<array_ptr>(right);
        }

        return false;
    }
}
