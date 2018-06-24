// Dijkstra's algorithm implemented with a strict fibonacci heap: O(e + vlogv)
#include <cstdio>
#include <vector>
#include <utility>
#include <chrono>
using namespace std::chrono;
#define MAXN 1000001
typedef long long ll;
typedef struct HeapNode* pnode;
typedef struct ActiveRecord* pactivenode;
typedef struct RanklistNode* pranklist;
typedef struct FixlistNode* pfixlist;
typedef struct Value* pvalue;
struct Value
{
	std::pair<ll, int> val; // Value, index of whatever node points here
	pnode inheap; // Pointer to the location in the heap which has this value
};
struct HeapNode // Represents a node in the heap
{
	pvalue val; // Value of this node
	pnode left, right, parent, child; // Left and right siblings, parent and leftmost child
	pactivenode active; // Stores whether the node is active or passive
	pnode qleft, qright; // Left and right nodes in the queue (front of the queue is leftmost node, back is rightmost)
	pranklist rank; // If active, points to the node in the ranklist corresponding to the rank of this node.
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
};
struct FixlistNode
{
	pnode node; // Node that this corresponds to
	pfixlist left, right; // Left and right nodes in the fixlist
	pranklist rank; // Points to the node in the ranklist with this rank
};

struct StrictFibonacciHeap // The actual heap
{
	pnode root;
	int sz;
	int counter; // Used to assign indexes to nodes
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
	StrictFibonacciHeap() // Assign the active node pointer
	{
		activenode = new ActiveRecord();
		activenode->isactive = 1;
	}
	int size()
	{
		return sz;
	}
	bool empty()
	{
		return !sz;
	}
	ll top()
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
		// TODO: Special case the root (passive linkable vs passive)
		assert(x->val->val < y->val->val);
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
	void insertIntoFixList(pfixlist a, pfixlist &active, pfixlist &one, pfixlist &two)
	{
		// Active is the pointer to the node of the same rank as x which is already in the fix list.
		// One and two are pointers to the first and second fix lists
		// Note if this is a insertion due to loss, active should be the loss pointer and one and two should be pointers to the forth and third fix lists
		if (active == NULL)
		{
			// insert into section 2
			if (a->node->loss < 2) addToFixList(a, two);
			else addToFixList(a, one);
			active = a;
		}
		else if ((active->right && active->right->rank == active->rank) || (active->node->loss > 1 && active->node != a->node))
		{
			if (active->node == a->node) return; // No need to insert
			// Already in section 1 - just insert next to the current one
			a->right = active->right;
			if (a->right) a->right->left = a;
			a->left = active;
			active->right = a;
		}
		else
		{
			// Remove active from section 2
			if (active->left) active->left->right = active->right;
			if (active->right) active->right->left = active->left;
			if (two == active) two = active->right;
			// Insert both a and active into the first fix list
			addToFixList(a, one);
			if (active != a) addToFixList(active, one);
		}
	}
	bool activeRootReduction() // Reduces number of active roots by 1, degree of root + at most 1
	{ 	// Returns whether it was possible or not
		if (one == NULL) return false; // No nodes to do an active root reduction to
		if (one->right == NULL) assert(false);
		if (one->right == NULL) return false; // No nodes to do an active root reduction to
		pfixlist x = one;
		pfixlist y = one->right;
		assert(x->rank == y->rank);
		one = y->right;
		if (one) one->left = NULL;
		if (one != NULL && one->rank == x->rank) // The ranklist should now point to this node
		{
			one->rank->active = one;
			if (!one->right || one->right->rank != one->rank) // Move to the second list
			{
				pfixlist a = one;
				one = one->right;
				if (one) one->left = NULL;
				addToFixList(a, two);
			}
		}
		else
		{
			x->rank->active = NULL;
		}
		if (one && one->rank->active != one)
		{
			assert(one->rank->active);
			assert(one->rank->active == one);
		}
		if (one) assert(one->rank == one->right->rank);

		if (x->node->val->val > y->node->val->val) std::swap(x, y); // Make sure x.val < y.val

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
		// Delete fixlist node y
	//	delete y;
		// Reinsert x into the fix list
		insertIntoFixList(x, x->rank->active, one, two);
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
		if (nonlinkablechild == NULL) nonlinkablechild = x;
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
		insertIntoFixList(a, r->active, one, two);
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
			insertIntoFixList(a, x->rank->loss, four, three);
		}
		else if (x->rank->loss->node == x) // Could need moving
		{
			if (x->rank->loss->right == NULL || x->rank->loss->right->rank != x->rank) // Needs moving from section 3 into section 4
			{
				insertIntoFixList(x->rank->loss, x->rank->loss, four, three);
			}
		}
	}
	void removeFromLossFixList(pnode x)
	{
		if (x->loss)
		{
			pfixlist a = x->rank->loss;
			assert(a);
			if (a->node != x)
			{
				if (a->node->loss != 1) // leave a here in section 4
				{
					a = a->right;
				}
				else
				{
				/*	pfixlist f = a->right;
					while (f->node != x)
					{
						printf("next...\n");
						f = f->right;
						assert(f);
					}
					if (f->node == x) printf("yge\n");*/
					// Move a to the third fix list
					pfixlist b = a->right;
					if (a->left) a->left->right = a->right;
					if (a->right) a->right->left = a->left;
					if (a == four) four = a->right;
					addToFixList(a, three);
					a = b;
				}
			}
			// Remove a from the fourth fix list
			if (a->left) a->left->right = a->right;
			if (a->right) a->right->left = a->left;
			if (a == three) three = a->right;
			if (a == four) four = a->right;
			if (x->rank->loss == a) 
			{
				x->rank->loss = NULL;
				if (four && four->rank == x->rank)
				{
					x->rank->loss = four;
					if (four->node->loss < 2)
					{
						// MUST Move to third fix list
						pfixlist newfour = four->right;
						if (newfour) newfour->left = NULL;
						addToFixList(four, three);
						four = newfour;
					}
				}
			}
		//	delete a;
		}
	}
	void removeFromActiveRootFixList(pnode x)
	{
		if (isActiveRoot(x))
		{
			pfixlist a = x->rank->active;
			assert(a);
			if (a->node != x)
			{
				assert(a->right->node == x);
				// Move a to the second fix list
				pfixlist b = a->right;
				if (a->left) a->left->right = a->right;
				if (a->right) a->right->left = a->left;
				if (a == one) one = a->right;
				addToFixList(a, two);
				a = b;
			}
			assert(a->node == x); // Needs to be the only one, otherwise would've been fixed
			// Remove a from the first fix list
			if (a->left) a->left->right = a->right;
			if (a->right) a->right->left = a->left;
			if (a == one) one = a->right;
			if (a == two) two = a->right;
			if (x->rank->active == a) x->rank->active = NULL;
			if (one && one->rank == x->rank)
			{
				// MUST MOVE TO TWO
				pfixlist newone = one->right;
				if (newone) newone->left = NULL;
				addToFixList(one, two);
				x->rank->active = one;
				one = newone;
				if (two->right && two->right->rank == two->rank) assert(false);
			}
		}
	}
	void decreaseRank(pnode x)
	{
		// Decreases the rank of x by 1
		if (isActiveRoot(x))
		{
			// First, remove from fix list
			removeFromActiveRootFixList(x);
		}
		else // Might have loss
		{
			// If x has loss, need to remove it from the fix list
			removeFromLossFixList(x);
		}
		// Decrease the rank
		x->rank = x->rank->right;
		if (isActiveRoot(x) || x->loss)
		{
			// Insert into fix list
			pfixlist a = new FixlistNode();
			a->node = x;
			a->rank = x->rank;
			if (x->loss) insertIntoFixList(a, x->rank->loss, four, three); // Into loss fix list
			else insertIntoFixList(a, x->rank->active, one, two); // Into active root fix list
		}
	}
	void increaseRank(pnode x)
	{
		// Increases the rank of x by 1
		// If x has loss, need to remove it from the fix list
		removeFromLossFixList(x);
		
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
		if (x->loss) insertIntoFixList(a, x->rank->loss, four, three);
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
		insertIntoFixList(a, a->rank->active, one, two);
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
		if (four) // Can do a loss reduction
		{
			if (four->node->loss >= 2) // one node loss reduction
			{
				pnode x = four->node;
				four = four->right;
				if (four)
				{
			//		delete(four->left); // Deallocate memory
					four->left = NULL;
				}
				// Update rank pointer
				if (four && four->rank == x->rank) x->rank->loss = four;
				else x->rank->loss = NULL;
				oneNodeLossReduction(x);
				lossReduction();
			}
			else // Two node loss reduction
			{
				assert(four->rank == four->right->rank);
				pnode x = four->node;
				four = four->right;
				pnode y = four->node;
				four = four->right;
				if (four)
				{
					four->left = NULL; // Remove left pointer
				}
				// Update rank pointer
				if (four && four->rank == x->rank) x->rank->loss = four;
				else x->rank->loss = NULL;
				twoNodeLossReduction(x, y);
				lossReduction();
			}
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
	pnode newNode(ll val) // returns a pnode with value of val and rank of 0 
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
		// active root reduction and root degree reduction
		while (rootDegreeReduction() || activeRootReduction()) {}
	}
	void push(ll val)
	{
		push(newNode(val));
	}
	void decreasekey(pnode x, ll val)
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
			removeFromLossFixList(x);
			x->loss = 0;
			// Make x an active root, insert into fix list
			pfixlist a = new FixlistNode();
			a->node = x;
			a->rank = x->rank;
			insertIntoFixList(a, a->rank->active, one, two);
		}
		link(root, x); // Make x a child of the root
		if (y->active->isactive && !isActiveRoot(y))
		{
			// Increase the loss of y
			increaseLoss(y);
		}
		lossReduction(); // A loss reduction if possible
		// DO six active root reductions and four root degree reductions, to the extent possible
		while (rootDegreeReduction() || activeRootReduction()) {}
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
			removeFromActiveRootFixList(x);
			// Make passive
			x->active = passive;
			// Make all active children of x active roots
			if (x->child)
			{
				a = x->child;
				while (a->active->isactive)
				{
					// Remove loss
					removeFromLossFixList(a);
					a->loss = 0;
					// Insert into fix list
					pfixlist b = new FixlistNode();
					b->node = a;
					b->rank = a->rank;
					insertIntoFixList(b, b->rank->active, one, two); // Add to active root fix list
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
			// Do a loss reduction
			lossReduction();
		}
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
};
int v, e;
std::vector<std::pair<int, ll> > adj[MAXN];
StrictFibonacciHeap pq;
pvalue nodes[MAXN];
int main()
{
	// Scan in the input
	scanf("%d%d", &v, &e);
	for (int i = 0; i < e; i++)
	{
		int a, b;
		ll c;
		scanf("%d%d%lld", &a, &b, &c);
		adj[a].emplace_back(b, c);
		adj[b].emplace_back(a, c);
	}
	// Start the timer
	milliseconds start = duration_cast<milliseconds>(system_clock::now().time_since_epoch());

	// Initialise the distance to each node
	for (int i = 0; i < v; i++) nodes[i] = new Value(); // Allocate memory
	nodes[0]->val = { 0, 0};
	pq.push(pq.newNode(nodes[0]));
	for (int i = 1; i < v; i++) // Push into pq
	{
		nodes[i]->val = { 1e18, i };
		pq.push(pq.newNode(nodes[i]));
	}

	// Run dijkstra
	while (!pq.empty())
	{
		int a = pq.root->val->val.second;
		ll d = pq.top();
		pq.pop();
		for (auto b : adj[a])
		{
			if (d + b.second < nodes[b.first]->val.first)
			{
				pq.decreasekey(nodes[b.first]->inheap, d + b.second);
			}
		}
	}
	// Print distance to node n-1;
	printf("%lld\n", nodes[v-1]->val.first);

	// End the timer, print the time
	milliseconds end = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
	ll totaltime = end.count() - start.count();
	printf("Time % 6lldms\n", totaltime);
}