// Maximum heap
// O(1) push, pop, increase key. O(n) pop.
struct NaiveHeap
{
	int heap[1000000];
	bool removed[1000000];
	int upto = 0;
	int mx = -1e9;
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
		return mx;
	}

	// Main functions
	void push(int val) // Just add the value to the end
	{
		heap[upto++] = val;
		if (val > mx) // Update max if needed
		{
			mx = val;
		}
	}
	void pop()
	{
		for (int i = 0; i < upto; i++) // Find the maximum value, set it as removed
		{
			if (heap[i] == val && !removed[i])
			{
				removed[i] = 1;
				break;
			}
		}
		mx = -1e9;
		for (int i = 0; i < upto; i++) // Look for the new maximum
		{
			if (heap[i] > mx && !removed[i]) mx = heap[i];
		}
	}
	void increasekey(int a, int val)
	{
		heap[a] = val; // Update value, update max if needed
		if (val > mx) mx = val;
	}
};