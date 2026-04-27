#include "context.hpp"

using namespace std;

namespace og::gs2 {
    auto context::get(string_view name) -> expected_value {
        if (self_->contains(name)) {
            return self_->get(name);
        }

        if (env_.contains(name)) {
            return env_.get(name);
        }

        return {};
    }

    auto context::get_scope(string_view name) -> expected_dictionary {
        if (self_->contains(name)) {
            return self_;
        }

        if (env_.contains(name)) {
            return env_.shared_from_this();
        }

        return self_;
    }
}
