#pragma once

/*Standart headers*/
#include <iterator>
#include <utility>

template <typename It>
class Range {
public:
    using ValueType = typename std::iterator_traits<It>::value_type;

    Range(It begin, It end) : begin_{ std::move(begin) }, end_{ std::move(end) } {}
    It begin() const { return begin_; }
    It end() const { return end_; }

private:
    It begin_;
    It end_;
};