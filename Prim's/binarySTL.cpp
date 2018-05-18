// Dijkstra's algorithm implemented with a STL binary heap: O((v + e) log e)
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
priority_queue<pair<ll, int>, vector<pair<ll, int> >, greater<pair<ll, int> > > pq;
ll dis[MAXN];
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
	dis[0] = 0;
	pq.emplace(0, 0);
	for (int i = 1; i < v; i++)
	{
		dis[i] = 1e18;
		pq.emplace(dis[i], i);
	}
	ll ans = 0;

	// Run prim's algorithm
	while (!pq.empty())
	{
		int a = pq.top().second;
		done[a] = 1;
		ll d = pq.top().first;
		pq.pop();
		if (dis[a] != d) continue;
		ans += d;
		for (auto b : adj[a])
		{
			if (!done[b.first] && b.second < dis[b.first])
			{
				dis[b.first] = b.second;
				pq.emplace(dis[b.first], b.first);
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