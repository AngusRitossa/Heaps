// Edmonds minimal branching algorithm with binomial heap, O(e log v)
// Calculates the total length of the minimum spanning arborescence rooted at 0. Can be modified to return the edges used. 
#include <cstdio>
#include <algorithm>
#include <queue>
#include <cassert>
#include <utility>
#include <chrono>
using namespace std;
using namespace std::chrono;
#define MAXN 1000000
#define MAXEDGES 10000000
typedef long long ll;
struct UFVal // Disjoint set union find data structure, used for supernodes
{
	int rep[2*MAXN]; // Stores the highest node we know on the path from this node to its root
	ll val[2*MAXN]; // Stores sum of all values on the path from the node to its root, excluding the value of the root 
	ll valatroot[2*MAXN]; // Stores the value at a root, or what value it had when it was a root
	int upto;
	UFVal() // Initialises the union find for MAXN elements
	{
		upto = MAXN;
		for (int i = 0; i < 2*MAXN; i++) rep[i] = i;
	}
	int findrep(int a) // Finds the root of the tree that a is in. Updates the value of a as well
	{
		if (rep[a] == a) return a;
		int b = rep[a];
		rep[a] = findrep(b);
		val[a] += val[b] + valatroot[a];
		valatroot[a] = 0;
		return rep[a];
	}
	bool connected(int a, int b) // Are a and b connected?
	{
		return findrep(a) == findrep(b);
	}
	void merge(int a, int b) // Joins the sets containing a and b
	{
		// Set a and b to their roots
		a = findrep(a);
		b = findrep(b);
		rep[a] = b;
	}
	void addval(int a, ll val) // Adds val to the set containing a
	{
		a = findrep(a);
		valatroot[a] += val;
	}
	ll findval(int a) // Returns the value of node a
	{
		int b = findrep(a);
		return val[a] + valatroot[b];
	}
};
struct UF // Simpler union find data structure that does not need to support adding a value. Used to see if a path exists between two nodes
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
	bool connected(int a, int b)
	{
		return findrep(a) == findrep(b);
	}
	void merge(int a, int b)
	{
		rep[findrep(a)] = findrep(b);
	}
};
UFVal supernodes; // Stores all the compacted 'supernodes'
UF trees; // Stores all the trees that have been constructed so far
struct Edge // Stores a directed edge from u to v
{
	int u, v;
	ll weight;
	ll val() // Returns the corrected weight of this edge, i.e. weight - the value subtracted from all edges going into v
	{
		return weight - supernodes.findval(v);
	}
	bool operator== (const Edge e) const
	{
		return e.u == u && v == e.v && weight == e.weight;
	}
};

// Binomial Heap
typedef struct BinomialNode* pnode;
struct BinomialNode
{
	Edge val;
	int node;
	int degree;
	pnode sibling, child, par; // sibling is either in the heap linked list of the sibling within a tree. 
};
namespace binomialheapalloc
{
// Nodes are allocated from this array
BinomialNode _heap[MAXEDGES]; 
int _heapallocupto;
pnode _newnode(Edge val)
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
	ll top()
	{
		return mn->val.val();
	}
	void swap(pnode &a, pnode &b) // Swaps two pnodes. Created to remove any reliance on STL
	{
		pnode c = a;
		a = b;
		b = c;
	}
	void swap(int &a, int &b) // Swaps two ints. Created to remove any reliance on STL
	{
		int c = a;
		a = b;
		b = c;
	}
	pnode mergetrees(pnode a, pnode b, pnode pre = NULL) // Merges trees with equal degree, creating one tree with degree+1
	{
		if (b->val.val() < a->val.val())
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
		if (mn == NULL || a->val.val() <= mn->val.val()) mn = a;
		return a;
	}

