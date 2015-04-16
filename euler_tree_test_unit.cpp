#include <iostream>
#include <vector>
#include <cstdlib>
#include <limits>
#include <cmath>

#include "statistics.h"
#include "euler_tree.h"

using namespace std;

typedef EulerTree<size_t> ET;
typedef EulerTree<size_t, false> ET2;
typedef LCAEulerTree<size_t> ET3;

template <class EulerTree>
void LCATest(size_t n) {
	typedef typename EulerTree::Node Node;
	typedef typename EulerTree::Edge Edge;

	EulerTree Tree;
	std::vector<Node *> nodes;
	std::vector<Edge> edges;
	for (size_t i = 0 ; i < n ; ++i) {
		nodes.push_back(Tree.Add(i));
	}

	// Build Complete Binary Tree
	for (size_t i = 1 ; i < n ; ++i) {
		edges.push_back(Tree.Link(nodes[i], nodes[(i-1)/2]));
	}

	for (size_t i = 0 ; i < n ; ++i) {
		size_t a = rand()%n, b = rand()%n;
		size_t d = rand()%edges.size();
		Tree.Cut(edges[d]);
		edges[d] = Tree.Link(nodes[d+1], nodes[d/2]);
		Node *lca = Tree.FindLCA(nodes[a], nodes[b]);
		if (a > b) std::swap(a, b);
		// go to the same depth
		while (floor(log2(b+1)) > floor(log2(a+1))) b = (b-1)/2;
		while (a != b) {
			assert(a > 0 && b > 0);
			a = (a-1)/2;
			b = (b-1)/2;
		}
		assert(lca == nodes[a]);
	}

	std::cout << "LCA Test Done" << std::endl;

}

void EvertableETSimpleTest(size_t n) {
	typedef typename ET::Node Node;
	typedef typename ET::Edge Edge;

	ET Tree;
	std::vector<Node *> nodes;
	std::vector<Edge> edges;
	for (size_t i = 0 ; i < n ; ++i) {
		nodes.push_back(Tree.Add(i));
	}

	for (size_t i = 2 ; i < n ; ++i) {
		edges.push_back(Tree.Link(nodes[i], nodes[i-2]));
	}

	for (size_t i = 0 ; i < n ; ++i) {
		if (i%2 == 0) assert(Tree.FindRoot(nodes[i]) == nodes[0]);
		else assert(Tree.FindRoot(nodes[i]) == nodes[1]);
		Tree.Evert(nodes[n-1]), Tree.Evert(nodes[n-2]);
		assert(Tree.FindRoot(nodes[i]) == nodes[n-1] || Tree.FindRoot(nodes[i]) == nodes[n-2]);
		Tree.Evert(nodes[0]), Tree.Evert(nodes[1]);
	}

	Tree.Evert(nodes[n-1]), Tree.Evert(nodes[n-2]);
	for (size_t i = 0 ; i < n-2 ; ++i) {
		assert(Tree.Parent(nodes[i]) == nodes[i+2]);
	}

	for (size_t i = edges.size()-3 ; i < edges.size() ; --i) {
		Node *v = edges[i]->key.node;
		Tree.Cut(edges[i]);
		if (i % 2 == 0) assert(Tree.FindRoot(nodes[0]) == v);
		else assert(Tree.FindRoot(nodes[1]) == v);
	}

	std::cout << "Evertable Simple Test Done" << std::endl;
}

void NonEvertableETSimpleTest(size_t n) {
	typedef typename ET2::Node Node;
	typedef typename ET2::Edge Edge;

	ET2 Tree;
	std::vector<Node *> nodes;
	std::vector<Edge> edges;
	for (size_t i = 0 ; i < n ; ++i) {
		nodes.push_back(Tree.Add(i));
	}

	for (size_t i = 2 ; i < n ; ++i) {
		edges.push_back(Tree.Link(nodes[i], nodes[i-2]));
	}

	for (size_t i = 0 ; i < n ; ++i) {
		if (i%2 == 0) assert(Tree.FindRoot(nodes[i]) == nodes[0]);
		else assert(Tree.FindRoot(nodes[i]) == nodes[1]);
	}

	for (size_t i = 0 ; i < n-2 ; ++i) {
		assert(Tree.Parent(nodes[i+2]) == nodes[i]);
	}

	for (size_t i = edges.size()-3 ; i < edges.size() ; --i) {
		Tree.Cut(edges[i]);
		assert(Tree.FindRoot(nodes[i+2]) == nodes[i+2]);
	}

	std::cout << "Non Evertable Simple Test Done" << std::endl;
}
int main(int argc, const char *argv[])
{
	srand(time(NULL));
	EvertableETSimpleTest(10000);
	NonEvertableETSimpleTest(10000);
	LCATest<ET>(10000);
	LCATest<ET2>(10000);
	LCATest<ET3>(100000);
	return 0;
}
