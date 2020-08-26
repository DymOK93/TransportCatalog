#include "json.h"
#include <algorithm>

using namespace std;

namespace Json {

    Document::Document(Node root) : root(move(root)) {
    }

    const Node& Document::GetRoot() const {
        return root;
    }

    Node LoadNode(string_view& input);

    Node LoadArray(string_view& input) {
        vector<Node> result;

        char ch{ ch = input.front() };
        input.remove_prefix(1);

        while ((ch = input.front()) != ']') {
            input.remove_prefix(ch == ',' || ch == ':');
            result.push_back(LoadNode(input));
        }

        input.remove_prefix(1);
        input = algo::string::Strict(input, IsService);

        return Node(move(result));
    }

    struct smart_uint64_t {
        uint64_t value{ 0 };
        size_t  length{ 0 };
    };

    /*string to uint64_t conversion*/
    smart_uint64_t ReadUInt64(string_view& input, optional<uint64_t> max_length = nullopt) {
        smart_uint64_t number;
        for (char ch; (!max_length || number.length < max_length) && isdigit(ch = input.front()); ++number.length) {
            number.value *= 10;
            number.value += static_cast<uint64_t>(ch) - '0';
            input.remove_prefix(1);
        }
        return number;
    }

    Node LoadNumber(string_view& input) {
        input = algo::string::Strict(input, IsService);

        NumberHolder number;
        input.remove_prefix(number.negative = (input.front() == '-'));

        number.whole = ReadUInt64(input).value;

        input.remove_prefix(input.front() == '.');
        auto [fractional, fractional_length] {ReadUInt64(input, NumberHolder::precision)};
        number.fractional = fractional;
        number.fractional_length = fractional_length;

        /*If the length of the fractional part
        is more than the max accuracy,
        we need to remove the extra characters*/
        ReadUInt64(input);

        input = algo::string::Strict(input, IsService);
        return Node(Number(number));
    }

    Node LoadBool(string_view& input) {
        input = algo::string::Strict(input, IsService);
        static constexpr string_view true_cond{ "true" },
            false_cond{ "false" };

        if (input.substr(0, true_cond.length()) == true_cond) {
            input.remove_prefix(true_cond.length());
            input = algo::string::Strict(input, IsService);
            return Node(true);
        }
        else {
            input.remove_prefix(false_cond.length());
            input = algo::string::Strict(input, IsService);
            return Node(false);
        }
    }

    Node LoadString(string_view& input) {
        input = algo::string::Strict(input, IsService);

        input.remove_prefix(1); //Remove the opening quote
        string str(input.substr(0, input.find_first_of('"')));
        input.remove_prefix(str.length() + 1);  //Removes str and closing quote

        input = algo::string::Strict(input, IsService);
        return str;
    }

    Node LoadDict(string_view& input) {
        map_t result;

        input.remove_prefix(1);

        for (char ch; (ch = input.front()) != '}'; ) {
            string key = LoadString(input).AsString();
            result.emplace(move(key), LoadNode(input));
        }
        input.remove_prefix(1);
        input = algo::string::Strict(input, IsService);

        return Node(move(result));
    }

    Node LoadNode(string_view& input) {
        input = algo::string::Strict(input, IsService);
        char ch{ input.front() };

        switch (ch) {
        case '[': return LoadArray(input);
        case '{': return LoadDict(input);
        case '\"': return LoadString(input);
        default: return isalpha(ch) ?
            LoadBool(input) : LoadNumber(input);
        };
    }

    Document Load(string_view input) {
        return Document{ LoadNode(input) };
    }

    bool IsService(char ch) {
        return isspace(ch) || ch == ',' || ch == ':';
    }

    string Read(istream& input) {
        string raw, parsed;

        do {
            getline(input, raw);
            parsed += raw;
        } while (!raw.empty() && input);

        return parsed;
    }

    template <class NodeTy>
    string SerializeNode(const Node& node);

    template <>
    string SerializeNode<bool_t>(const Node& node) {
        return node.AsBool() ? "true" : "false";
    }

    template <>
    string SerializeNode<number_t>(const Node& node) {
        const auto& number{ node.AsNumber() };
        string result;
        result.reserve(2 * Number::GetMaxPrecision() + 2);  //Whole, fractional, point and minus

        if (number.IsNegative()) {
            result.push_back('-');
        }
        result += algo::string::number_to_string(number.GetWhole());
        if (number.GetFractionalLength()) {
            result.push_back('.');
            result += algo::string::number_to_string(
                number.GetFractional(),
                number.GetFractionalLength()
            );
        }
        return result;
    }

    template <>
    string SerializeNode<string_t>(const Node& node) {
        return '\"' + node.AsString() + '\"';
    }

    template <>
    string SerializeNode<array_t>(const Node& node) {
        string serialized_array{ "[\n" };
        const auto& array_data{ node.AsArray() };
        bool first = true;

        for (auto node_it = array_data.begin(); node_it != array_data.end(); ++node_it) {
            if (!first) {
                serialized_array += ",\n";
            }
            first = false;
            serialized_array += SerializeNode<Node>(*node_it);
        }

        return serialized_array += "\n]";
    }

    template <>
    string SerializeNode<map_t>(const Node& node) {
        string serialized_dict{ "{\n" };
        const auto& dict_data{ node.AsMap() };
        bool first = true;

        for (auto node_it = dict_data.begin(); node_it != dict_data.end(); ++node_it) {
            if (!first) {
                serialized_dict += ",\n";
            }
            first = false;
            serialized_dict += '\"' + node_it->first + "\": ";
            serialized_dict += SerializeNode<Node>(node_it->second);
        }
        return serialized_dict += "\n}";
    }

    template <class NodeTy>
    string SerializeNode(const Node& node) {
        string result;
        visit(
            [&result, &node](const auto& value) {
                result = SerializeNode<decay_t<decltype(value)>>(node); //Without decay_t we call general version of SerializeNode and get stack overflow
            },
            node.GetBase()
       );
        return result;
    }

    string Serialize(const Document& doc) {
        return SerializeNode<Node>(doc.GetRoot());
    }
}
