// Quake heap, O(1) push, decrease-key, O(log(n)) pop, merge
#define MAXN 1000001
#define MXRANK 80
#define A1 4 // Required ratio of amounts of each rank = A1/A2
#define A2 5
typedef struct QuakeValue* pvalue;
typedef struct QuakeNode* pnode;
struct QuakeValue
{
	pnode inheap; // Highest occurrence of it in the heap
	int val; // Value
};
struct QuakeNode
{
	// A tournament node in the heap
	pnode left, right, par; // Left & right children, parent within one tree
	pnode l, r; // Left and right trees, if this node is a root
	pvalue val; // Value stored in this tree, i.e. lowest out of its two children
	int rank;
};
// Used for allocating memory for quakevalue
int allocupto;
QuakeValue allocarray[MAXN];
pvalue newvaluenode(int val)
{
	pvalue _new = allocarray + allocupto++;
	_new->val = val;
	return _new;
}
// Used for allocating memory for quakenodes. Faster than dynamic allocation
// Since O(nlogn) nodes will be used over the life, but only O(n) at once, nodes will be reused. This is faster the reallocating
int allocupto2;
QuakeNode allocarray2[5*MAXN];
pnode reusable; // Singly linked list of reusable nodes
void deletenode(pnode a)
{
	a->right = reusable;
	reusable = a;
}
pnode newnode()
{
	pnode _new;
	if (!reusable)
	{
		_new = allocarray2 + allocupto2++;
	}
	else 
	{
		_new = reusable;
		reusable = reusable->right;
		_new->left = _new->right = NULL;
	}
	_new->rank = 0;
	return _new;
}
pnode ofrank[100]; // Used for intermediary storage during pop
struct QuakeHeap
{
	pnode root;
	int sz;
	int am[MXRANK]; // Stores the amount of nodes of each rank
	int mxrank;
	QuakeHeap() // Initialise
	{
		root = NULL;
		sz = 0;
		for (int i = 0; i < MXRANK; i++) am[i] = 0;
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
		return root->val->val;
	}
	void insertIntoHeap(pnode a, pnode &root) // Inserts a into the heap rooted at root
	{
		a->par = NULL;
		if (!root) // Only node in the heap
		{
			root = a->l = a->r = a;
			return;
		}
		// Insert to the right of the root
		pnode b = root->r;
		a->r = b;
		b->l = a;
		root->r = a;
		a->l = root;
		// Update root if needed
		if (a->val->val < root->val->val) root = a;
	}
	pnode mergetrees(pnode a, pnode b)
	{
		// Create a new node, c which will be the parent of a and b
		pnode c = newnode();
		c->rank = a->rank+1;
		am[c->rank]++; // One more node of this rank
		c->left = a;
		c->right = b;
		a->par = b->par = c;
		// Set the value at c to be the minimum of the two children
		if (a->val->val < b->val->val) c->val = a->val;
		else c->val = b->val;
		c->val->inheap = c; // Update the maximum node in the tree for the value at c
		return c;
	}
	void addNode(pnode a) // Used during pop to add a node to the ofrank array and do necessary merges
	{
		if (ofrank[a->rank])
		{
			// Merge them
			a = mergetrees(a, ofrank[a->rank]);
			ofrank[a->rank-1] = NULL;
			// Try again
			addNode(a);
		}
		else
		{
			if (a->rank > mxrank) mxrank = a->rank;
			ofrank[a->rank] = a;
		}
	}
	void quake(pnode a, int hei) // Quake operation, remove all nodes of rank > hei
	{
		if (a->rank <= hei)
		{
			// We should keep this one, insert it into the heap
			// Also, this is now the heighest occurrence of the value stored here
			a->val->inheap = a;
			insertIntoHeap(a, root);
		}
		else
		{
			// Recurse into both children
			if (a->left) quake(a->left, hei);
			if (a->right) quake(a->right, hei);
			am[a->rank]--;
			deletenode(a);
		}
	}

	// Main functions
	void push(pvalue val)
	{
		sz++;
		pnode a = newnode();
		a->val = val;
		val->inheap = a;
		am[0]++; // Another node of rank 0
		insertIntoHeap(a, root);
	}
	void push(int val)
	{
		push(newvaluenode(val));
	}
	void merge(pnode a)
	{
		// Merge the linked lists
		if (!a) return;
		if (!root) 
		{
			root = a;
			return;
		}
		pnode b = a->r;
		pnode x = root->r;
		root->r = b;
		b->l = root;
		a->r = x;
		x->l = a;
		if (a->val->val < root->val->val) root = a; // Update root if needed
	}
	void merge(QuakeHeap *a)
	{
		sz += a->sz;
		merge(a->root);
		// Then, merge the rank lists
		for (int i = 0; i <= a->mxrank; i++) am[i] += a->am[i];
		if (a->mxrank > mxrank) mxrank = a->mxrank;
	}
	void pop()
	{	
		sz--;
		if (!sz)
		{
			root = NULL;
			return;
		}
		mxrank = 0;
		// Add all non-root trees to the new heap
		pnode a = root->r;
		while (a != root)
		{
			addNode(a);
			a = a->r;
		}
		// Remove the path from the root to the maximum node
		a = root;
		while (true)
		{
			pnode b = a;
			am[a->rank]--;
			if (a->left && a->right)
			{
				// Insert the other child into the new heap, 'recurse' into the child with the same value
				if (a->left->val == a->val)
				{
					addNode(a->right);
					a = a->left;
				}
				else
				{
					addNode(a->left);
					a = a->right;
				}
			}
			else if (a->left) a = a->left;
			else if (a->right) a = a->right;
			else
			{
				deletenode(a);
				break;
			}
			deletenode(b);
		}
		// Construct new heap
		root = NULL;
		for (int i = 0; i <= mxrank; i++)
		{
			if (ofrank[i])
			{
				insertIntoHeap(ofrank[i], root);
				ofrank[i] = NULL;
			}
		}
		// Check if any of the ranks fail the amount condition
		for (int i = 0; i < mxrank; i++)
		{
			if ((am[i]*A1)/A2 < am[i+1]) 
			{
				// Remove all nodes with rank > i
				pnode oldroot = root;
				root = NULL;
				a = oldroot;
				do
				{
					pnode next = a->r;
					quake(a, i);
					a = next;
				}
				while (a != oldroot);
				break;
			}
		}
	}
	void decreasekey(pvalue a, int val)
	{
		a->val = val;
		pnode x = a->inheap;
		if (x->par) // Needs to be cut from tree, reinserted
		{
			// Remove from parent
			if (x->par->left == x) x->par->left = NULL;
			else x->par->right = NULL;
			// Reinsert into heap
			insertIntoHeap(x, root);
		} 
		else if (val < root->val->val) root = x; // Update root if needed
	}
};