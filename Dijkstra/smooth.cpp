// Dijkstra's algorithm implemented with a smooth heap
#include <cstdio>
#include <vector>
#include <utility>
#include <chrono>
using namespace std::chrono;
#define MAXN 1000001
typedef long long ll;
typedef struct Node* pnode;
struct Node
{
	ll val;
	int node;
	pnode left, right, child, par; // Left and right siblings in tree/heap, leftmost child, parent
	// Need these pointers to support access to: Left & right siblings of a node for removal during decrease-key
	// + Need parent because leftmost child can no longer store parent because it needs to store rightmost sibling
};
// Memory allocation
Node allocarray[MAXN];
int allocupto;
pnode newnode()
{
	return allocarray + allocupto++;
} 
struct SmoothHeap
{
	int sz;
	pnode root;
	pnode mn; // Minimum node in the heap
	SmoothHeap() // Initialisation 
	{
		sz = 0;
		root = NULL;
		mn = NULL;
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
		return mn->val;
	}
	void insertIntoHeap(pnode a) // Adds a to the left of the heap
	{
		a->par = NULL;
		if (!root)
		{
			root = mn = a;
			return;
		}
		// Insert to the left of root
		a->left = NULL;
		a->right = root;
		root->left = a;
		root = a;
		if (a->val < mn->val) mn = a; // Update mn if needed
	}
	void link(pnode &a) // Stable links a to a->right
	{
		pnode b = a->right;
		if (a->val < b->val)
		{
			// Remove b from the root-list
			a->right = b->right;
			if (a->right) a->right->left = a;
			// Make b the rightmost child of a
			if (!a->child)
			{
				a->child = b->left = b->right = b;
			}
			else
			{
				b->right = a->child;
				b->left = a->child->left;
				a->child->left = b;
				b->left->right = b;
			}
			b->par = a;
		}
		else
		{
			// Remove a from the root list
			b->left = a->left;
			if (b->left) b->left->right = b;
			// Make a the leftmost child of b
			if (!b->child)
			{
				b->child = a->left = a->right = a;
			}
			else
			{
				a->right = b->child;
				a->left = b->child->left;
				b->child->left = a;
				a->left->right = a;
				b->child = a;
			}
			a->par = b;
			a = b;
		}
	}

	// Main functions
	void push(pnode a)
	{
		sz++;
		insertIntoHeap(a);
	}
	void push(ll val)
	{
		pnode a = newnode();
		a->val = val;
		sz++;
		insertIntoHeap(a);
	}
	void decreasekey(pnode a, ll val)
	{	
		a->val = val;
		if (!a->par)
		{
			assert(mn);
			if (val < mn->val) mn = a; // Update mn if needed
			return; // A is already a root, doesn't need removal
		}
		if (val > a->par->val) return; // A doesn't break heap-order, doesn't need removal
		// Remove a from its parent
		if (a->left == a) // Only child
		{
			a->par->child = NULL;
		}
		else
		{
			a->left->right = a->right;
			a->right->left = a->left;
			if (a->par->child == a) a->par->child = a->right;
		}
		// Insert into the heap
		insertIntoHeap(a);
	}
	void pop()
	{
		sz--;
		if (!sz)
		{
			root = mn = NULL;
			return;
		}
		// Remove mn
		if (mn == root)
		{
			root = mn->right;
			if (root) root->left = NULL;
		}
		else
		{
			if (mn->right) mn->right->left = mn->left;
			if (mn->left) mn->left->right = mn->right;
		}
		// Add children of mn to the heaplist
		if (mn->child)
		{
			pnode a = mn->child;
			pnode b = mn->child->left; // Rightmost child
			b->right = root;
			if (root) root->left = b;
			a->left = NULL;
			root = a;
		}
		// Do restructuring
		pnode x = root;
		while (x->right)
		{
			if (x->val < x->right->val) x = x->right; // x is not a local maximum
			else
			{
				bool dolast = 1;
				while (x->left)
				{
					if (x->left->val > x->right->val) // Link x and x->left
					{
						x = x->left;
						link(x);
					}
					else
					{
						link(x);
						dolast = 0;
						break; // Go back to while (x->right)
					}
				}
				if (dolast) link(x); // x->left does not exist, merge x and x->right
			}
		}
		// Now, list of roots is sorted
		// Just merge them all
		while (x->left)
		{
			x = x->left;
			link(x);
		}
		root = mn = x;
		root->par = NULL;
	}
};
int v, e;
std::vector<std::pair<int, ll> > adj[MAXN];
SmoothHeap pq;
pnode nodes[MAXN];
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
	nodes[0] = newnode();
	nodes[0]->node = 0;
	pq.push(nodes[0]);
	for (int i = 1; i < v; i++)
	{
		nodes[i] = newnode();
		nodes[i]->val = 1e18;
		nodes[i]->node = i;
		pq.push(nodes[i]);
	}

	// Run dijkstra
	while (!pq.empty())
	{
		int a = pq.mn->node;
		ll d = pq.top();
		pq.pop();
		for (auto b : adj[a])
		{
			if (d + b.second < nodes[b.first]->val)
			{
				pq.decreasekey(nodes[b.first], d + b.second);
			}
		}
	}
	// Print distance to node n-1;
	printf("%lld\n", nodes[v-1]->val);

	// End the timer, print the time
	milliseconds end = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
	ll totaltime = end.count() - start.count();
	printf("Time % 6lldms\n", totaltime);
}