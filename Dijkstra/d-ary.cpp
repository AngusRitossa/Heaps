// Dijkstra's algorithm implemented with a d-ary heap: O((v + e) log v)
#include <cstdio>
#include <vector>
#include <utility>
#include <queue>
#include <chrono>
using namespace std;
using namespace chrono;
#define D 8
#define MAXN 1000001
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
int v, e;
vector<pair<int, ll> > adj[MAXN];
daryheap pq;
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
	pq.push(0);
	for (int i = 1; i < v; i++)
	{
		pq.push(1e18);
	}

	// Run dijkstra
	while (!pq.empty())
	{
		int a = pq.node[0];
		ll d = pq.top();
		pq.pop();
		for (auto b : adj[a])
		{
			if (d + b.second < pq.heap[pq.at[b.first]])
			{
				pq.decreasekey(pq.at[b.first], d + b.second);
			}
		}
	}
	// Print distance to node n-1;
	printf("%lld\n", pq.heap[pq.at[v-1]]);

	// End the timer, print the time
	milliseconds end = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
	ll totaltime = end.count() - start.count();
	printf("Time % 6lldms\n", totaltime);
}