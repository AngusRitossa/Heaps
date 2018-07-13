// Edmonds minimal branching algorithm with hollow heap, O(e log v)
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

// Hollow Heap
typedef struct Item* pitem;
typedef struct Node* pnode;
struct Item // The actual items in the heap
{
	pnode inheap;
};
struct Node // Node in the heap. Nodes can be hollow or full. Full nodes contain an item, hollow nodes do not.
{
	Edge val; // Value of the node
	int rank; // Rank, node with rank r has at least F(r+3)-1 descendants, including itself, where F(i) is the ith fibonacci number  
	pitem item;
	pnode child, sibling; // Left most child + right sibling
	pnode secondparent; // If node has two parents, this pointer is to the second
};
// Memory allocation
Item itemalloc[MAXEDGES];
Node nodealloc[MAXEDGES];
int itemallocupto, nodeallocupto;
pitem newitem()
{
	return itemalloc + itemallocupto++;
}
pnode newnode(pitem item, Edge val)
{
	pnode _new = nodealloc + nodeallocupto++;
	_new->item = item;
	_new->val = val;
	item->inheap = _new;
	return _new;
}
// Used for intermediary storage during pop
int mxrank;
pnode ofrank[100];
struct HollowHeap
{
	pnode root;
	int sz;
	HollowHeap() // Initialise 
	{
		root = NULL;
		sz = 0;
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
	ll top()
	{
		return root->val.val();
	}
	void link(pnode a, pnode b) // Makes a the first child of b
	{
		a->sibling = b->child;
		b->child = a;
	}
	void insertIntoHeap(pnode a) // Make a the first child of the root, or the root
	{
		if (a->val.val() > root->val.val())
		{
			// Link a to the root
			link(a, root);
		}
		else
		{
			// a is the new root
			link(root, a);
			root = a;
		}
	}
	void ranklinks(pnode a) // Merges until it is the only node of its rank
	{
		if (!ofrank[a->rank]) // Only one of its rank
		{
			if (a->rank > mxrank) mxrank = a->rank;
			ofrank[a->rank] = a; 
		}
		else // Do merge
		{
			pnode b = ofrank[a->rank];
			ofrank[a->rank] = NULL;
			if (a->val.val() < b->val.val())
			{
				// Make a parent of b
				link(b, a);
				a->rank++;
				ranklinks(a);
			}
			else
			{
				// Make b parent of a
				link(a, b);
				b->rank++;
				ranklinks(b);
			}
		}
	}

	// Main functions
	void push(pnode a)
	{
		sz++;
		if (sz == 1) // A is the only node
		{
			root = a;
			return;
		}
		insertIntoHeap(a);
	}
	void push(pitem item, Edge val)
	{
		push(newnode(item, val));
	}
	void push(Edge val)
	{
		push(newitem(), val);
	}
	void merge(HollowHeap *a)
	{
		sz += a->sz;
		if (!root) // Set root to a->root, since this heap is empty
		{
			root = a->root;
		}
		else if (a->root) insertIntoHeap(a->root);
	}
	void pop() // Remove the smallest item from the heap
	{
		sz--;
		if (!sz) // Heap should now be empty
		{
			root = NULL;
			return;
		}
		mxrank = 0;
		pnode l = root; // Linked list of hollow nodes to delete
		while (l)
		{
			pnode v = l;
			l = l->sibling;
			pnode a = v->child;
			while (a) // Process all children of v
			{
				pnode next = a->sibling;
				if (a->item) // Is not hollow
				{
					a->sibling = NULL;
					ranklinks(a); // Merge with nodes of the same rank, leaving only one node with its rank
				}
				else // Is hollow
				{
					if (a->secondparent) // Has two parents
					{
						if (a->secondparent == v) next = NULL; // A is the last child of v
						else a->sibling = NULL; // A now has no more siblings
						a->secondparent = NULL; // A now has one parent
					}
					else // This was its only parent, add to l so it can be removed from the heap
					{
						a->sibling = l;
						l = a;
					}
				}
				a = next;
			}
		}
		root = NULL;
		// Now, merge all nodes to form one heap
		for (int i = 0; i <= mxrank; i++)
		{
			if (ofrank[i])
			{
				if (!root) // Make the root
				{
					root = ofrank[i];
				}
				else if (ofrank[i]->val.val() < root->val.val()) // Link root to this then make this the root
				{
					link(root, ofrank[i]);
					root = ofrank[i];
				}
				else // Link to the root
				{
					link(ofrank[i], root);
				}
				ofrank[i] = NULL; // Clear the array for future usage
			}
		}
	}
	void erase(pitem item) // Remove item from the heap
	{
		pnode a = item->inheap;
		if (a == root) pop(); // Is the minimum item, do a pop
		else
		{
			a->item = NULL; // Make hollow
			item->inheap = NULL;
		}
	}
};
HollowHeap* incoming[2*MAXN]; // Stores incoming edges in a pairing heap
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
		incoming[i] = new HollowHeap();
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