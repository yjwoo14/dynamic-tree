#ifndef __EULER_TREE_H__
#define __EULER_TREE_H__

#include <unordered_set>
#include "splay_tree.h"
#include "statistics.h"

template <class T, bool Evertable = true, class Stat = Statistic>
class EulerTree {
public:
	class STKey;
	struct FalseComp {
		bool operator()(const STKey& l, const STKey& r) const {
			return false;
		}
	};

	class Node;
	typedef std::reference_wrapper<const Node> NodeRef;
	typedef SplayNode<STKey, Stat> STNode;
	typedef SplayTree<STKey, Stat, STNode, FalseComp > ST;
	typedef STNode* Edge;
	typedef T ItemType;
	const bool EVERTABLE = Evertable;

	struct Node {
		T key;
		STNode *repr;
		Node (const T& k) : key(k), repr(nullptr) {}
	};

	struct STKey {
		Node *node;
		STNode *prev;
		STNode *next;
		STKey(Node *k) : node(k), prev(nullptr), next(nullptr) {}
		STKey(Node *k, STNode *p, STNode *n) : node(k), prev(p), next(n) {}
	};

	EulerTree() : size(0) {}

	Node* Add(const T& u) {
		Node *node = new Node(u);
		STNode *st_node = ST::CreateNode(STKey(node));
		node->repr = st_node;
		st_node->key.prev = st_node->key.next = st_node;
		nodes.insert(node);
		++size;
		return node;
	}

	void Remove(Node *u) {
		nodes.erase(u);
		--size;
	}

	// v becomes the parent of u
	Edge Link(Node *u, Node *v) {
		assert(!Parent(u));
		assert(FindRoot(u) == u);
		if (Evertable) {
			Node *r = FindRoot(v);
			Evert(v);
			STNode *nu = u->repr, *nv = v->repr->key.prev;
			assert(!ST::Succ(nv));
			LinkPortion(nu, nv);
			// nv is the last occurrence of v
			STNode *l = CreateOccur(nv);
			assert(!nv->Parent());
			LinkPortion(l, nv);
			Evert(r);
			assert(ST::Succ(nv) == nu);
			assert(ST::Pred(nu) == nv);
			assert(ST::InOrder(nu, nv->key.next));
			assert(nv->key.node == v && nu->key.node == u);
			return Edge(nv);
		} else {
			// v->repr is always the first occurrence of v
			STNode *nv = v->repr->key.prev, *nu = u->repr, *nr = nullptr;
			ST::SplayNode(nv);
			if (nv->Right()) { // if v is not the root of v
				nr = nv->Right();
				CutChild<false>(nv);
			}
			STNode *l = CreateOccur(nv);
			LinkPortion(nu, nv);
			LinkPortion(l, nu);
			if (nr)	LinkPortion(nr, nu);
			assert(ST::Succ(nv) == nu);
			assert(ST::Pred(nu) == nv);
			assert(ST::InOrder(nu, nv->key.next));
			return Edge(nv);
		}
	}

	void Cut(Edge e) {
		STNode *begin = e, *end = e->key.next, *repr;
		if (Evertable && !ST::InOrder(begin, end)) {
			// meaning that it's everted..
			repr = end;
			begin = ST::Pred(end);
			end = begin->key.next;
			assert(ST::InOrder(begin, end));
		} else repr = ST::Succ(begin);
		ST::SplayNode(begin);
		CutChild<false>(begin);
		ST::SplayNode(end);
		CutChild<true>(end);
		LinkChild<true>(begin, end);
		Compress(begin);
		// Fix representatives
		if (Evertable) repr->key.node->repr = repr;
		// This should be done before compress for statistic
		if (end->key.node->repr == begin) end->key.node->repr = end;
		ST::Update(end);
	}

	// Detach from parent.
	// In evertable case, this become more complicated..
	void Cut(Node *u) {
		assert(!Evertable);
		Cut(Edge(ST::Pred(u->repr)));
	}

