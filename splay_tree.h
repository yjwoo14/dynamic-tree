#ifndef __SPRAY_TREE_H__
#define __SPRAY_TREE_H__

#include <cassert>
#include <unordered_set>

// TODO: Insert multiple elements of same value..

template <class Node>
struct BasicTreeNode {
	Node *p, *l, *r;
	BasicTreeNode (Node *p = NULL, Node *l = NULL,  Node *r = NULL)
		: p(p), l(l), r(r) {}
	~BasicTreeNode() { delete l, delete r; }
	Node *&Parent() { return p; }
	Node *&Left() { return l; }
	Node *&Right() { return r; }

	Node *Parent() const { return p; }
	Node *Left() const { return l; }
	Node *Right() const { return r; }
};

template <class T, class ST>
struct SplayNode : BasicTreeNode < SplayNode<T, ST> > {
	typedef ST Statistic;
	T key;
	ST stat;
	SplayNode (const T &key, SplayNode<T,ST> *p = NULL, SplayNode<T,ST> *l = NULL, SplayNode<T,ST> *r = NULL)
		: key(key), BasicTreeNode < SplayNode<T, ST> >(p,l,r) {}

	void Update() {
		stat.Init(key);
		if (this->Left()) stat.UpdateLeft(this->Left()->stat);
		if (this->Right()) stat.UpdateRight(this->Right()->stat);
	}
};

template < class T, class Node, class Comp >
class SplayTreeBase {
public:
	// ***************************************************
	// Static functions required by dynamic tree structure
	// ***************************************************
	// Functions need to be accessed only by link cut tree
	// Splay a node to the root (but not setting to root)
	static void SplayNode(Node* x) {
		if (!x) return;
		while (!IsRoot(x)) {
			Node* y = x->Parent();
			if (IsRoot(y)) Rotate(x);
			else if ((y->Parent()->Left() == y && x == y->Left())
					||(y->Parent()->Right() == y && x == y->Right()))
				Rotate(y), Rotate(x);
			else Rotate(x), Rotate(x);
		} 
	}

	static bool IsRoot(const Node* x) {
		// !x->Parent() condition is not enough for link-cut tree
		return (!x->Parent() || (x->Parent()->Left() != x && x->Parent()->Right() != x));
	}

	// Recompute information in a node (if needed)
	static void Update(Node* x) {
		if (!x) return;
		x->Update();
	}

	// Rotate x one level up
	static void Rotate(Node* x) {
		if (IsRoot(x)) return;
		Node* y = x->Parent();
		x->Parent() = y->Parent();
		if (!IsRoot(y)) (y->Parent()->Left()==y)?(y->Parent()->Left()=x):(y->Parent()->Right()=x);
		y->Parent() = x;
		if (y->Left() == x) {
			if (x->Right()) x->Right()->Parent() = y;
			y->Left() = x->Right(), x->Right() = y;
		} else {
			if (x->Left()) x->Left()->Parent() = y;
			y->Right() = x->Left(), x->Left() = y;
		}
		assert(!(x->Left()) || !Comp()(x->key, x->Left()->key)); 
		assert(!(x->Right()) || !Comp()(x->Right()->key, x->key));
		// TODO: better update strategy?
		Update(y), Update(x);
		if (!IsRoot(x)) Update(x->Parent());
	}

	static Node* CreateNode(const T& x, Node* p = NULL, Node* l = NULL, Node* r = NULL) {
		Node *node = new Node(x,p,l,r);
		node->stat.Add();
		node->Update();
		return node;
	}

	static Node *Pred(Node *x) {
		SplayNode(x);
		if (!x->Left()) return NULL;
		x = x->Left();
		while (x->Right()) x = x->Right();
		SplayNode(x);
		return x;
	}

	static Node *Succ(Node *x) {
		SplayNode(x);
		if (!x->Right()) return NULL;
		x = x->Right();
		while (x->Left()) x = x->Left();
		SplayNode(x);
		return x;
	}

	static bool InOrder(Node *x, Node *y) {
		if (x == y) return false;
		SplayNode(x);
		while (y->Parent() != x) {
			y = y->Parent();
			assert(y);
		}
		bool ret = false;
		if (x->Right() == y) ret = true;
		SplayNode(y); // Amortization
		return ret;
	}
};

template < class T, class ST, class Node = SplayNode<T, ST>, class Comp = std::less<T> >
class SplayTree : public SplayTreeBase<T, Node, Comp> {
public:
	typedef SplayTreeBase<T, Node, Comp> STBase;

	// *******************************************
	// Basic interfaces for basic splay tree usage
	// *******************************************
	// Construct an empty splay tree
	SplayTree() : root(NULL) {}
	// Disassemble the tree
	~SplayTree() { delete root; }

