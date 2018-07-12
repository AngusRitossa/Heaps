// Smooth Heap (minimum), forest
#define MAXN 1000001
typedef struct Node* pnode;
struct Node
{
	int val;
	pnode left, right, child, par; // Left and right siblings in tree/heap, leftmost child, parent
	// Need these pointers to support access to: Left & right siblings of a node for removal during decrease-key
	// + Need parent because leftmost child can no longer store parent because it needs to store rightmost sibling
};
// Memory allocation
Node allocarray[MAXN];
int allocupto;
pnode newnode()
{
	return allocarray + allocupto++;
} 
struct SmoothHeap
{
	int sz;
	pnode root;
	SmoothHeap() // Initialisation 
	{
		sz = 0;
		root = NULL;
	}
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
	void insertIntoHeap(pnode a) // Adds a to the left of the root
	{
		a->par = NULL;
		if (!root)
		{
			root = a->left = a->right = a;
			return;
		}
		// Insert to the left of root
		a->left = root->left;
		a->left->right = a;
		a->right = root;
		root->left = a;
		if (a->val < root->val) root = a; // Update mn if needed
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
	void push(int val)
	{
		pnode a = newnode();
		a->val = val;
		sz++;
		insertIntoHeap(a);
	}
	void decreasekey(pnode a, int val)
	{	
		a->val = val;
		if (!a->par)
		{
			if (val < root->val) root = a; // Update mn if needed
			return; // A is already a root, doesn't need removal
		}
		if (val > a->par->val) return; // A doesn't break heap-order, doesn't need removal
		// Remove a from its parent
		if (a->left == a) // Only child
		{
			a->par->child = NULL;
		}
		else
		{
			a->left->right = a->right;
			a->right->left = a->left;
			if (a->par->child == a) a->par->child = a->right;
		}
		// Insert into the heap
		insertIntoHeap(a);
	}
	void pop()
	{
		sz--;
		if (!sz)
		{
			root = NULL;
			return;
		}
		// Remove root
		pnode x = root;
		if (root->left == root)
		{
			root = NULL;
		}
		else
		{
			// Remove old root, make linear
			root->right->left = NULL;
			root->left->right = NULL;
			root = root->right;
		}
		// Add children of x to the heaplist
		if (x->child)
		{
			pnode a = x->child;
			pnode b = x->child->left; // Rightmost child
			b->right = root;
			if (root) root->left = b;
			a->left = NULL;
			root = a;
		}
		// Do restructuring
		x = root;
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
		root = x->left = x->right = x;
	}
	void merge(SmoothHeap* a)
	{
		if (!a->sz) return;
		sz += a->sz;
		if (!root) // Just set our root to theirs
		{
			root = a->root;
			return;
		}
		// Merge root lists
		pnode rr = root->right;
		pnode aroot = a->root;
		pnode ar = aroot->right;
		root->right = ar;
		ar->left = root;
		aroot->right = rr;
		rr->left = aroot;
		if (aroot->val < root->val) root = aroot; // Update root if needed
	}
};