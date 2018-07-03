// Strict fibonacci heap (minimum)
// Just added merge
// Worst case complexities: O(1) push, top, decrease-key, merge, O(log(n)) pop & erase
typedef struct HeapNode* pnode;
typedef struct ActiveRecord* pactivenode;
typedef struct RanklistNode* pranklist;
typedef struct FixlistNode* pfixlist;
typedef struct Value* pvalue;
struct Value
{
	std::pair<int, int> val; // Value, index of whatever node points here
	pnode inheap; // Pointer to the location in the heap which has this value
};
struct HeapNode // Represents a node in the heap
{
	pvalue val; // Value of this node
	pnode left, right, parent, child; // Left and right siblings, parent and leftmost child
	pactivenode active; // Stores whether the node is active or passive
	pnode qleft, qright; // Left and right nodes in the queue (front of the queue is leftmost node, back is rightmost)
	pranklist rank; // If active, points to the node in the ranklist corresponding to the rank of this node.
	pfixlist fix; // If in the fix list, contains a pointer to the location in the fix list
	int loss; // Total loss of the node
};
struct ActiveRecord // Stores whether or not a node is active
{
	bool isactive; // True if all nodes pointing here are active, false otherwise
	int count; // Number of nodes pointing here
};
pactivenode passive; // Nodes can point here to be assigned as passive
struct RanklistNode
{
	pranklist left, right; // Left and right nodes in the ranklist (rank+1, rank-1)
	pfixlist loss; // Points to a node in the fixlist with the rank assigned to this node that has positive loss (if it exists)
	pfixlist active; // Points to an active root in the fixlist with this rank if it exists
	int count; // Number of nodes pointing here
	int rootcount; // Number of nodes in the active root fix list 
	int losscount; // Number of nodes in the loss fix list 
};
struct FixlistNode
{
	pnode node; // Node that this corresponds to
	pfixlist left, right; // Left and right nodes in the fixlist
	pranklist rank; // Points to the node in the ranklist with this rank
};
int counter; // Used to assign indexes to nodes
struct StrictFibonacciHeap // The actual heap
{
	pnode root;
	int sz;
	pactivenode activenode; // Nodes in the heap point here if they are active
	pnode nonlinkablechild; // Leftmost passive, non linkable child. Otherwise its the rightmost active child. Else null.
	pnode qfront; // Front of the queue
	pranklist ranklist; // Rightmost (lowest) node in the ranklist
	// Fix list pointers
	pfixlist one, two, three, four; // Point to leftmost nodes in the four sections of the fix list
	// All sections are their own separate, linear linked list
	// Part 1: Active roots of a rank with multiple active roots
	// Part 2: All other active roots
	// Part 3: Active nodes with loss of 1, where there is only one node of that rank with positive loss
	// Part 4: Active nodes with loss > 1 or loss 1 where there are multiple nodes of the same rank

