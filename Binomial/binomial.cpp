// Binomial heap with push, pop, top (minimum), merge, decrease key, delete
template<class T> struct BinomialNode;
template<class T> struct BinomialHeapNode // The actual node in the heap
{
	typedef struct BinomialHeapNode<T>* pnode;
	typedef struct BinomialNode<T>* pvalue;
	pvalue val;
	int degree;
	pnode sibling, child, par; // sibling is either in the heap linked list of the sibling within a tree. 
};
template<class T> struct BinomialNode // The value, stores a pointer to its node in the heap
{
	typedef struct BinomialHeapNode<T>* pnode;
	T val;
	pnode inheap;
};

template<class T> BinomialHeapNode<T>* _binomialnewheapnode(BinomialNode<T>* val)
{
	typedef struct BinomialHeapNode<T>* pnode;
	pnode _new = new BinomialHeapNode<T>(); // Other method of allocating memory
	_new->val = val;
	return _new;
}

template<class T> BinomialNode<T>* _binomialnewnode(T val)
{
	typedef struct BinomialNode<T>* pvalue;
	pvalue _new = new BinomialNode<T>(); // Other method of allocating memory
	_new->val = val;
	return _new;
}

template<class T> struct binomial
{
	typedef struct BinomialHeapNode<T>* pnode;
	typedef struct BinomialNode<T>* pvalue;
	pnode root = 0;
	int sz = 0;
	pnode mn = 0;
	// Auxiliary functions
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
		return mn->val->val;
	}
	void swap(pnode &a, pnode &b) // Swaps two pnodes. Created to remove any reliance on STL
	{
		pnode c = a;
		a = b;
		b = c;
	}
	pnode mergetrees(pnode a, pnode b, pnode pre = 0) // Merges trees with equal degree, creating one tree with degree+1
	{
		if (b->val->val < a->val->val)
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
		if (mn == 0 || a->val->val <= mn->val->val) mn = a;
		return a;
	}

	// Main functions
	void push(pnode _new)
	{
		_new->val->inheap = _new;
		// Worst case O(log(n)) - average O(1)
		sz++;
		if (mn == 0 || _new->val->val < mn->val->val) mn = _new; // Update mn if needed
		_new->sibling = root; // Set this new node as the first one
		root = _new; // set as root
		while (_new->sibling && _new->degree == _new->sibling->degree) // Will merge until no longer necessary, then break
		{
			_new = mergetrees(_new, _new->sibling);
		}
	}
	void push(pvalue val)
	{
		push(_binomialnewheapnode<T>(val));
	}
	pvalue push(T val)
	{
		pvalue _new = _binomialnewnode<T>(val);
		push(_new);
		return _new;
	}
	void merge(pnode b)
	{
		pnode a = root;
		pnode pre = 0;
		while (a && b)
		{
			if (mn == 0 || b->val->val < mn->val->val) mn = b; // update mn if needed
			if (mn == 0 || a->val->val < mn->val->val) mn = a; // update mn if needed
			b->par = 0; // Its a root - has no parent
			a->par = 0; // Its a root - has no parent
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
			if (mn == 0 || a->val->val < mn->val->val) mn = a; // update mn if needed
			a->par = 0; // Its a root - has no parent
			if (pre) pre->sibling = a;
			else root = a;
			pre = a;
			a = a->sibling;
		}
		while (b) // Add to the end
		{
			if (mn == 0 || b->val->val < mn->val->val) mn = b; // update mn if needed
			b->par = 0; // Its a root - has no parent
			if (pre) pre->sibling = b;
			else root = b;
			pre = b;
			b = b->sibling;
		}
		pre->sibling = 0;
	}
	void merge(binomial* A)
	{
		if (!A->root) return;
		sz += A->sz;
		merge(A->root);		
	}
	void pop()
	{
		sz--;
		pnode a = root;
		pnode pre = 0;
		while (a != mn) // Find the tree with mn
		{
			pre = a;
			a = a->sibling;
		}	
		mn = 0;
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
		pre = 0;
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
				if (mn == 0 || b->val->val < mn->val->val) mn = b;
				b = b->sibling;
			}
		}
	}
	void decreasekey(pnode &a, pvalue val)
	{
		while (a->par && a->par->val->val > val->val)
		{
			// Swap a and its parent
			a->val = a->par->val;
			a->val->inheap = a;
			a = a->par;
		}
		a->val = val;
		if (mn == 0 || val->val < mn->val->val) mn = a;
	}
	void decreasekey(pvalue a, T val)
	{
		a->val = val;
		decreasekey(a->inheap, a);
	}
	void erase(pnode a) // Remove a node from the heap
	{
		// Update the value to -infinity
		decreasekey(a, -(1 << 30));
		// Since a should now be the greatest element, pop
		pop();
	}
};