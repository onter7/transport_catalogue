#pragma once

#include "json.h"

#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

namespace json {

	class Builder {
	private:
		class Context;
		class ArrayItemContext;
		class DictItemContext;
		class KeyContext;
		class ArrayValueContext;

		class Context {
		protected:
			Context(Builder& builder);
			Builder& builder_;
		};

		class DictItemContext : private Context {
		public:
			DictItemContext(Builder& builder);
			KeyContext Key(const std::string& key);
			Builder& EndDict();
		};

		class ArrayItemContext : private Context {
		public:
			ArrayItemContext(Builder& builder);
			ArrayValueContext Value(json::Node value);
			DictItemContext StartDict();
			ArrayItemContext StartArray();
			Builder& EndArray();
		};

		class KeyContext : private Context {
		public:
			KeyContext(Builder& builder);
			DictItemContext Value(json::Node value);
			DictItemContext StartDict();
			ArrayItemContext StartArray();
		};

		class ArrayValueContext : private Context {
		public:
			ArrayValueContext(Builder& builder);
			ArrayValueContext Value(json::Node value);
			DictItemContext StartDict();
			ArrayItemContext StartArray();
			Builder& EndArray();
		};
	public:
		enum class NodeType {
			START,
			VALUE
		};
		DictItemContext StartDict();
		ArrayItemContext StartArray();
		Builder& EndDict();
		Builder& EndArray();
		KeyContext Key(const std::string& key);
		Builder& Value(json::Node value);
		json::Node Build();
	private:
		json::Node root_;
		std::vector<json::Node*> nodes_stack_;

		void CheckReady() const;
		Builder& AddNode(json::Node node, NodeType node_type);

		template <typename NodeType>
		Builder& EndNode() {
			using namespace std::literals;
			CheckReady();
			if ((std::is_same_v<NodeType, json::Dict> && !nodes_stack_.back()->IsDict()) ||
				(std::is_same_v<NodeType, json::Array> && !nodes_stack_.back()->IsArray())) {
				throw std::logic_error("End type must match start type"s);
			}
			nodes_stack_.pop_back();
			return *this;
		}
	};

}