	// Auxilary functions
	StrictFibonacciHeap() // Assign the active node pointer, initialise variables to 0/NULL
	{
		activenode = new ActiveRecord();
		activenode->isactive = 1;
		nonlinkablechild = qfront = root = NULL;
		ranklist = NULL;
		one = two = three = four = NULL;
		sz = 0;
	}
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
		return root->val->val.first;
	}
	void swap(pnode &a, pnode &b)
	{
		pnode c = a;
		a = b;
		b = c;
	}
	bool isPassiveLinkable(pnode x) // Returns if the node is passive linkable i.e. if it is passive and has only passive children
	{
		if (!x) return 0;
		if (x->active->isactive) return 0; // Node is active
		// Leftmost child is active if it exists - so just check if leftmost child is active
		if (x->child == NULL) return 1; // Childless
		if (x->child->active->isactive) return 0;
		else return 1;
	}
	bool isActiveRoot(pnode x) // Returns if this node is an active root i.e. is active and has a passive parent
	{
		if (!x->active->isactive) return 0; // Is passive
		if (x->parent->active->isactive) return 0; 
		else return 1;
	}
	void addToFixList(pfixlist a, pfixlist &fixlist) // Inserts a to the front of the fixlist 
	{
		a->left = NULL;
		a->right = fixlist;
		if (fixlist) fixlist->left = a;
		fixlist = a;
	}

	// Transformations
	void link(pnode x, pnode y) // Makes y a child of x
	{
		assert(y->val->val > x->val->val);
		if (y->parent) // If y is not the old root
		{
			if (y == nonlinkablechild) // Update the non linkable child if needed.
			{
				if (y->right == y) nonlinkablechild = NULL;
				else if (y != y->parent->child) nonlinkablechild = y->left;
				else nonlinkablechild = y->right;
				if (isPassiveLinkable(nonlinkablechild)) nonlinkablechild = NULL;
			}
			if (y->left == y) // y is the only child of its parent
			{
				y->parent->child = NULL;
			}
			else
			{
				// Remove y from its sibling linked list
				pnode a = y->right;
				pnode b = y->left;
				a->left = b;
				b->right = a;
				if (y->parent->child == y) // y is the leftmost child of its parent
				{
					y->parent->child = a;
				}
			}
			if (y->parent == nonlinkablechild && isPassiveLinkable(y->parent))
			{
				// Move across one
				if (!isPassiveLinkable(y->parent->left)) nonlinkablechild = y->parent->left;
				else nonlinkablechild = NULL;
			}
		}
		y->parent = x;
		// Link y to x
		if (x == root) // SPECIAL CASE: Need to consider whether passive linkable or not
		{
			if (x->child == NULL) // x is currently a leaf, make y the only child
			{
				x->child = y;
				y->left = y->right = y;
				if (x->active->isactive || !isPassiveLinkable(y)) nonlinkablechild = y; // Nonlinkablechild can't be a passive linkable node
			}
			else if (y->active->isactive || isPassiveLinkable(y)) // Make leftmost or rightmost child
			{
				pnode a = x->child;
				pnode b = a->left;
				b->right = y;
				y->left = b;
				a->left = y;
				y->right = a;
				if (y->active->isactive) // If active make leftmost, else will be rightmost
				{
					x->child = y;
				}
				if (nonlinkablechild == NULL && y->active->isactive) nonlinkablechild = y;
			}
			else // Is passive but not linkable, place to the right of non-linkable
			{
				if (!nonlinkablechild)
				{
					// make rightmost child
					assert(!x->child->active->isactive);
					pnode a = x->child;
					pnode b = a->left;
					a->left = y;
					y->right = a;
					b->right = y;
					y->left = b;
					x->child = y;
					nonlinkablechild = y;
				}
				else
				{
					pnode a = nonlinkablechild;
					pnode b = nonlinkablechild->right;
					a->right = y;
					y->left = a;
					b->left = y;
					y->right = b;
				}
			}
			return;
		}
		if (x->child == NULL) // x is currently a leaf, make y the only child
		{
			x->child = y;
			y->left = y->right = y;
		}
		else
		{
			// Add y as a child of x
			pnode a = x->child;
			pnode b = a->left;
			a->left = y;
			b->right = y;
			y->left = b;
			y->right = a;
			if (y->active->isactive) // y is active - make leftmost child, otherwise it will be the rightmost
			{
				x->child = y;
			}
		}
	}
	void moveSection(pfixlist x, pfixlist &one, pfixlist &two)
	{
		// Move x from section 2 to 1
		if (x->left) x->left->right = x->right;
		if (x->right) x->right->left = x->left;
		if (x == two) two = x->right;
		addToFixList(x, one);
	}
	void insertIntoFixList(pfixlist a, pfixlist &active, int &count, pfixlist &one, pfixlist &two)
	{
		// Active is the pointer to the node of the same rank as x which is already in the fix list.
		// Count is the number of nodes of that rank in the fix list
		// One and two are pointers to the first and second fix lists
		// Note if this is a insertion due to loss, active should be the loss pointer and one and two should be pointers to the forth and third fix lists
			
		a->node->fix = a;
		if (!count)
		{
			// insert into section 2
			if (a->node->loss < 2) addToFixList(a, two);
			else addToFixList(a, one);
			active = a;
			count++; // Increment count
		}
		else
		{
			if (count == 1 && active->node->loss < 2) // Is in section 2, will need moving to section 1
			{
				moveSection(active, one, two);
			}
			// insert x to the right of active
			a->right = active->right;
			a->left = active;
			active->right = a;
			if (a->right) a->right->left = a;
			count++; // Increment count
		}
	}
	bool activeRootReduction() // Reduces number of active roots by 1, degree of root + at most 1
	{ 	// Returns whether it was possible or not
		if (one == NULL) return false; // No nodes to do an active root reduction to
		if (one->right == NULL) assert(false);
		if (one->right == NULL) return false; // No nodes to do an active root reduction to
		pfixlist x = one;
		pfixlist y = one->right;
		x->node->fix = y->node->fix = NULL;
		assert(x->rank == y->rank);
		one = y->right;
		if (one) one->left = NULL;
		x->rank->rootcount-=2;
		if (x->rank->rootcount) // Still active roots left with same rank as x and y
		{
			assert(one->rank == x->rank);
			one->rank->active = one;
			if (one->rank->rootcount == 1) // Move sections from 1 to 2
			{
				moveSection(one, two, one);
			}
		}
		else x->rank->active = NULL;

		if (one && one->rank->active != one)
		{
			assert(one->rank->active);
			assert(one->rank->active == one);
		}
		if (one) assert(one->rank == one->right->rank);

		if (x->node->val->val > y->node->val->val) std::swap(x, y); // Make sure x.val < y.val
		assert(x != y);
		link(x->node, y->node); // Make y a child of x
		// If the rightmost child of x is passive, make it a child of the root
		if (x->node->child != NULL)
		{
			pnode a = x->node->child->left; // Rightmost child
			if (!a->active->isactive) link(root, a);
		}

		// Increase the rank of x by one
		if (x->rank->left == NULL) // Need a new node
		{
			pranklist newrank = new RanklistNode();
			newrank->right = x->rank;
			x->rank->left = newrank;
		}
		x->rank->count--;
		x->rank = x->rank->left;
		x->node->rank = x->rank;
		x->rank->count++;
		// Reinsert x into the fix list
		insertIntoFixList(x, x->rank->active, x->rank->rootcount, one, two);
		return true;
	}
	bool rootDegreeReduction() // Reduces the degree of the root by 2, creating one new active root
	{ 	// Returns true if its possible - false otherwise
		// Find three rightmost children of the root - must all be passive-linkable
		if (!root->child) return false;
		pnode x = root->child;
		x = x->left;
		pnode y = x->left;
		pnode z = y->left;
		if (x == y || x == z || y == z) return false;
		// Check if they are all passive linkable
		if (!isPassiveLinkable(x) || !isPassiveLinkable(y) || !isPassiveLinkable(z)) return false;
		// sort x, y and z by value so that x.key < y.key < z.key
		if (x->val->val > y->val->val) swap(x, y);
		if (y->val->val > z->val->val) swap(y, z);
		if (x->val->val > y->val->val) swap(x, y);
		assert(x->val->val < y->val->val && y->val->val < z->val->val);
		// Mark x and y as active
		assert(nonlinkablechild != y && nonlinkablechild != z);
		x->active = y->active = activenode;
		// x and y have zero loss
		x->loss = y->loss = 0;
		// rank of y becomes zero
		link(y, z);
		link(x, y);
		// Make x the leftmost child of the root
		root->child = x;
		// Update nonlinkablechild if needed
		if (nonlinkablechild == NULL && !isPassiveLinkable(x)) nonlinkablechild = x;
		pranklist r = ranklist;
		if (r == NULL)
		{
			r = new RanklistNode();
			ranklist = r;
		}
		y->rank = r;
		// rank of x becomes one
		r = r->left;
		if (r == NULL)
		{
			r = new RanklistNode();
			r->right = ranklist;
			ranklist->left = r;
		}
		x->rank = r;

		// insert x into the fix list
		pfixlist a = new FixlistNode();
		a->node = x;
		a->rank = x->rank;
		insertIntoFixList(a, r->active, r->rootcount, one, two);
		return true;
	}
	void increaseLoss(pnode x) // Increases the loss of x by one
	{
		x->loss++;
		if (x->loss == 1)
		{
			// Insert into the fix list
			pfixlist a = new FixlistNode();
			a->node = x;
			a->rank = x->rank;
			assert(x->active->isactive);
			insertIntoFixList(a, x->rank->loss, x->rank->losscount, four, three);
		}
		else if (x->rank->losscount == 1 && x->loss == 2) // Needs moving
		{
			moveSection(x->fix, four, three);
		}
	}
	void removeFromFixList(pnode x, pfixlist &active, int &count, pfixlist &one, pfixlist &two) // Remove x from the fix list, moving the other items if needed
	{
		if (x->fix && x->active->isactive)
		{
			pfixlist a = x->fix;	
			assert(a);
			count--;
			// Update one and two pointers if needed
			if (a == one) one = a->right;
			if (a == two) two = a->right;
			// Remove a
			if (a->right) a->right->left = a->left;
			if (a->left) a->left->right = a->right;
			if (!count) // Was the only one
			{
				active = NULL;
			}
			else
			{
				if (a == active) // Update pointer if needed
				{
					active = a->right;
				}
				// Move sections if needed
				if (count == 1 && active->node->loss < 2)
				{
					moveSection(active, two, one);
				}
			}
			delete a;
			x->fix = NULL;
		}
	}
	
	void decreaseRank(pnode x)
	{
		// Decreases the rank of x by 1
		if (isActiveRoot(x))
		{
			// First, remove from fix list
			removeFromFixList(x, x->rank->active, x->rank->rootcount, one, two);
		}
		else // Might have loss
		{
			// If x has loss, need to remove it from the fix list
			removeFromFixList(x, x->rank->loss, x->rank->losscount, four, three);
		}
		// Decrease the rank
		x->rank = x->rank->right;
		if (isActiveRoot(x) || x->loss)
		{
			// Insert into fix list
			pfixlist a = new FixlistNode();
			a->node = x;
			a->rank = x->rank;
			if (x->loss) insertIntoFixList(a, x->rank->loss, x->rank->losscount, four, three); // Into loss fix list
			else insertIntoFixList(a, x->rank->active, x->rank->rootcount, one, two); // Into active root fix list
		}
	}
	void increaseRank(pnode x)
	{
		// Increases the rank of x by 1
		// If x has loss, need to remove it from the fix list
		removeFromFixList(x, x->rank->loss, x->rank->losscount, four, three);
		
		if (!x->rank->left) // Need new ranklist node
		{
			pranklist a = new RanklistNode();
			a->right = x->rank;
			x->rank->left = a;
		}
		// Increase the rank
		x->rank = x->rank->left;

		// Insert into fix list
		pfixlist a = new FixlistNode();
		a->node = x;
		a->rank = x->rank;
		if (x->loss) insertIntoFixList(a, x->rank->loss, x->rank->losscount, four, three);
	}
	void oneNodeLossReduction(pnode x)
	{
		// Link it to the root and make into an active root, rank of its parent y is decreased (y must be active)
		pnode y = x->parent;
		link(root, x);
		x->loss = 0;
		// x is now an active root
		// Insert into fix list
		pfixlist a = new FixlistNode();
		a->node = x;
		a->rank = x->rank;
		insertIntoFixList(a, a->rank->active, a->rank->rootcount, one, two);
		// Rank of y is decreased
		decreaseRank(y);
		// If y is not active, loss is increased
		if (!isActiveRoot(y)) increaseLoss(y);
	}
	void twoNodeLossReduction(pnode x, pnode y)
	{
		if (x->val->val > y->val->val) swap(x, y); // Make sure x.val < y.val
		pnode z = y->parent; // parent of y
		x->loss = y->loss = 0;
		link(x, y); // make y a child of x
		increaseRank(x); // Increase the rank of x
		// Decrease rank of z, and if not active root increase loss
		decreaseRank(z); 
		if (!isActiveRoot(z)) increaseLoss(z);
	}
	void lossReduction() // Does one of the two loss reductions
	{
		if (four)
		{
			if (four->node->loss > 1) // One node loss reduction
			{
				pnode x = four->node;
				removeFromFixList(x, x->rank->loss, x->rank->losscount, four, three); // Remove x from the fix list
				oneNodeLossReduction(x);
			}
			else // Two node loss reduction
			{
				pnode x = four->node;
				assert(four->rank->loss);
				removeFromFixList(x, x->rank->loss, x->rank->losscount, four, three); // Remove x from the fix list
				pnode y = x->rank->loss->node;	
				assert(x->rank == y->rank);
				removeFromFixList(y, y->rank->loss, y->rank->losscount, four, three); // Remove y from the fix list
				twoNodeLossReduction(x, y);
			}
		}
	}
	void addToBackOfQueue(pnode a, pnode &qfront)
	{
		if (!qfront) // A is the only thing in the queue
		{
			qfront = a->qleft = a->qright = a;
		}
		else // Insert to the left of qfront
		{
			a->qleft = qfront->qleft;
			a->qleft->qright = a;
			a->qright = qfront;
			qfront->qleft = a;
		}
	}
	pnode newNode(pvalue v)
	{
		pnode x = new HeapNode();
		v->inheap = x;
		x->val = v;
		if (!passive) // If passive does not exist
		{
			passive = new ActiveRecord();
		}
		x->active = passive; // Node is passive
		if (!ranklist) ranklist = new RanklistNode(); // If there is no ranklist, make one
		x->rank = ranklist;
		return x;
	}
	pnode newNode(int val) // returns a pnode with value of val and rank of 0 
	{
		pvalue v = new Value(); // Create a pointer to the value
		v->val = { val, counter++ };
		return newNode(v);
	}

	// Main operations
	void push(pnode x)
	{
		sz++;
		if (sz == 1) // only node
		{
			root = x;
			return;
		}
		if (x->val->val > root->val->val) // Just make a child of the root
		{
			link(root, x);
			// Insert into the queue
			if (!qfront)
			{
				qfront = x;
				x->qleft = x->qright = x;
			}
			else // Insert to the front
			{
				x->qleft = qfront->qleft;
				qfront->qleft = x;
				x->qright = qfront;
				x->qleft->qright = x;

				qfront = x;
			}
		}
		else
		{	
			// Link the root to x
			pnode oldroot = root;
			root = x;
			nonlinkablechild = NULL;
 			link(root, oldroot);
			// the old root onto the queue
			if (!qfront)
			{
				qfront = oldroot;
				oldroot->qleft = oldroot->qright = oldroot;
			}
			else // Insert to the front
			{
				oldroot->qleft = qfront->qleft;
				qfront->qleft = oldroot;
				oldroot->qright = qfront;
				oldroot->qleft->qright = oldroot;

				qfront = oldroot;
			}
		}
		// Active root reduction and root degree reduction
		int arrcount = 0;
		int rdrcount = 0;
		while (arrcount < 1 && rdrcount < 1)
		{
			if (arrcount < 1 && activeRootReduction()) arrcount++;
			else if (rdrcount < 1 && rootDegreeReduction()) rdrcount++;
			else break; // Not possible, break
		}
	}
	void push(int val)
	{
		push(newNode(val));
	}
	void decreasekey(pnode x, int val)
	{
		x->val->val.first = val; // Update value
		if (x->parent == NULL) return; // If x is the root
		pnode y = x->parent;
		if (x->val->val < root->val->val)
		{
			// swap values
			std::swap(x->val, root->val);
			std::swap(x->val->inheap, root->val->inheap);
		}
		if (x->active->isactive && !isActiveRoot(x))
		{
			// Decrease rank of y
			decreaseRank(y);
			// Make loss of x 0
			removeFromFixList(x, x->rank->loss, x->rank->losscount, four, three);
			x->loss = 0;
			// Make x an active root, insert into fix list
			pfixlist a = new FixlistNode();
			a->node = x;
			a->rank = x->rank;
			insertIntoFixList(a, a->rank->active, a->rank->rootcount, one, two);
		}
		link(root, x); // Make x a child of the root
		if (y->active->isactive && !isActiveRoot(y))
		{
			// Increase the loss of y
			increaseLoss(y);
		}
		lossReduction(); // A loss reduction if possible
		// DO six active root reductions and four root degree reductions, to the extent possible
		int arrcount = 0;
		int rdrcount = 0;
		while (arrcount < 6 && rdrcount < 4)
		{
			if (arrcount < 6 && activeRootReduction()) arrcount++;
			else if (rdrcount < 4 && rootDegreeReduction()) rdrcount++;
			else break; // Not possible, break
		}
	}
	void pop()
	{
		sz--;
		if (sz == 0)
		{
			root = NULL;
			qfront = NULL;
			return;
		}
		pnode x = root->child;
		// x will be the child with lowest priority
		pnode a = x;
		do
		{
			if (a->val->val < x->val->val) x = a;
			a = a->right;
		}
		while (a != root->child);
		if (x->active->isactive)
		{
			// Remove from active root fix list
			removeFromFixList(x, x->rank->active, x->rank->rootcount, one, two);
			// Make passive
			x->active = passive;
			// Make all active children of x active roots
			if (x->child)
			{
				a = x->child;
				while (a->active->isactive)
				{
					// Remove loss
					removeFromFixList(a, a->rank->loss, a->rank->losscount, four, three);
					a->loss = 0;
					// Insert into fix list
					pfixlist b = new FixlistNode();
					b->node = a;
					b->rank = a->rank;
					insertIntoFixList(b, b->rank->active, b->rank->rootcount, one, two); // Add to active root fix list
					a = a->right;
					if (a == x->child) break;
				}
			}
		}
		nonlinkablechild = NULL;
		if (x->child)
		{
			// move passive linkable children to the right
			pnode a = x->child;
			do
			{
				if (isPassiveLinkable(a) && a->left != a)
				{
					pnode next = a->left;
					// remove
					a->left->right = a->right;
					a->right->left = a->left;
					if (x->child == a) x->child = a->right;
					// reinsert at the very right
					pnode b = x->child;
					pnode c = b->left;
					c->right = a;
					a->left = c;
					b->left = a;
					a->right = b;

					a = next;
				}
				else a = a->left;
			}
			while (a != x->child);
			// Fix the nonlinkablechild pointer
			if (x->child->active->isactive) // first active-non active transition
			{
				a = x->child;
				do
				{
					if (a->active->isactive && !a->right->active->isactive)
					{
						// set as non linkable child
						nonlinkablechild = a;
						break;
					}
					a = a->right;
				}
				while (a != x->child);
				if (!nonlinkablechild) nonlinkablechild = x->child->left; // make it the rightmost child
				if (nonlinkablechild) assert(nonlinkablechild->active->isactive);
			}
			else if (!isPassiveLinkable(x->child)) // Make it the first child
			{
				nonlinkablechild = x->child;
			}
		}
		// Make all other children of the root a child of x
		delete root;
		root = x;
		x->parent = NULL;
		a = x->right;
		while (a != x)
		{
			a->parent = NULL;
			pnode next = a->right;
			link(root, a); // Make a child of the root
			a = next;
		}
		// Remove x from the queue
		if (qfront == x) qfront = x->qright;
		x->qright->qleft = x->qleft;
		x->qleft->qright = x->qright;
		// Do this twice
		for (int i = 0; i < 2; i++)
		{
			pnode y = qfront;
			// Move to back of q
			qfront = qfront->qright;
			// link two rightmost children of y to the root, if they are passive
			if (y->child && !y->child->left->active->isactive)
			{
				link(root, y->child->left);
			}
			if (y->child && !y->child->left->active->isactive)
			{
				link(root, y->child->left);
			}
		}
		// Do a loss reduction
		lossReduction();
		// Do active root reductions and root degree reductions while possible
		while (rootDegreeReduction() || activeRootReduction()) {}
	}
	void erase(pnode x)
	{
		// Decrease key to -inf
		decreasekey(x, -2e9);
		// Pop
		pop();
	}

	void merge(StrictFibonacciHeap* x) // Merge operation, seems to be behaving correctly
	{
		if (!x->sz)
		{
			// No point merging
			return;
		}
		if (x->sz > sz) // Swap the heaps
		{
			// Set our fix list to x's fix list
			one = x->one;
			two = x->two;
			three = x->three;
			four = x->four;
			ranklist = x->ranklist;
			// Swap queues & activenodes
			swap(qfront, x->qfront);
			std::swap(activenode, x->activenode);
			if (!sz) // We currently have no elements, just set to x
			{
				sz = x->sz;
				root = x->root;
				return;
			}
		}
		// Make all nodes in x passive
		x->activenode->isactive = false;
		pnode newqueue = x->qfront;
		if (root->val->val < x->root->val->val)
		{
			if (x->sz > sz) nonlinkablechild = NULL; // all children of root are now passive linkable
			// Link x->root to root;
			link(root, x->root);
			// Add x->root to the end of the queue
			addToBackOfQueue(x->root, newqueue);
		}
		else
		{
			if (x->sz <= sz) nonlinkablechild = NULL; // all children of x->root are now passive linkable
			else nonlinkablechild = x->nonlinkablechild;
			// Since all nodes in x are passive-linkable now, set non-linkable child to null
			pnode a = root;
			root = x->root;
			link(root, a);
			// Add a to the end of the queue
			addToBackOfQueue(a, newqueue);
		}
		sz += x->sz;
		// Now, add our old queue to the end of the queue
		if (qfront)
		{
			pnode last = qfront->qleft;

			newqueue->qleft->qright = qfront;
			qfront->qleft = newqueue->qleft;

			newqueue->qleft = last;
			last->qright = newqueue;
		}
		if (nonlinkablechild) assert(!isPassiveLinkable(nonlinkablechild));
		qfront = newqueue;
		// Active root reduction and root degree reduction
		int arrcount = 0;
		int rdrcount = 0;

		while (arrcount < 1 && rdrcount < 1)
		{
			if (arrcount < 1 && activeRootReduction()) arrcount++;
			else if (rdrcount < 1 && rootDegreeReduction()) rdrcount++;
			else break; // Not possible, break
		}
	}
};