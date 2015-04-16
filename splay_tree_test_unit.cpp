#include <iostream>
#include <vector>
#include <cstdlib>
#include "statistics.h"
#include "splay_tree.h"
#include "navigator.h"

using namespace std;

// TODO: Test on the element not in the splay tree 
// but the interval of the elements in the splay tree
// cover the element
//
// TODO: Test with a new SubtreeSizeStatistic and navigate

void splay_tree_test(size_t N) {
	SplayTree<int, SubtreeSizeStatistic> st;
	for (size_t i = 0 ; i < N ; i++) {
		st.Insert(i);
	}
	for (size_t i = 0 ; i < N ; i++) {
		size_t r = rand() % N;
		SubtreeSizeStatistic s = st.StatisticComp(r);
		assert(s.ss == r + 1);
	}
	for (size_t i = 0; i < N; ++i) {
		RankNavigator nav(i+1);
		assert(st.Find(nav) == i);
	}
	for (size_t i = N-1; i < N; --i) {
		st.Erase(i);
		size_t r = rand() % N;
		SubtreeSizeStatistic s = st.StatisticComp(r);
		if (r >= i) assert(s.ss == i);
		else assert(s.ss == r + 1);
	}
	std::cout << "I'm Done" << std::endl;
}

int main(int argc, const char *argv[])
{
	splay_tree_test(10000);
	return 0;
}
