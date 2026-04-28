#pragma once

#include "prototype.hpp"

#include <functional>

namespace og::gs2 {
    class environment;

    using native_function = std::function<expected_value(environment &env, const values &args)>;

    class environment : public basic_dictionary {
      public:
        void register_function(std::string_view name, const native_function &function);
        void register_type(prototype_ptr prototype);
        auto find_type(std::string_view type_name) -> prototype_ptr;

      private:
        registry<prototype_ptr> types_;
    };
}
