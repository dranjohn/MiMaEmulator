#pragma once

#include <functional>
#include <memory>

template<typename Data>
class BinarySearchTree;

template<typename Data>
class BinarySearchTreeNode {
	friend BinarySearchTree<Data>;

private:
	const size_t index;
	std::shared_ptr<Data> data;
	const std::function<std::shared_ptr<Data>(void)>& emptyConstructor;

	BinarySearchTreeNode<Data>* leftChild = nullptr;
	BinarySearchTreeNode<Data>* rightChild = nullptr;

	BinarySearchTreeNode(const size_t& index, const std::shared_ptr<Data>& data, const std::function<std::shared_ptr<Data>(void)>& emptyConstructor) :
		index(index),
		data(data),
		emptyConstructor(emptyConstructor)
	{}
	BinarySearchTreeNode(const size_t & index, const std::function<std::shared_ptr<Data>(void)>& emptyConstructor) :
		index(index),
		data(emptyConstructor()),
		emptyConstructor(emptyConstructor)
	{}

	~BinarySearchTreeNode() {
		if (leftChild != nullptr) delete leftChild;
		if (rightChild != nullptr) delete rightChild;
	}


	std::shared_ptr<Data> find(const size_t& index) {
		if (index < this->index) {
			if (leftChild == nullptr) {
				leftChild = new BinarySearchTreeNode<Data>(index, emptyConstructor);
				return leftChild->get();
			}
			return leftChild->find(index);
		}

		if (this->index < index) {
			if (rightChild == nullptr) {
				rightChild = new BinarySearchTreeNode<Data>(index, emptyConstructor);
				return rightChild->get();
			}
			return rightChild->find(index);
		}

		return this->data;
	}

	void place(const size_t& index, const std::shared_ptr<Data>& data) {
		if (index < this->index) {
			if (leftChild == nullptr) {
				leftChild = new BinarySearchTreeNode<Data>(data);
				return;
			}

			leftChild->place(index, data);
			return;
		}

		if (this->index < index) {
			if (rightChild == nullptr) {
				rightChild = new BinarySearchTreeNode<Data>(data);
				return;
			}

			rightChild->place(index, data);
			return;
		}

		this->data = data;
	}

	inline std::shared_ptr<Data> get() { return data; };
};

template<typename Data>
class BinarySearchTree {
private:
	const std::function<std::shared_ptr<Data>(void)>& emptyConstructor;
	BinarySearchTreeNode<Data> head;

public:
	BinarySearchTree(const size_t& headIndex, const std::function<std::shared_ptr<Data>(void)>& emptyConstructor) :
		emptyConstructor(emptyConstructor),
		head(headIndex, emptyConstructor)
	{}

	inline std::shared_ptr<Data> find(const size_t& index) { return head.find(index); }
	inline void place(const size_t& index, const std::shared_ptr<Data>& data) { head.place(index, data); }
};
