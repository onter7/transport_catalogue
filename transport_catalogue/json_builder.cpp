#include "json_builder.h"

#include <utility>

using namespace std::literals;

namespace json {

	// Context

	Builder::Context::Context(Builder& builder)
		: builder_(builder) {
	}

	// DictItemContext

	Builder::DictItemContext::DictItemContext(Builder& builder)
		: Context(builder) {
	}

	Builder::KeyContext Builder::DictItemContext::Key(const std::string& key) {
		return builder_.Key(key);
	}

	Builder& Builder::DictItemContext::EndDict() {
		return builder_.EndDict();
	}

	// ArrayItemContext

	Builder::ArrayItemContext::ArrayItemContext(Builder& builder)
		: Context(builder) {
	}

	Builder::ArrayValueContext Builder::ArrayItemContext::Value(json::Node value) {
		return ArrayValueContext(builder_.Value(value));
	}

	Builder::DictItemContext Builder::ArrayItemContext::StartDict() {
		return builder_.StartDict();
	}

	Builder::ArrayItemContext Builder::ArrayItemContext::StartArray() {
		return builder_.StartArray();
	}

	Builder& Builder::ArrayItemContext::EndArray() {
		return builder_.EndArray();
	}

	// KeyContext

	Builder::KeyContext::KeyContext(Builder& builder)
		: Context(builder) {
	}

	Builder::DictItemContext Builder::KeyContext::Value(json::Node value) {
		return DictItemContext(builder_.Value(value));
	}

	Builder::DictItemContext Builder::KeyContext::StartDict() {
		return builder_.StartDict();
	}

	Builder::ArrayItemContext Builder::KeyContext::StartArray() {
		return builder_.StartArray();
	}

	// ValueContext

	Builder::ArrayValueContext::ArrayValueContext(Builder& builder)
		: Context(builder) {
	}

	Builder::ArrayValueContext Builder::ArrayValueContext::Value(json::Node value) {
		builder_.Value(value);
		return *this;
	}

	Builder::DictItemContext Builder::ArrayValueContext::StartDict() {
		return builder_.StartDict();
	}

	Builder::ArrayItemContext Builder::ArrayValueContext::StartArray() {
		return builder_.StartArray();
	}

	Builder& Builder::ArrayValueContext::EndArray() {
		return builder_.EndArray();
	}

	// Builder

	Builder& Builder::AddNode(json::Node node, NodeType node_type) {
		CheckReady();
		if (root_ == nullptr) {
			root_ = std::move(node);
			if (node_type == NodeType::START) {
				nodes_stack_.push_back(&root_);
			}
		}
		else {
			json::Node* back = nodes_stack_.back();
			if (back->IsArray()) {
				json::Array& array = const_cast<json::Array&>(back->AsArray());
				array.push_back(std::move(node));
				if (node_type == NodeType::START) {
					nodes_stack_.push_back(&array.back());
				}
			}
			else if (nodes_stack_.size() > 1u && nodes_stack_[nodes_stack_.size() - 2u]->IsDict()) {
				*back = std::move(node);
				if (node_type == NodeType::VALUE) {
					nodes_stack_.pop_back();
				}
			}
			else {
				throw std::logic_error("Cannot add new node"s);
			}
		}
		return *this;
	}

	Builder& Builder::Value(json::Node value) {
		return AddNode(value, NodeType::VALUE);
	}

	Builder::DictItemContext Builder::StartDict() {
		return DictItemContext(AddNode(json::Dict{}, NodeType::START));
	}

	Builder::ArrayItemContext Builder::StartArray() {
		return ArrayItemContext(AddNode(json::Array{}, NodeType::START));
	}

	Builder& Builder::EndDict() {
		return EndNode<json::Dict>();
	}

	Builder& Builder::EndArray() {
		return EndNode<json::Array>();
	}

	Builder::KeyContext Builder::Key(const std::string& key) {
		CheckReady();
		if (!nodes_stack_.back()->IsDict()) {
			throw std::logic_error("Key is allowed to be created only in the context of a dictionary"s);
		}
		json::Dict& dict = const_cast<json::Dict&>(nodes_stack_.back()->AsDict());
		nodes_stack_.push_back(&dict[key]);
		return KeyContext(*this);
	}

	json::Node Builder::Build() {
		if (root_ == nullptr || !nodes_stack_.empty()) {
			throw std::logic_error("Cannot build not ready object"s);
		}
		json::Node result{ std::move(root_) };
		root_ = nullptr;
		return result;
	}

	void Builder::CheckReady() const {
		if (root_ != nullptr && nodes_stack_.empty()) {
			throw std::logic_error("Cannot add new nodes to ready object"s);
		}
	}

}