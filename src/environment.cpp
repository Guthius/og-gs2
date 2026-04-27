#include "environment.hpp"

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
}
