// Shortest path faster algorithm (SPFA). Modification of Bellman-Ford, and is still worst case O(VE)
// Is said to perform well on randomly generated graphs, based on empirical evidence alone.
#include <cstdio>
#include <vector>
#include <utility>
#include <queue>
#include <chrono>
using namespace std;
using namespace chrono;
#define MAXN 1000001
typedef long long ll;
int v, e;
vector<pair<int, ll> > adj[MAXN];
deque<int> pq;
bool onq[MAXN]; // Stores whether a node is on the queue at the moment or not
ll dis[MAXN];
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
	dis[0] = 0;
	pq.push_front(0);
	for (int i = 1; i < v; i++)
	{
		dis[i] = 1e18;
	}

	// Run SPFA
	while (!pq.empty())
	{
		int a = pq.front();
		onq[a] = false;
		ll d = dis[a];
		pq.pop_front();
		for (auto b : adj[a])
		{
			if (d + b.second < dis[b.first])
			{
				dis[b.first] = d + b.second;
				if (!onq[b.first]) 
				{
					if (!pq.empty() && dis[b.first] < dis[pq.front()]) pq.push_front(b.first);
					else pq.push_back(b.first);
					onq[b.first] = true;
				}
			}
		}
	}
	// Print distance to node n-1;
	printf("%lld\n", dis[v-1]);

	// End the timer, print the time
	milliseconds end = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
	ll totaltime = end.count() - start.count();
	printf("Time % 6lldms\n", totaltime);
}