	// Insert key x in the tree
	void Insert(const T&x) {
		if (!root) { root = STBase::CreateNode(x); Splay(root); return; }
		Node* cur = Search(x);
		if (Comp()(x, cur->key)) cur->Left() = STBase::CreateNode(x, cur), Splay(cur->Left());
		else if (Comp()(cur->key, x)) cur->Right() = STBase::CreateNode(x, cur), Splay(cur->Right());
		else cur->stat.Add(), Splay(cur);
	}

	// Erase elements with key x
	void Erase(const T& x) {
		if (!root) return;
		Splay(Search(x)); 
		if (x != root->key) return;
		root->stat.Remove(); 
		if (root->stat.Exist()) return;
		while (root->Left() && root->Left()->Right()) STBase::Rotate(root->Left()->Right());
		Node* tmp = root;
		if (!root->Left() && !root->Right()) root = NULL;
		else if (!root->Left()) root = root->Right();
		else if (!root->Right()) root = root->Left();
		else { 
			root->Left()->Right() = root->Right(); 
			root->Left()->Right()->Parent() = root->Left(); 
			root = root->Left(); 
		}
		if (root) root->Parent() = NULL, STBase::Update(root->Left()), STBase::Update(root->Right());
		STBase::Update(root);
		tmp->Left() = tmp->Right() = NULL;
		delete tmp;
	}

	// *******************
	// Statistic Functions
	// *******************
	// Get Statistic Functions
	ST Statistic(const T& x) {
		return Splay(Search(x)), (root && x == root->key)?root->stat:ST();
	}

	// Make Statistics on the elements such that Compare()(elements, x) is true 
	// or !Compare()(elements, x) and !Compare()(x, elements) (meaning the same)
	ST StatisticComp(const T& x) {
		if (!root) return ST();
		Splay(Search(x));
		ST ret;
		if ((!Comp()(root->key, x) && !Comp()(x, root->key)) ||
				(Comp()(root->key, x))) {
			ret = root->stat;
			ret.Init(root->key);
		}
		if (root->Left()) ret.UpdateLeft(root->Left()->stat);
		return ret;
	}

	// According to given navigator
	// it follows the tree and makes a new statistic
	template<class Navigator>
	T Find(Navigator nav, const T & empty = T()) {
		if (!root) return empty;
		Node* cur = root;
		nav.Init();
		while (cur) {
			typename Navigator::Direction next = nav.Next(cur);
			switch (next) {
				case Navigator::LEFT:
					cur = cur->Left(); break;
				case Navigator::RIGHT:
					cur = cur->Right();	break;
				case Navigator::TARGET:
					Splay(cur); // amortization
					return cur->key;
				case Navigator::LOST: 
					return empty;
				// UP is not allowed here (time complexity)
				case Navigator::UP:
					assert(false);
			}
		}
		return empty;
	}


	/******************************
	 * Static Statistic Functions *
	 ******************************/
	static ST RangeStatistic(Node *f, Node *t) {
		ST stat;
		Node *cur = f;
		STBase::SplayNode(t);
		stat.Init(cur->key);
		if (cur->Right() && cur->Parent()) stat.UpdateRight(cur->Right()->stat);
		while (cur->Parent()) {
			Node *parent = cur->Parent();
			assert(parent);
			if (cur == parent->Left()) {
				ST tmp;
				tmp.Init(parent->key);
				stat.UpdateRight(tmp);
				if (parent->Right() && parent != t)
					stat.UpdateRight(parent->Right()->stat);
			}
			cur = parent;
		}
		STBase::SplayNode(f); // Amortization
		return stat;
	}

	// This function allow to move both up and down direction
	// The caller should make sure the amortized cost, 
	// that is, caller should manage splaynode(f) at some point
	template <class Navigator>
	static Node * NavigatorSearch(Navigator nav, Node *f, Node *t) {
		STBase::SplayNode(t);
		Node * cur = f;
		nav.Init();
		while (cur) {
			typename Navigator::Direction next = nav.Next(cur);
			switch (next) {
				case Navigator::LEFT:
					cur = cur->Left(); break;
				case Navigator::RIGHT:
					cur = cur->Right();	break;
				case Navigator::UP:
					cur = cur->Parent(); break;
				case Navigator::TARGET:
					STBase::SplayNode(cur); // amortization
					return cur;
				case Navigator::LOST: 
					return NULL;
			}
		}
		return NULL;
	}

private:
	Node* Search(const T& x) {
		for (Node* cur = root; cur;) {
			if (Comp()(x, cur->key)) { if (cur->Left())cur=cur->Left(); else return cur; }
			else if (Comp()(cur->key, x)) { if (cur->Right())cur=cur->Right(); else return cur; }
			else return cur;
		}
		return NULL;
	}

	void Splay(Node* x) {
		STBase::SplayNode(x);
		root = x;
	}

	Node *root;
};

#endif
