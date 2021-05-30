#pragma once

#include <cstddef>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

namespace json {

	class Node;
	using Dict = std::map<std::string, Node>;
	using Array = std::vector<Node>;

	// Эта ошибка должна выбрасываться при ошибках парсинга JSON
	class ParsingError : public std::runtime_error {
	public:
		using runtime_error::runtime_error;
	};

	class Node final : private std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string> {
	public:
		using variant::variant;
		using Value = variant;

		const Value& GetValue() const;
		const Array& AsArray() const;
		const Dict& AsDict() const;
		bool AsBool() const;
		int AsInt() const;
		double AsDouble() const;
		const std::string& AsString() const;

		bool IsNull() const;
		bool IsArray() const;
		bool IsDict() const;
		bool IsBool() const;
		bool IsInt() const;
		bool IsDouble() const;
		bool IsPureDouble() const;
		bool IsString() const;
	};

	bool operator==(const Node& lhs, const Node& rhs);
	bool operator!=(const Node& lhs, const Node& rhs);

	struct NodePrinter {
		std::ostream& out;
		void operator()(std::nullptr_t) const;
		void operator()(const Array& node) const;
		void operator()(const Dict& node) const;
		void operator()(const bool node) const;
		void operator()(const int node) const;
		void operator()(const double node) const;
		void operator()(const std::string& node) const;
	};

	class Document {
	public:
		explicit Document(Node root);

		const Node& GetRoot() const;

	private:
		Node root_;
	};

	bool operator==(const Document& lhs, const Document& rhs);
	bool operator!=(const Document& lhs, const Document& rhs);

	Document Load(std::istream& input);

	void Print(const Document& doc, std::ostream& output);

}  // namespace json