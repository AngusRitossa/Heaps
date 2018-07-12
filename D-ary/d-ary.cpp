// Minimum D-ary heap, generalisation of binary heap
// O(log) push, pop, decrease key
#define D 16
#define MAXN 10000000
struct daryheap
{
	int heap[MAXN];
	int inheap[MAXN];
	int at[MAXN];
	int upto, sz;

	// Auxilary functions
	int size()
	{
		return sz;
	}
	bool empty()
	{
		return !sz;
	}
	int top()
	{
		return heap[0];
	}
	void swap(int a, int b) // Swaps two elements in the heap
	{
		int c = heap[a];
		heap[a] = heap[b];
		heap[b] = c;
	}
	void bubbleup(int a) // Swaps with parents until its in the correct position
	{
		while (a)
		{
			int p = (a-1)/D;
			if (heap[a] < heap[p]) // Should perform the swap
			{
				swap(a, p);
				a = p;
			}
			else break; // We are finished
		}
	}
	void bubbledown(int a) // Swaps with children until its in the correct position
	{
		while (true)
		{
			int mn = heap[a];
			int mnchild = -1;
			for (int i = 1; i <= D; i++)
			{
				int c = a*D+i;
				if (c < sz && heap[c] < mn) // This is the new greatest value
				{
					mn = heap[c];
					mnchild = c;
				}
			}
			if (mnchild == -1) break; // We are done
			swap(a, mnchild);
			a = mnchild;
		}
	}

	// Main functions
	void push(int val)
	{
		// Insert as a leaf
		heap[sz] = val;
		at[upto++] = sz;
		// Bubble up
		bubbleup(sz++);
	}
	void pop()
	{
		sz--;
		// Replace root with last child, bubble down
		swap(0, sz);
		// Bubble down
		bubbledown(0);
	}
	void decreasekey(int a, int val)
	{
		// Update value, bubble up
		heap[a] = val;
		bubbleup(a);
	}
};