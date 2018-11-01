// Smooth Heap (minimum), one tree
template<class T> struct SmoothNode
{
	typedef struct SmoothNode<T>* pnode;
	T val;
	pnode left, right, child, par; // Left and right siblings in tree/heap, leftmost child, parent
	// Need these pointers to support access to: Left & right siblings of a node for removal during decrease-key
	// + Need parent because leftmost child can no longer store parent because it needs to store rightmost sibling
};
// Memory allocation
template<class T> SmoothNode<T>* _newsmoothnode()
{
	return new SmoothNode<T>();
}
template<class T> struct smooth
{
	typedef struct SmoothNode<T>* pnode;
	int sz = 0;
	pnode root = nullptr;
	// Auxilary functions
	int size()
	{
		return sz;
	}
	bool empty()
	{
		return !sz;
	}
	T top()
	{
		return root->val;
	}
	void insertIntoHeap(pnode a) // Adds a to the left of the heap
	{
		if (!root)
		{
			root = a;
			a->par = nullptr;
			return;
		}
		if (a->val < root->val)
		{
			// Make root a child of a
			if (!a->child) // Only child
			{
				a->child = root->left = root->right = root;
			}
			else // Make leftmost child
			{
				root->right = a->child;
				root->left = a->child->left;
				a->child->left = root;
				root->left->right = root;
				a->child = root;
			}
			root->par = a;
			root = a;
			a->par = nullptr;
		}
		else
		{
			// Make a the leftmost child of root
			if (!root->child) // Only child
			{
				root->child = a->left = a->right = a;
			}
			else // Make leftmost child
			{
				a->right = root->child;
				a->left = root->child->left;
				root->child->left = a;
				a->left->right = a;
				root->child = a;
			}
			a->par = root;
		}
	}
	void link(pnode &a) // Stable links a to a->right
	{
		pnode b = a->right;
		if (a->val < b->val)
		{
			// Remove b from the root-list
			a->right = b->right;
			if (a->right) a->right->left = a;
			// Make b the rightmost child of a
			if (!a->child)
			{
				a->child = b->left = b->right = b;
			}
			else
			{
				b->right = a->child;
				b->left = a->child->left;
				a->child->left = b;
				b->left->right = b;
			}
			b->par = a;
		}
		else
		{
			// Remove a from the root list
			b->left = a->left;
			if (b->left) b->left->right = b;
			// Make a the leftmost child of b
			if (!b->child)
			{
				b->child = a->left = a->right = a;
			}
			else
			{
				a->right = b->child;
				a->left = b->child->left;
				b->child->left = a;
				a->left->right = a;
				b->child = a;
			}
			a->par = b;
			a = b;
		}
	}

	// Main functions
	void push(pnode a)
	{
		sz++;
		insertIntoHeap(a);
	}
	pnode push(T val)
	{
		pnode a = _newsmoothnode<T>();
		a->val = val;
		sz++;
		insertIntoHeap(a);
		return a;
	}
	void decreasekey(pnode a, T val)
	{	
		a->val = val;
		if (a == root) return; // Is the root, no need to do anything
		if (val > a->par->val) return; // A doesn't break heap-order, doesn't need removal
		// Remove a from its parent
		a->left->right = a->right;
		a->right->left = a->left;
		if (a->par->child == a)
		{
			if (a->right == a) a->par->child = nullptr;
			else a->par->child = a->right;
		}
		// Insert into the heap
		insertIntoHeap(a);
	}
	void pop()
	{
		sz--;
		if (!sz)
		{
			root = nullptr;
			return;
		}
		// Remove mn
		
		// Do restructuring
		pnode x = root->child;
		// Make linear
		x->left->right = nullptr;
		x->left = nullptr;
		while (x->right)
		{
			if (x->val < x->right->val) x = x->right; // x is not a local maximum
			else
			{
				bool dolast = 1;
				while (x->left)
				{
					if (x->left->val > x->right->val) // Link x and x->left
					{
						x = x->left;
						link(x);
					}
					else
					{
						link(x);
						dolast = 0;
						break; // Go back to while (x->right)
					}
				}
				if (dolast) link(x); // x->left does not exist, merge x and x->right
			}
		}
		// Now, list of roots is sorted
		// Just merge them all
		while (x->left)
		{
			x = x->left;
			link(x);
		}
		root = x;
		root->par = nullptr;
	}
	void merge(smooth* a)
	{
		if (!a->sz) return;
		sz += a->sz;
		if (!root) // Just set our root to theirs
		{
			root = a->root;
			return;
		}
		// Link roots
		insertIntoHeap(a->root);
	}
};