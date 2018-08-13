// Stoer-Wagner's algorithm implemented with a rank-pairing heap: O(ve + v^2 log v)
#include <cstdio>
#include <vector>
#include <utility>
#include <chrono>
using namespace std::chrono;
using namespace std;
#define MAXN 5000001
typedef long long ll;
typedef struct Node* pnode;
struct Node
{
	ll val; // Value of the node
	int rank; // Rank of the node
	int node;
	pnode par, left, right; // Structure is maintained as a binary heap maintaining partial heap order. In particular, the left children satifsy it.
	bool isRoot; // Stores if the node is a root
	// Note: Roots are stored in a doubly linked circular linked list. 
	// In this case, left and right refer to the left and right siblings in this list
	// The root will have one left child, which will be stored in the par pointer. 
};
pnode nodesOfRank[100]; // Used for intermediate storage during pop functions

namespace rankpairingalloc // For allocating memory
{
	Node heapalloc[MAXN];
	int upto;
	pnode _newnode(ll val)
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
	ll top()
	{
		return root->val;
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
		if (a->val < root->val) root = a;
	}
	pnode mergetrees(pnode x, pnode y) // Merges trees with equal rank into one tree
	{
		if (x->val > y->val) swap(x, y); // Guarantee that x->val <= y->val
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
	void push(ll val) // Add the value a into the heap
	{
		pnode a = rankpairingalloc::_newnode(val);
		sz++;
		addIntoHeap(root, a);
	}
	void merge(RPHeap &a)
	{
		if (!a.root) return;
		sz+=a.sz;
		// Add the circular linked list a next to the root
		pnode b = root->right;
		pnode c = a.root;
		pnode d = c->right;

		root->right = d;
		d->left = root;

		b->left = c;
		c->right = b;

		// Update root if needed
		if (c->val < root->val) root = c;
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
	void decreasekey(pnode a, ll val) // Decrease the value at a to val. Uses type-2 rank reduction 
	{
		a->val = val;
		if (a->isRoot) 
		{
			if (a->val < root->val) root = a;
			return;
		}
		// Detach a from its parent, and set its right child in its place
		pnode p = a->par;
		pnode c = a->right;
		// Replace a with c as p's child
		if (p->left == a) p->left = c;
		else if (p->right == a) p->right = c;
		else p->par = c;
		if (c) c->par = p;
		// Insert a into the heap
		a->par = a->left;
		a->isRoot = 1;
		addIntoHeap(root, a);
		// Do the rank-decrease step
		while (true)
		{
			if (p->isRoot) // Set to rank(child)+1
			{
				p->rank = rank(p->par)+1;
				break;
			}
			int lrank = rank(p->left);
			int rrank = rank(p->right);
			if (lrank < rrank) // Swap the values
			{
				int s = lrank;
				lrank = rrank;
				rrank = s;
			}
			int k = lrank-rrank > 1 ? lrank : lrank+1;

			// If k < rank of p, update and go to p's parent
			// Else we are done
			if (k < p->rank) p->rank = k;
			else break;
			p = p->par;
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
RPHeap pq;
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
	for (int i = 0; i < v; i++) nodes[i] = rankpairingalloc::_newnode(0), nodes[i]->node = i;
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
				nodes[i]->par = nodes[i]->left = nodes[i]->right = NULL;
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