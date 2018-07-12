// Minimum rank-pairing heap. 
typedef struct Node* pnode;
#define MAXN 1000000
struct Node
{
	int val, rank; // Value of the node
	pnode par, left, right; // Structure is maintained as a binary heap maintaining partial heap order. In particular, the left children satifsy it.
	bool isRoot; // Stores if the node is a root
	// Note: Roots are stored in a doubly linked circular linked list. 
	// In this case, left and right refer to the left and right siblings in this list
	// The root will have one left child, which will be stored in the par pointer. 
};
pnode nodesOfRank[100]; // Used for intermediate storage during pop functions

namespace rankpairingalloc // For allocating memory
{
	Node heapalloc[MAXN];
	int upto;
	pnode newnode(int val)
	{
		pnode _new = heapalloc + upto++;
		// pnode _new = new Node();
		_new->val = val;
		return _new;
	}
}
struct RPHeap
{
	int sz = 0;
	pnode root = NULL;
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
	void swap(pnode &a, pnode &b) // Swaps pnodes
	{
		pnode c = a;
		a = b;
		b = c;
	}
	void addIntoHeap(pnode &root, pnode a) // Inserts a into the heap rooted at root
	{
		a->isRoot = 1;
		if (!root) // This node is the only node in the heap
		{
			a->left = a->right = root = a;
			return;
		}
		// Insert to the right of the root
		a->left = root;
		a->right = root->right;
		a->right->left = a;
		root->right = a;	
		// Update root if needed
		if (a->val < root->val) root = a;
	}
	pnode mergetrees(pnode x, pnode y) // Merges trees with equal rank into one tree
	{
		if (x->val > y->val) swap(x, y); // Guarantee that x->val <= y->val
		// Make y the left child of x, stored in the par pointer
		y->left = y->par;
		y->right = x->par;
		if (y->right) y->right->par = y;
		x->par = y;
		y->par = x;
		x->rank++; // Increase rank of x
		y->isRoot = 0; // Y is no longer a root
		return x;
	}

	// Main functions
	void push(pnode a) // Add the node a into the heap
	{
		sz++;
		addIntoHeap(root, a);
	}
	void push(int val) // Add the value a into the heap
	{
		pnode a = rankpairingalloc::newnode(val);
		sz++;
		addIntoHeap(root, a);
	}
	void merge(RPHeap* a)
	{
		if (!a->root) return;
		sz+=a->sz;
		if (!root)
		{
			root = a->root;
			return;
		}
		// Add the circular linked list a next to the root
		pnode b = root->right;
		pnode c = a->root;
		pnode d = c->right;

		root->right = d;
		d->left = root;

		b->left = c;
		c->right = b;

		// Update root if needed
		if (c->val < root->val) root = c;
	}
	int rank(pnode a) // Returns the rank of a, or -1 if a doesn't exist
	{
		if (a) return a->rank;
		else return -1;
	}
	void pop() // Removes root from the heap
	{
		sz--;
		pnode newroot = NULL;
		int mxrank = 0; // Maximum rank seen
		pnode c = root->par;
		while (c) // Insert the chain of right children into new heap
		{
			pnode d = c->right; // Next node to be processed
			c->rank = rank(c->left)+1;
			c->par = c->left;
			if (nodesOfRank[c->rank]) // Do merge, insert into heap
			{
				pnode a = nodesOfRank[c->rank];
				nodesOfRank[c->rank] = NULL;
				addIntoHeap(newroot, mergetrees(a, c));
			}
			else // Store for a possible future merge
			{
				nodesOfRank[c->rank] = c;
				if (c->rank > mxrank) mxrank = c->rank;
			}
			c = d;
		}
		c = root->right; // Now process all the other roots
		while (c != root) 
		{
			pnode d = c->right; // Next node to be processed
			if (nodesOfRank[c->rank]) // Do merge, insert into heap
			{
				pnode a = nodesOfRank[c->rank];
				nodesOfRank[c->rank] = NULL;
				addIntoHeap(newroot, mergetrees(a, c));
			}
			else // Store for a possible future merge
			{
				nodesOfRank[c->rank] = c;
				if (c->rank > mxrank) mxrank = c->rank;
			}
			c = d;
		}
		// Insert all leftover nodes into the new heap
		for (int i = 0; i <= mxrank; i++)
		{
			if (nodesOfRank[i])
			{
				addIntoHeap(newroot, nodesOfRank[i]);
				nodesOfRank[i] = NULL;
			}
		}
		root = newroot;
	}
	void decreasekey(pnode a, int val) // Decrease the value at a to val. Uses type-2 rank reduction 
	{
		a->val = val;
		if (a->isRoot) 
		{
			if (a->val < root->val) root = a;
			return;
		}
		// Detach a from its parent, and set its right child in its place
		pnode p = a->par;
		pnode c = a->right;
		// Replace a with c as p's child
		if (p->left == a) p->left = c;
		else if (p->right == a) p->right = c;
		else p->par = c;
		if (c) c->par = p;
		// Insert a into the heap
		a->par = a->left;
		a->isRoot = 1;
		addIntoHeap(root, a);
		// Do the rank-decrease step
		while (true)
		{
			if (p->isRoot) // Set to rank(child)+1
			{
				p->rank = rank(p->par)+1;
				break;
			}
			int lrank = rank(p->left);
			int rrank = rank(p->right);
			if (lrank < rrank) // Swap the values
			{
				int s = lrank;
				lrank = rrank;
				rrank = s;
			}
			int k = lrank-rrank > 1 ? lrank : lrank+1;

			// If k < rank of p, update and go to p's parent
			// Else we are done
			if (k < p->rank) p->rank = k;
			else break;
			p = p->par;
		}
	}
};