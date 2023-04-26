#include "json.h"

using namespace std;

namespace json {

    namespace {

        using Number = std::variant<int, double>;

        Node LoadNode(istream& input);

        Node LoadArray(istream& input) {
            Array result;

            for (char c; input >> c && c != ']';) {
                if (c != ',') {
                    input.putback(c);
                }
                result.push_back(LoadNode(input));
            }

            if (!input) {
                throw ParsingError("Load array error"s);
            }

            return Node(move(result));
        }

        //Node LoadInt(istream& input) {
        //    int result = 0;
        //    while (isdigit(input.peek())) {
        //        result *= 10;
        //        result += input.get() - '0';
        //    }
        //    return Node(result);
        //}

        Number LoadNumber(std::istream& input) {
            using namespace std::literals;

            std::string parsed_num;

            // Считывает в parsed_num очередной символ из input
            auto read_char = [&parsed_num, &input] {
                parsed_num += static_cast<char>(input.get());
                if (!input) {
                    throw ParsingError("Failed to read number from stream"s);
                }
            };

            // Считывает одну или более цифр в parsed_num из input
            auto read_digits = [&input, read_char] {
                if (!std::isdigit(input.peek())) {
                    throw ParsingError("A digit is expected"s);
                }
                while (std::isdigit(input.peek())) {
                    read_char();
                }
            };

            if (input.peek() == '-') {
                read_char();
            }
            // Парсим целую часть числа
            if (input.peek() == '0') {
                read_char();
                // После 0 в JSON не могут идти другие цифры
            }
            else {
                read_digits();
            }

            bool is_int = true;
            // Парсим дробную часть числа
            if (input.peek() == '.') {
                read_char();
                read_digits();
                is_int = false;
            }

            // Парсим экспоненциальную часть числа
            if (int ch = input.peek(); ch == 'e' || ch == 'E') {
                read_char();
                if (ch = input.peek(); ch == '+' || ch == '-') {
                    read_char();
                }
                read_digits();
                is_int = false;
            }

            try {
                if (is_int) {
                    // Сначала пробуем преобразовать строку в int
                    try {
                        return std::stoi(parsed_num);
                    }
                    catch (...) {
                        // В случае неудачи, например, при переполнении,
                        // код ниже попробует преобразовать строку в double
                    }
                }
                return std::stod(parsed_num);
            }
            catch (...) {
                throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
            }
        }

        Node LoadNum(std::istream& input) {
            auto num = LoadNumber(input);
            if (std::holds_alternative<int>(num)) {
                return Node(std::get<int>(num));
            }
            
                return Node(std::get<double>(num));
            
        }

        std::string LoadString(istream & input) {
            using namespace std::literals;

            auto it = std::istreambuf_iterator<char>(input);
            auto end = std::istreambuf_iterator<char>();
            std::string s;
            while (true) {
                if (it == end) {
                    // Поток закончился до того, как встретили закрывающую кавычку?
                    throw ParsingError("String parsing error");
                }
                const char ch = *it;
                if (ch == '"') {
                    // Встретили закрывающую кавычку
                    ++it;
                    break;
                }
                else if (ch == '\\') {
                    // Встретили начало escape-последовательности
                    ++it;
                    if (it == end) {
                        // Поток завершился сразу после символа обратной косой черты
                        throw ParsingError("String parsing error");
                    }
                    const char escaped_char = *(it);
                    // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
                    switch (escaped_char) {
                    case 'n':
                        s.push_back('\n');
                        break;
                    case 't':
                        s.push_back('\t');
                        break;
                    case 'r':
                        s.push_back('\r');
                        break;
                    case '"':
                        s.push_back('"');
                        break;
                    case '\\':
                        s.push_back('\\');
                        break;
                    default:
                        // Встретили неизвестную escape-последовательность
                        throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
                    }
                }
                else if (ch == '\n' || ch == '\r') {
                    // Строковый литерал внутри- JSON не может прерываться символами \r или \n
                    throw ParsingError("Unexpected end of line"s);
                }
                else {
                    // Просто считываем очередной символ и помещаем его в результирующую строку
                    s.push_back(ch);
                }
                ++it;
            }

            return s;
        }

