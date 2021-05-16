#include "json.h"

#include <sstream>
#include <utility>

using namespace std::literals;

namespace json {

	const Node::Value& Node::GetValue() const {
		return *this;
	}

	const Array& Node::AsArray() const {
		if (!std::holds_alternative<Array>(*this)) {
			throw std::logic_error("Node does not hold Array"s);
		}
		return std::get<Array>(*this);
	}

	const Dict& Node::AsMap() const {
		if (!std::holds_alternative<Dict>(*this)) {
			throw std::logic_error("Node does not hold Dict"s);
		}
		return std::get<Dict>(*this);
	}

	bool Node::AsBool() const {
		if (!IsBool()) {
			throw std::logic_error("Node does not hold bool"s);
		}
		return std::get<bool>(*this);
	}

	int Node::AsInt() const {
		if (!IsInt()) {
			throw std::logic_error("Node does not hold int"s);
		}
		return std::get<int>(*this);
	}

	double Node::AsDouble() const {
		if (!IsDouble()) {
			throw std::logic_error("Node does not hold double"s);
		}
		return IsInt() ? static_cast<double>(std::get<int>(*this)) : std::get<double>(*this);
	}

	const std::string& Node::AsString() const {
		if (!IsString()) {
			throw std::logic_error("Node does not hold string"s);
		}
		return std::get<std::string>(*this);
	}

	bool Node::IsNull() const {
		return std::holds_alternative<std::nullptr_t>(*this);
	}

	bool Node::IsArray() const {
		return std::holds_alternative<Array>(*this);
	}

	bool Node::IsMap() const {
		return std::holds_alternative<Dict>(*this);
	}

	bool Node::IsBool() const {
		return std::holds_alternative<bool>(*this);
	}

	bool Node::IsInt() const {
		return std::holds_alternative<int>(*this);
	}

	bool Node::IsDouble() const {
		return IsInt() || IsPureDouble();
	}

	bool Node::IsPureDouble() const {
		return std::holds_alternative<double>(*this);
	}

	bool Node::IsString() const {
		return std::holds_alternative<std::string>(*this);
	}

	void NodePrinter::operator()(std::nullptr_t) const {
		out << "null"sv;
	}

	void NodePrinter::operator()(const Array& node) const {
		out << "["sv;
		bool first = true;
		for (const auto& item : node) {
			if (first) {
				first = false;
			}
			else {
				out << ","sv;
			}
			std::visit(NodePrinter{ out }, item.GetValue());
		}
		out << "]"sv;
	}

	void NodePrinter::operator()(const Dict& node) const {
		out << "{"sv;
		bool first = true;
		for (const auto& [key, value] : node) {
			if (first) {
				first = false;
			}
			else {
				out << ","sv;
			}
			out << "\""sv << key << "\""sv << ":"sv;
			std::visit(NodePrinter{ out }, value.GetValue());
		}
		out << "}"sv;
	}

	void NodePrinter::operator()(const bool node) const {
		if (node) {
			out << "true"sv;
		}
		else {
			out << "false"sv;
		}
	}

	void NodePrinter::operator()(const int node) const {
		out << node;
	}

	void NodePrinter::operator()(const double node) const {
		out << node;
	}

	void NodePrinter::operator()(const std::string& node) const {
		out << '"';
		for (const char c : node) {
			switch (c) {
			case '\r':
				out << "\\r"sv;
				break;
			case '\n':
				out << "\\n"sv;
				break;
			case '"':
				[[fallthrough]];
			case '\\':
				out << '\\';
				[[fallthrough]];
			default:
				out << c;
				break;
			}
		}
		out << '"';
	}

	bool operator==(const Node& lhs, const Node& rhs) {
		return lhs.GetValue() == rhs.GetValue();
	}

	bool operator!=(const Node& lhs, const Node& rhs) {
		return !(lhs == rhs);
	}

	namespace {

		Node LoadNode(std::istream& input);

		Node LoadNumber(std::istream& input) {
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
						return Node{ std::stoi(parsed_num) };
					}
					catch (...) {
						// В случае неудачи, например, при переполнении
						// код ниже попробует преобразовать строку в double
					}
				}
				return Node{ std::stod(parsed_num) };
			}
			catch (...) {
				throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
			}
		}

		Node LoadArray(std::istream& input) {
			Array result;

			for (char c; input >> c && c != ']';) {
				if (c != ',') {
					input.putback(c);
				}
				result.push_back(LoadNode(input));
			}

			if (!input) {
				throw ParsingError("Failed to parse Array"s);
			}

			return Node(move(result));
		}

		std::string LoadReservedWord(std::istream& input) {
			std::stringstream ss;
			char c;
			while (std::isalpha(input.peek())) {
				input.get(c);
				ss << c;
			}
			return ss.str();
		}

		Node LoadString(std::istream& input) {
			std::stringstream ss;
			char c;
			while (true) {
				if (!input.get(c)) {
					throw ParsingError("Failed to read string"s);
				}
				if (c == '"') {
					break;
				}
				if (c == '\\') {
					if (!input.get(c)) {
						throw ParsingError("Failed to read char from input"s);
					}
					switch (c) {
					case 'n':
						ss << '\n';
						break;
					case 't':
						ss << '\t';
						break;
					case 'r':
						ss << '\r';
						break;
					case '"':
						ss << '\"';
						break;
					case '\\':
						ss << '\\';
						break;
					default:
						throw ParsingError("Failed to read escape character from input"s);
					}
				}
				else
				{
					ss << c;
				}
			}
			return Node(std::move(ss.str()));
		}

		Node LoadDict(std::istream& input) {
			Dict result;

			for (char c; input >> c && c != '}';) {
				if (c == ',') {
					input >> c;
				}

				std::string key = LoadString(input).AsString();
				input >> c;
				result.insert({ std::move(key), LoadNode(input) });
			}

			if (!input) {
				throw ParsingError("Failed to parse Dict"s);
			}

			return Node(std::move(result));
		}

		Node LoadNull(std::istream& input) {
			if (const auto null_str = LoadReservedWord(input); null_str != "null"sv) {
				throw ParsingError("Failed to parse null"s);
			}
			return Node{ nullptr };
		}

		Node LoadBool(std::istream& input) {
			const std::string bool_str = LoadReservedWord(input);
			if (bool_str != "true"sv && bool_str != "false"sv) {
				throw ParsingError("Failed to parse bool"s);
			}
			return Node{ bool_str == "true"sv };
		}

		Node LoadNode(std::istream& input) {
			char c;
			input >> c;

			if (c == '[') {
				return LoadArray(input);
			}
			else if (c == '{') {
				return LoadDict(input);
			}
			else if (c == '"') {
				return LoadString(input);
			}
			else if (c == 'n') {
				input.putback(c);
				return LoadNull(input);
			}
			else if (c == 't' || c == 'f') {
				input.putback(c);
				return LoadBool(input);
			}
			else {
				input.putback(c);
				return LoadNumber(input);
			}
		}

	}  // namespace    

	Document::Document(Node root)
		: root_(std::move(root)) {
	}

	const Node& Document::GetRoot() const {
		return root_;
	}

	bool operator==(const Document& lhs, const Document& rhs) {
		return lhs.GetRoot() == rhs.GetRoot();
	}

	bool operator!=(const Document& lhs, const Document& rhs) {
		return !(lhs == rhs);
	}

	Document Load(std::istream& input) {
		return Document{ LoadNode(input) };
	}

	void Print(const Document& doc, std::ostream& output) {
		std::visit(NodePrinter{ output }, doc.GetRoot().GetValue());
	}

}  // namespace json