	// When evert u, the u->repr becomes the first occurrence of u
	void Evert(Node *u) {
		assert(Evertable);
		STNode *cur = u->repr;
		ST::SplayNode(cur);
		if (cur->Left()) { // If not it is already root
			Node *ur = FindRoot(u);
			assert(FindRoot(u) != u);
			ST::SplayNode(cur);

			STNode *p = cur->Left();
			CutChild<true>(cur);
			while (cur->Right()) cur = cur->Right();
			ST::SplayNode(cur); // Amortization
			// cur is a root now
			assert(!cur->Right());
			STNode *p2 = cur->Left();
			// Remove the last occurrence of the old root
			assert(cur->key.node == ur);
			CutChild<true>(cur);
			DropOccur(cur);
			// Attach the detached node
			LinkPortion(p, p2);
			// Create a new last occurrence
			STNode *l = CreateOccur(u->repr->key.prev);
			// Attach the new last occurrence
			LinkPortion(l, p2);
		}
		assert(!ST::Pred(u->repr));
		assert(!ST::Succ(u->repr->key.prev));
	}

	Node* FindLCA(Node *u, Node *v) {
		// Very naive algorithm
		std::vector<STNode *> path;
		STNode *p = u->repr;
		while (p) {
			p = FindFirstOccur(p->key.node);
			path.push_back(p);
			p = ST::Pred(p);
		}
		p = v->repr;
		while (p) {
			p = FindFirstOccur(p->key.node);
			for (int i = 0 ; i < path.size() ; ++i) {
				if (p == path[i]) return p->key.node;
			}
			p = ST::Pred(p);
		}  
		return nullptr;
	}

	Node* Parent(Node *u) {
		if (Evertable) {
			// It's just linear search
			STNode *cur = u->repr;
			cur = FindFirstOccur(cur->key.node);
			STNode *prev = ST::Pred(cur);
			if (!prev) return nullptr;
			return prev->key.node;
		} else {
			STNode *par = ST::Pred(u->repr);
			if (!par) return nullptr;
			return par->key.node;
		}
		return nullptr;
	}

	Node* FindRoot(Node *u) {
		STNode *cur = u->repr;
		ST::SplayNode(cur);
		while(cur->Left()) cur = cur->Left();
		ST::SplayNode(cur); // Amortization
		assert(cur->key.node->repr == cur);
		return cur->key.node;
	}

	bool IsRoot(Node *u) {
		return Parent(u) == nullptr;
	}

	size_t Size() {
		return size;
	}

private:
	void DropOccur(STNode *node) {
		assert(node->key.prev && node->key.next);
		node->key.prev->key.next = node->key.next;
		node->key.next->key.prev = node->key.prev;
		if (node == node->key.node->repr) {
			assert(node->key.next->key.node == node->key.node);
			node->key.node->repr = node->key.next;
		}
		assert(node != node->key.node->repr);
		node->Left() = node->Right() = nullptr;
		delete node;
	}

	STNode *CreateOccur(STNode *last) {
		STNode *occur = ST::CreateNode(last->key.node);
		occur->key.prev = last;
		occur->key.next = last->key.next;
		last->key.next->key.prev = occur;
		last->key.next = occur;
		return occur;
	}

	template <bool Left = true>
	void CutChild(STNode *node) {
		if (Left) {
			assert(node->Left()->Parent() == node);
			node->Left() = node->Left()->Parent() = nullptr;
		} else {
			assert(node->Right()->Parent() == node);
			node->Right() = node->Right()->Parent() = nullptr;
		}
	}

	template <bool Left = true>
	void LinkChild(STNode *child, STNode *parent) {
		assert(parent);
		if (Left) {
			assert(!parent->Left());
			parent->Left() = child;
		} else {
			assert(!parent->Right());
			parent->Right() = child;
		}
		assert(!child->Parent());
		child->Parent() = parent;
	} 

	// Link two portions and return the root (splay tree)
	void LinkPortion(STNode *u, STNode *v) {
		ST::SplayNode(v);
		while (v->Right()) v = v->Right();
		assert(!v->Right());
		assert(!v->Right());
		ST::SplayNode(u); // Make u root
		LinkChild<false>(u, v);
		// This lazy amortization resolve statistic problem
		ST::SplayNode(v); // Amortization
	}
	
