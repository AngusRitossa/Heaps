// Dijkstra's algorithm implemented with a STL binary heap: O((v + e) log e)
#include <cstdio>
#include <vector>
#include <utility>
#include <queue>
using namespace std;
#define MAXN 1000001
typedef long long ll;
int v, e;
vector<pair<int, ll> > adj[MAXN];
priority_queue<pair<ll, int>, vector<pair<ll, int> >, greater<pair<ll, int> > > pq;
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

	// Initialise the distance to each node
	dis[0] = 0;
	pq.emplace(0, 0);
	for (int i = 1; i < v; i++)
	{
		dis[i] = 1e18;
		pq.emplace(dis[i], i);
	}

	// Run dijkstra
	while (!pq.empty())
	{
		int a = pq.top().second;
		ll d = pq.top().first;
		pq.pop();
		if (dis[a] != d) continue;
		for (auto b : adj[a])
		{
			if (d + b.second < dis[b.first])
			{
				dis[b.first] = d + b.second;
				pq.emplace(dis[b.first], b.first);
			}
		}
	}
	// Print distance to node n-1;
	printf("%lld\n", dis[v-1]);
}