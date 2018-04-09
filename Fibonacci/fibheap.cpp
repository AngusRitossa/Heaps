// Fibonacci Heap with push, pop, top (max), merge & increase key

typedef struct FibNode* pnode;
struct FibNode
{
	int val, degree;
	bool onechildcut; // Whether one of its children has been cut due to decrease key
	pnode left, right, child, siblingleft, siblingright, par; // Left and right in the linked list of binomial trees, child, sibling and parent are within a tree
};
pnode _ofsize[100];
int _ofsizedone[100];
int _ofsizeupto;
FibNode _heap[20000000];
int _heapallocupto;
pnode _newnode(int val)
{
	pnode _new = _heap + _heapallocupto++; // Dynamic allocation is slow ... this is much faster
	//pnode _new = new FibNode();
	_new->val = val;
	return _new;
}
void swap(pnode &a, pnode &b) // Because why use stl
{
	pnode c = a;
	a = b;
	b = c;
}
struct FibHeap
{
	pnode mx; // Pointer to the maximum value in the heap
	int sz; // Number of elements in the heap
	pnode temproot; // Used in the pop function
	// Auxiliary functions
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
		return mx->val;
	}
	pnode mergetrees(pnode a, pnode b)
	{
		if (b->val > a->val)
		{
			// Swap them & replace a with b in the linked list
			b->left = a->left;
			b->right = a->right;
			b->left->right = b->right->left = b;
			if (temproot == a) temproot = b;
			swap(a, b);
			if (a->left == b) // If there is one tree in the heap, need to fix this
			{
				a->left = a->right = a;
			}
		}
		a->degree++;
		b->siblingright = a->child;
		if (b->siblingright) b->siblingright->siblingleft = b;
		b->siblingleft = NULL;
		a->child = b;
		b->par = a;
		return a;
	}
	void addintoheap(pnode _new)
	{
		_new->right = mx->right;
		_new->left = mx;
		mx->right = _new;
		_new->right->left = _new;
		// If the new value is larger, set as root
		if (_new->val > mx->val)
		{
			mx = _new;
		}
	}

	// Main functions
	void push(pnode _new)
	{
		if (!sz)
		{
			// If the heap is empty, just set this as the only node
			sz++;
			mx = _new;
			mx->left = mx->right = mx; // Make sure linked list is circular
			return;
		}
		// Add new node into linked list
		sz++;
		addintoheap(_new);
	}
	void push(int val)
	{
		pnode _new = _newnode(val);
		push(_new);
	}

	void pop()
	{
		sz--;
		if (!sz) // If only one element, just remove it
		{
			mx = NULL;
			return;
		}
		// Remove mx from the heap
		temproot = mx->left;
		temproot->right = mx->right;
		temproot->right->left = temproot;

		// Add the children of mx to the heap
		pnode child = mx->child;
		while (child != NULL)
		{
			// Insert child into heap
			child->right = temproot->right;
			child->left = temproot;
			temproot->right = child;
			child->right->left = child;
			child->par = NULL;
			// Set child to its sibling
			child = child->siblingright;
		}
		// Fix the heap by merging trees of the same priority
		pnode a = temproot;
		mx = temproot;
		_ofsizeupto++;
		do
		{
			while (_ofsizedone[a->degree] == _ofsizeupto) // While there is a tree to merge with
			{
				_ofsizedone[a->degree] = 0;
				pnode b = _ofsize[a->degree];
				// remove b from tree
				if (b == temproot)
				{
					temproot = b->right;
				}
				b->left->right = b->right;
				b->right->left = b->left;
				// merge a & b
				a = mergetrees(a, b);
			}
			_ofsizedone[a->degree] = _ofsizeupto;
			_ofsize[a->degree] = a;
			if (mx->val <= a->val) mx = a;
			a = a->right;
		}
		while (a != temproot);
	}
	void merge(FibHeap &a)
	{
		// Cut each heap between their maximum and the element to the right of that, then splice together
		pnode b = a.mx; 
		pnode br = b->right;
		pnode mxr = mx->right;

		b->right = mxr;
		mxr->left = b;

		mx->right = br;
		br->left = mx;

		if (b->val > mx->val) mx = b; // Update max if needed
	}
	void cutfromtree(pnode a)
	{
		pnode p = a->par;
		if (p == NULL) return;
		if (p->child == a) // A is the first child of p
		{
			p->child = a->siblingright;
			if (a->siblingright) a->siblingright->siblingleft = NULL;
		}
		else // A is somewhere in the middle. Remove a from the child linked-list
		{
			if (a->siblingleft) a->siblingleft->siblingright = a->siblingright;
			if (a->siblingright) a->siblingright->siblingleft = a->siblingleft;
		}
	//	a->siblingleft = a->siblingright = NULL;
		a->onechildcut = 0;
		// insert a into the heap linked list
		addintoheap(a);
	}
	void increasekey(pnode a, int val)
	{
		// Update the value of a
		a->val = val;
		if (a->par != NULL && a->par->val < a->val) // heap order has been violated
		{
			cutfromtree(a); // Cut a from the tree
			pnode p = a->par;
			a->par = NULL;
			while (p && p->onechildcut) // If any parents are marked, cut from tree
			{
				cutfromtree(p);
				pnode _newpar = p->par;
				p->par = NULL;
				p = _newpar;
			}
			if (p) p->onechildcut = 1; // Mark the parent
		}
		else // Update max if needed
		{
			if (a->val > mx->val) mx = a;
		}
	}
};