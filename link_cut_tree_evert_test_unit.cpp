#include <iostream>
#include <vector>
#include <cstdlib>
#include <limits>

#include "statistics.h"
#include "link_cut_tree.h"

using namespace std;

typedef LinkCutTree<size_t, SumStatistic<size_t>, true > LCT;
typedef typename LCT::Node Node;

void link_cut_tree_evert_test(size_t N) {
	LCT lct;
	vector<Node *> node;
	vector<size_t> rank(N, 0);
	vector<size_t> dist(N, 0);
	vector<size_t> par(N, 0);
	// define the weight of the edge from node i
	for (size_t i = 1; i < N; ++i) {
		rank[i] = rand() % N + 1;
	}

	// build a tree of size N
	for (size_t i = 0; i < N; ++i) {
		node.push_back(lct.Add(rank[i]));
	}

	// build link
	for (size_t i = 1; i < N; ++i) {
		size_t num = rand()%i;
		par[i] = num;
		lct.Link(node[i], node[num]);
	}

	// compute the distance
	for (size_t i = 1; i < N; ++i) {
		size_t cur = i;
		while (par[cur] != 0) {
			dist[i] += rank[cur];
			cur = par[cur];
		}
		dist[i] += rank[cur];
	}

	// verify distance
	for (size_t i = 0; i < N; ++i) {
		SumStatistic<size_t> ss = lct.Path(node[i]);
		assert(ss.sum == dist[i]);
		// Evert
		Node * r = lct.FindRoot(node[i]);
		lct.Evert(node[i]);
		ss = lct.Path(r);
		assert(ss.sum == dist[i]);
		lct.Evert(r);
	}
	std::cout << "Done." << std::endl;
}


int main(int argc, const char *argv[])
{
	link_cut_tree_evert_test(10000);

	return 0;
}