        bool LoadBool(istream& input) {
            std::string res;
            char c;
            int len = static_cast<char>(input.peek()) == 't' ? 4 : 5;

            for (int i = 0; i < len; ++i) {
                input >> c;
                res += c;
            }

            if (res != "true"s && res != "false"s) {
                throw ParsingError("Bool load error"s);
            }

            if (res == "true"s) {
                return true;
            }

           
                return false;
            
        }

        Node LoadNull(istream& input) {
            std::string check;
            char ch;
            while (check.size() < 4) {
                ch = EOF;
                input >> ch;
                check += ch;
            }

            if (check != "null") {
                throw ParsingError("Failed to read null"s);
            }
            return Node();
        }

        Dict LoadDict(istream& input) {
            Dict result;
            char c;
            for (; input >> c && c != '}';) {
                if (c == ',') {
                    input >> c;
                }

                string key = Node(LoadString(input)).AsString();
                input >> c;
                result.insert({ move(key), LoadNode(input) });
            }
            if (c == '}') {
                return result;
            }
            else {
                throw ParsingError("Error load dict"s);
            }

            //return Node(move(result));
        }

        Node LoadNode(istream& input) {
            char c;
            input >> c;

            if (c == '[') {
                return LoadArray(input);
            }
            else if (c == '{') {
                return Node(LoadDict(input));
            }
            else if (c == '"') {
                return Node(move(LoadString(input)));
            }
            else if (c == 't') {
                input.putback(c);
                return Node(LoadBool(input));
            }
            else if (c == 'f') {
                input.putback(c);
                return Node(LoadBool(input));
            }
            else if (c == 'n') {
                input.putback(c);
                return LoadNull(input);
            }
            else {
                input.putback(c);
                //return LoadInt(input);
                return LoadNum(input);
            }
        }

    }  // namespace

    Node::Node(Array array)
        : value_(move(array)) {
    }

    Node::Node(Dict map)
        : value_(move(map)) {
    }

    Node::Node(int value)
        : value_(value) {
    }

    Node::Node(string value)
        : value_(move(value)) {
    }

    Node::Node(nullptr_t )
         {
    }

    Node::Node(bool value)
        : value_(value) {
    }

    Node::Node(double value)
        : value_(value) {
    }

    const Array& Node::AsArray() const {
        if (!IsArray()) {
            throw std::logic_error("not an array"s);
        }
        return get<Array>(value_);
    }

    const Dict& Node::AsMap() const {
        if (!IsMap()) {
            throw std::logic_error("not a map"s);
        }
        return get<Dict>(value_);
    }

    int Node::AsInt() const {
        if (!IsInt()) {
            throw std::logic_error("not an int"s);
        }
        return get<int>(value_);
    }

    const string& Node::AsString() const {
        if (!IsString()) {
            throw std::logic_error("not a string"s);
        }
        return get<string>(value_);
    }

    bool Node::AsBool() const {
        if (!IsBool()) {
            throw std::logic_error("not a bool"s);
        }
        return get<bool>(value_);
    }
    double Node::AsDouble() const {
        if (!IsDouble()) {
            throw std::logic_error("not a double"s);
        }
        return IsPureDouble() ? get<double>(value_) : static_cast<double>(AsInt());
    }
    
    bool Node::IsInt() const {
        return std::holds_alternative<int>(value_);
    }
    bool Node::IsDouble() const {
        return IsInt() || IsPureDouble();
    }
    bool Node::IsPureDouble() const {
        return std::holds_alternative<double>(value_);
    }
    bool Node::IsBool() const {
        return std::holds_alternative<bool>(value_);
    }
    bool Node::IsString() const {
        return std::holds_alternative<std::string>(value_);
    }
    bool Node::IsNull() const {
        return std::holds_alternative<std::nullptr_t>(value_);
    }
    bool Node::IsArray() const {
        return std::holds_alternative<Array>(value_);
    }
    bool Node::IsMap() const {
        return std::holds_alternative<Dict>(value_);
    }

