// Stoer-Wagner's algorithm implemented with a d-ary heap: O(ve log v)
#include <cstdio>
#include <vector>
#include <utility>
#include <queue>
#include <chrono>
using namespace std;
using namespace chrono;
#define D 4
#define MAXN 5000000
typedef long long ll;
struct daryheap
{
	ll heap[MAXN];
	int node[MAXN];
	int at[MAXN];
	int upto, sz;

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
		return heap[0];
	}
	void swap(int a, int b) // Swaps two elements in the heap
	{
		ll c = heap[a];
		heap[a] = heap[b];
		heap[b] = c;
		
		c = node[a];
		node[a] = node[b];
		node[b] = c;

		at[node[a]] = a;
		at[node[b]] = b;
	}
	void bubbleup(int a) // Swaps with parents until its in the correct position
	{
		while (a)
		{
			int p = (a-1)/D;
			if (heap[a] < heap[p]) // Should perform the swap
			{
				swap(a, p);
				a = p;
			}
			else break; // We are finished
		}
	}
	void bubbledown(int a) // Swaps with children until its in the correct position
	{
		while (true)
		{
			ll mn = heap[a];
			int mnchild = -1;
			for (int i = 1; i <= D; i++)
			{
				int c = a*D+i;
				if (c < sz && heap[c] < mn) // This is the new minimum value
				{
					mn = heap[c];
					mnchild = c;
				}
			}
			if (mnchild == -1) break; // We are done
			swap(a, mnchild);
			a = mnchild;
		}
	}

	// Main functions
	void push(ll val)
	{
		// Insert as a leaf
		heap[sz] = val;
		node[sz] = upto;
		at[upto++] = sz;
		// Bubble up
		bubbleup(sz++);
	}
	void pop()
	{
		sz--;
		// Replace root with last child, bubble down
		swap(0, sz);
		// Bubble down
		bubbledown(0);
	}
	void decreasekey(int a, ll val)
	{
		// Update value, bubble up
		heap[a] = val;
		bubbleup(a);
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
daryheap pq;
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

	// Run the algorithm
	for (int c = 1; c <= v-1; c++) // Run this v-1 times
	{
		// Construct "legal ordering"
		// Push all nodes onto the pq
		for (int i = 0; i < v; i++)
		{
			if (uf.findrep(i) == i) // If it hasn't been contracted
			{
				pq.upto = i;
				pq.push(0);
			}
		}
		int last;
		while (pq.size() > 1)
		{
			int a = pq.node[0]; // A is the next node in the legal ordering
			last = a;
			inlegal[a] = c;
			pq.pop();
			for (pedge e = adj[a]; e; e = e->adj) 
			{
				// For each outgoing edge, do a decrease-key
				int b = uf.findrep(e->v);
				if (inlegal[b] != c)
				{
					pq.decreasekey(pq.at[b], pq.heap[pq.at[b]] - e->weight);
				}
			}
		}
		ans = min(ans, -pq.top()); // Update answer if needed
		int a = pq.node[0];
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