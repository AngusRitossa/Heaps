// Generates a test case with the goal of having many decreasekey calls when dijkstra is run
// Generates a complete graph
#include <bits/stdc++.h>
using namespace std;
typedef long long ll;
int n, r;
vector<pair<pair<int, int>, ll > > edges;
ll dis[100000];
int main()
{
	srand(time(NULL));
	scanf("%d", &n);
	scanf("%d", &r);
	for (int i = 1; i < n; i++)
	{
		dis[i] = 1e12-i;
		if (i == 1) dis[i] = 1e5;
		edges.push_back({ { 0, i }, dis[i]});
	}
	for (int a = 1; a < n; a++)
	{
		for (int i = a+2; i < n; i++)
		{
			ll diff = dis[i]-dis[a]-n;
			assert(dis[a] + diff < dis[i]);
			edges.push_back({{ a, i }, diff});
			dis[i] = dis[a] + diff;
		}
		if (a != n-1)
		{	
			ll diff = (1e5) + rand()%((ll)(1e2));
			edges.push_back({{ a, a+1 }, diff});
			dis[a+1] = dis[a] + diff;
		}
	}
	if (r) random_shuffle(edges.begin(), edges.end());
	printf("%d %lu\n", n, edges.size());
	for (auto a : edges) printf("%d %d %lld\n", a.first.first, a.first.second, a.second);
}