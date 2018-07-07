// Edmonds minimal branching algorithm with pairing heap, O(e log v)
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

// Fibonacci Heap
typedef struct FibNode* pnode;
struct FibNode
{
	Edge val;
	int degree; // Value of the node and number of children
	bool onechildcut; // Whether one of its children has been cut due to increase key
	pnode left, right, child, par; // When the node is the root of the heap, left and right refer to its neighbours in the heap linked list
	// When the node is not the root of the heap, left and right refer to its neighbouring siblings
};
// Global variables, used for intermediate storage during the pop function
pnode _ofsize[100];
int _ofsizedone[100], _ofsizeupto;
namespace fibheapalloc
{
// Nodes are allocated from this array
FibNode _heap[MAXEDGES]; 
int _heapallocupto;
pnode _newnode(Edge val)
{
	pnode _new = _heap + _heapallocupto++; // Dynamic allocation is slow ... this is much faster
	//pnode _new = new FibNode(); // Other method of allocating memory
	_new->val = val;
	return _new;
}
}
struct FibHeap
{
	pnode mn; // Pointer to the minimum value in the heap
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
		return mn->val.val();
	}
	void swap(pnode &a, pnode &b) // Swaps two pnodes. Created to remove any reliance on STL
	{
		pnode c = a;
		a = b;
		b = c;
	}
	pnode mergetrees(pnode a, pnode b) // Merges trees with equal degree, creating one tree with degree+1
	{
		if (b->val.val() < a->val.val())
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
		// If the new value is smaller, set as root
		if (_new->val.val() < mn->val.val())
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
	void push(Edge val) // Insert a value into the heap
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
		// Remove mx from the heap
		if (mn->left == mn) // If there was only one node in the heap - special case
		{
			// The heap will just consist of the children of mx
			// Set the first child as the root, insert the rest
			mn = temproot = mn->child;
			pnode child = mn->right;
			mn->left = mn->right = mn;
			while (child != NULL)
			{
				pnode nextchild = child->right;
				addintoheap(child);
				if (child->val.val() < mn->val.val()) mn = child;
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

			// Add the children of mx to the heap
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
			if (mn->val.val() >= a->val.val()) mn = a;
			a = a->right;
		}
		while (a != temproot);
	}
	void merge(FibHeap *a) // Merge Fibonacci Heap a into this heap
	{
		// Cut each heap between their minimum and the element to the right of that, then splice together
		sz += a->sz; // update size of heap
		if (!mn)
		{
			mn = a->mn;
			return;
		}
		if (!a->mn) return;
		pnode b = a->mn; 
		pnode br = b->right;
		pnode mnr = mn->right;

		b->right = mnr;
		
		mnr->left = b;

		mn->right = br;
		br->left = mn;
		if (b->val.val() < mn->val.val()) mn = b; // Update min if needed
	}
};
FibHeap* incoming[2*MAXN]; // Stores incoming edges in a pairing heap
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
		incoming[i] = new FibHeap();
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