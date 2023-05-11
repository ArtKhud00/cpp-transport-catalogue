#include "json_builder.h"

namespace json {


	KeyContext Builder::Key(std::string key) {
		if (key_ == std::nullopt && CheckIfDict()) {
			key_ = std::move(key);
		}
		else {
			throw std::logic_error("Error calling Key");
		}
		return KeyContext{ *this };
	}

	BaseContext Builder::Value(Node::Value value) {
		if (root_ == nullptr) {
			root_.GetValue() = std::move(value);
			return BaseContext{ *this };
		}
		if (nodes_stack_.empty()) {
			throw std::logic_error("Error calling Value, wrong position");
		}
		if (nodes_stack_.back()->IsArray()) {
			json::Array& Array_ = std::get<json::Array>(nodes_stack_.back()->GetValue());
			Array_.emplace_back(std::move(value));
		}
		else if (nodes_stack_.back()->IsDict()) {
			if (key_ != std::nullopt) {

				json::Dict& dict = std::get<json::Dict>(nodes_stack_.back()->GetValue());
				dict.emplace(key_.value(), value);
				key_ = std::nullopt;
			}
			else {
				throw std::logic_error("Calling Value for key field, not for value");
			}
		}
		else {
			throw std::logic_error("Unexpected calling Value");
		}
		return BaseContext{ *this };

	}

	DictItemContext Builder::StartDict() {
		if (root_ == nullptr) {
			root_ = Dict();
			nodes_stack_.emplace_back(&root_);
			return DictItemContext{ *this };
		}
		if (nodes_stack_.empty()) {
			throw std::logic_error("Error calling StartDict, wrong position");
		}
		if (nodes_stack_.back()->IsArray()) {
			const_cast<Array&>(nodes_stack_.back()->AsArray()).push_back(Dict());
			nodes_stack_.emplace_back(&const_cast<Array&>(nodes_stack_.back()->AsArray()).back());
		}
		else if (nodes_stack_.back()->IsDict()) {
			if (key_ != std::nullopt) {
				const_cast<Dict&>(nodes_stack_.back()->AsDict())[key_.value()] = Dict();
				nodes_stack_.emplace_back(&const_cast<Dict&>(nodes_stack_.back()->AsDict()).at(key_.value()));
				key_ = std::nullopt;
			}
			else {
				throw std::logic_error("Error calling StartDict, expected calling at value position not at key");
			}
		}
		else {
			throw std::logic_error("Unexpected calling StartDict");
		}
		return DictItemContext{ *this };
	}

	ArrayItemContext Builder::StartArray() {
		if (root_ == nullptr) {
			root_ = Array();
			nodes_stack_.emplace_back(&root_);
			return ArrayItemContext{ *this };
		}
		if (nodes_stack_.empty()) {
			throw std::logic_error("Error calling StartArray, wrong position");
		}
		if (nodes_stack_.back()->IsArray()) {
			const_cast<Array&>(nodes_stack_.back()->AsArray()).push_back(Array());
			nodes_stack_.emplace_back(&const_cast<Array&>(nodes_stack_.back()->AsArray()).back());
		}
		else if (nodes_stack_.back()->IsDict()) {
			if (key_ != std::nullopt) {
				const_cast<Dict&>(nodes_stack_.back()->AsDict())[key_.value()] = Array();
				nodes_stack_.emplace_back(&const_cast<Dict&>(nodes_stack_.back()->AsDict()).at(key_.value()));
				key_ = std::nullopt;
			}
			else {
				throw std::logic_error("Error calling StartArray, expected calling at value position not at key");
			}
		}
		else {
			throw std::logic_error("Unexpected calling StartArray");
		}
		return ArrayItemContext{ *this };
	}

	BaseContext Builder::EndDict() {
		if (nodes_stack_.empty()) {
			throw std::logic_error("Calling EndDict with empty stack");
		}
		else if (!nodes_stack_.back()->IsDict()) {
			throw std::logic_error("Calling EndDict without StartDict");
		}
		else {
			nodes_stack_.pop_back();
		}
		return BaseContext{ *this };
	}

	BaseContext Builder::EndArray() {
		if (nodes_stack_.empty()) {
			throw std::logic_error("Calling EndArray with empty stack");
		}
		else if (!nodes_stack_.back()->IsArray()) {
			throw std::logic_error("Calling EndArray without StartArray");
		}
		else {
			nodes_stack_.pop_back();
		}
		return BaseContext{ *this };
	}

	json::Node Builder::Build() {
		if (root_ == nullptr) {
			throw std::logic_error("Error calling Build for empty document");
		}
		else if (!nodes_stack_.empty()) {
			throw std::logic_error("Error calling Build, document not finished");
		}
		return root_;
	}

	bool Builder::CheckIfNull() const {
		return !nodes_stack_.empty() && nodes_stack_.back()->IsNull();
	}

	bool Builder::CheckIfDict() const {
		return !nodes_stack_.empty() && nodes_stack_.back()->IsDict();
	}

	bool Builder::CheckIfArr() const {
		return !nodes_stack_.empty() && nodes_stack_.back()->IsArray();
	}


	KeyContext BaseContext::Key(std::string key) {
		return builder_.Key(std::move(key));
	}

	BaseContext BaseContext::Value(Node::Value value) {
		return builder_.Value(value);
	}

	DictItemContext BaseContext::StartDict() {
		return builder_.StartDict();
	}

	ArrayItemContext BaseContext::StartArray() {
		return builder_.StartArray();
	}

	BaseContext BaseContext::EndDict() {
		return builder_.EndDict();
	}

	BaseContext BaseContext::EndArray() {
		return builder_.EndArray();
	}

	json::Node BaseContext::Build() {
		return builder_.Build();
	}

	DictItemContext KeyContext::Value(Node::Value val) {
		return BaseContext::Value(std::move(val));
	}

	ArrayItemContext KeyContext::StartArray() {
		return BaseContext::StartArray();
	}

	DictItemContext KeyContext::StartDict() {
		return BaseContext::StartDict();
	}

	ArrayItemContext ArrayItemContext::Value(Node::Value val) {
		return BaseContext::Value(std::move(val));
	}

	DictItemContext ArrayItemContext::StartDict() {
		return BaseContext::StartDict();
	}

	ArrayItemContext ArrayItemContext::StartArray() {
		return BaseContext::StartArray();
	}

	KeyContext DictItemContext::Key(std::string key) {
		return BaseContext::Key(key);
	}

}