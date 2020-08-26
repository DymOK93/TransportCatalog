#pragma once
/*Для enable_if_t и is_floating_point_v */ 
#include <type_traits>
/*Для epsilon*/ 
#include <numeric>
/*Для abs*/ 
#include <cmath>

namespace algo {
	namespace numeric {
		/*К сожалению, std::abs() - не constexpr. Определим свою версию*/
		template <typename Number>
		constexpr std::enable_if_t <std::is_signed_v<Number>, Number> my_abs(Number num) noexcept {
			return num > 0 ? num : -num;
		}

		template <typename Number>
		constexpr std::enable_if_t <!std::is_signed_v<Number>, Number> my_abs(Number num) noexcept {
			return num;
		}

		/*При нежелании тащить за собой тяжёлый хедер type_traits трейты enable_if, is_signed и is_floating_point можно написать самостоятельно.
		inline-переменные появились лишь в C++17. Для С++14 и более старых вместо std::is_floating_point_v<Number>
		использовать std::is_floating_point<Number>::value*/

		template <typename Number>
		constexpr std::enable_if_t<std::is_floating_point_v<Number>, bool> equal_to(Number left, Number right) noexcept {
			return my_abs(right - left) < std::numeric_limits<Number>::epsilon();
		}
	}
}


