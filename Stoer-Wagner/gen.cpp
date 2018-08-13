// Generates the input
// It is guaranteed to be connected
// No self edges
#include <bits/stdc++.h>
using namespace std;
typedef long long ll;
#define MXCOST (ll)(1e5) // Maximum weight of an edge
vector<pair<pair<int, int>, ll> > edges;
struct UF // Union find data structure
{
	int rep[10000000];
	UF()
	{
		for (int i = 0; i < 10000000; i++) rep[i] = i;
	}
	int findrep(int a)
	{
		if (rep[a] == a) return a;
		return rep[a] = findrep(rep[a]);
	}
	void merge(int a, int b)
	{
		rep[findrep(a)] = findrep(b);
	}
	bool connected(int a, int b)
	{
		return findrep(a) == findrep(b);
	}
};
UF uf;
int main()
{
	srand(time(NULL));
	int v, e;
	scanf("%d%d", &v, &e);
	printf("%d %d\n", v, e);
	e-=v-1;
	assert(e >= 0);
	for (int i = 1; i < v; i++)
	{
		int a = rand()%v;
		int b = rand()%v;
		ll c = (rand()%MXCOST) + 1;
		if (uf.connected(a, b)) 
		{
			i--;
			continue;
		}
		edges.push_back({ {a, b}, c } );
		uf.merge(a, b);
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