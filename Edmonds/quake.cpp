// Edmonds minimal branching algorithm with quake heap, O(e log v)
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
#define MAXEDGES 6000000
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

// Quake Heap
#define MXRANK 60
#define A1 4 // Required ratio of amounts of each rank = A1/A2
#define A2 5
typedef struct QuakeValue* pvalue;
typedef struct QuakeNode* pnode;
struct QuakeValue
{
	pnode inheap; // Highest occurrence of it in the heap
	Edge val; // Value
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
QuakeValue allocarray[MAXEDGES];
pvalue newvaluenode(Edge val)
{
	pvalue _new = allocarray + allocupto++;
	_new->val = val;
	return _new;
}
// Used for allocating memory for quakenodes. Faster than dynamic allocation
// Since O(nlogn) nodes will be used over the life, but only O(n) at once, nodes will be reused. This is faster the reallocating
int allocupto2;
QuakeNode allocarray2[5*MAXEDGES];
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
	int mxrank; // Maximum rank in the heap
	int am[MXRANK]; // Stores the amount of nodes of each rank
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
		return root->val->val.val();
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
		if (a->val->val.val() < root->val->val.val()) root = a;
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
		if (a->val->val.val() < b->val->val.val()) c->val = a->val;
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
	void push(Edge val)
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
		if (a->val->val.val() < root->val->val.val()) root = a; // Update root if needed
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
};
QuakeHeap* incoming[2*MAXN]; // Stores incoming edges in a heap
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
		incoming[i] = new QuakeHeap();
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
			e = incoming[a]->root->val->val;
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