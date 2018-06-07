// Dijkstra's algorithm implemented with a pairing heap: O((e + v)log v)
#include <cstdio>
#include <vector>
#include <utility>
#include <chrono>
using namespace std::chrono;
#define MAXN 1000001
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
int v, e;
std::vector<std::pair<int, ll> > adj[MAXN];
PairingHeap pq;
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
	nodes[0] = pairingheapalloc::newnode(0);
	nodes[0]->node = 0;
	pq.push(nodes[0]);
	for (int i = 1; i < v; i++)
	{
		nodes[i] = pairingheapalloc::newnode(1e18);
		nodes[i]->node = i;
		pq.push(nodes[i]);
	}

	// Run dijkstra
	while (!pq.empty())
	{
		int a = pq.root->node;
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