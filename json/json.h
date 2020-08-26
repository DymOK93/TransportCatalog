#pragma once
#include "json_number.h"
#include "string_algorithms.h"

#include <iostream>
#include <numeric>
#include <cctype>
#include <cmath>
#include <unordered_map>
#include <string>
#include <string_view>
#include <variant>
#include <array>
#include <vector>
#include <functional>
#include <type_traits>

#include <map>

namespace Json {
    class Node;

    /*Types of JSON Nodes*/
    using bool_t = bool;
    using number_t = Number;
    using string_t = std::string;
    using array_t = std::vector<Node>;   

    /*We can't use unordered map because in GCC unordered map template instantiation requires complete type*/
    using map_t = std::map<std::string, Node>;  

    /*JSON Node*/
    class Node : std::variant
        <bool_t,
        number_t,
        string_t,
        array_t,
        map_t> {
    public:
        /*Overloaded c-tors*/
        using variant::variant;

        /*Getting the variant base*/
        constexpr const variant& GetBase() const noexcept {
            return *this;
        }
    public:
        const auto& AsArray() const {
            return std::get<array_t>(*this);
        }
        const auto& AsMap() const {
            return std::get<map_t>(*this);
        }
        const auto& AsString() const {
            return std::get<string_t>(*this);
        }
        const auto& AsNumber() const {
            return std::get<number_t>(*this);
        }
        bool AsBool() const {
            return std::get<bool_t>(*this);
        }
    };

    /*This class encapculates unmodifiable JSON Tree*/
    class Document {
    public:
        explicit Document(Node root);
        const Node& GetRoot() const;
    private:
        Node root;
    };

    /*spaces, comma and colons are service symbols*/
    bool IsService(char ch);

    /*Reads lines from the istream until it encounters an empty line*/
    std::string Read(std::istream& input);

    /*Creates unmodifiable JSON Tree from string (string_view)*/
    Document Load(std::string_view input);

    /*Serializes unmodifiable JSON Tree to string*/
    std::string Serialize(const Document& doc);
}
