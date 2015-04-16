// This is navigator function to find elements at rank
// Rank starts from 0
class NavigatorBasic {
	public:
	enum Direction { LEFT, RIGHT, TARGET, LOST, UP };
	virtual void Init() = 0;
	template <class Node>	
	Direction Next(const Node *) {}
};

class RankNavigator : public NavigatorBasic {
	public:
	unsigned int rank, cnt;
	typedef SubtreeSizeStatistic SSStatistic;

	public:
	RankNavigator() {}
	RankNavigator(unsigned int rank) : rank(rank) {}
	// Essential Functions for Navigating
	void Init() {
		cnt = rank;
	}
	template <class Node>
	Direction Next(const Node * c) {
		Node *l = c->Left(), *r = c->Right();
		size_t lss = (l?l->stat.ss:0);
		assert(lss + (r?r->stat.ss:0) + c->stat.cnt == c->stat.ss);
		if (cnt <= lss) return LEFT;
		if (cnt <= lss + c->stat.cnt && cnt > lss) return TARGET;
		if (cnt > lss + c->stat.cnt) {
			cnt -= lss;
			cnt -= c->stat.cnt;
			return RIGHT;
		}
		return LOST;
	}
};

// Find the nearest predecessor node satisfying the cond operator
// The caller should guarantee the node satisfying the cond operator
// exists on the way to root from the starting node.
template <class Cond, class Node>
class PredNavigator : public NavigatorBasic {
	public:
	typedef typename Node::Statistic Stat;

	Stat stat;
	Cond cond;
	const Node *prev;
	bool up;

	PredNavigator(){}
	PredNavigator(Cond cond) : cond(cond) {}

	void Init() {
		prev = nullptr;
		up = true;
	}

	Direction Next(const Node *c) {
		Direction next = LOST;
		Stat tmp_stat;
		if (up) {
			if (prev && prev == c->Left()) next = UP;
			else { 
				// First visit
				if (!prev) stat.Init(c->key);
				if (prev) {
					// Test visiting node and build new statistic
					tmp_stat.Init(c->key);
					if (cond(tmp_stat, stat)) return TARGET;
					stat.UpdateLeft(tmp_stat);
				}
				next = UP;
				if (c->Left()) {
					// Test left node of c and build new statistic
					tmp_stat = c->Left()->stat;
					if (cond(tmp_stat, stat)) up = false, next = LEFT;
					else stat.UpdateLeft(tmp_stat);
				}
			}
		} else if (!up) { // On the way down
			assert(cond(c->stat, stat));
			if (c->Right()) {
				// Test if the target is on the right side
				tmp_stat = c->Right()->stat;
				if (cond(tmp_stat, stat)) next = RIGHT;
				else stat.UpdateLeft(tmp_stat);
			}
			tmp_stat.Init(c->key);
			if (next == LOST) {
				// Test if the target is current visit node
				if (cond(tmp_stat, stat)) return TARGET;
				else {
					stat.UpdateLeft(tmp_stat);
					next = LEFT;
				}
			}
		}
		prev = c;
		return next;
	}
};

