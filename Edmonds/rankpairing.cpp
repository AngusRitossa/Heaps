// Edmonds minimal branching algorithm with rank-pairing heap, O(e log v)
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
typedef struct Node* pnode;
struct Node
{
	Edge val; 
	int rank; // Value of the node
	pnode par, left, right; // Structure is maintained as a binary heap maintaining partial heap order. In particular, the left children satifsy it.
	bool isRoot; // Stores if the node is a root
	// Note: Roots are stored in a doubly linked circular linked list. 
	// In this case, left and right refer to the left and right siblings in this list
	// The root will have one left child, which will be stored in the par pointer. 
};
pnode nodesOfRank[100]; // Used for intermediate storage during pop functions

namespace rankpairingalloc // For allocating memory
{
	Node heapalloc[MAXEDGES];
	int upto;
	pnode newnode(Edge val)
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
		return root->val.val();
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
		if (a->val.val() < root->val.val()) root = a;
	}
	pnode mergetrees(pnode x, pnode y) // Merges trees with equal rank into one tree
	{
		if (x->val.val() > y->val.val()) swap(x, y); // Guarantee that x->val <= y->val
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
	void push(Edge val) // Add the value a into the heap
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
		if (c->val.val() < root->val.val()) root = c;
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
};
RPHeap* incoming[2*MAXN]; // Stores incoming edges in a pairing heap
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
		incoming[i] = new RPHeap();
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
	// Print time taken to push
	milliseconds end = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
	ll totaltime = end.count() - start.count();
	printf("Time taken to push % 6lldms\n", totaltime);
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
	end = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
	totaltime = end.count() - start.count();
	printf("Time % 6lldms\n", totaltime);
}