// Generates the input
// It is guaranteed to be connected
// No self edges
#include <bits/stdc++.h>
using namespace std;
typedef long long ll;
#define MXCOST (ll)(1e12) // Maximum weight of an edge
vector<pair<pair<int, int>, ll> > edges;
int main()
{
	srand(time(NULL));
	int v, e;
	scanf("%d%d", &v, &e);
	printf("%d %d\n", v, e);
	e-=v-1;
	assert(e >= 0);
	// Guarantee a directed spanning tree rooted at 0 exists
	for (int i = 1; i < v; i++)
	{
		int a = rand()%i;
		ll c = (rand()%MXCOST) + 1;
		edges.push_back({ {a, i}, c } );
	}
	for (int i = 0; i < e; i++)
	{
		int a = rand()%v;
		int b = rand()%v;
		if (a == b)
		{ 
			i--; continue; 
		}
		ll c = (rand()%MXCOST) + 1;
		edges.push_back({ {a, b}, c } );
	}
	random_device rd;
    mt19937 g(rd());
	shuffle(edges.begin(), edges.end(), g);
	for (auto a : edges)
	{
		printf("%d %d %lld\n", a.first.first, a.first.second, a.second);
	}
}