#include <algorithm>
// Hollow heap (minimum), two-parent, one-tree
// Amortised O(1) push, decrease-key, merge. O(log(N)) pop/erase
// Note that N is number of push + decrease-key operations
template<class T> struct HollowHeapNode;
template<class T> struct HollowNode // The actual items in the heap
{
	typedef struct HollowHeapNode<T>* pnode;
	pnode inheap;
};
template<class T> struct HollowHeapNode // Node in the heap. Nodes can be hollow or full. Full nodes contain an item, hollow nodes do not.
{
	typedef struct HollowHeapNode<T>* pnode;
	typedef struct HollowNode<T>* pitem;
	T val; // Value of the node
	int rank; // Rank, node with rank r has at least F(r+3)-1 descendants, including itself, where F(i) is the ith fibonacci number  
	pitem item;
	pnode child, sibling; // Left most child + right sibling
	pnode secondparent; // If node has two parents, this pointer is to the second
};
// Memory allocation

template<class T> HollowNode<T>* _hollownewitem()
{
	return new HollowNode<T>();
}
template<class T> HollowHeapNode<T>* _hollownewnode(HollowNode<T>* item, T val)
{
	typedef struct HollowHeapNode<T>* pnode;
	pnode _new = new HollowHeapNode<T>();
	_new->item = item;
	_new->val = val;
	item->inheap = _new;
	return _new;
}

template<class T> struct hollow
{
	typedef struct HollowHeapNode<T>* pnode;
	typedef struct HollowNode<T>* pitem;
	// Used for intermediary storage during pop
	int mxrank;
	pnode ofrank[50];
	hollow()
	{
		std::fill_n(ofrank, 50, nullptr);
		mxrank = 0;
		root = 0;
		sz = 0;
	}

	pnode root;
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
	T top()
	{
		return root->val;
	}
	void link(pnode a, pnode b) // Makes a the first child of b
	{
		a->sibling = b->child;
		b->child = a;
	}
	void insertIntoHeap(pnode a) // Make a the first child of the root, or the root
	{
		if (a->val > root->val)
		{
			// Link a to the root
			link(a, root);
		}
		else
		{
			// a is the new root
			link(root, a);
			root = a;
		}
	}
	void ranklinks(pnode a) // Merges until it is the only node of its rank
	{
		if (!ofrank[a->rank]) // Only one of its rank
		{
			if (a->rank > mxrank) mxrank = a->rank;
			ofrank[a->rank] = a; 
		}
		else // Do merge
		{
			pnode b = ofrank[a->rank];
			ofrank[a->rank] = 0;
			if (a->val < b->val)
			{
				// Make a parent of b
				link(b, a);
				a->rank++;
				ranklinks(a);
			}
			else
			{
				// Make b parent of a
				link(a, b);
				b->rank++;
				ranklinks(b);
			}
		}
	}

	// Main functions
	void push(pnode a)
	{
		sz++;
		if (sz == 1) // A is the only node
		{
			root = a;
			return;
		}
		insertIntoHeap(a);
	}
	void push(pitem item, T val)
	{
		push(_hollownewnode<T>(item, val));
	}
	pitem push(T val)
	{
		pitem _new = _hollownewitem<T>();
		push(_new, val);
		return _new;
	}
	void decreasekey(pitem item, T val)
	{
		pnode a = item->inheap;
		if (a == root)
		{
			// Update value, then done
			a->val = val;
			return;
		}
		pnode b = _hollownewnode(item, val);
		// Rank of b is max(0, a->rank - 2)
		if (a->rank > 2) b->rank = a->rank - 2;
		// Make a hollow, and make b the second parent 	
		a->item = 0;
		b->child = a;
		a->secondparent = b;
		insertIntoHeap(b); // Make b a child of the root, or the parent of the root
	}
	void pop() // Remove the smallest item from the heap
	{
		sz--;
		if (!sz) // Heap should now be empty
		{
			root = 0;
			return;
		}
		mxrank = 0;
		pnode l = root; // Linked list of hollow nodes to delete
		while (l)
		{
			pnode v = l;
			l = l->sibling;
			pnode a = v->child;
			while (a) // Process all children of v
			{
				pnode next = a->sibling;
				if (a->item) // Is not hollow
				{
					a->sibling = 0;
					ranklinks(a); // Merge with nodes of the same rank, leaving only one node with its rank
				}
				else // Is hollow
				{
					if (a->secondparent) // Has two parents
					{
						if (a->secondparent == v) next = 0; // A is the last child of v
						else a->sibling = 0; // A now has no more siblings
						a->secondparent = 0; // A now has one parent
					}
					else // This was its only parent, add to l so it can be removed from the heap
					{
						a->sibling = l;
						l = a;
					}
				}
				a = next;
			}
		}
		root = 0;
		// Now, merge all nodes to form one heap
		for (int i = 0; i <= mxrank; i++)
		{
			if (ofrank[i])
			{
				if (!root) // Make the root
				{
					root = ofrank[i];
				}
				else if (ofrank[i]->val < root->val) // Link root to this then make this the root
				{
					link(root, ofrank[i]);
					root = ofrank[i];
				}
				else // Link to the root
				{
					link(ofrank[i], root);
				}
				ofrank[i] = 0; // Clear the array for future usage
			}
		}
	}
	void merge(hollow *a)
	{
		sz += a->sz;
		if (!root) // Set root to a->root, since this heap is empty
		{
			root = a->root;
		}
		else if (a->root) insertIntoHeap(a->root);
	}
	void erase(pitem item) // Remove item from the heap
	{
		pnode a = item->inheap;
		if (a == root) pop(); // Is the minimum item, do a pop
		else
		{
			a->item = 0; // Make hollow
			item->inheap = 0;
		}
	}
};