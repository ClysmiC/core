#include "util/misc.h"

// HMM - The TreeBuilder basically has two different sets of assumptions (and sometimes implementations) depending
//  on if you are "rebuilding" on top of an existing tree or not. This makes it more complicated than it should be.
//  It was done to serve an immediate mode UI library, which usually rebuilds more-or-less the same tree
//  every frame, but still needs to track the controls from last frame that got killed off this frame,
//  so we wanted to keep things around in the tree that we might not have added this frame. Hence, the
//  tree "rebuilding" functionality. I think it might be better to offload this complexity to the Ui
//  library, since most tree building use-cases won't need it. We could probably just make each Ui_Control
//  maintain two copies of all the tree links, and alternate which ones it passes to the tree builder each
//  frame, and the tree builder always builds the tree from scratch.

// TODO - Add unit test? Especially for the "rebuilding" scenario?

// --- Helper to intrusively build trees implemented using embedded singly linked lists. Supports singly and double embedded lists.
//     Requires 0 allocation (since the links are required to be embedded)
//     To build a tree from scratch, make sure that the nodes' link members are set to nullptr
//     To re-build over an existing tree (only permitted if mPrevSibling.isValid):
//      - Passing in a node that is the same as the cursor's next sibling (or if the cursor is null, the parent's first child) will not change any links and advance the cursor.
//      - Otherwise a node with null siblings is treated as an insertion at the cursor.
//      - Otherwise, nodes with non-null siblings will unlink from their siblings and be reinserted at the cursor.

template <typename T>
struct TreeBuilder
{
    T* root;
    T* current_parent;
    T* insert_child_after; // aka "cursor". if null inserts at current_parent.first_child, else inserts after the cursor

    MemberRef<T, T*> mFirstChild;  // Required
    MemberRef<T, T*> mNextSibling; // Required
    
    // HMM - If you just want to be able to walk the built tree, this isn't really required.
    //  But we require it because we use it in the building process. (see EndChild(..))
    MemberRef<T, T*> mParent;      // Required
    
    MemberRef<T, T*> mPrevSibling; // Optional (if you maintain siblings as a doubly-linked list)
    MemberRef<T, T*> mLastChild;   // Optional (if parent stores pointer to last child)

#if DEBUG_BUILD
    bool is_finished;
#endif
    
    // ---

    // HMM - The parameter order here is not obvious
    
    TreeBuilder() = default;
    TreeBuilder(
        MemberRef<T, T*> mFirstChild,
        MemberRef<T, T*> mNextSibling,
        MemberRef<T, T*> mParent,
        MemberRef<T, T*> mPrevSibling = MemberRef_Nil<T, T*>(),
        MemberRef<T, T*> mLastChild = MemberRef_Nil<T, T*>())
    {
        Assert(mFirstChild.isValid);
        Assert(mNextSibling.isValid);
        Assert(mParent.isValid);
        
        *this = {};
        this->mFirstChild = mFirstChild;
        this->mNextSibling = mNextSibling;
        this->mParent = mParent;
        this->mPrevSibling = mPrevSibling;
        this->mLastChild = mLastChild;
    }
};

template <typename T>
function void
BeginRoot(TreeBuilder<T> * builder, T* root)
{
    // HMM - Should we just make BeginRoot/EndRoot a branch in BeginChild/EndChild to make the interface smaller?
    
    Assert(!builder->is_finished);
    Assert(builder->root == nullptr);
    Assert(builder->current_parent == nullptr);
    Assert(builder->insert_child_after == nullptr);
    
    builder->root = root;
    builder->current_parent = root;
    builder->insert_child_after = nullptr;
}

template <typename T>
function void
EndRoot(TreeBuilder<T> * builder, bool shouldResetBuilder=false)
{
    Assert(!builder->is_finished);
    Assert(builder->current_parent == builder->root);
    
#if DEBUG_BUILD
    builder->is_finished = true;
#endif


    if (shouldResetBuilder)
    {
        // NOTE - Only needed if you want to re-use the same TreeBuilder for a new tree
        
        new (builder)TreeBuilder<T>(builder->mFirstChild,
                                    builder->mNextSibling,
                                    builder->mParent,
                                    builder->mPrevSibling);
    }
}

