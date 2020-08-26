#include "json_number.h"

namespace Json {
    Number::Number(const NumberHolder& number_) noexcept
        : number{ number_ }
    {
    }
    Number::Number(uint64_t num) noexcept {
        number.whole = num;
    }
    Number::Number(double num) noexcept {
        number.whole = static_cast<uint64_t>(num);
        number.fractional_length = NumberHolder::precision;
        number.fractional = static_cast<uint64_t>((num - number.whole) * pow(static_cast<uint64_t>(10), NumberHolder::precision));
    }
    Number::operator uint64_t() const noexcept { 
        return number.whole; 
    }
    Number::operator double() const noexcept {
        double result{ static_cast<double>(number.whole) };
        if (number.fractional_length) {
            result += static_cast<double>(number.fractional) / pow(10, static_cast<double>(number.fractional_length));
        }
        return number.negative ?
            -result : result;
    }

    uint64_t Number::GetWhole() const noexcept {
        return number.whole;
    }

    uint64_t Number::GetFractional() const noexcept {
        return number.fractional;
    }

    uint64_t Number::GetFractionalLength() const noexcept {
        return number.fractional_length;
    }

    bool Number::IsNegative() const noexcept {
        return number.negative;
    }
}
