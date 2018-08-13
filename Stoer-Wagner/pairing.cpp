// Stoer-Wagner's algorithm implemented with a pairing heap: O(ve log v)
#include <cstdio>
#include <vector>
#include <utility>
#include <chrono>
using namespace std::chrono;
using namespace std;
#define MAXN 5000000
typedef long long ll;
typedef struct PairingNode* pnode;
struct PairingNode
{
	ll val; // Value of the node
	int node;
	pnode child, left, right; // Point to leftmost child, left sibling and right sibling
	// If a node is the leftmost child, left points to the parent.
};
namespace pairingheapalloc
{
// Nodes are allocated from this array
PairingNode _heap[MAXN]; 
int _heapallocupto;
pnode newnode(ll val)
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
	ll top() // Maximum value
	{
		return root->val;
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
		if (b->val < a->val) swap(a, b); // Make the heap with the smallest root the new root
		b->right = a->child;
		if (b->right) b->right->left = b;
		a->child = b;
		b->left = a;
		return a;
	}
	void merge(PairingHeap &a) // Merges a heap into this heap
	{
		sz += a.sz;
		root = merge(root, a.root);
	}
	void push(pnode _new) // Inserts a pnode into the heap
	{
		sz++;
		if (sz == 1) // Just make this node the heap
		{
			root = _new;
			return;
		}
		if (_new->val < root->val) // Make _new the new root
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
	void push(ll val) // Inserts an element into the heap
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
	void pop() // Removes the smallest element from the heap
	{
		sz--;
		root = recursivemerge(root->child);
		if (root) root->right = NULL;
	}
	void decreasekey(pnode a, ll val)
	{
		a->val = val;
		if (a == root) return;
		if (a->left->child == a) // A is the leftmost child
		{
			a->left->child = a->right;
			if (a->right) a->right->left = a->left;
			a->left = a->right = NULL;
			root = merge(root, a);
		}
		else
		{
			a->left->right = a->right;
			if (a->right) a->right->left = a->left;
			a->left = a->right = NULL;
			root = merge(root, a);
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
pnode nodes[MAXN];
PairingHeap pq;
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
	for (int i = 0; i < v; i++) nodes[i] = pairingheapalloc::newnode(0), nodes[i]->node = i;
	// Run the algorithm
	for (int c = 1; c <= v-1; c++) // Run this v-1 times
	{
		// Construct "legal ordering"
		// Push all nodes onto the pq
		for (int i = 0; i < v; i++)
		{
			if (uf.findrep(i) == i) // If it hasn't been contracted
			{
				nodes[i]->val = 0;
				nodes[i]->child = nodes[i]->left = nodes[i]->right = NULL;
				pq.push(nodes[i]);
			}
		}
		int last;
		while (pq.size() > 1)
		{
			int a = pq.root->node; // A is the next node in the legal ordering
			last = a;
			inlegal[a] = c;
			pq.pop();
			for (pedge e = adj[a]; e; e = e->adj) 
			{
				// For each outgoing edge, do a decrease-key
				int b = uf.findrep(e->v);
				if (inlegal[b] != c)
				{
					pq.decreasekey(nodes[b], nodes[b]->val - e->weight);
				}
			}
		}
		ans = min(ans, -pq.top()); // Update answer if needed
		int a = pq.root->node;
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