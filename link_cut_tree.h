#ifndef __LINK_CUT_TREE_H__
#define __LINK_CUT_TREE_H__

#include <limits>
#include <vector>
#include <unordered_map>
#include <unordered_set>

#include "splay_tree.h"

// Usage Note.
// To use Remove(), coder make sure that there is no connection to the vertex

template <class T, class Stat, bool Evertable = false>
class LinkCutTree {
public:
	typedef T ItemType;
	// Splay tree is keyed by depth implicitly
	// we therefore use a comparator always returning false
	struct Node : BasicTreeNode <Node> {
		typedef T ItemType;
		T key;
		Stat stat;
		bool reverse;
		Node (const T& key, Node *p = NULL, Node *l = NULL, Node *r = NULL)
			: key(key), BasicTreeNode<Node>(p,l,r), reverse(false) {}
		// Statistic function should be commutative 
		// if link cut tree is evertable
		void Update() {
			stat.Init(key);
			if (this->Left()) stat.UpdateLeft(this->Left()->stat);
			if (this->Right()) stat.UpdateRight(this->Right()->stat);
		}
	};

private:
	struct FalseComp {
		bool operator()(const T& l, const T& r) const {
			return false;
		}
	};
	
	typedef SplayTreeBase<T, Node, FalseComp> ST;
	typedef typename std::unordered_set<Node*> NodeSet;
	typedef typename NodeSet::iterator ns_it;

public:
	LinkCutTree () : size(0) {}
	LinkCutTree (const LinkCutTree &) = delete;
	~LinkCutTree() {
		for (ns_it it = nodes.begin(); it != nodes.end(); ++it) {
			(*it)->Left() = (*it)->Right() = NULL;
			delete (*it);
		}
	}
	
	Node* Access(Node* v) {
		assert(v);
		Splay(v);
		Node* last_splay_node = v;
		Node* u = v->Right();
		Disconnect(v, u);
		if(u) u->Parent() = v; // Reconnect but using path parent
		assert(!v->Right());
		Node* w = v->Parent();
		while (w) {
			Splay(w);
			last_splay_node = w;
			u = w->Right();
			Disconnect(w, u);
			if(u) u->Parent() = w; // Reconnect but using path parent
			// Merge
			MergeRight(w, v);
			assert(!ST::IsRoot(v) && ST::IsRoot(w));
			Splay(v);
			w = v->Parent();
		}
		assert(ST::IsRoot(v));
		return last_splay_node;
	}

	Node* Add(const T& value) {
		++size;
		Node* node = ST::CreateNode(value);
		nodes.insert(node);
		return node;
	}

	void Remove(Node* v) {
		Cut(v);

		nodes.erase(v);

		// Delete tree
		--size;
		delete v;
	}

	Node* FindRoot(Node* v) {
		Access(v);
		assert(ST::IsRoot(v));
		Node* u = v;
		if (Evertable) PushReverse(u);
		while (u->Left()) {
			u = u->Left();
			if (Evertable) PushReverse(u);
		}
		Splay(u);
		return u;
	}

	void Cut(Node* v) {
		assert(v);
		Access(v);
		assert(!v->reverse);
		if (!v->Left())	return;
		Node* w = v->Left();
		Disconnect<true>(v, w);
		// Update path parent
		assert(ST::IsRoot(w) && ST::IsRoot(v));
		w->Parent() = v->Parent();
		v->Parent() = NULL;
	}

	// w becomes the parent of v
	void Link(Node* v, Node* w) {
		assert(v != w);
		assert(FindRoot(v) == v && FindRoot(w) != v);
		Access(v);
		Access(w);
		assert(ST::IsRoot(v) && ST::IsRoot(w));
		assert(v->Left() == NULL);
	
		// Before merge inherit the parent information
		v->Parent() = w->Parent();
		// Merge
		MergeLeft(v, w);
		assert(ST::IsRoot(v) && !ST::IsRoot(w) && w->Parent() == v);
	}

	Node* FindLCA(Node *v, Node *w) {
		if (FindRoot(v) != FindRoot(w)) return NULL;
		Access(v);
		return Access(w);
	}

	void Evert(Node *v) { 
		if (Evertable) {
			Access(v);
			v->reverse ^= 1;
		} else {
			assert(false);
		}
	}

	Stat Path(Node* v) {
		Access(v);
		return v->stat;
	}

	Node *Parent(Node *v) {
		Access(v);
		assert(!v->reverse);
		if (!(v = v->Left())) return NULL;
		while (v->Right()) {
			v = v->Right();
			if (Evertable) PushReverse(v);
		}
		Access(v); // Amortization
		return v;
	}

	bool IsRoot(Node *v) {
		Access(v);
		assert(!v->reverse);
		return v->Left() == NULL;
	}

	size_t Size() const {
		return size;
	}

	// *************************************************
	// Reverse functions used only when Evertable = true
	// *************************************************
	void PushReverse(Node *v) {
		if (v->reverse) {
			std::swap(v->l, v->r);
			if (v->l) v->l->reverse ^= 1;
			if (v->r) v->r->reverse ^= 1;
			v->reverse = false;
		}
	}

private:
	void ResolveReverse(Node *v) {
		// Resolve Reverse flag on the way to the root
		// Sweep through the path from root to v
		// If there is set flag recursively push down
		std::vector<Node *> path;
		path.push_back(v);
		while(!ST::IsRoot(path.back())) 
			path.push_back(path.back()->Parent());
		for (size_t i = path.size() ; i --> 0 ;) 
			PushReverse(path[i]);
	}

	// TODO : Evert check for every splay?? 
	void Splay(Node *v) {
		// Find splay tree that v belongs to
		assert(v);
		if (Evertable) ResolveReverse(v);
		if (ST::IsRoot(v)) return;
		// Splay on node v in the splay tree
		ST::SplayNode(v);
		// Replace root map of the splay tree to v
		assert(ST::IsRoot(v));
	}

	template <bool isLeft = false>
	void Disconnect(Node* v, Node* w) {
		if (!w) return;
		Splay(v);
		(isLeft)?v->Left()=NULL:v->Right()=NULL;
		w->Parent() = NULL;
		ST::Update(v);
	}

	// Attach w to the right of v
	void MergeRight(Node* v, Node* w) {
		assert(!v->Right());
		v->Right() = w;
		w->Parent() = v;
		ST::Update(v);
	}
	
	// Attach w to the left of v
	void MergeLeft(Node* v, Node* w) {
		assert(!v->Left());
		assert(!v->reverse);
		v->Left() = w;
		w->Parent() = v;
		ST::Update(v);
	}

	size_t size;
	NodeSet nodes;	
	
};

#endif

