#pragma once
#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>

namespace json {

    class Node;
    // Сохраните объявления Dict и Array без изменения
    using Dict = std::map<std::string, Node>;
    using Array = std::vector<Node>;

    // Эта ошибка должна выбрасываться при ошибках парсинга JSON
    class ParsingError : public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };

    class Node {
    public:
        /* Реализуйте Node, используя std::variant */

        using Value = std::variant <std::nullptr_t, Array, Dict, bool, int, double, std::string >;

        Node() = default;
        Node(Array array);
        Node(Dict map);
        Node(int value);
        Node(std::string value);
        Node(std::nullptr_t value);
        Node(bool value);
        Node(double value);

        const Array& AsArray() const;
        const Dict& AsMap() const;
        int AsInt() const;
        const std::string& AsString() const;
        bool AsBool() const;
        double AsDouble() const;

        bool IsInt() const;
        bool IsDouble() const;
        bool IsPureDouble() const;
        bool IsBool() const;
        bool IsString() const;
        bool IsNull() const;
        bool IsArray() const;
        bool IsMap() const;

        const Value& GetValue() const;

        bool operator==(const Node& rhs) const;
        bool operator!=(const Node& rhs) const;
       
    private:
        Value value_;
    };
    //inline bool operator==(const Node& lhs, const Node& rhs);
    //inline bool operator!=(const Node& lhs, const Node& rhs);

    //bool operator==(const Array& lhs, const Array& rhs);
    //bool operator!=(const Array& lhs, const Array& rhs);
    //bool operator==(const Dict& lhs, const Dict& rhs);
    //bool operator!=(const Dict& lhs, const Dict& rhs);

    //void PrintNode(const Node& node, std::ostream& output);

    class Document {
    public:
        explicit Document(Node root);

        const Node& GetRoot() const;

        bool operator==(const Document& rhs) const;
        bool operator!=(const Document& rhs) const;
    private:
        Node root_;
    };

    //inline bool operator==(const Document& lhs,
    //    const Document& rhs);

    //inline bool operator!=(const Document& lhs,
    //    const Document& rhs);

    Document Load(std::istream& input);

    void Print(const Document& doc, std::ostream& output);

}  // namespace json