template <typename T>
function void
BeginChild(TreeBuilder<T> * builder, T* child)
{
    T* prevBeforeInsert = builder->mPrevSibling.isValid ? *PMember(child, builder->mPrevSibling) : nullptr;
    T* nextBeforeInsert = *PMember(child, builder->mNextSibling);
    
    // ---Verify inputs
    
    bool isRebuilding = *PMember(child, builder->mParent);
    if (isRebuilding)
    {
        Assert(*PMember(child, builder->mParent) == builder->current_parent);
        Assert(builder->mPrevSibling.isValid); // Rebuilding over an existing tree requires prev pointers to fix up links as needed
    }
    else
    {
        Assert(*PMember(child, builder->mFirstChild) == nullptr);
        Assert(nextBeforeInsert == nullptr);
        Assert(*PMember(child, builder->mParent) == nullptr);
        Assert(prevBeforeInsert == nullptr); // NOTE - Skips isValid because we already set this var to null if the member is invalid
        Assert(Implies(builder->mLastChild.isValid, *PMember(child, builder->mLastChild) == nullptr));
    }

    
    // --- Link to our parent

    T* parent = builder->current_parent;
    *PMember(child, builder->mParent) = parent;

    // --- Compute who our siblings should be
    
    T* nextAfterInsert = nullptr;
    T* prevAfterInsert = builder->insert_child_after;

    if (isRebuilding)
    {
        if (prevAfterInsert == prevBeforeInsert)
        {
            // We are staying in the same place
            
            nextAfterInsert = *PMember(child, builder->mNextSibling);
        }
        else if (!prevAfterInsert)
        {
            // We are moving to the head
            
            nextAfterInsert = *PMember(parent, builder->mFirstChild);
        }
        else
        {
            // We are moving somewhere else
            
            nextAfterInsert = *PMember(prevAfterInsert, builder->mNextSibling);
        }
    }
    else
    {
        if (!prevAfterInsert)
        {
            // We are being inserted into the head
            
            nextAfterInsert = *PMember(parent, builder->mFirstChild);
        }
        else
        {
            // We are being inserted somewhere else
            
            nextAfterInsert = *PMember(prevAfterInsert, builder->mNextSibling);
        }
    }

    // --- Unlink from the old siblings

    if (isRebuilding)
    {
        Assert(Iff(prevBeforeInsert == prevAfterInsert,
                   nextBeforeInsert == nextAfterInsert));
        
        if (prevBeforeInsert != prevAfterInsert)
        {
            // We moved somewhere
            
            if (prevBeforeInsert)
            {
                *PMember(prevBeforeInsert, builder->mNextSibling) = nextBeforeInsert;
            }
            else
            {
                // We moved to be at the head. (Only unlinking for now.)
                
                Assert(*PMember(parent, builder->mFirstChild) == child);
                *PMember(parent, builder->mFirstChild) = nextBeforeInsert;
            }
            
            if (nextBeforeInsert)
            {
                *PMember(nextBeforeInsert, builder->mPrevSibling) = prevBeforeInsert;
            }
            else if (builder->mLastChild.isValid)
            {
                // We moved to be at the tail. (Only unlinking for now.)
                
                Assert(*PMember(parent, builder->mLastChild) == child);
                *PMember(parent, builder->mLastChild) = prevBeforeInsert;
            }
        }
    }


    // --- Relink to the new siblings
    
    *PMember(child, builder->mNextSibling) = nextAfterInsert;
    if (prevAfterInsert)
    {
        *PMember(prevAfterInsert, builder->mNextSibling) = child;
    }
    
    if (builder->mPrevSibling.isValid)
    {
        *PMember(child, builder->mPrevSibling) = prevAfterInsert;
        if (nextAfterInsert)
        {
            *PMember(nextAfterInsert, builder->mPrevSibling) = child;
        }
    }

    // --- Link parent to us
    
    if (prevAfterInsert == nullptr)
    {
        *PMember(parent, builder->mFirstChild) = child;
    }

    if (builder->mLastChild.isValid &&
        nextAfterInsert == nullptr)
    {
        *PMember(parent, builder->mLastChild) = child;
    }
    
    // --- Update builder
    
    builder->insert_child_after = nullptr;
    builder->current_parent = child;
}

template <typename T>
function void
EndChild(TreeBuilder<T> * builder)
{
    Assert(builder->current_parent != builder->root); // Should you be calling EndRoot instead?

    builder->insert_child_after = builder->current_parent;
    builder->current_parent = *PMember(builder->current_parent, builder->mParent);
}

template <typename T>
inline void
LeafChild(TreeBuilder<T> * builder, T* child)
{
    BeginChild(builder, child);
    EndChild(builder);
}

template <typename T>
inline void
SetInsertionCursorAfter(TreeBuilder<T> * builder, T* insert_child_after)
{
    // NOTE - passing nullptr for insert_child_after makes the next inserted
    //  item become the head of the list
    
    builder->insert_child_after = insert_child_after;
}
