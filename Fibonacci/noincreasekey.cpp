#include <bits/stdc++.h>
using namespace std;
struct FibNode
{
	int left, right, child, sibling, val, degree, done;
};
namespace _fibheap
{
	FibNode heap[20000010];
	int nodeupto;
	int ofsize[100], done[100], upto;
}
struct FibHeap
{
	int a;
	int mergetrees(int a, int b)
	{
		if (_fibheap::heap[a].val < _fibheap::heap[b].val) 
		{ 
			int al = _fibheap::heap[a].left;
			int ar = _fibheap::heap[a].right;
			_fibheap::heap[_fibheap::heap[b].left].right = a;
			_fibheap::heap[_fibheap::heap[b].right].left = a;
			_fibheap::heap[al].right = b;
			_fibheap::heap[ar].left = b;
			swap(_fibheap::heap[a].left, _fibheap::heap[b].left);
			swap(_fibheap::heap[a].right, _fibheap::heap[b].right);
			swap(a, b);
		}
		_fibheap::heap[b].sibling = _fibheap::heap[a].child;
		_fibheap::heap[a].child = b;
		_fibheap::heap[a].degree++;
		_fibheap::heap[_fibheap::heap[b].right].left = _fibheap::heap[b].left;
		_fibheap::heap[_fibheap::heap[b].left].right = _fibheap::heap[b].right;
		return a;
	}
	int fixup(int a)
	{
		_fibheap::upto++;
		int b = a;
		while (_fibheap::heap[b].done != _fibheap::upto)
		{
			_fibheap::heap[b].done = _fibheap::upto;
			while (_fibheap::done[_fibheap::heap[b].degree] == _fibheap::upto)
			{
				_fibheap::done[_fibheap::heap[b].degree] = 0;
				int c = _fibheap::ofsize[_fibheap::heap[b].degree];
				b = mergetrees(b, c);
			}
			_fibheap::done[_fibheap::heap[b].degree] = _fibheap::upto;
			_fibheap::ofsize[_fibheap::heap[b].degree] = b;
			if (_fibheap::heap[b].val >= _fibheap::heap[a].val) a = b;
			b = _fibheap::heap[b].right;
		}
		return a;
	}
	void push2(int a, int b)
	{
		_fibheap::heap[b].left = a;
		_fibheap::heap[b].right = _fibheap::heap[a].right;
		_fibheap::heap[_fibheap::heap[b].right].left = _fibheap::heap[a].right = b;
	}
	void push(int val)
	{
		if (!a)
		{
			++_fibheap::nodeupto;
			_fibheap::heap[_fibheap::nodeupto].val = val;
			_fibheap::heap[_fibheap::nodeupto].left = _fibheap::heap[_fibheap::nodeupto].right = _fibheap::nodeupto;
			a = _fibheap::nodeupto;
			return;
		}
		++_fibheap::nodeupto;
		_fibheap::heap[_fibheap::nodeupto].val = val;
		push2(a, _fibheap::nodeupto);
		if (_fibheap::heap[_fibheap::nodeupto].val > _fibheap::heap[a].val) a = _fibheap::nodeupto;
	}
	int top()
	{
		return _fibheap::heap[a].val;
	}
	void pop()
	{
		int b = _fibheap::heap[a].child;
		while (b)
		{
			push2(a, b);
			b = _fibheap::heap[b].sibling;
		}
		int newa = _fibheap::heap[a].right;
		_fibheap::heap[newa].left = _fibheap::heap[a].left;
		_fibheap::heap[_fibheap::heap[newa].left].right = newa;
		a = fixup(newa);
	}
	void merge(FibHeap &B)
	{	
		int b = B.a;
		int c = _fibheap::heap[b].right;
		_fibheap::heap[b].right = _fibheap::heap[a].right;
		_fibheap::heap[a].right = c;
		_fibheap::heap[c].left = a;
		_fibheap::heap[_fibheap::heap[b].right].left = b;
		a = fixup(a);
	}
};
/*Fib_fibheap::heap fib;
priority_queue<int> pq;
int main()
{
	srand(time(NULL));
	for (int i = 0; i < 20000000; i++)
	{
		int a = rand()%1000000;
	//	pq.push(a);
		fib.push(a);
//		printf("%d - %d %d\n", a, pq.top(), top(root));
	//	assert(fib.top() == pq.top());
		if (rand()%3 && i > 10)
		{
	//		pq.pop();
			fib.pop();
//			printf("POPPING\n");
		}
	}
	
}*/