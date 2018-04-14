// Binomial heap with push, pop, top (maximum), merge, increase key, delete
typedef struct BinomialNode* pnode;
struct BinomialNode
{
	int val, degree;
	pnode sibling, child, par; // sibling is either in the heap linked list of the sibling within a tree. 
};
namespace binomialheapalloc
{
// Nodes are allocated from this array
BinomialNode _heap[20000000]; 
int _heapallocupto;
pnode _newnode(int val)
{
	pnode _new = _heap + _heapallocupto++; // Dynamic allocation is slow ... this is much faster
	//pnode _new = new BinomialNode(); // Other method of allocating memory
	_new->val = val;
	return _new;
}
}
struct BinomialHeap
{
	pnode root = NULL;
	int sz = 0;
	pnode mx = NULL;
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
	void swap(pnode &a, pnode &b) // Swaps two pnodes. Created to remove any reliance on STL
	{
		pnode c = a;
		a = b;
		b = c;
	}
	pnode mergetrees(pnode a, pnode b, pnode pre = NULL) // Merges trees with equal degree, creating one tree with degree+1
	{
		if (b->val > a->val)
		{
			// Swap them & replace a with b in the linked list
			if (pre) pre->sibling = b;
			if (root == a) root = b;
			a->sibling = b->sibling;
			swap(a, b);
		}
		// Make tree b the child of tree a
		a->sibling = b->sibling;
		a->degree++;
		b->sibling = a->child;
		a->child = b;
		b->par = a;
		if (mx == NULL || a->val >= mx->val) mx = a;
		return a;
	}

	// Main functions
	void push(pnode _new)
	{
		// Worst case O(log(n)) - average O(1)
		sz++;
		if (mx == NULL || _new->val > mx->val) mx = _new; // Update mx if needed
		_new->sibling = root; // Set this new node as the first one
		root = _new; // set as root
		while (_new->sibling && _new->degree == _new->sibling->degree) // Will merge until no longer necessary, then break
		{
			_new = mergetrees(_new, _new->sibling);
		}
	}
	void push(int val)
	{
		pnode _new = binomialheapalloc::_newnode(val);
		push(_new);
	}
	void merge(pnode b)
	{
		pnode a = root;
		pnode pre = NULL;
		while (a && b)
		{
			if (mx == NULL || b->val > mx->val) mx = b; // update mx if needed
			if (mx == NULL || a->val > mx->val) mx = a; // update mx if needed
			b->par = NULL; // Its a root - has no parent
			a->par = NULL; // Its a root - has no parent
			if (a->degree < b->degree)
			{
				// add a to the new list
				if (pre) pre->sibling = a;
				else root = a;
				pre = a;
				a = a->sibling;
			}
			else if (b->degree < a->degree)
			{
				// add b to the new list
				if (pre) pre->sibling = b;
				else root = b;
				pre = b;
				b = b->sibling;
			}
			else
			{
				// add a to b's linked list - merge where necessary
				pnode asib = a->sibling;
				a->sibling = b;
				b = a;
				a = asib;
				while (b->sibling && b->degree == b->sibling->degree) // Will merge until no longer necessary, then break
				{
					b = mergetrees(b, b->sibling);
				}
			}
		}
		while (a) // Add to the end
		{
			if (mx == NULL || a->val > mx->val) mx = a; // update mx if needed
			a->par = NULL; // Its a root - has no parent
			if (pre) pre->sibling = a;
			else root = a;
			pre = a;
			a = a->sibling;
		}
		while (b) // Add to the end
		{
			if (mx == NULL || b->val > mx->val) mx = b; // update mx if needed
			b->par = NULL; // Its a root - has no parent
			if (pre) pre->sibling = b;
			else root = b;
			pre = b;
			b = b->sibling;
		}
		pre->sibling = NULL;
	}
	void merge(BinomialHeap& A)
	{
		merge(A.root);		
	}
	void pop()
	{
		pnode a = root;
		pnode pre = NULL;
		while (a != mx) // Find the tree with mx
		{
			pre = a;
			a = a->sibling;
		}	
		mx = NULL;
		// Remove it from the tree
		if (pre)
		{
			pre->sibling = a->sibling;
		}
		else
		{
			root = a->sibling;
		}

		// Reverse the linked list of A's children
		pre = NULL;
		a = a->child;
		while (a)
		{
			pnode next = a->sibling;
			a->sibling = pre;
			pre = a;
			a = next;
		}
		if (pre) merge(pre); // Merge the children back into the heap.
		else
		{
			pnode b = root;
			while (b)
			{
				if (mx == NULL || b->val > mx->val) mx = b;
				b = b->sibling;
			}
		}
	}
	void increasekey(pnode a, int val)
	{
		a->val = val;
		if (mx == NULL || val > mx->val) mx = a;
		while (a->par && a->par->val < val)
		{
			// Swap a and its parent
			pnode b = a->par; 
			pnode sib = a->sibling;
			a->sibling = b->sibling;
			b->sibling = sib;
			b->child = a->child;
			a->child = b;
			a->par = b->par;
			b->par = a;
		}
	}
	void erase(pnode a) // Remove a node from the heap
	{
		// Update the value to infinity (in this case, (2^31)-1)
		increasekey(a, (1 << 31)-1);
		// Since a should now be the greatest element, pop
		pop();
	}
};