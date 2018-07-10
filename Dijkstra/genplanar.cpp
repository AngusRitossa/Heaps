#include <bits/stdc++.h>
using namespace std;
#define MAXN 1000000
#define MULTIPLIER 100
#define MXCOORD 1000000
typedef long long ll;
// Generates n points in a plane, then a random graph. Specifially, generates k*multiplier outgoing edges from each node
// Then takes the k closest. This is to resemble real-life representations where edges generally exist between closer locations
ll x[MAXN], y[MAXN];
int v, k;
vector<pair<pair<int, int>, ll> > edges;
ll dis(int a, int b) // Euclidian distance 
{
	return sqrt((x[a]-x[b])*(x[a]-x[b]) + (y[a]-y[b])*(y[a]-y[b]));
}
int main()
{
	srand(time(NULL));
	scanf("%d%d", &v, &k);
	for (int i = 0; i < v; i++)
	{
		x[i] = rand()%MXCOORD;
		y[i] = rand()%MXCOORD;
	}
	for (int i = 0; i < v; i++)
	{
		vector<pair<ll, int> > currentedges;
		while (currentedges.size() < k*MULTIPLIER)
		{
			int a = rand()%v;
			if (a != i)
			{
				ll d = dis(a, i);
				currentedges.emplace_back(d, a);
			}
		}
		sort(currentedges.begin(), currentedges.end());
		for (int j = 0; j < k; j++)
		{
			edges.push_back( { { i, currentedges[j].second }, currentedges[j].first } );
		}
	}
	printf("%d %lu\n", v, edges.size());
	random_shuffle(edges.begin(), edges.end());
	for (auto a : edges)
	{
		printf("%d %d %lld\n", a.first.first, a.first.second, a.second);
	}
}