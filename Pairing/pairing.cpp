// Pairing Heap with push, pop, erase, top (max), merge & increase key
typedef struct PairingNode* pnode;
#define MAXN 1000000
struct PairingNode
{
	int val; // Value of the node
	pnode child, left, right; // Point to leftmost child, left sibling and right sibling
	// If a node is the leftmost child, left points to the parent.
};
namespace pairingheapalloc
{
// Nodes are allocated from this array
PairingNode _heap[MAXN]; 
int _heapallocupto;
pnode newnode(int val)
{
	pnode _new = _heap + _heapallocupto++; // Dynamic allocation is slow ... this is much faster
	//pnode _new = new PairingNode(); // Other method of allocating memory
	_new->val = val;
	return _new;
}
}
struct PairingHeap
{
	pnode root; // Pointer to root
	int sz = 0; // Number of elements in the heap
	// Auxiliary functions
	int size()
	{
		return sz;
	}
	bool empty()
	{
		return !sz;
	}
	int top() // Maximum value
	{
		return root->val;
	}
	void swap(pnode &a, pnode &b) // Swaps two pnodes. Created to remove any reliance on STL
	{
		pnode c = a;
		a = b;
		b = c;
	}

	// Main functions
	pnode merge(pnode a, pnode b) // Merges two heaps into one heap
	{
		if (!b) return a;
		if (b->val > a->val) swap(a, b); // Make the heap with the largest root the new root
		b->right = a->child;
		if (b->right) b->right->left = b;
		a->child = b;
		b->left = a;
		return a;
	}
	void merge(PairingHeap &a) // Merges a heap into this heap
	{
		sz += a.sz;
		root = merge(root, a.root);
	}
	void push(pnode _new) // Inserts a pnode into the heap
	{
		sz++;
		if (sz == 1) // Just make this node the heap
		{
			root = _new;
			return;
		}
		if (_new->val > root->val) // Make _new the new root
		{
			_new->child = root;
			root->left = _new;
			root = _new;
		}
		else // Make _new the leftmost child of the root
		{
			_new->right = root->child;
			if (_new->right) _new->right->left = _new;
			root->child = _new;
			_new->left = root;
		}
	}
	void push(int val) // Inserts an element into the heap
	{
		pnode _new = pairingheapalloc::newnode(val);
		push(_new);
	}
	pnode recursivemerge(pnode a) // Helps with the pop function. First merges pairs of trees, then merges the pairs into one tree
	{
		if (!a) return a;
		pnode b = a->right;
		if (!b) return a;
		pnode c = b->right;
		return merge(merge(a, b), recursivemerge(c));
	}
	void pop() // Removes the largest element from the heap
	{
		sz--;
		root = recursivemerge(root->child);
		if (root) root->right = NULL;
	}
	void increasekey(pnode a, int val)
	{
		a->val = val;
		if (a == root) return;
		if (a->left->child == a) // A is the leftmost child
		{
			a->left->child = a->right;
			if (a->right) a->right->left = a->left;
			a->left = a->right = NULL;
			root = merge(root, a);
		}
		else
		{
			a->left->right = a->right;
			if (a->right) a->right->left = a->left;
			a->left = a->right = NULL;
			root = merge(root, a);
		}
	}
};