#pragma once

#include <string>
#include <optional>

namespace SKW_WPX {

    namespace Cache {

        class ICache {
        public:
            virtual ~ICache() = default;

            virtual std::optional<int> get(const std::string &key) = 0;

            virtual void put(const std::string &key, int value) = 0;
        };
    }
}