    const Node::Value& Node::GetValue() const {
        return value_;
    }

    bool Node::operator==(const Node& rhs) const {
        return value_ == rhs.value_;
    }
    bool Node::operator!=(const Node& rhs) const {
        return value_ != rhs.value_;
    }



    namespace {
        struct PrintContext {
            std::ostream& out;
            int indent_step = 4;
            int indent = 0;

            void PrintIndent() const {
                for (int i = 0; i < indent; ++i) {
                    out.put(' ');
                }
            }

            // Возвращает новый контекст вывода с увеличенным смещением
            PrintContext Indented() const {
                return { out, indent_step, indent_step + indent };
            }
        };

        void PrintNode(const Node& node, const PrintContext& ctx);

        // Шаблон, подходящий для вывода double и int
        template <typename Value>
        void PrintValue(const Value& value, const PrintContext& ctx) {
            ctx.out << value;
        }

        // Перегрузка функции PrintValue для вывода значений null
        void PrintValue(const std::nullptr_t, const PrintContext& ctx) {
            ctx.out << "null"sv;
        }

        void PrintValue(const bool value, const PrintContext& ctx) {
            ctx.out << (value ? "true"s : "false"s);
        }

        void PrintString(const std::string& value,
            std::ostream& out) {
            out.put('"');

            for (const char c : value) {
                switch (c) {
                case '\r':
                    out << "\\r"sv;
                    break;
                case '\n':
                    out << "\\n"sv;
                    break;
                case '"':
                    // Символы " и \ выводятся как \" или \\,
                    // соответственно
                    [[fallthrough]];
                case '\\':
                    out.put('\\');
                    [[fallthrough]];
                default:
                    out.put(c);
                    break;
                }
            }

            out.put('"');
        }


        void PrintValue(const std::string& value, const PrintContext& ctx) {
            PrintString(value, ctx.out);
        }

        void PrintValue(const Array& value, const PrintContext& ctx) {
            std::ostream& out = ctx.out;
            out << "[\n"sv;
            bool first = true;
            auto inner_ctx = ctx.Indented();
            for (const Node& node : value) {
                if (first) {
                    first = false;
                }
                else {
                    out << ",\n"sv;
                }
                inner_ctx.PrintIndent();
                PrintNode(node, inner_ctx);
            }
            out.put('\n');
            ctx.PrintIndent();
            out.put(']');
        }

        void PrintValue(const Dict& value, const PrintContext& ctx) {
            std::ostream& out = ctx.out;
            out << "{\n"sv;
            bool first = true;
            auto inner_ctx = ctx.Indented();
            for (const auto& [key, node] : value) {
                if (first) {
                    first = false;
                }
                else {
                    out << ",\n"sv;
                }
                inner_ctx.PrintIndent();
                PrintString(key, ctx.out);
                out << ": "sv;
                PrintNode(node, inner_ctx);
            }
            out.put('\n');
            ctx.PrintIndent();
            out.put('}');
        }

        // Другие перегрузки функции PrintValue пишутся аналогично

        void PrintNode(const Node& node, const PrintContext& ctx) {
            std::visit(
                [&ctx](const auto& value) { PrintValue(value, ctx); },
                node.GetValue());
        }
    }

    // -------Document-----------

    Document::Document(Node root)
        : root_(move(root)) {
    }

    const Node& Document::GetRoot() const {
        return root_;
    }


    bool Document::operator==(const Document& rhs) const {
        return root_.GetValue() == rhs.root_.GetValue();
    }

    bool Document::operator!=(const Document& rhs) const {
        return root_.GetValue() != rhs.root_.GetValue();
    }


    Document Load(istream& input) {
        return Document{ LoadNode(input) };
    }

    void Print(const Document& doc, std::ostream& output) {
        //(void)&doc;
        //(void)&output;
        PrintNode(doc.GetRoot(), PrintContext{ output });
        // Реализуйте функцию самостоятельно
    }

}  // namespace json