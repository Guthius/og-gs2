#include <gs2/context.hpp>

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
}
