// Stoer-Wagner's algorithm implemented with a binomial heap: O(ve log v)
#include <cstdio>
#include <vector>
#include <utility>
#include <chrono>
using namespace std::chrono;
using namespace std;
typedef long long ll;
#define MAXN 5000000
typedef struct BinomialNode* pnode;
struct BinomialNode
{
	ll val;
	int node;
	int degree;
	pnode sibling, child, par; // sibling is either in the heap linked list of the sibling within a tree. 
};
namespace binomialheapalloc
{
// Nodes are allocated from this array
BinomialNode _heap[MAXN]; 
int _heapallocupto;
pnode _newnode(ll val)
{
	pnode _new = _heap + _heapallocupto++; // Dynamic allocation is slow ... this is much faster
	//pnode _new = new BinomialNode(); // Other method of allocating memory
	_new->val = val;
	return _new;
}
}
pnode nodes[MAXN];
struct BinomialHeap
{
	pnode root = NULL;
	int sz = 0;
	pnode mn = NULL;
	// Auxiliary functions
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
	void swap(pnode &a, pnode &b) // Swaps two pnodes. Created to remove any reliance on STL
	{
		pnode c = a;
		a = b;
		b = c;
	}
	void swap(int &a, int &b) // Swaps two ints. Created to remove any reliance on STL
	{
		int c = a;
		a = b;
		b = c;
	}
	pnode mergetrees(pnode a, pnode b, pnode pre = NULL) // Merges trees with equal degree, creating one tree with degree+1
	{
		if (b->val < a->val)
		{
			// Swap them & replace a with b in the linked list
			if (pre) pre->sibling = b;
			if (root == a) root = b;
			a->sibling = b->sibling;
			swap(a, b);
		}
		// Make tree b the child of tree a
		a->sibling = b->sibling;
		a->degree++;
		b->sibling = a->child;
		a->child = b;
		b->par = a;
		if (mn == NULL || a->val <= mn->val) mn = a;
		return a;
	}

	// Main functions
	void push(pnode _new)
	{
		// Worst case O(log(n)) - average O(1)
		sz++;
		if (mn == NULL || _new->val < mn->val) mn = _new; // Update mn if needed
		_new->sibling = root; // Set this new node as the first one
		root = _new; // set as root
		while (_new->sibling && _new->degree == _new->sibling->degree) // Will merge until no longer necessary, then break
		{
			_new = mergetrees(_new, _new->sibling);
		}
	}
	void push(ll val)
	{
		pnode _new = binomialheapalloc::_newnode(val);
		push(_new);
	}
	void merge(pnode b)
	{
		pnode a = root;
		pnode pre = NULL;
		while (a && b)
		{
			if (mn == NULL || b->val < mn->val) mn = b; // update mn if needed
			if (mn == NULL || a->val < mn->val) mn = a; // update mn if needed
			b->par = NULL; // Its a root - has no parent
			a->par = NULL; // Its a root - has no parent
			if (a->degree < b->degree)
			{
				// add a to the new list
				if (pre) pre->sibling = a;
				else root = a;
				pre = a;
				a = a->sibling;
			}
			else if (b->degree < a->degree)
			{
				// add b to the new list
				if (pre) pre->sibling = b;
				else root = b;
				pre = b;
				b = b->sibling;
			}
			else
			{
				// add a to b's linked list - merge where necessary
				pnode asib = a->sibling;
				a->sibling = b;
				b = a;
				a = asib;
				while (b->sibling && b->degree == b->sibling->degree) // Will merge until no longer necessary, then break
				{
					b = mergetrees(b, b->sibling);
				}
			}
		}
		while (a) // Add to the end
		{
			if (mn == NULL || a->val < mn->val) mn = a; // update mn if needed
			a->par = NULL; // Its a root - has no parent
			if (pre) pre->sibling = a;
			else root = a;
			pre = a;
			a = a->sibling;
		}
		while (b) // Add to the end
		{
			if (mn == NULL || b->val < mn->val) mn = b; // update mn if needed
			b->par = NULL; // Its a root - has no parent
			if (pre) pre->sibling = b;
			else root = b;
			pre = b;
			b = b->sibling;
		}
		pre->sibling = NULL;
	}
	void merge(BinomialHeap& A)
	{
		sz += A.sz;
		merge(A.root);		
	}
	void pop()
	{
		sz--;
		pnode a = root;
		pnode pre = NULL;
		while (a != mn) // Find the tree with mn
		{
			pre = a;
			a = a->sibling;
		}	
		mn = NULL;
		// Remove it from the tree
		if (pre)
		{
			pre->sibling = a->sibling;
		}
		else
		{
			root = a->sibling;
		}

		// Reverse the linked list of A's children
		pre = NULL;
		a = a->child;
		while (a)
		{
			pnode next = a->sibling;
			a->sibling = pre;
			pre = a;
			a = next;
		}
		if (pre) merge(pre); // Merge the children back into the heap.
		else
		{
			pnode b = root;
			while (b)
			{
				if (mn == NULL || b->val < mn->val) mn = b;
				b = b->sibling;
			}
		}
	}
	void decreasekey(pnode &a, ll val)
	{
		while (a->par && a->par->val > val)
		{
			// Swap a and its parent
			swap(a->node, a->par->node);
			a->val = a->par->val;
			nodes[a->node] = a;
			a = a->par;
		}
		a->val = val;
		nodes[a->node] = a;
		if (mn == NULL || val < mn->val) mn = a;
	}
	void erase(pnode a) // Remove a node from the heap
	{
		// Update the value to infinity (in this case, (2^31)-1)
		decreasekey(a, -((1 << 31)-1));
		// Since a should now be the greatest element, pop
		pop();
	}
};
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
BinomialHeap pq;
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
	for (int i = 0; i < v; i++) nodes[i] = binomialheapalloc::_newnode(0), nodes[i]->node = i;
	// Run the algorithm
	for (int c = 1; c <= v-1; c++) // Run this v-1 times
	{
		// Construct "legal ordering"
		// Push all nodes onto the pq
		for (int i = 0; i < v; i++)
		{
			if (uf.findrep(i) == i) // If it hasn't been contracted
			{
				nodes[i]->degree = nodes[i]->val = 0;
				nodes[i]->par = nodes[i]->child = nodes[i]->sibling = NULL;
				pq.push(nodes[i]);
			}
		}
		int last;
		while (pq.size() > 1)
		{
			int a = pq.mn->node; // A is the next node in the legal ordering
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
		int a = pq.mn->node;
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