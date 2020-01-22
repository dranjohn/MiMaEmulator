#pragma once

#include <functional>
#include <memory>
#include <stack>
#include <utility>

template<typename Data>
class BinarySearchTree;

template<typename Data>
class BinarySearchTreeNode;

template<typename Data>
class BinarySearchTreeIterator;



// -----------------------
// Binary search tree node
// 
// A single node in a
// binary search tree with
// a left and right child
// node for higher and
// lower indices,
// respectively.
// -----------------------

template<typename Data>
class BinarySearchTreeNode {
	friend BinarySearchTree<Data>;
	friend BinarySearchTreeIterator<Data>;

private:
	const size_t index; //the index/value of the data stored in this node, which the search tree sorts/searches by
	std::shared_ptr<Data> data; //the actual data stored in this node
	const std::function<std::shared_ptr<Data>(void)>& emptyConstructor; //a reference to the constructor creating empty data pieces in case a non-existent node is searched for

	//left (lower) and right (higher) child
	std::shared_ptr<BinarySearchTreeNode<Data>> leftChild;
	std::shared_ptr<BinarySearchTreeNode<Data>> rightChild;


	//creates a node with given data
	BinarySearchTreeNode(const size_t& index, const std::shared_ptr<Data>& data, const std::function<std::shared_ptr<Data>(void)>& emptyConstructor) :
		index(index),
		data(data),
		emptyConstructor(emptyConstructor)
	{}
	//create a node with empty data
	BinarySearchTreeNode(const size_t & index, const std::function<std::shared_ptr<Data>(void)>& emptyConstructor) :
		index(index),
		data(emptyConstructor()),
		emptyConstructor(emptyConstructor)
	{}


private:
	//find data by its index
	std::shared_ptr<Data> find(const size_t& index) {
		//if the searched index is lower than the index of this node, look for the data in the left sub-tree
		if (index < this->index) {
			if (leftChild == nullptr) {
				leftChild.reset(new BinarySearchTreeNode<Data>(index, emptyConstructor));
				return leftChild->data;
			}
			return leftChild->find(index);
		}

		//if the searched index is higher than the index of this node, look for the data in the right sub-tree
		if (this->index < index) {
			if (rightChild == nullptr) {
				rightChild.reset(new BinarySearchTreeNode<Data>(index, emptyConstructor));
				return rightChild->data;
			}
			return rightChild->find(index);
		}

		//assertion: index == this->index
		//the data searched for is stored in this node
		return this->data;
	}

	//create a new node or update an existing node with the given data
	void place(const size_t& index, const std::shared_ptr<Data>& data) {
		//if the given index is lower than the index of this node, place the data in the left sub-tree
		if (index < this->index) {
			if (leftChild == nullptr) {
				//if the left sub-tree doesn't exist, create a new node with the given index and data
				leftChild = new BinarySearchTreeNode<Data>(index, data, emptyConstructor);
				return;
			}

			//assertion: the left sub-tree exists
			//place the data under the given index in the left sub-tree
			leftChild->place(index, data);
			return;
		}

		//if the given index is higher than the index of this node, place the data in the right sub-tree
		if (this->index < index) {
			if (rightChild == nullptr) {
				//if the right sub-tree doesn't exist, create a new node with the given index and data
				rightChild = new BinarySearchTreeNode<Data>(index, data, emptyConstructor);
				return;
			}

			//assertion: the right sub-tree exists
			//place the data under the given index in the right sub-tree
			rightChild->place(index, data);
			return;
		}

		//assertion: index == this->index
		//update this node
		this->data = data;
	}

public:
	bool operator==(const BinarySearchTreeNode<Data>& other) const {
		return (*(this->data)) == (*(other.data))
			&& this->index == other.index
			&& (this->leftChild == nullptr) ? (other.leftChild == nullptr) : (other.leftChild != nullptr && *(this->leftChild) == *(other.leftChild))
			&& (this->rightChild == nullptr) ? (other.rightChild == nullptr) : (other.rightChild != nullptr && *(this->rightChild) == *(other.rightChild));
	}
	inline bool operator!=(const BinarySearchTreeNode<Data>& other) const {
		return !(*this == other);
	}
};



// --------------------
// Binary search tree
//
// Stores the head node
// and keeps a copy of
// the constructor for
// empty data elements.
// --------------------

template<typename Data>
class BinarySearchTree {
	friend BinarySearchTreeIterator<Data>;
	friend BinarySearchTreeIterator<Data>;

private:
	const std::function<std::shared_ptr<Data>(void)> emptyConstructor;
	BinarySearchTreeNode<Data> root;

public:
	BinarySearchTree(const size_t& headIndex, const std::function<std::shared_ptr<Data>(void)>& emptyConstructor) :
		emptyConstructor(emptyConstructor),
		root(headIndex, emptyConstructor)
	{}

	inline std::shared_ptr<Data> find(const size_t& index) { return root.find(index); }
	inline void place(const size_t& index, const std::shared_ptr<Data>& data) { root.place(index, data); }
};



// ---------------------------
// Binary search tree iterator
//
// Iterates over the data
// stored in a binary search
// tree from lowest to highest
// index.
// ---------------------------

template<typename Data>
class BinarySearchTreeIterator {
private:
	//the stack of nodes for the depth first search
	std::stack<BinarySearchTreeNode<Data>> dfsStack;

private:
	//builds the depth first stack using the top element of the dfs stack as a starting point
	//to recursively add all left children
	void buildDFSStack() {
		if (dfsStack.empty()) {
			return;
		}

		while (dfsStack.top().leftChild != nullptr) {
			dfsStack.push(*(dfsStack.top().leftChild));
		}
	}

	//removes the top element from the dfs stack
	//if the top element had a right child, add it to the dfs stack and build it from there
	void next() {
		if (dfsStack.empty()) {
			return;
		}

		BinarySearchTreeNode<Data> topElement = dfsStack.top();
		dfsStack.pop();

		if (topElement.rightChild != nullptr) {
			dfsStack.push(*(topElement.rightChild));
			buildDFSStack();
		}
	}

public:
	BinarySearchTreeIterator() {}
	BinarySearchTreeIterator(const BinarySearchTree<Data>& tree) { dfsStack.push(tree.root); buildDFSStack(); }

	//(in)equality operator
	bool operator==(const BinarySearchTreeIterator<Data>& other) const {
		std::stack<BinarySearchTreeNode<Data>> thisDFSStack = dfsStack;
		std::stack<BinarySearchTreeNode<Data>> otherDFSStack = other.dfsStack;

		while (!thisDFSStack.empty()) {
			if (otherDFSStack.empty()) {
				return false;
			}

			if (thisDFSStack.top() != otherDFSStack.top()) {
				return false;
			}

			thisDFSStack.pop();
			otherDFSStack.pop();
		}

		if (!otherDFSStack.empty()) {
			return false;
		}

		return true;
	}
	inline bool operator!=(const BinarySearchTreeIterator<Data>& other) const { return !(*this == other); }

	//pre/post-increment operator
	inline BinarySearchTreeIterator<Data>& operator++() { next(); return *this; };
	inline BinarySearchTreeIterator<Data>& operator++(int) { BinarySearchTreeIterator<Data>& ref = *this; next(); return ref; };

	//dereference operator
	inline std::pair<size_t, std::shared_ptr<Data>> operator*() { return { dfsStack.top().index, dfsStack.top().data }; }
};
