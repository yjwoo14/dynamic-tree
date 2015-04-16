#include <iostream>
#include <vector>
#include <cstdlib>
#include <limits>

#include "statistics.h"
#include "link_cut_tree.h"

using namespace std;

// TODO: Test with a statistic 
typedef LinkCutTree<size_t, SumStatistic<size_t> > LCT2;
typedef typename LCT2::Node Node2;

void link_cut_tree_sum_test(size_t N) {
	LCT2 lct;
	vector<Node2 *> node;
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
	}

	// advanced verification
	for (size_t i = 0; i < N; ++i) {
		size_t f = rand() % (N - 1) + 1;
		size_t t = rand() % f;
		par[f] = t, dist[f] = 0;
		size_t cur = f;
		while (par[cur] != 0) {
			dist[f] += rank[cur];
			cur = par[cur];
		}
		dist[f] += rank[cur];
		lct.Cut(node[f]);
		lct.Link(node[f], node[t]);
		SumStatistic<size_t> ss = lct.Path(node[f]);
		assert(ss.sum == dist[f]);
	}

	std::cout << "Sum test Done" << std::endl;
}

typedef LinkCutTree<size_t, SubtreeSizeStatistic> LCT;
typedef typename LCT::Node Node;

void link_cut_tree_simple_test(size_t N) {
	LCT lct;
	vector<Node *> node;

	// Add N vertices
	for (size_t i = 0; i < N; ++i) {
		node.push_back(lct.Add(i));
	}

	// Build a giant path
	for (size_t i = 1; i < N; ++i) {
		lct.Link(node[i], node[i-1]);
		assert(lct.Path(node[i]).ss == i+1);
		assert(lct.Path(node[0]).ss == 1);
	}

	// Compute the length of the path from 0 to node i
	for (size_t i = 0; i < N; ++i) {
		assert(lct.Path(node[i]).ss == i+1);
	}

	// Compute the length of the path from the root of the tree to node i
	for (size_t i = 1; i < N; ++i) {
		lct.Cut(node[i]);
		assert(lct.FindRoot(node[i]) == node[i]);
		assert(lct.Path(node[i]).ss == 1);
	}

	// Remove all vertices
	for (size_t i = 0; i < N; ++i) {
		lct.Remove(node[i]);
	}

	std::cout << "I'm Done" << std::endl;
}

int main(int argc, const char *argv[])
{
	link_cut_tree_simple_test(10000);
	link_cut_tree_sum_test(10000);
	return 0;
}
