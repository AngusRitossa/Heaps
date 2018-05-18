// Dijkstra's algorithm implemented with a binary heap: O((v + e) log v)
#include <cstdio>
#include <vector>
#include <utility>
#include <queue>
#include <chrono>
using namespace std;
using namespace chrono;
#define MAXN 1000001
typedef long long ll;

struct BinaryHeap
{
	ll heap[MAXN];
	int node[MAXN];
	int at[MAXN];
	int upto = 0;
	// Auxiliary functions
	int size()
	{
		return upto;
	}
	bool empty()
	{
		return !upto;
	}
	ll top()
	{
		return heap[1];
	}
	void swap(int a, int b) // Swaps two elements
	{
		ll c = heap[a];
		int n = node[a];
		heap[a] = heap[b];
		heap[b] = c;
		node[a] = node[b];
		node[b] = n;
		at[node[a]] = a;
		at[node[b]] = b;
	}
	void bubbleup(int a) // Moves a value up the heap until it reaches the correct spot
	{
		int am = 0;
		while (a != 1 && heap[a] < heap[a/2])
		{
			swap(a, a/2);
			a /= 2;
		}
	}
	void bubbledown(int a) // Moves a value down the heap until it reaches the correct spot
	{
		while (2*a < upto) // Swap with the greatest of the two children
		{
			if (heap[2*a] <= heap[2*a+1] && heap[2*a] < heap[a])  
			{
				swap(a, 2*a);
				a*=2;
			}
			else if (heap[2*a+1] < heap[2*a] && heap[2*a+1] < heap[a])
			{
				swap(a, 2*a+1);
				a*=2;
				a++;
			}
			else break;
		}
		if (2*a == upto) // Only has one child, try swapping with that
		{
			if (heap[2*a] < heap[a])
			{
				swap(a, 2*a);
				a*=2;
			}
		}
	}

	// Main functions
	void push(ll a, int n)
	{
		// Set the last value to a, then bubble up to the corect position
		heap[++upto] = a;
		node[upto] = n;
		at[n] = upto;
		bubbleup(upto);
	}
	void pop()
	{
		// Move last element to the top
		swap(1, upto);
		// Decrease size
		upto--;
		// Bubble down from 1
		bubbledown(1);
	}
	void decreasekey(int a, ll val)
	{
		// Update the value
		heap[a] = val;
		// Move up to new position
		bubbleup(a);
	}
};

int v, e;
vector<pair<int, ll> > adj[MAXN];
BinaryHeap pq;
bool done[MAXN];
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
	pq.push(0, 0);
	for (int i = 1; i < v; i++)
	{
		pq.push(1e18, i);
	}
	ll ans = 0;

	// Run prim's algorithm
	while (!pq.empty())
	{
		int a = pq.node[1];

		ll d = pq.top();
		ans += d;
		pq.pop();
		for (auto b : adj[a])
		{
			if (!done[b.first] && b.second < pq.heap[pq.at[b.first]])
			{
				pq.decreasekey(pq.at[b.first], b.second);
			}
		}
	}
	// Print sum of edge weights
	printf("%lld\n", ans);

	// End the timer, print the time
	milliseconds end = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
	ll totaltime = end.count() - start.count();
	printf("Time % 6lldms\n", totaltime);
}