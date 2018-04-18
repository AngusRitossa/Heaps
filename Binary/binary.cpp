// Maximum binary heap

struct BinaryHeap
{
	int heap[1000000];
	int upto = 0;
	// Auxiliary functions
	int size()
	{
		return upto;
	}
	bool empty()
	{
		return !upto;
	}
	int top()
	{
		return heap[1];
	}
	void swap(int a, int b) // Swaps two elements
	{
		int c = heap[a];
		heap[a] = heap[b];
		heap[b] = c;
	}
	void bubbleup(int a) // Moves a value up the heap until it reaches the correct spot
	{
		while (a != 1 && heap[a] > heap[a/2])
		{
			swap(a, a/2);
			a /= 2;
		}
	}
	void bubbledown(int a) // Moves a value down the heap until it reaches the correct spot
	{
		while (2*a < upto) // Swap with the greatest of the two children
		{
			if (heap[2*a] >= heap[2*a+1] && heap[2*a] > heap[a])  
			{
				swap(a, 2*a);
				a*=2;
			}
			else if (heap[2*a+1] > heap[2*a] && heap[2*a+1] > heap[a])
			{
				swap(a, 2*a+1);
				a*=2;
				a++;
			}
			else break;
		}
		if (2*a == upto) // Only has one child, try swapping with that
		{
			if (heap[2*a] > heap[a])
			{
				swap(a, 2*a);
				a*=2;
			}
		}
	}

	// Main functions
	void push(int a)
	{
		// Set the last value to a, then bubble up to the corect position
		heap[++upto] = a;
		bubbleup(upto);
	}
	void pop()
	{
		// Move last element to the top
		heap[1] = heap[upto];
		// Decrease size
		upto--;
		// Bubble down from 1
		bubbledown(1);
	}
	void merge(BinaryHeap &a)
	{
		// Very slow ... one by one insert elements from a
		while (!a.empty())
		{
			push(a.top());
			a.pop();
		}
	}
};
