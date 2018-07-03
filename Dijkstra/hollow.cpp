// Dijkstra's algorithm implemented with a hollow heap: O(e + vloge)
#include <cstdio>
#include <vector>
#include <utility>
#include <chrono>
#define MAXn 1000000 // Maximum number of items (push)
#define MAXN 15000000 // Maximum number of nodes (push + decrease-key)
using namespace std::chrono;
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
int v, e;
std::vector<std::pair<int, ll> > adj[MAXn];
HollowHeap pq;
pitem nodes[MAXn];
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
	// Start the timer
	milliseconds start = duration_cast<milliseconds>(system_clock::now().time_since_epoch());

	// Initialise the distance to each node
	nodes[0] = newitem();
	nodes[0]->node = 0;
	pq.push(nodes[0], 0);
	for (int i = 1; i < v; i++)
	{
		nodes[i] = newitem();
		nodes[i]->node = i;
		pq.push(nodes[i], 1e18);
	}

	// Run dijkstra
	while (!pq.empty())
	{
		int a = pq.root->item->node;
		ll d = pq.top();
		pq.pop();
		for (auto b : adj[a])
		{
			if (d + b.second < nodes[b.first]->inheap->val)
			{
				pq.decreasekey(nodes[b.first], d + b.second);
			}
		}
	}
	// Print distance to node n-1;
	printf("%lld\n", nodes[v-1]->inheap->val);

	// End the timer, print the time
	milliseconds end = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
	ll totaltime = end.count() - start.count();
	printf("Time % 6lldms\n", totaltime);
}