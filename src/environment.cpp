#include <gs2/environment.hpp>
#include "gs2/prototype.hpp"

using namespace std;

namespace og::gs2 {
    auto basic_dictionary::contains(string_view name) -> bool {
        return fields_.contains(name);
    }

    auto basic_dictionary::get(string_view name) -> value {
        auto iter = fields_.find(name);
        if (iter == fields_.end()) {
            return {};
        }

        return iter->second;
    }

    void basic_dictionary::put(string_view name, value value) {
        fields_[string(name)] = std::move(value);
    }

    auto basic_dictionary::erase(string_view name) -> bool {
        auto iter = fields_.find(name);
        if (iter == fields_.end()) {
            return false;
        }
        fields_.erase(iter);
        return true;
    }

    namespace {
        struct callable_impl : callable {
            environment &env;
            const native_function &function;

            callable_impl(environment &env, const native_function &function) : env(env), function(function) {
            }

            auto invoke(const values &args) -> expected_value override {
                return function(env, args);
            }
        };
    }

    void environment::register_function(std::string_view name, const native_function &function) {
        put(name, std::make_shared<callable_impl>(*this, function));
    }

    void environment::register_type(prototype_ptr prototype) {
        types_[prototype->name] = std::move(prototype);
    }

    auto environment::find_type(std::string_view type_name) -> prototype_ptr {
        auto iter = types_.find(type_name);
        if (iter == types_.end()) {
            return nullptr;
        }

        return iter->second;
    }
}
