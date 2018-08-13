// Stoer-Wagner's algorithm implemented with a hollow heap: O(ve + v^2 log v)
#include <cstdio>
#include <vector>
#include <utility>
#include <chrono>
#define MAXn 5000000 // Maximum number of items (push)
#define MAXN 15000000 // Maximum number of nodes (push + decrease-key)
using namespace std::chrono;
using namespace std;
typedef struct Item* pitem;
typedef struct Node* pnode;
typedef long long ll;
struct Item // The actual items in the heap
{
	pnode inheap;
	int node; // Which node in dijkstra's
};
struct Node // Node in the heap. Nodes can be hollow or full. Full nodes contain an item, hollow nodes do not.
{
	ll val; // Value of the node
	int rank; // Rank, node with rank r has at least F(r+3)-1 descendants, including itself, where F(i) is the ith fibonacci number  
	pitem item;
	pnode child, sibling; // Left most child + right sibling
	pnode secondparent; // If node has two parents, this pointer is to the second
};
// Memory allocation
Item itemalloc[MAXn];
Node nodealloc[MAXN];
int itemallocupto, nodeallocupto;
pitem newitem()
{
	return itemalloc + itemallocupto++;
}
pnode newnode(pitem item, ll val)
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
		return root->val;
	}
	void link(pnode a, pnode b) // Makes a the first child of b
	{
		a->sibling = b->child;
		b->child = a;
	}
	void insertIntoHeap(pnode a) // Make a the first child of the root, or the root
	{
		if (a->val > root->val)
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
			if (a->val < b->val)
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
	void push(pitem item, ll val)
	{
		push(newnode(item, val));
	}
	void push(ll val)
	{
		push(newitem(), val);
	}
	void decreasekey(pitem item, ll val)
	{
		pnode a = item->inheap;
		if (a == root)
		{
			// Update value, then done
			a->val = val;
			return;
		}
		pnode b = newnode(item, val);
		// Rank of b is max(0, a->rank - 2)
		if (a->rank > 2) b->rank = a->rank - 2;
		// Make a hollow, and make b the second parent 	
		a->item = NULL;
		b->child = a;
		a->secondparent = b;
		insertIntoHeap(b); // Make b a child of the root, or the parent of the root
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
				else if (ofrank[i]->val < root->val) // Link root to this then make this the root
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
pitem nodes[MAXN];
HollowHeap pq;
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
	for (int i = 0; i < v; i++) nodes[i] = newitem(), nodes[i]->node = i;
	// Run the algorithm
	for (int c = 1; c <= v-1; c++) // Run this v-1 times
	{
		// Construct "legal ordering"
		// Push all nodes onto the pq
		for (int i = 0; i < v; i++)
		{
			if (uf.findrep(i) == i) // If it hasn't been contracted
			{
				pq.push(nodes[i], 0);
			}
		}
		int last;
		while (pq.size() > 1)
		{
			int a = pq.root->item->node; // A is the next node in the legal ordering
			last = a;
			inlegal[a] = c;
			pq.pop();
			for (pedge e = adj[a]; e; e = e->adj) 
			{
				// For each outgoing edge, do a decrease-key
				int b = uf.findrep(e->v);
				if (inlegal[b] != c)
				{
					pq.decreasekey(nodes[b], nodes[b]->inheap->val - e->weight);
				}
			}
		}
		ans = min(ans, -pq.top()); // Update answer if needed
		int a = pq.root->item->node;
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