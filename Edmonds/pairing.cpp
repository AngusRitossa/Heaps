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

// Pairing Heap
typedef struct PairingNode* pnode;
struct PairingNode
{
	Edge val; // Value of the node
	pnode child, left, right; // Point to leftmost child, left sibling and right sibling
	// If a node is the leftmost child, left points to the parent.
};
namespace pairingheapalloc
{
// Nodes are allocated from this array
PairingNode _heap[MAXEDGES]; 
int _heapallocupto;
pnode newnode(Edge val)
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
	ll top() // Minimum value
	{
		return root->val.val();
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
		if (!a) return b;
		if (b->val.val() < a->val.val()) swap(a, b); // Make the heap with the smaller root the new root
		b->right = a->child;
		if (b->right) b->right->left = b;
		a->child = b;
		b->left = a;
		return a;
	}
	void merge(PairingHeap *a) // Merges a heap into this heap
	{
		sz += a->sz;
		root = merge(root, a->root);
	}
	void push(pnode _new) // Inserts a pnode into the heap
	{
		sz++;
		if (sz == 1) // Just make this node the heap
		{
			root = _new;
			return;
		}
		if (_new->val.val() < root->val.val()) // Make _new the new root
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
	void push(Edge val) // Inserts an element into the heap
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
};
PairingHeap* incoming[2*MAXN]; // Stores incoming edges in a pairing heap
int v, e; // Numbers of vertices and edges
ll ans; // Stores the weight of the minimum spanning tree
queue<int> roots; // Stores all the roots to be processed
Edge par[2*MAXN]; // Stores the parent edge of a node, used to find cycles
Edge edges[MAXEDGES];
int main()
{
	// Scan in input
	scanf("%d%d", &v, &e);
	// Declare memory
	for (int i = 0; i < 2*MAXN; i++)
	{
		incoming[i] = new PairingHeap();
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
		edges[i] = e;
	}

	// Start the timer
	milliseconds start = duration_cast<milliseconds>(system_clock::now().time_since_epoch());

	for (int i = 0; i < e; i++) // Push onto heaps
	{
		incoming[edges[i].v]->push(edges[i]);
	}
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
			e = incoming[a]->root->val;
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