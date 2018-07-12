// Generates a test case with the goal of having many decreasekey calls when dijkstra is run
// Not a complete graph
#include <bits/stdc++.h>
using namespace std;
typedef long long ll;
int v, e;
vector<pair<pair<int, int>, ll > > edges;
ll dis[100000];
ll lowest = 1e13;
int main()
{
	srand(time(NULL));
	scanf("%d%d", &v, &e); // E is the number of outgoing edges per node
	for (int i = 1; i < v; i++)
	{
		dis[i] = 1e13;
		if (i == 1) 
		{
			dis[i] = 1e5;
			edges.push_back({ { 0, i }, dis[i]});
		}
	}
	for (int i = 2; i < 3*e && i < v; i++)
	{
		edges.push_back({ { 0, i }, dis[i]});
	}
	for (int a = 1; a < v; a++)
	{
		if (a < v-2)
		{
			for (int i = 0; i < e-1; i++)
			{
				int b = rand()%(v-2-a);
				b += a+2;
				ll d = rand()%100000;
				d++;
				lowest-=d;
				dis[b] = lowest;
				assert(dis[b] - dis[a] > 0);
				edges.push_back({{ a, b }, dis[b] - dis[a]});
			}
		}
		if (a != v-1)
		{	
			ll diff = (1e5) + rand()%((ll)(1e2));
			edges.push_back({{ a, a+1 }, diff});
			dis[a+1] = dis[a] + diff;
		}
	}
	printf("%d %lu\n", v, edges.size());
	for (auto a : edges) printf("%d %d %lld\n", a.first.first, a.first.second, a.second);
}