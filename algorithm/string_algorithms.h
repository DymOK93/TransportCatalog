#pragma once
#include <string>
#include <utility>
#include <optional>
#include <string_view>
#include <type_traits>

namespace algo {
    namespace string {
        template <typename Number>
        std::enable_if_t<std::is_arithmetic_v<Number>, std::string> number_to_string(
            Number number,
            std::optional<size_t> required_length = std::nullopt
        ) {
            std::string result{ std::to_string(number) };
            if (required_length) {
                if (*required_length <= result.length()) {
                    result.resize(*required_length);
                }
                else {
                    result.insert(0, *required_length - result.length(), '0');      //Adding leading zeros
                }
            }
            return result;
        }

        template <class Predicate>
        std::string_view Strict(std::string_view str, Predicate pred) {
            size_t prefix_length{ 0 };
            for (; prefix_length < str.length() && pred(str[prefix_length]); ++prefix_length);
            str.remove_prefix(prefix_length);

            size_t suffix_length{ str.length() };
            for (; suffix_length > 0 && pred(str[suffix_length - 1]); --prefix_length);

            str.remove_suffix(str.length() - suffix_length);

            return str;
        }
    }
}
