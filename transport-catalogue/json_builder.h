#pragma once
#include "json.h"
#include <optional>

namespace json {

	class Builder;
	class BaseContext;
	class KeyContext;
	class DictItemContext;
	class ArrayItemContext;
	class KeyValueContext;
	class ArrayValueContext;

	class Builder {
	public:

		//Builder& Key(std::string);

		//Builder& Value(Node::Value);

		//Builder& StartDict();

		//Builder& StartArray();

		//Builder& EndDict();

		//Builder& EndArray();

		KeyContext Key(std::string);

		BaseContext Value(Node::Value);

		DictItemContext StartDict();

		ArrayItemContext StartArray();

		BaseContext EndDict();

		BaseContext EndArray();

		json::Node Build();


	private:
		/*Node root_ = nullptr;
		std::vector<Node*> nodes_stack_ = { &root_ };
		std::optional<std::string> key_ = std::nullopt;*/

		Node root_ = nullptr;
		std::vector<Node*> nodes_stack_;
		std::optional<std::string> key_;

		json::Node* AddValue(json::Node::Value value);
		bool CheckIfNull() const;
		bool CheckIfDict() const;
		bool CheckIfArr() const;

	};


	class BaseContext {
	public:
		BaseContext(Builder& builder) : builder_(builder) {}

		KeyContext Key(std::string);

		BaseContext Value(Node::Value);

		ArrayItemContext StartArray();

		DictItemContext StartDict();

		BaseContext EndArray();

		BaseContext EndDict();

		json::Node Build();

	private:
		Builder& builder_;
	};

	class KeyContext : public BaseContext {
	public:
		KeyContext(Builder& builder) : BaseContext(builder) {}
		DictItemContext Value(Node::Value);
		ArrayItemContext StartArray();
		DictItemContext StartDict();

		KeyContext Key(std::string) = delete;
		BaseContext EndDict() = delete;
		BaseContext EndArray() = delete;
		json::Node Build() = delete;
	};

	class ArrayItemContext : public BaseContext {
	public:
		ArrayItemContext(Builder& builder) : BaseContext(builder) {}
		ArrayItemContext(BaseContext base_context) : BaseContext(base_context) {}
		ArrayItemContext Value(Node::Value);
		DictItemContext StartDict();
		ArrayItemContext StartArray();

		KeyContext Key(std::string) = delete;
		BaseContext EndDict() = delete;
		json::Node Build() = delete;
	};

	class DictItemContext : public BaseContext {
	public:
		DictItemContext(Builder& builder) : BaseContext(builder) {}
		DictItemContext(BaseContext base_context) : BaseContext(base_context) {}
		KeyContext Key(std::string);

		DictItemContext StartDict() = delete;
		ArrayItemContext StartArray() = delete;
		BaseContext Value(Node::Value) = delete;
		BaseContext EndArray() = delete;
		json::Node Build() = delete;
	};
}

