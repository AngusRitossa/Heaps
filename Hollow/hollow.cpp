// Hollow heap (minimum), two-parent, one-tree
// Amortised O(1) push, decrease-key, merge. O(log(N)) pop/erase
// Note that N is number of push + decrease-key operations
#define MAXn 1000000 // Maximum number of items (push)
#define MAXN 10000000 // Maximum number of nodes (push + decrease-key)
typedef struct Item* pitem;
typedef struct Node* pnode;
struct Item // The actual items in the heap
{
	pnode inheap;
};
struct Node // Node in the heap. Nodes can be hollow or full. Full nodes contain an item, hollow nodes do not.
{
	int val; // Value of the node
	int rank; // Rank, node with rank r has at least F(r+3)-1 descendants, including itself, where F(i) is the ith fibonacci number  
	pitem item;
	pnode child, sibling; // Left most child + right sibling
	pnode secondparent; // If node has two parents, this pointer is to the second
};
// Memory allocation
Item itemalloc[MAXn];
Node nodealloc[MAXN];
int itemallocupto, nodeallocupto;
pitem newitem()
{
	return itemalloc + itemallocupto++;
}
pnode newnode(pitem item, int val)
{
	pnode _new = nodealloc + nodeallocupto++;
	_new->item = item;
	_new->val = val;
	item->inheap = _new;
	return _new;
}
// Used for intermediary storage during pop
int mxrank;
pnode ofrank[100];
struct HollowHeap
{
	pnode root;
	int sz;
	HollowHeap() // Initialise 
	{
		root = NULL;
		sz = 0;
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
			ofrank[a->rank] = NULL;
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
	void push(pitem item, int val)
	{
		push(newnode(item, val));
	}
	void push(int val)
	{
		push(newitem(), val);
	}
	void decreasekey(pitem item, int val)
	{
		pnode a = item->inheap;
		if (a == root)
		{
			// Update value, then done
			a->val = val;
			return;
		}
		pnode b = newnode(item, val);
		// Rank of b is max(0, a->rank - 2)
		if (a->rank > 2) b->rank = a->rank - 2;
		// Make a hollow, and make b the second parent 	
		a->item = NULL;
		b->child = a;
		a->secondparent = b;
		insertIntoHeap(b); // Make b a child of the root, or the parent of the root
	}
	void pop() // Remove the smallest item from the heap
	{
		sz--;
		if (!sz) // Heap should now be empty
		{
			root = NULL;
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
					a->sibling = NULL;
					ranklinks(a); // Merge with nodes of the same rank, leaving only one node with its rank
				}
				else // Is hollow
				{
					if (a->secondparent) // Has two parents
					{
						if (a->secondparent == v) next = NULL; // A is the last child of v
						else a->sibling = NULL; // A now has no more siblings
						a->secondparent = NULL; // A now has one parent
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
		root = NULL;
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
				ofrank[i] = NULL; // Clear the array for future usage
			}
		}
	}
	void erase(pitem item) // Remove item from the heap
	{
		pnode a = item->inheap;
		if (a == root) pop(); // Is the minimum item, do a pop
		else
		{
			a->item = NULL; // Make hollow
			item->inheap = NULL;
		}
	}
};