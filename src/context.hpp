#pragma once

#include "environment.hpp"

namespace og::gs2 {
    class context {
      public:
        context(environment &env, dictionary_ptr self) : env_(env), self_(std::move(self)) {
        }

        auto get(std::string_view name) -> expected_value;
        auto get_scope(std::string_view name) -> expected_dictionary;

        [[nodiscard]]
        auto get_scope() const -> const dictionary_ptr & {
            return self_;
        }

      private:
        environment &env_;
        dictionary_ptr self_;
    };
}
