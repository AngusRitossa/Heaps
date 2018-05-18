// Dijkstra's algorithm implemented with a naive O(n) heap: O(n^2 + e)
#include <cstdio>
#include <vector>
#include <utility>
#include <chrono>
using namespace std::chrono;
#define MAXN 1000001
typedef long long ll;
struct NaiveHeap
{
	ll heap[MAXN];
	bool removed[MAXN];
	int upto = 0;
	int sz = 0;
	ll mn = 1e18;
	int mnloc = 0;
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
		return mn;
	}

	// Main functions
	void push(ll val) // Just add the value to the end
	{
		sz++;
		heap[upto++] = val;
		if (val < mn) // Update max if needed
		{
			mn = val;
			mnloc = upto-1;
		}
	}
	void pop()
	{
		sz--;
		for (int i = 0; i < upto; i++) // Find the maximum value, set it as removed
		{
			if (heap[i] == mn && !removed[i])
			{
				removed[i] = 1;
				break;
			}
		}
		mn = 1e18;
		for (int i = 0; i < upto; i++) // Look for the new maximum
		{
			if (heap[i] < mn && !removed[i]) 
			{
				mn = heap[i];
				mnloc = i;
			}
		}
	}
	void decreasekey(int a, ll val)
	{
		heap[a] = val; // Update value, update max if needed
		if (val < mn) 
		{
			mn = val;
			mnloc = a;
		}
	}
};
NaiveHeap pq;
int v, e;
std::vector<std::pair<int, ll> > adj[MAXN];
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
		int a = pq.mnloc;
		ll d = pq.top();
		pq.pop();
		for (auto b : adj[a])
		{
			if (d + b.second < pq.heap[b.first])
			{
				pq.decreasekey(b.first, d + b.second);
			}
		}
	}
	// Print distance to node n-1;
	printf("%lld\n", pq.heap[v-1]);

	// End the timer, print the time
	milliseconds end = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
	ll totaltime = end.count() - start.count();
	printf("Time % 6lldms\n", totaltime);
}