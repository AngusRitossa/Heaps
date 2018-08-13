// Stoer-Wagner's algorithm implemented with a fibonacci heap: O(ve + v^2 log v)
#include <cstdio>
#include <vector>
#include <utility>
#include <chrono>
using namespace std::chrono;
using namespace std;
#define MAXN 5000000
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
FibNode _heap[MAXN]; 
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
	pnode mn = NULL; // Pointer to the maximum value in the heap
	int sz = 0; // Number of elements in the heap
	pnode temproot = NULL; // Used in the pop function
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
		_new->par = NULL;
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
			mn->par = NULL;
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
		if (a->onechildcut) 
		{
			a->onechildcut = 0;
			a->degree--;
		}
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
		// insert a into the heap linked list
		addintoheap(a);
	}
	void decreasekey(pnode a, ll val) // Increase the value of a node. If val > a->val the heap-order will be broken
	{
		// Update the value of a
		a->val = val;
		if (a->par != NULL && a->par->val > a->val) // heap order has been violated
		{
			pnode p = a->par;
			cutfromtree(a); // Cut a from the tree
			while (p && p->onechildcut) // If any parents are marked, cut from tree
			{
				p->degree--; // P has lost a child, subtract one from the degree
				pnode _newpar = p->par;
				cutfromtree(p);
				p = _newpar;
			}
			if (p)
			{
				p->onechildcut = 1; // Mark the parent
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
// Stoer-Wagner code
struct UF // Union-find data structure (path compression) - used for merging nodes
{	
	int rep[MAXN];
	UF()
	{
		for (int i = 0; i < MAXN; i++) rep[i] = i;
	}
	int findrep(int a)
	{
		if (rep[a] == a) return a;
		return rep[a] = findrep(rep[a]);
	}
	void merge(int a, int b)
	{
		rep[findrep(a)] = findrep(b);
	}
};
UF uf;
typedef struct Edge* pedge;
struct Edge // Adjacency list is stored as a linked list for O(1) merging
{
	pedge adj;
	int v;
	ll weight;
};
pedge adj[MAXN], lastadj[MAXN]; // Outgoing edges
int v, e, inlegal[MAXN];
ll ans = 1e18;
pnode nodes[MAXN];
FibHeap pq;
int main()
{
	// Scan input
	scanf("%d%d", &v, &e);
	for (int i = 0; i < e; i++)
	{
		int a, b;
		ll c;
		scanf("%d%d%lld", &a, &b, &c);
		pedge e = new Edge();
		e->v = b;
		e->weight = c;
		e->adj = adj[a];
		adj[a] = e;
		if (!lastadj[a]) lastadj[a] = e;
		e = new Edge();
		e->v = a;
		e->weight = c;
		e->adj = adj[b];
		adj[b] = e;
		if (!lastadj[b]) lastadj[b] = e;
	}

	// Start the timer
	milliseconds start = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
	for (int i = 0; i < v; i++) nodes[i] = fibheapalloc::_newnode(0), nodes[i]->node = i;
	// Run the algorithm
	for (int c = 1; c <= v-1; c++) // Run this v-1 times
	{
		// Construct "legal ordering"
		// Push all nodes onto the pq
		for (int i = 0; i < v; i++)
		{
			if (uf.findrep(i) == i) // If it hasn't been contracted
			{
				nodes[i]->val = 0;
				nodes[i]->degree = nodes[i]->onechildcut = 0;
				nodes[i]->par = nodes[i]->child = nodes[i]->left = nodes[i]->right = NULL;
				pq.push(nodes[i]);
			}
		}
		int last;
		while (pq.size() > 1)
		{
			int a = pq.mn->node; // A is the next node in the legal ordering
			last = a;
			inlegal[a] = c;
			pq.pop();
			for (pedge e = adj[a]; e; e = e->adj) 
			{
				// For each outgoing edge, do a decrease-key
				int b = uf.findrep(e->v);
				if (inlegal[b] != c)
				{
					pq.decreasekey(nodes[b], nodes[b]->val - e->weight);
				}
			}
		}
		ans = min(ans, -pq.top()); // Update answer if needed
		int a = pq.mn->node;
		pq.pop();
		// Merge a and last
		uf.merge(a, last);
		if (adj[a]) 
		{
			if (adj[last])
			{
				lastadj[a]->adj = adj[last];
				adj[last] = adj[a];
			}
			else
			{
				adj[last] = adj[a];
				lastadj[last] = lastadj[a];
			}
		}	
	}
	printf("%lld\n", ans);

	// End the timer, print the time
	milliseconds end = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
	ll totaltime = end.count() - start.count();
	printf("Time % 6lldms\n", totaltime);
}