#pragma once

#include "registry.hpp"
#include "value.hpp"

namespace og::gs2 {
    class basic_dictionary : public dictionary {
      public:
        auto contains(std::string_view name) -> bool override;
        auto get(std::string_view name) -> value override;
        void put(std::string_view name, value value) override;
        auto erase(std::string_view name) -> bool override;

      private:
        registry<value> fields_;
    };

    class environment : public basic_dictionary {
    };
}