	// node should have a parent
	void Compress(STNode *node) {
		assert(node->Parent() && (!node->Left() || !node->Right()));
		assert(node->Parent()->Left() == node || node->Parent()->Right() == node);
		STNode *parent = node->Parent();
		STNode *&child = (parent->Right() == node)?parent->Right():parent->Left();
		assert(child == node);
		if (node->Left()) child = node->Left();
		else child = node->Right();
		if (child) child->Parent() = parent;
		DropOccur(node);
		assert(child == parent->Left() || child == parent->Right());
		assert(!child || child->Parent() == parent);
	}

	// This function is very bad if it is evertable
	STNode* FindFirstOccur(Node *u) {
		if (!Evertable) return u->repr;
		STNode *cur = u->repr;
		while (cur->key.prev != cur) {
			ST::SplayNode(cur);
			STNode *prev = cur->key.prev;
			while (prev->Parent() != cur) prev = prev->Parent();
			if (prev == cur->Right()) break;
			cur = cur->key.prev;
		}
		ST::SplayNode(cur);
		return cur;
	}


	size_t size;
	std::unordered_set<Node *> nodes;

};

class LCAStatistic : public Statistic {
public:
	void *key;
	size_t left_depth, right_depth;
	bool left_begin;
	bool init;
	LCAStatistic() : Statistic(), key(nullptr), left_depth(0), right_depth(0), left_begin(false) {}
	
	template <typename T>
	void Init(const T& key) {
		this->key = (void *)&key;
		left_depth = right_depth = 0;
		left_begin = false;
		init = false;
		if (key.node->repr && &(key.node->repr->key) == &key) left_begin = true;
		if (key.node->repr) init = true;
	}

	template <bool Left = false, typename ST>
	long long Dist(const ST &s) const {
		if (Left) {
			bool advantage = !left_begin;
			long long me_to_bound = left_depth + advantage;
			long long s_to_bound = s.right_depth + !advantage;
			return me_to_bound - s_to_bound;
		} else {
			bool advantage = s.left_begin;
			long long me_to_bound = right_depth + advantage;
			long long s_to_bound = s.left_depth + !advantage;
			return me_to_bound - s_to_bound;
		}
		return 0;
	}

	template <bool Left, typename ST>
	void Update(const ST& s) {
		long long dist = Dist<Left>(s);
		if (Left) {
			if (dist > 0) {
				left_depth = dist + s.left_depth;
			} else {
				key = s.key;
				left_depth = s.left_depth;
				right_depth -= dist;
			}
			left_begin = s.left_begin;
		} else {
			if (dist > 0) {
				right_depth = dist + s.right_depth;
			} else {
				key = s.key;
				right_depth = s.right_depth;
				left_depth -= dist;
			}
		}
	}

	template <typename ST>
	void UpdateLeft(const ST& s) {
		Update<true>(s);
	}

	template <typename ST>
	void UpdateRight(const ST& s) {
		Update<false>(s);
	}

};

template <class T>
class LCAEulerTree : public EulerTree<T, false, LCAStatistic> {
	public:
	typedef EulerTree<T, false, LCAStatistic> ET;
	typedef typename ET::NodeRef NodeRef;
	typedef LCAStatistic Stat;
	typedef typename ET::STNode STNode;
	typedef typename ET::ST ST;
	typedef typename ET::Node Node;
	typedef typename ET::Edge Edge;
	typedef typename ET::STKey STKey;
	typedef T ItemType;

	Node* FindLCA(Node *u, Node *v) {
		STNode *n1 = u->repr, *n2 = v->repr;
		if (n1 == n2) return u;
		if (!ST::InOrder(n1, n2)) std::swap(n1, n2);
		return ((STKey *)ST::RangeStatistic(n1, n2).key)->node;
	}
};


#endif /* __EULER_TREE_H__ */
