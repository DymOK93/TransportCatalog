#pragma once
/*Standart headers*/
#include <cstdint>
#include <limits>

/*For pow(x, y)*/
#include <cmath>

namespace Json {
	struct NumberHolder {
        bool negative { false };
        uint64_t whole { 0 },
            fractional { 0 },
            fractional_length { 0 };
        static constexpr uint64_t precision{ std::numeric_limits<uint64_t>::digits10 };
	};

	class Number {
    public:
        Number() = default;
        Number(const NumberHolder& number_) noexcept;
        explicit Number(uint64_t num) noexcept;
        explicit  Number(double num) noexcept;
    public:
        uint64_t GetWhole() const noexcept;
        uint64_t GetFractional() const noexcept;
        uint64_t GetFractionalLength() const noexcept;
        bool IsNegative() const noexcept;
    public:
        static constexpr uint64_t GetMaxPrecision() noexcept {
            return NumberHolder::precision;
        }
    public:
        operator uint64_t() const noexcept;
        operator double() const noexcept;
    private:
        NumberHolder number;
	};
}
