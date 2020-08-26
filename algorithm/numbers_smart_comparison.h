#pragma once
/*��� enable_if_t � is_floating_point_v */ 
#include <type_traits>
/*��� epsilon*/ 
#include <numeric>
/*��� abs*/ 
#include <cmath>

namespace algo {
	namespace numeric {
		/*� ���������, std::abs() - �� constexpr. ��������� ���� ������*/
		template <typename Number>
		constexpr std::enable_if_t <std::is_signed_v<Number>, Number> my_abs(Number num) noexcept {
			return num > 0 ? num : -num;
		}

		template <typename Number>
		constexpr std::enable_if_t <!std::is_signed_v<Number>, Number> my_abs(Number num) noexcept {
			return num;
		}

		/*��� ��������� ������ �� ����� ������ ����� type_traits ������ enable_if, is_signed � is_floating_point ����� �������� ��������������.
		inline-���������� ��������� ���� � C++17. ��� �++14 � ����� ������ ������ std::is_floating_point_v<Number>
		������������ std::is_floating_point<Number>::value*/

		template <typename Number>
		constexpr std::enable_if_t<std::is_floating_point_v<Number>, bool> equal_to(Number left, Number right) noexcept {
			return my_abs(right - left) < std::numeric_limits<Number>::epsilon();
		}
	}
}


