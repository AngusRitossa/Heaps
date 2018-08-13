// Binomial heap with push, pop, top (minimum), merge, decrease key, delete
typedef struct BinomialNode* pnode;
#define MAXN 1000001
struct BinomialNode
{
	int val, degree;
	pnode sibling, child, par; // sibling is either in the heap linked list of the sibling within a tree. 
};
namespace binomialheapalloc
{
// Nodes are allocated from this array
BinomialNode _heap[MAXN]; 
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
	pnode mn = NULL;
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
		return mn->val;
	}
	void swap(pnode &a, pnode &b) // Swaps two pnodes. Created to remove any reliance on STL
	{
		pnode c = a;
		a = b;
		b = c;
	}
	pnode mergetrees(pnode a, pnode b, pnode pre = NULL) // Merges trees with equal degree, creating one tree with degree+1
	{
		if (b->val < a->val)
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
		if (mn == NULL || a->val <= mn->val) mn = a;
		return a;
	}

	// Main functions
	void push(pnode _new)
	{
		// Worst case O(log(n)) - average O(1)
		sz++;
		if (mn == NULL || _new->val < mn->val) mn = _new; // Update mn if needed
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
			if (mn == NULL || b->val < mn->val) mn = b; // update mn if needed
			if (mn == NULL || a->val < mn->val) mn = a; // update mn if needed
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
			if (mn == NULL || a->val < mn->val) mn = a; // update mn if needed
			a->par = NULL; // Its a root - has no parent
			if (pre) pre->sibling = a;
			else root = a;
			pre = a;
			a = a->sibling;
		}
		while (b) // Add to the end
		{
			if (mn == NULL || b->val < mn->val) mn = b; // update mn if needed
			b->par = NULL; // Its a root - has no parent
			if (pre) pre->sibling = b;
			else root = b;
			pre = b;
			b = b->sibling;
		}
		pre->sibling = NULL;
	}
	void merge(BinomialHeap* A)
	{
		if (!A->root) return;
		sz += A->sz;
		merge(A->root);		
	}
	void pop()
	{
		sz--;
		pnode a = root;
		pnode pre = NULL;
		while (a != mn) // Find the tree with mn
		{
			pre = a;
			a = a->sibling;
		}	
		mn = NULL;
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
				if (mn == NULL || b->val < mn->val) mn = b;
				b = b->sibling;
			}
		}
	}
	void decreasekey(pnode &a, int val)
	{
		while (a->par && a->par->val > val)
		{
			// Swap a and its parent
			a->val = a->par->val;
			a = a->par;
		}
		a->val = val;
		if (mn == NULL || val < mn->val) mn = a;
	}
	void erase(pnode a) // Remove a node from the heap
	{
		// Update the value to -infinity
		decreasekey(a, -(1 << 30));
		// Since a should now be the greatest element, pop
		pop();
	}
};