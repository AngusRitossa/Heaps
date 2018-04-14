#include <bits/stdc++.h>
#include "Binomial/binomial.cpp"
using namespace std;
typedef struct BinomialNode* pnode;
vector<pnode> v;
BinomialHeap fib;
int main()
{
	while (true)
	{
		pnode _new = binomialheapalloc::_newnode(rand()%10000);
		v.push_back(_new);
		fib.push(_new);

		int mx = 0;
		for (auto a : v)
		{
			mx = max(mx, a->val);
		}
		if (mx != fib.top())
		{
			printf("%d - %d\n", mx, fib.top());
		}
		assert(mx == fib.top());
		if (fib.size() > 10 && rand()%2)
		{
			for (int i = 0; i < v.size(); i++)
			{
				if (v[i] == fib.mx)
				{
					v.erase(v.begin()+i);
					break;
				}
			}
			fib.pop();
			mx = 0;
			for (auto a : v)
			{
				mx = max(mx, a->val);
			}
		}
		if (mx != fib.top())
		{
			printf("a %d - %d\n", mx, fib.top());
		}
		assert(mx == fib.top());
		if (fib.size() > 10 && rand()%2 && false)
		{
			int i = rand()%v.size();
			v[i]->val += rand()%1000;
			fib.increasekey(v[i], v[i]->val);
		}
	}
}