	// Main functions
	void push(pnode _new)
	{
		// Worst case O(log(n)) - average O(1)
		sz++;
		if (mn == NULL || _new->val.val() < mn->val.val()) mn = _new; // Update mn if needed
		_new->sibling = root; // Set this new node as the first one
		root = _new; // set as root
		while (_new->sibling && _new->degree == _new->sibling->degree) // Will merge until no longer necessary, then break
		{
			_new = mergetrees(_new, _new->sibling);
		}
	}
	void push(Edge val)
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
			if (mn == NULL || b->val.val() < mn->val.val()) mn = b; // update mn if needed
			if (mn == NULL || a->val.val() < mn->val.val()) mn = a; // update mn if needed
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
			if (mn == NULL || a->val.val() < mn->val.val()) mn = a; // update mn if needed
			a->par = NULL; // Its a root - has no parent
			if (pre) pre->sibling = a;
			else root = a;
			pre = a;
			a = a->sibling;
		}
		while (b) // Add to the end
		{
			if (mn == NULL || b->val.val() < mn->val.val()) mn = b; // update mn if needed
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
				if (mn == NULL || b->val.val() < mn->val.val()) mn = b;
				b = b->sibling;
			}
		}
	}
};
BinomialHeap* incoming[2*MAXN]; // Stores incoming edges in a pairing heap
int v, e; // Numbers of vertices and edges
ll ans; // Stores the weight of the minimum spanning tree
queue<int> roots; // Stores all the roots to be processed
Edge par[2*MAXN]; // Stores the parent edge of a node, used to find cycles
int main()
{
	// Scan in input
	scanf("%d%d", &v, &e);
	// Declare memory
	for (int i = 0; i < 2*MAXN; i++)
	{
		incoming[i] = new BinomialHeap();
	}
	for (int i = 0; i < e; i++)
	{
		int a, b;
		ll c;
		scanf("%d%d%lld", &a, &b, &c);
		Edge e;
		e.u = a;
		e.v = b;
		e.weight = c;
		incoming[b]->push(e);
	}
	// Start the timer
	milliseconds start = duration_cast<milliseconds>(system_clock::now().time_since_epoch());

	// Push all nodes onto the queue to be processed, except the root
	for (int i = 1; i < v; i++)
	{
		roots.push(i);
	}
	// Run the algorithm
	while (!roots.empty())
	{
		int a = roots.front();
		roots.pop();
		if (a != supernodes.findrep(a)) continue; // Ensure a is the root
		// Find the minimum incoming edge
		Edge e;
		while (true)
		{
			if (incoming[a]->empty()) assert(false); // If this is the case, branching is not possible
			e = incoming[a]->mn->val;
			incoming[a]->pop();
			if (!supernodes.connected(e.u, e.v)) break; // We have found the edge!
 		}
 		ans += e.val(); // Add value to the answer
 		par[a] = e; 
 		if (trees.findrep(e.u) != trees.findrep(e.v)) // They are in different trees
 		{
 			trees.merge(e.u, e.v);
 		}
 		else // They are in the same tree, therefore a cycle was formed. Contract into a supernode
 		{
 			Edge mxedge = e; // Maximum edge on the cycle
 			int b = supernodes.findrep(e.u);
 			while (b != a) // Find the cycle, up
 			{
 				if (par[b].val() > mxedge.val()) mxedge = par[b]; // Update maximum edge if needed
 				b = supernodes.findrep(par[b].u);
 			}
 			// Remove mxedge from cost
 			ans -= mxedge.val();
 			
 			int s = supernodes.upto++; // New supernode
 			// Go over the cycle again, merging the incoming edge lists
 			b = supernodes.findrep(e.u);
 			while (supernodes.findrep(b) != s)
 			{
 				// Subtract weight of par[b] - mxedge from the weight of all edges going into b
 				supernodes.addval(b, par[b].val() - mxedge.val());
 				// Merge the incoming edges of b into a
 				incoming[s]->merge(incoming[b]);
 				// Merge b and a
 				supernodes.merge(b, s);
 				b = supernodes.findrep(par[b].u);
 			}
 			roots.push(s);
 		}
	}
	// Print the answer
	printf("%lld\n", ans);

	// End the timer, print the time
	milliseconds end = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
	ll totaltime = end.count() - start.count();
	printf("Time % 6lldms\n", totaltime);
}