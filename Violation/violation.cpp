// Minimum violation heap, O(1) push, decrease-key, merge. O(log(n)) pop
#define MAXN 1000000
typedef struct ViolationNode* pnode;
struct ViolationNode
{
	int val;
	int rank;
	bool isRoot;
	pnode child, left, right; // Pointer two leftmost child, right/left nodes in sibling linked list/main linked list
};

// Nodes are allocated from this array
ViolationNode nodesForAlloc[MAXN]; 
int heapallocupto;
pnode newnode(int val)
{
	pnode _new = nodesForAlloc + heapallocupto++; // Dynamic allocation is slow ...
	_new->val = val;
	return _new;
}
pnode _first[100], _second[100]; // Used for intermediary storage during pop
int mxrank; // Used for intermediary storage
struct ViolationHeap
{
	pnode root; // Pointer to root of heap containing minimum value
	int sz;
	// Auxilary functions
	int size()
	{
		return sz;
	}
	bool empty()
	{
		return !sz;
	}
	int top()
	{
		return root->val;
	}
	int rank(pnode a) // Computes the rank of the node
	{
		// Calculated by adding 1 to the ceiling of the average of the rank of the first two children
		int r = 0;
		if (a->child)
		{
			r += a->child->rank;
			if (a->child->right)
			{
				r += a->child->right->rank;
			}
			else r--; // Rank of NULL node is -1
		}
		else r--; // Rank of NULL node is -1
		return r/2 + 1;
	}
	void swap(pnode &a, pnode &b) // Swaps two pnodes
	{
		pnode c = a;
		a = b;
		b = c;
	}
	void insertIntoHeap(pnode a, pnode &root) // Inserts a into the heap rooted at root
	{
		a->isRoot = true;
		if (!root) // Heap is empty, just add in a
		{
			root = a->left = a->right = a;
			return;
		}
		// Add to the right of root
		pnode b = root->right;
		b->left = a;
		a->right = b;
		root->right = a;
		a->left = root;
		// Update root if needed
		if (a->val < root->val) root = a;
	}
	void swapFirstTwoChildren(pnode a) // Swaps first two children of a so that the rank of the first is at least the rank of the second
	{
		if (!a->child || !a->child->right) return; // Doesn't have two children
		pnode b = a->child;
		pnode c = b->right;
		if (b->rank < c->rank) // Needs swapping
		{
			c->left = a;
			b->right = c->right;
			if (c->right) c->right->left = b;
			c->right = b;
			b->left = c;
			a->child = c;
		}
	}
	pnode merge(pnode a, pnode b, pnode c)
	{
		// Swap so that a->val <= b->val && c->val
		if (b->val <= a->val && b->val <= c->val) swap(a, b); // B is the smallest
		else if (c->val <= a->val && c->val <= b->val) swap(a, c); // C is the smallest
		swapFirstTwoChildren(a); // Ensure old active children of a are sorted by rank
		// Make b and c children of a
		if (a->child) a->child->left = b;
		b->right = a->child;
		b->left = c;
		c->right = b;
		c->left = a;
		a->child = c; 
		a->rank = rank(a); // Update rank
		b->isRoot = c->isRoot = false;
		return a;
	}

	// Main functions
	void push(pnode a)
	{
		sz++;
		insertIntoHeap(a, root);
	}
	void push(int val)
	{
		push(newnode(val));
	}
	void merge(pnode a)
	{
		if (!root)
		{
			// Just make a the root
			root = a;
			return;
		}
		// Insert a's heap in between a and a->right
		pnode b = a->right;
		pnode x = root->right;

		root->right = b;
		b->left = root;

		a->right = x;
		x->left = a;
		// Update root if needed
		if (a->val < root->val) root = a;
	}
	void merge(ViolationHeap &a)
	{
		sz += a.sz;
		merge(a.root);
	}
	void dealWithNode(pnode a) // Does possible merges
	{
		int r = a->rank;
		if (r > mxrank) mxrank = r; // Update maximum seen rank if needed
		if (!_first[r]) _first[r] = a; // First occurrence of this rank
		else if (!_second[r]) _second[r] = a; // Second occurrence of this rank
		else
		{
			// Merge them
			a = merge(a, _first[r], _second[r]);
			_first[r] = _second[r] = NULL;
			dealWithNode(a);
		}
	}
	void pop()
	{
		sz--;
		if (!sz) // Heap is now empty
		{
			root = NULL;
			return;
		}
		mxrank = 0;
		// Insert all children of the old root into the new heap, then insert all the other heaps
		// Merging where possible
		pnode a = root->child;
		while (a)
		{
			pnode next = a->right;
			dealWithNode(a);
			a = next;
		}
		// Now deal with all other nodes in the heap
		a = root->right;
		while (a != root)
		{
			pnode next = a->right;
			dealWithNode(a);
			a = next;
		}
		root = NULL;
		// Make the new heap
		for (int i = 0; i <= mxrank; i++)
		{
			if (_first[i]) 
			{
				insertIntoHeap(_first[i], root);
				_first[i] = NULL;
			}
			if (_second[i]) 
			{
				insertIntoHeap(_second[i], root);
				_second[i] = NULL;
			}
		}
	}
	pnode parent(pnode a)
	{
		// Returns the parent of a if a is active, else returns NULL
		if (a->isRoot) return NULL;
		if (a->left->child == a) return a->left;
		if (a->left->left->child == a->left) return a->left->left;
		return NULL;
	}
	void decreasekey(pnode a, int val)
	{
		a->val = val;
		if (a->isRoot)
		{
			// Swap with root if needed
			if (a->val < root->val) root = a;
			return;
		}
		swapFirstTwoChildren(a);
		pnode p = parent(a); // Store the parent of a
		// Place first child in the place of a
		pnode b = a->child;
		if (b)
		{
			// Remove b from a
			if (b->right) b->right->left = a;
			a->child = b->right;
			// Update rank of a
			a->rank = rank(a);
			// Put b in a's place
			b->left = a->left;
			b->right = a->right;
			if (b->right) b->right->left = b;
			if (b->left->child == a) // Is the first child
			{
				b->left->child = b;
			}
			else b->left->right = b;
		}
		else // Just remove a
		{
			if (a->left->child == a) a->left->child = a->right;
			else a->left->right = a->right;
			if (a->right) a->right->left = a->left;
		}
		// Insert a back into heap
		insertIntoHeap(a, root);
		while (p)
		{
			int r = rank(p);
			if (r == p->rank) break; // Done with propagation
			p->rank = r;
			p = parent(p);
		}
	}
	void erase(pnode a)
	{
		// Decrease key to -inf, then pop
		decreasekey(a, -2e9);
		pop();
	}
};