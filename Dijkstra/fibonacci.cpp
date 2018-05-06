// Dijkstra's algorithm implemented with a fibonacci heap: O(e + vlogv)
#include <cstdio>
#include <vector>
#include <utility>
typedef long long ll;
typedef struct FibNode* pnode;
struct FibNode
{
	ll val;
	int degree, node; // Value of the node and number of children
	bool onechildcut; // Whether one of its children has been cut due to decrease key
	pnode left, right, child, par; // When the node is the root of the heap, left and right refer to its neighbours in the heap linked list
	// When the node is not the root of the heap, left and right refer to its neighbouring siblings
};
// Global variables, used for intermediate storage during the pop function
pnode _ofsize[100];
int _ofsizedone[100], _ofsizeupto;
namespace fibheapalloc
{
// Nodes are allocated from this array
FibNode _heap[20000000]; 
int _heapallocupto;
pnode _newnode(ll val)
{
	pnode _new = _heap + _heapallocupto++; // Dynamic allocation is slow ... this is much faster
	//pnode _new = new FibNode(); // Other method of allocating memory
	_new->val = val;
	return _new;
}
}
struct FibHeap
{
	pnode mn; // Pointer to the maximum value in the heap
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
	ll top()
	{
		return mn->val;
	}
	void swap(pnode &a, pnode &b) // Swaps two pnodes. Created to remove any reliance on STL
	{
		pnode c = a;
		a = b;
		b = c;
	}
	pnode mergetrees(pnode a, pnode b) // Merges trees with equal degree, creating one tree with degree+1
	{
		if (b->val < a->val)
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
		// Make tree b the child of tree a
		a->degree++;
		b->right = a->child;
		if (b->right) b->right->left = b;
		b->left = NULL;
		a->child = b;
		b->par = a;
		return a;
	}
	void addintoheap(pnode _new) // Inserts the node into the heap linked list
	{
		_new->right = mn->right;
		_new->left = mn;
		mn->right = _new;
		_new->right->left = _new;
		// If the new value is larger, set as root
		if (_new->val < mn->val)
		{
			mn = _new;
		}
	}

	// Main functions
	void push(pnode _new) // Insert a node into the heap
	{
		if (!sz)
		{
			// If the heap is empty, just set this as the only node
			sz++;
			mn = _new;
			mn->left = mn->right = mn; // Make sure linked list is circular
			return;
		}
		// Add new node into linked list
		sz++;
		addintoheap(_new);
	}
	void push(ll val) // Insert a value into the heap
	{
		pnode _new = fibheapalloc::_newnode(val);
		push(_new);
	}

	void pop() // Remove the largest element from the heap
	{
		sz--;
		if (!sz) // If only one element, just remove it
		{
			mn = NULL;
			return;
		}
		// Remove mn from the heap
		if (mn->left == mn) // If there was only one node in the heap - special case
		{
			// The heap will just consist of the children of mn
			// Set the first child as the root, insert the rest
			mn = temproot = mn->child;
			pnode child = mn->right;
			mn->left = mn->right = mn;
			while (child != NULL)
			{
				pnode nextchild = child->right;
				addintoheap(child);
				if (child->val < mn->val) mn = child;
				child = nextchild;
			}
			// The heap is now sufficient (since there were at most log children)
			return;
		}
		else
		{	
			temproot = mn->left;
			temproot->right = mn->right;
			temproot->right->left = temproot;

			// Add the children of mn to the heap
			pnode child = mn->child;
			while (child != NULL)
			{
				pnode nextchild = child->right; // Store the next child because it will be lost when we insert child into the heap
				// Insert child into heap
				child->right = temproot->right;
				child->left = temproot;
				temproot->right = child;
				child->right->left = child;
				child->par = NULL;
				// Set child to its sibling
				child = nextchild;
			}
		}
		// Fix the heap by merging trees of the same priority
		pnode a = temproot;
		mn = temproot;
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
			if (mn->val >= a->val) mn = a;
			a = a->right;
		}
		while (a != temproot);
	}
	void merge(FibHeap &a) // Merge Fibonacci Heap a into this heap
	{
		// Cut each heap between their maximum and the element to the right of that, then splice together
		pnode b = a.mn; 
		pnode br = b->right;
		pnode mnr = mn->right;

		b->right = mnr;
		mnr->left = b;

		mn->right = br;
		br->left = mn;

		if (b->val < mn->val) mn = b; // Update max if needed

		sz += a.sz; // update size of heap
	}
	void cutfromtree(pnode a) // Removes this node from its parent and inserts it into the heap
	{
		pnode p = a->par;
		if (p == NULL) return;
		if (p->child == a) // A is the first child of p
		{
			p->child = a->right;
			if (a->right) a->right->left = NULL;
		}
		else // A is somewhere in the middle. Remove a from the child linked-list
		{
			if (a->left) a->left->right = a->right;
			if (a->right) a->right->left = a->left;
		}
		a->par = NULL;
		a->onechildcut = 0;
		// insert a into the heap linked list
		addintoheap(a);
	}
	void decreasekey(pnode a, ll val) // Increase the value of a node. If val > a->val the heap-order will be broken
	{
		// Update the value of a
		a->val = val;
		if (a->par != NULL && a->par->val > a->val) // heap order has been violated
		{
			cutfromtree(a); // Cut a from the tree
			pnode p = a->par;
			a->par = NULL;
			while (p && p->onechildcut) // If any parents are marked, cut from tree
			{
				p->degree--; // P has lost a child, subtract one from the degree
				cutfromtree(p);
				pnode _newpar = p->par;
				p->par = NULL;
				p = _newpar;
			}
			if (p)
			{
				p->onechildcut = 1; // Mark the parent
				p->degree--; // P has lost a child, subtract one from the degree
			}
		}
		else // Update max if needed
		{
			if (a->val < mn->val) mn = a;
		}
	}
	void erase(pnode a) // Remove a node from the heap
	{
		// Update the value to infinity (in this case, (2^31)-1)
		decreasekey(a, -((1ll << 31)-1));
		// Since a should now be the greatest element, pop
		pop();
	}
};
int v, e;
std::vector<std::pair<ll, int> > adj[10000000];
FibHeap pq;
pnode nodes[10000000];
int main()
{
	// Scan in the input
	scanf("%d%d", &v, &e);
	for (int i = 0; i < e; i++)
	{
		int a, b;
		ll c;
		scanf("%d%d%lld", &a, &b, &c);
		adj[a].emplace_back(b, c);
		adj[b].emplace_back(a, c);
	}

	// Initialise the distance to each node
	nodes[0] = fibheapalloc::_newnode(0);
	nodes[0]->node = 0;
	pq.push(nodes[0]);
	for (int i = 1; i < v; i++)
	{
		nodes[i] = fibheapalloc::_newnode(1e18);
		nodes[i]->node = i;
		pq.push(nodes[i]);
	}

	// Run dijkstra
	while (!pq.empty())
	{
		int a = pq.mn->node;
		ll d = pq.top();
		pq.pop();
		for (auto b : adj[a])
		{
			if (d + b.second < nodes[b.first]->val)
			{
				pq.decreasekey(nodes[b.first], d + b.second);
			}
		}
	}
	// Print distance to node n-1;
	printf("%lld\n", nodes[v-1]->val);
}