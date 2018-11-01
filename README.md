# Heaps

## General Information

The following heaps are implemented in C++. All of the heaps have been used to implement Dijkstra's shortest path algorithm and the Stoer-Wagner min-cut algorithm. Mergeable heaps have been used to implement Edmonds' minimum branching algorithm in O(e log v).
- Binomial
- D-ary
- Fibonacci 
- Hollow
- Pairing 
- Quake
- Rank-Pairing 
- Smooth
- Strict Fibonacci
- Violation

## Usage

All the heaps use templates and, **with the exception of d-ary heaps**, conform to the following syntax. 
### Declaring
General:
```cpp
heaptype<valuetype> pq;
```
Example:
```cpp
binomial<int> pq;
```

### Without decrease-key
The functions push(), pop(), top(), size() and empty() are all used the same as STL priority queue, except that these are minimum heaps. 
The merge() function works by passing in a pointer to the heap that is being merged into it.  
  
Example:
```cpp
binomial<int>* pq1 = new binomial<int>();
binomial<int>* pq2 = new binomial<int>();
pq1->push(5);
pq2->push(1);
pq1->merge(pq2);
printf("%d\n", pq1->top()); // Prints 1
pq1->pop();
```
### With decrease-key
In order to use decrease-key, you need to store a pointer to the object in the heap. All of these objects use the naming convention HeapTypeNode\<valuetype\>. Within this object, the value stored in the heap is node->val. The functions pop(), top(), size(), empty() and merge() work as described above. Push() can take in a HeapTypeNode as an argument, or the above described function can be used as it returns a HeapTypeNode\*. The function decreasekey() takes two arguments, the HeapTypeNode* that is being updated and the new value.  
  
Example:
```cpp
binomial<int> pq;
BinomialNode<int>* a = pq.push(4);
BinomialNode<int>* b = new BinomialNode<int>();
b->val = 5;
pq.push(b); // In hollow heaps, b->val = 5; pq.push(b); is replaced with pq.push(b, 5);
pq.decreasekey(a, 1);
printf("%d\n", pq.top()); // Prints 1
```
