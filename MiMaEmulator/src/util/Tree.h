#pragma once

#include <vector>

template<typename Data>
class Tree;

template<typename Data>
class DataNode {
	friend Tree<Data>;
	typedef typename std::vector<DataNode<Data>>::const_iterator DataNodeIterator;

private:
	Data data;
	std::vector<DataNode<Data>> children;

	DataNode(const Data& data) : data(data) {}

public:
	inline void setData(const Data& data) { this->data = data; }
	inline Data getData() const { return data; }

	inline DataNode<Data>& addChild(const Data& data) { children.push_back(data); return children.back(); }
	inline DataNodeIterator getChildren() const { return children.begin(); }
	inline DataNodeIterator getChildrenEnd() const { return children.end(); }
};

template<typename Data>
class Tree {
private:
	DataNode<Data> root;

public:
	Tree(Data rootData) : root(rootData) {}

	inline DataNode<Data>& getRoot() { return root; }
};
