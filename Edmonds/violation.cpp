// Edmonds minimal branching algorithm with violation heap, O(e log v)
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

// Violation Heap
typedef struct ViolationNode* pnode;
struct ViolationNode
{
	Edge val;
	int rank;
	bool isRoot;
	pnode child, left, right; // Pointer two leftmost child, right/left nodes in sibling linked list/main linked list
};

// Nodes are allocated from this array
ViolationNode nodesForAlloc[MAXEDGES]; 
int heapallocupto;
pnode newnode(Edge val)
{
	pnode _new = nodesForAlloc + heapallocupto++; // Dynamic allocation is slow ...
	_new->val = val;
	return _new;
}
pnode _first[100], _second[100]; // Used for intermediary storage during pop
int mxrank; // Used for intermediary storage
struct ViolationHeap
{
	pnode root; // Pointer to root of heap containing minimum value
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
	ll top()
	{
		return root->val.val();
	}
	int rank(pnode a) // Computes the rank of the node
	{
		// Calculated by adding 1 to the ceiling of the average of the rank of the first two children
		int r = 0;
		if (a->child)
		{
			r += a->child->rank;
			if (a->child->right)
			{
				r += a->child->right->rank;
			}
			else r--; // Rank of NULL node is -1
		}
		else r--; // Rank of NULL node is -1
		return r/2 + 1;
	}
	void swap(pnode &a, pnode &b) // Swaps two pnodes
	{
		pnode c = a;
		a = b;
		b = c;
	}
	void insertIntoHeap(pnode a, pnode &root) // Inserts a into the heap rooted at root
	{
		a->isRoot = true;
		if (!root) // Heap is empty, just add in a
		{
			root = a->left = a->right = a;
			return;
		}
		// Add to the right of root
		pnode b = root->right;
		b->left = a;
		a->right = b;
		root->right = a;
		a->left = root;
		// Update root if needed
		if (a->val.val() < root->val.val()) root = a;
	}
	void swapFirstTwoChildren(pnode a) // Swaps first two children of a so that the rank of the first is at least the rank of the second
	{
		if (!a->child || !a->child->right) return; // Doesn't have two children
		pnode b = a->child;
		pnode c = b->right;
		if (b->rank < c->rank) // Needs swapping
		{
			c->left = a;
			b->right = c->right;
			if (c->right) c->right->left = b;
			c->right = b;
			b->left = c;
			a->child = c;
		}
	}
	pnode merge(pnode a, pnode b, pnode c)
	{
		// Swap so that a->val <= b->val && c->val
		if (b->val.val() <= a->val.val() && b->val.val() <= c->val.val()) swap(a, b); // B is the smallest
		else if (c->val.val() <= a->val.val() && c->val.val() <= b->val.val()) swap(a, c); // C is the smallest
		swapFirstTwoChildren(a); // Ensure old active children of a are sorted by rank
		// Make b and c children of a
		if (a->child) a->child->left = b;
		b->right = a->child;
		b->left = c;
		c->right = b;
		c->left = a;
		a->child = c; 
		a->rank = rank(a); // Update rank
		b->isRoot = c->isRoot = false;
		return a;
	}

	// Main functions
	void push(pnode a)
	{
		sz++;
		insertIntoHeap(a, root);
	}
	void push(Edge val)
	{
		push(newnode(val));
	}
	void merge(pnode a)
	{
		if (!root)
		{
			// Just make a the root
			root = a;
			return;
		}
		// Insert a's heap in between a and a->right
		pnode b = a->right;
		pnode x = root->right;

		root->right = b;
		b->left = root;

		a->right = x;
		x->left = a;
		// Update root if needed
		if (a->val.val() < root->val.val()) root = a;
	}
	void merge(ViolationHeap *a)
	{
		sz += a->sz;
		if (!root) root = a->root;
		else if (a->root) merge(a->root);
	}
	void dealWithNode(pnode a) // Does possible merges
	{
		int r = a->rank;
		if (r > mxrank) mxrank = r; // Update maximum seen rank if needed
		if (!_first[r]) _first[r] = a; // First occurrence of this rank
		else if (!_second[r]) _second[r] = a; // Second occurrence of this rank
		else
		{
			// Merge them
			a = merge(a, _first[r], _second[r]);
			_first[r] = _second[r] = NULL;
			dealWithNode(a);
		}
	}
	void pop()
	{
		sz--;
		if (!sz) // Heap is now empty
		{
			root = NULL;
			return;
		}
		mxrank = 0;
		// Insert all children of the old root into the new heap, then insert all the other heaps
		// Merging where possible
		pnode a = root->child;
		while (a)
		{
			pnode next = a->right;
			dealWithNode(a);
			a = next;
		}
		// Now deal with all other nodes in the heap
		a = root->right;
		while (a != root)
		{
			pnode next = a->right;
			dealWithNode(a);
			a = next;
		}
		root = NULL;
		// Make the new heap
		for (int i = 0; i <= mxrank; i++)
		{
			if (_first[i]) 
			{
				insertIntoHeap(_first[i], root);
				_first[i] = NULL;
			}
			if (_second[i]) 
			{
				insertIntoHeap(_second[i], root);
				_second[i] = NULL;
			}
		}
	}
	pnode parent(pnode a)
	{
		// Returns the parent of a if a is active, else returns NULL
		if (a->isRoot) return NULL;
		if (a->left->child == a) return a->left;
		if (a->left->left->child == a->left) return a->left->left;
		return NULL;
	}
};
ViolationHeap* incoming[2*MAXN]; // Stores incoming edges in a heap
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
		incoming[i] = new ViolationHeap();
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